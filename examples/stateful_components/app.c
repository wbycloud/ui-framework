/* Complete framework-owned application: reuse the integration scene and its
 * worker/GL lifecycle; only storage and its semantic commands are added here. */
#define UI_FIXTURE_EMBEDDED 1
#include "../../tests/api7_fixture.c"
#define STATE_LIMIT 70000u
#define STATE_HEADER 32u
typedef struct state_app {
    fixture_t *scene;
    HANDLE profile_lock;
    wchar_t path[4096];
    unsigned char profile;
} state_app_t;
static uint32_t read32(const unsigned char *p)
{return p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void write32(unsigned char *p,uint32_t v)
{for(unsigned i=0;i<4;++i)p[i]=(unsigned char)(v>>(i*8));}
static uint32_t checksum(const unsigned char *p,size_t bytes)
{uint32_t v=2166136261u;for(size_t i=0;i<bytes;++i)if(i<20||i>=24){v^=p[i];v*=16777619u;}return v;}
static ui_status_t snapshot(state_app_t *s,unsigned char **out,size_t *bytes)
{
    size_t columns=0,layout_bytes=0;ui_status_t status;ui_shell_t *shell=ui_host_get_shell(s->scene->context.host);unsigned char *p;
    *out=NULL;status=ui_component_save_columns(s->scene->table,NULL,0,&columns);if(status!=UI_STATUS_OK)return status;
    status=ui_shell_save_layout(shell,NULL,0,&layout_bytes);if(status!=UI_STATUS_OK)return status;
    if(columns>2640||layout_bytes>65536||STATE_HEADER+columns+layout_bytes>STATE_LIMIT)return UI_STATUS_LIMIT_EXCEEDED;
    *bytes=STATE_HEADER+columns+layout_bytes;p=calloc(1,*bytes);if(!p)return UI_STATUS_OUT_OF_MEMORY;
    memcpy(p,"UST1",4);write32(p+4,1);write32(p+8,(uint32_t)columns);write32(p+12,(uint32_t)layout_bytes);p[16]=s->profile;
    status=ui_component_save_columns(s->scene->table,p+STATE_HEADER,columns,&columns);
    if(status==UI_STATUS_OK)status=ui_shell_save_layout(shell,p+STATE_HEADER+columns,layout_bytes,&layout_bytes);
    if(status!=UI_STATUS_OK){free(p);return status;}write32(p+20,checksum(p,*bytes));*out=p;return UI_STATUS_OK;
}
static ui_status_t apply_snapshot(state_app_t *s,const unsigned char *p,size_t bytes)
{
    unsigned char *prior=NULL;size_t prior_bytes=0;uint32_t columns,layout_bytes;ui_status_t status,rollback;
    ui_shell_t *shell=ui_host_get_shell(s->scene->context.host);
    if(bytes<STATE_HEADER||bytes>STATE_LIMIT||memcmp(p,"UST1",4))return UI_STATUS_INVALID_ARGUMENT;
    if(read32(p+4)!=1)return UI_STATUS_UNSUPPORTED;
    columns=read32(p+8);layout_bytes=read32(p+12);
    if(columns<80||columns>2640||layout_bytes<80||layout_bytes>65536||bytes!=STATE_HEADER+(size_t)columns+layout_bytes||p[16]!=s->profile||p[17]||p[18]||p[19]||read32(p+24)||read32(p+28)||checksum(p,bytes)!=read32(p+20))return UI_STATUS_INVALID_ARGUMENT;
    status=snapshot(s,&prior,&prior_bytes);if(status!=UI_STATUS_OK)return status;
    /* UI thread, no message pump between apply and rollback. Each framework
     * restore validates its whole payload. Keep both final states atomic. */
    status=ui_component_restore_columns(s->scene->table,p+STATE_HEADER,columns);
    if(status==UI_STATUS_OK)status=ui_shell_restore_layout(shell,p+STATE_HEADER+columns,layout_bytes);
    if(status!=UI_STATUS_OK){rollback=ui_component_restore_columns(s->scene->table,prior+STATE_HEADER,read32(prior+8));
        if(rollback==UI_STATUS_OK)rollback=ui_shell_restore_layout(shell,prior+STATE_HEADER+read32(prior+8),read32(prior+12));
        if(rollback!=UI_STATUS_OK)status=rollback;}
    free(prior);return status;
}
static ui_status_t load_state(state_app_t *s)
{
    HANDLE file=CreateFileW(s->path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    LARGE_INTEGER length;unsigned char *p;DWORD bytes;ui_status_t status;
    if(file==INVALID_HANDLE_VALUE)return GetLastError()==ERROR_FILE_NOT_FOUND?UI_STATUS_NOT_FOUND:UI_STATUS_PLATFORM_ERROR;
    if(!GetFileSizeEx(file,&length)||length.QuadPart<STATE_HEADER||length.QuadPart>STATE_LIMIT){CloseHandle(file);return UI_STATUS_INVALID_ARGUMENT;}
    p=malloc((size_t)length.QuadPart);if(!p){CloseHandle(file);return UI_STATUS_OUT_OF_MEMORY;}
    status=ReadFile(file,p,(DWORD)length.QuadPart,&bytes,NULL)&&bytes==(DWORD)length.QuadPart?apply_snapshot(s,p,bytes):UI_STATUS_PLATFORM_ERROR;
    free(p);CloseHandle(file);return status;
}
static ui_status_t save_state(state_app_t *s)
{
    unsigned char *p=NULL;size_t bytes=0;DWORD written;HANDLE file;wchar_t temporary[4096];ui_status_t status=snapshot(s,&p,&bytes);
    if(status!=UI_STATUS_OK)return status;
    if(_snwprintf_s(temporary,4096,_TRUNCATE,L"%s.tmp",s->path)<0){free(p);return UI_STATUS_INVALID_ARGUMENT;}
    file=CreateFileW(temporary,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file==INVALID_HANDLE_VALUE){free(p);return UI_STATUS_PLATFORM_ERROR;}
    status=WriteFile(file,p,(DWORD)bytes,&written,NULL)&&written==bytes&&FlushFileBuffers(file)?UI_STATUS_OK:UI_STATUS_PLATFORM_ERROR;
    free(p);CloseHandle(file);if(status==UI_STATUS_OK&&!MoveFileExW(temporary,s->path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))status=UI_STATUS_PLATFORM_ERROR;return status;
}
static void report(state_app_t *s,const char *operation,ui_status_t status)
{char text[160];if(status==UI_STATUS_OK)snprintf(text,sizeof(text),"State %s: profile %c",operation,s->profile);
 else if(status==UI_STATUS_NOT_FOUND)snprintf(text,sizeof(text),"No user state: profile %c; current state retained",s->profile);
 else snprintf(text,sizeof(text),"State rejected: profile %c, status %d",s->profile,status);
 (void)ui_component_set_text(s->scene->status,text);}
static void state_command(ui_host_t *host,uint64_t request,const char *id,const char *params,const char *origin,void *data)
{
    state_app_t *s=data;ui_status_t status=UI_STATUS_OK;(void)params;(void)origin;
    if(!strcmp(id,"state.save"))status=save_state(s);
    else if(!strcmp(id,"state.load"))status=load_state(s);
    else if(!strcmp(id,"state.reset-column"))status=ui_component_reset_columns(s->scene->table,"name");
    else{status=ui_component_reset_columns(s->scene->table,NULL);if(status==UI_STATUS_OK)status=ui_shell_reset_layout(ui_host_get_shell(host));}
    report(s,!strcmp(id,"state.save")?"saved":!strcmp(id,"state.load")?"loaded":"reset",status);
    (void)ui_host_reply(host,request,status==UI_STATUS_OK,"{}");
}
static ui_status_t acquire_profile(state_app_t *s)
{
    wchar_t root[4000],lock_path[4096],explicit_profile[8];DWORD n=GetEnvironmentVariableW(L"UI_STATE_EXAMPLE_HOME",root,4000),selected=GetEnvironmentVariableW(L"UI_STATE_EXAMPLE_PROFILE",explicit_profile,8);
    if(n>=4000||selected>=8||(selected&&(selected!=1||(explicit_profile[0]!=L'A'&&explicit_profile[0]!=L'B'))))return UI_STATUS_INVALID_ARGUMENT;
    if(!n){n=GetEnvironmentVariableW(L"LOCALAPPDATA",root,4000);if(!n||n>=3900)return UI_STATUS_INVALID_ARGUMENT;wcscat_s(root,4000,L"\\UiFrameworkStateExample");}
    if(!CreateDirectoryW(root,NULL)&&GetLastError()!=ERROR_ALREADY_EXISTS)return UI_STATUS_PLATFORM_ERROR;
    for(unsigned char profile='A';profile<='B';++profile){if(selected&&explicit_profile[0]!=profile)continue;
        if(_snwprintf_s(lock_path,4096,_TRUNCATE,L"%s\\profile-%c.lock",root,profile)<0)return UI_STATUS_INVALID_ARGUMENT;
        s->profile_lock=CreateFileW(lock_path,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
        if(s->profile_lock!=INVALID_HANDLE_VALUE){s->profile=profile;return _snwprintf_s(s->path,4096,_TRUNCATE,L"%s\\profile-%c.ust",root,profile)<0?UI_STATUS_INVALID_ARGUMENT:UI_STATUS_OK;}
        if(GetLastError()!=ERROR_SHARING_VIOLATION)return UI_STATUS_PLATFORM_ERROR;
    }return UI_STATUS_ALREADY_EXISTS;
}
static ui_status_t schema_variant(fixture_t *scene)
{
    char variant[32];ui_column_desc_t columns[64]={0};ui_component_desc_t d={0};ui_status_t status;DWORD length=GetEnvironmentVariableA("UI_STATE_EXAMPLE_SCHEMA",variant,sizeof(variant));
    if(!length)return UI_STATUS_OK;if(length>=sizeof(variant))return UI_STATUS_INVALID_ARGUMENT;
    if(strcmp(variant,"reorder")&&strcmp(variant,"evolve"))return UI_STATUS_INVALID_ARGUMENT;
    columns[0]=(ui_column_desc_t){sizeof(columns[0]),"name","Name",UI_VALUE_TEXT,200};columns[1]=(ui_column_desc_t){sizeof(columns[1]),"value","Value",UI_VALUE_NUMBER,120};
    if(!strcmp(variant,"evolve"))strcpy_s(scene->column_ids[63],12,"added");
    for(size_t i=2;i<64;++i){columns[i].size=sizeof(columns[i]);columns[i].id=columns[i].title=scene->column_ids[i];columns[i].kind=UI_VALUE_TEXT;columns[i].width=180;}
    if(!strcmp(variant,"reorder")){ui_column_desc_t swap=columns[0];columns[0]=columns[1];columns[1]=swap;}
    status=ui_component_unregister(scene->table);scene->table=NULL;if(status!=UI_STATUS_OK)return status;
    d.size=sizeof(d);d.web_backend=scene->backend;d.id="table";d.title="Complete 100000 row source";d.kind=UI_COMPONENT_TABLE;d.columns=columns;d.column_count=64;d.source=source;d.user_data=scene;d.commands.select="test.select";d.commands.edit="test.edit";d.sort_command="test.sort";d.selection_flags=UI_SELECTION_MULTIPLE;
    return ui_component_register(scene->context.host,&d,&scene->table);
}
static ui_status_t UI_APP_CALL state_create(const ui_app_context_t *context,void **out,ui_assistant_config_t *assistant)
{
    state_app_t *s=calloc(1,sizeof(*s));ui_status_t status;void *scene=NULL;const char *ids[]={"state.save","state.load","state.reset-column","state.reset"},*titles[]={"Save interface state","Reload interface state","Reset Name width","Reset widths and layout"};
    if(!s)return UI_STATUS_OUT_OF_MEMORY;*out=s;status=acquire_profile(s);if(status!=UI_STATUS_OK)return status;
    status=create(context,&scene,assistant);s->scene=scene;if(status!=UI_STATUS_OK)return status;status=schema_variant(s->scene);if(status!=UI_STATUS_OK)return status;
    for(size_t i=0;i<4;++i){ui_command_desc_t cmd={0};ui_menu_item_desc_t item={0};cmd.size=sizeof(cmd);cmd.id=ids[i];cmd.title=titles[i];cmd.handler=state_command;cmd.user_data=s;
        status=ui_host_register_command(context->host,&cmd);if(status!=UI_STATUS_OK)return status;
        item.size=sizeof(item);item.id=ids[i];item.menu_path="State";item.title=titles[i];item.command_id=ids[i];item.order=(int)i;status=ui_host_register_menu_item(context->host,&item);if(status!=UI_STATUS_OK)return status;
    }return UI_STATUS_OK;
}
static ui_status_t UI_APP_CALL state_mount(void *data,const ui_app_context_t *context)
{state_app_t *s=data;ui_status_t status=mount(s->scene,context);if(status!=UI_STATUS_OK)return status;status=load_state(s);report(s,"loaded",status);return UI_STATUS_OK;}
static void UI_APP_CALL state_active(void *data,int enabled){state_app_t *s=data;active(s->scene,enabled);}
static ui_app_close_decision_t UI_APP_CALL state_close(void *data,ui_app_close_reason_t reason){state_app_t *s=data;return close_app(s->scene,reason);}
static void UI_APP_CALL state_unmount(void *data){state_app_t *s=data;unmount(s->scene);}
static void UI_APP_CALL state_destroy(void *data)
{state_app_t *s=data;if(s->scene)destroy(s->scene);if(s->profile_lock&&s->profile_lock!=INVALID_HANDLE_VALUE)CloseHandle(s->profile_lock);free(s);}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t d={sizeof(d),1,UI_FRAMEWORK_API_VERSION,state_create,state_mount,state_active,state_close,state_unmount,state_destroy,shutdown_module};return &d;}
