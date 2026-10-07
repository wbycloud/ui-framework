#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/components.h"
#include "ui_framework/menus.h"
#ifdef UI_EXPERIENCE_WEBVIEW2
#include "ui_framework/webview2.h"
#include <objbase.h>
#endif
static int failures,runtime,queries,last_column,sorts,delay;
static ui_component_query_t late;
static char column_ids[64][16];
#define CHECK(x) do{if(!(x)){fprintf(stderr,"widths line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(DWORD ms)
{ULONGLONG end=GetTickCount64()+ms;do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}Sleep(1);}while(GetTickCount64()<end);}
static ui_status_t present(ui_component_t *c,const char *id,ui_element_presentation_t *p)
{ULONGLONG end=GetTickCount64()+10000;ui_status_t s;do{s=ui_component_get_presentation(c,id,p);if(s!=UI_STATUS_PENDING)return s;pump(1);}while(GetTickCount64()<end);return s;}
static void input(ui_component_t *c,ui_input_event_t *e)
{CHECK(ui_component_dispatch_input(c,e)==UI_STATUS_OK);if(runtime)pump(40);}
static int click(ui_component_t *c,const char *id)
{ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);if(present(c,id,&p)!=UI_STATUS_OK||!p.visible)return 0;e.size=sizeof(e);e.pointer_button=1;e.x=p.clip.x+p.clip.width/2;e.y=p.clip.y+p.clip.height/2;e.kind=UI_INPUT_POINTER_DOWN;input(c,&e);e.kind=UI_INPUT_POINTER_UP;input(c,&e);return 1;}
static void source(ui_component_t *c,const ui_component_query_t *q,void *user)
{ui_component_batch_t b={0};size_t n=q->count;(void)user;if(q->kind!=UI_QUERY_ROWS)return;++queries;late=*q;if(delay)return;
 if(n>128)n=128;if(q->first+n>100000)n=(size_t)(100000-q->first);
 ui_row_t *rows=calloc(n?n:1,sizeof(*rows));ui_cell_t *cells=calloc((n?n:1)*64,sizeof(*cells));CHECK(rows&&cells);if(!rows||!cells){free(rows);free(cells);return;}
 for(size_t i=0;i<n;++i){rows[i].size=sizeof(*rows);rows[i].id=9007199254741000ULL+q->first+i;rows[i].title="稳定行及完整中文内容";rows[i].content_version=1;rows[i].cells=cells+i*64;rows[i].cell_count=q->column_count;
 for(size_t j=0;j<q->column_count;++j){size_t k=q->first_column+j;ui_cell_t *v=&cells[i*64+j];v->size=sizeof(*v);v->column_id=column_ids[k];v->kind=k==1?UI_VALUE_BOOLEAN:UI_VALUE_TEXT;v->text=k==1?"true":"这是一个足够长的中文单元格内容，用于有界适配和完整查看";if(k==63)last_column=1;}}
 b.size=sizeof(b);b.component_generation=q->component_generation;b.request_id=q->request_id;b.first=q->first;b.total_count=100000;b.rows=rows;b.row_count=n;CHECK(ui_component_submit(c,&b)==UI_STATUS_OK);free(cells);free(rows);
}
static void command(ui_host_t *h,uint64_t request,const char *id,const char *params,const char *origin,void *user)
{(void)id;(void)params;(void)origin;(void)user;++sorts;ui_host_reply(h,request,1,"{}");}
static void drag(ui_component_t *c,const char *id,int delta,int cancel)
{ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);CHECK(present(c,id,&p)==UI_STATUS_OK&&p.visible);if(!p.visible)return;
 e.size=sizeof(e);e.pointer_button=1;e.x=p.clip.x+p.clip.width/2;e.y=p.clip.y+p.clip.height/2;e.kind=UI_INPUT_POINTER_DOWN;input(c,&e);e.kind=UI_INPUT_POINTER_MOVE;e.x+=delta;input(c,&e);
 if(cancel==3)return;if(cancel==1){e.kind=UI_INPUT_KEY_DOWN;e.key_code=VK_ESCAPE;input(c,&e);}else if(cancel==2){e.kind=UI_INPUT_CANCEL;input(c,&e);}else{e.kind=UI_INPUT_POINTER_UP;input(c,&e);}}
int main(int argc,char **argv)
{ui_host_config_t hc={0};ui_host_t *h;ui_component_t *table,*other;ui_column_desc_t cols[64]={0};ui_component_desc_t d={0};ui_command_desc_t cmd={0};ui_native_shell_t *shell=NULL;ui_web_backend_t *backend=NULL;HWND root=NULL;int width;size_t bytes;void *saved;ui_component_state_t state={0};ui_element_presentation_t p={0},cell={0};runtime=argc==2&&!strcmp(argv[1],"--webview2");
#ifdef UI_EXPERIENCE_WEBVIEW2
 if(runtime){ui_webview2_backend_config_t config={0};ui_status_t status;CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));config.size=sizeof(config);config.framework_components=1;backend=ui_webview2_backend_create(&config,&status);CHECK(backend&&status==UI_STATUS_OK);if(!backend)return 1;root=CreateWindowW(L"STATIC",L"Current column experience",WS_OVERLAPPEDWINDOW,60,60,1000,700,NULL,NULL,NULL,NULL);ShowWindow(root,SW_SHOW);hc.native_parent=root;}
#else
 if(runtime)return 2;
#endif
 hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;h=ui_host_create(&hc);CHECK(h);if(!h)return 1;if(runtime){ui_native_shell_config_t config={0};CHECK(ui_host_resize(h,980,660)==UI_STATUS_OK);config.size=sizeof(config);config.host=h;shell=ui_native_shell_create_web(&config);CHECK(shell);}
 cmd.size=sizeof(cmd);cmd.id="sort";cmd.title="Sort";cmd.handler=command;CHECK(ui_host_register_command(h,&cmd)==UI_STATUS_OK);
 for(size_t i=0;i<64;++i){snprintf(column_ids[i],16,"col%zu",i);cols[i].size=sizeof(cols[i]);cols[i].id=column_ids[i];cols[i].title="长中文表头";cols[i].kind=i==1?UI_VALUE_BOOLEAN:UI_VALUE_TEXT;cols[i].width=i==1?32:100;}
 d.size=sizeof(d);d.id="stable-table";d.kind=UI_COMPONENT_TABLE;d.columns=cols;d.column_count=64;d.source=source;d.web_backend=backend;d.sort_command="sort";d.selection_flags=UI_SELECTION_MULTIPLE;CHECK(ui_component_register(h,&d,&table)==UI_STATUS_OK);
 d.id="second-table";CHECK(ui_component_register(h,&d,&other)==UI_STATUS_OK);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==100);
 CHECK((runtime?ui_component_mount(table,ui_shell_get_content_slot(ui_host_get_shell(h),NULL)):ui_component_mount_offscreen(table,640,480,96))==UI_STATUS_OK);
 CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==100);CHECK(click(table,"sort-col0")&&sorts==1);drag(table,"resize-col0",60,0);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==160&&sorts==1);
 p.size=cell.size=sizeof(p);CHECK(present(table,"sort-col0",&p)==UI_STATUS_OK);CHECK(present(table,"cell-9007199254741000-col0",&cell)==UI_STATUS_OK&&p.rect.x==cell.rect.x&&cell.rect.width==160);
 CHECK(present(table,"check-9007199254741000-col1",&p)==UI_STATUS_OK&&p.visible&&p.clip.width>=24);
 {ui_input_event_t e={0};CHECK(click(table,"cell-9007199254741000-col0"));e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code=VK_F1;input(table,&e);CHECK(present(table,"text-detail-value",&p)==UI_STATUS_OK&&p.visible&&p.clip.height>0&&strstr(p.text_utf8,"完整查看"));e.key_code=VK_ESCAPE;input(table,&e);CHECK(present(table,"cell-9007199254741000-col0",&p)==UI_STATUS_OK&&p.focused);}
 {ui_input_event_t e={0};CHECK(click(table,"cell-9007199254741000-col0"));e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code=VK_F2;input(table,&e);e.key_code='A';e.modifiers=UI_INPUT_MODIFIER_CONTROL;input(table,&e);e.kind=UI_INPUT_TEXT;e.modifiers=0;e.text_utf8="retained width draft";input(table,&e);CHECK(ui_component_set_column_width(table,"col0",176)==UI_STATUS_OK);pump(60);CHECK(present(table,"edit-9007199254741000-col0",&p)==UI_STATUS_OK&&p.focused&&!strcmp(p.text_utf8,"retained width draft"));uint64_t selected[2];size_t count=0;CHECK(ui_component_get_selection(table,selected,2,&count)==UI_STATUS_OK&&count==1&&selected[0]==9007199254741000ULL);e.kind=UI_INPUT_KEY_DOWN;e.key_code=VK_ESCAPE;e.text_utf8=NULL;input(table,&e);CHECK(ui_component_set_column_width(table,"col0",160)==UI_STATUS_OK);}
 drag(table,"resize-col0",50,1);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==160);drag(table,"resize-col0",-80,2);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==160);
 drag(table,"resize-col0",40,3);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==200);CHECK(ui_component_set_source(table,source,NULL)==UI_STATUS_OK);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==160);
 drag(table,"resize-col0",40,3);CHECK(ui_component_set_sort(table,"col0",-1)==UI_STATUS_OK);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==160);
 {ui_component_t *dialog;ui_component_desc_t modal={0};modal.size=sizeof(modal);modal.id="width-modal";modal.kind=UI_COMPONENT_DIALOG;modal.web_backend=backend;CHECK(ui_component_register(h,&modal,&dialog)==UI_STATUS_OK);drag(table,"resize-col0",40,3);CHECK(ui_component_show_dialog(dialog)==UI_STATUS_OK);pump(200);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==160);CHECK(ui_component_set_column_width(table,"col0",200)==UI_STATUS_CANCELLED);CHECK(ui_component_close_dialog(dialog)==UI_STATUS_OK);pump(200);}
 CHECK(ui_component_set_column_width(table,"col0",23)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_component_set_column_width(table,"col0",4097)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_component_get_column_width(other,"col0",&width)==UI_STATUS_OK&&width==100);
 CHECK(ui_component_save_columns(table,NULL,0,&bytes)==UI_STATUS_OK&&bytes<=2640);saved=malloc(bytes);CHECK(saved);CHECK(ui_component_save_columns(table,saved,bytes,&bytes)==UI_STATUS_OK);CHECK(ui_component_restore_columns(other,saved,bytes)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_component_save_columns(table,saved,bytes-1,&bytes)==UI_STATUS_LIMIT_EXCEEDED);
 CHECK(ui_component_reset_columns(table,"col0")==UI_STATUS_OK);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==100);CHECK(ui_component_restore_columns(table,saved,bytes)==UI_STATUS_OK);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==160);
 ((unsigned char *)saved)[4]=2;CHECK(ui_component_restore_columns(table,saved,bytes)==UI_STATUS_UNSUPPORTED);((unsigned char *)saved)[4]=1;CHECK(ui_component_restore_columns(table,saved,bytes-1)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==160);
 {ui_host_config_t config={0};ui_host_t *new_host;ui_component_t *changed=NULL;ui_column_desc_t definitions[2]={cols[0],cols[0]};ui_component_desc_t description={0};config.size=sizeof(config);config.api_version=UI_FRAMEWORK_API_VERSION;new_host=ui_host_create(&config);definitions[0].id="new-column";definitions[0].width=77;description.size=sizeof(description);description.id="stable-table";description.kind=UI_COMPONENT_TABLE;description.columns=definitions;description.column_count=2;CHECK(new_host&&ui_component_register(new_host,&description,&changed)==UI_STATUS_OK);CHECK(ui_component_restore_columns(changed,saved,bytes)==UI_STATUS_OK);CHECK(ui_component_get_column_width(changed,"new-column",&width)==UI_STATUS_OK&&width==77);CHECK(ui_component_get_column_width(changed,"col0",&width)==UI_STATUS_OK&&width==160);ui_host_destroy(new_host);}
 {unsigned char duplicate[160]={0};memcpy(duplicate,saved,bytes);duplicate[8]=2;memcpy(duplicate+120,duplicate+80,40);CHECK(ui_component_restore_columns(table,duplicate,sizeof(duplicate))==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==160);}
 {int before=queries;CHECK(ui_component_fit_column(table,"col0")==UI_STATUS_OK);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width>160&&width<=4096);CHECK(queries<=before+2);}
 CHECK(ui_component_reset_columns(table,NULL)==UI_STATUS_OK);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==100);CHECK(ui_component_get_column_width(table,"col1",&width)==UI_STATUS_OK&&width==32);
 CHECK(ui_component_select(table,9007199254741001ULL)==UI_STATUS_OK);CHECK(ui_component_set_source(table,source,NULL)==UI_STATUS_OK);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==100);
 for(unsigned dpi=96;dpi<=192;dpi+=48){ui_pixel_buffer_t capture={0};capture.size=sizeof(capture);CHECK(ui_component_capture_rgba(table,640,480,dpi,&capture)==UI_STATUS_OK);pump(80);CHECK(ui_component_set_column_width(table,"col0",128)==UI_STATUS_OK);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==128);}
 CHECK(ui_component_reset_columns(table,NULL)==UI_STATUS_OK);p.size=sizeof(p);CHECK(present(table,"scroll-h-thumb",&p)==UI_STATUS_OK&&p.visible);
 {ui_element_presentation_t track={0};ui_input_event_t e={0};track.size=sizeof(track);CHECK(present(table,"scroll-h-track",&track)==UI_STATUS_OK);e.size=sizeof(e);e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;e.x=p.clip.x+p.clip.width/2;e.y=p.clip.y+p.clip.height/2;input(table,&e);e.kind=UI_INPUT_POINTER_MOVE;e.x=track.clip.x+track.clip.width+100;input(table,&e);e.kind=UI_INPUT_POINTER_UP;input(table,&e);}
 state.size=sizeof(state);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.first_column==63&&last_column&&state.cached_bytes<=2097152&&state.rendered_nodes<=1024);
 CHECK(present(table,"cell-9007199254741000-col63",&cell)==UI_STATUS_OK&&cell.visible&&cell.clip.width>0&&strstr(cell.text_utf8,"中文"));
 delay=1;CHECK(ui_component_set_source(table,source,NULL)==UI_STATUS_OK);ui_component_query_t obsolete=late;CHECK(ui_component_set_column_width(table,"col63",180)==UI_STATUS_OK);ui_component_batch_t b={0};b.size=sizeof(b);b.component_generation=obsolete.component_generation;b.request_id=obsolete.request_id;b.first=obsolete.first;b.total_count=100000;CHECK(ui_component_submit(table,&b)==UI_STATUS_CANCELLED);delay=0;CHECK(ui_component_set_source(table,source,NULL)==UI_STATUS_OK);
 {unsigned char *record=(unsigned char *)saved+80;unsigned char prior=record[33];record[33]=1;CHECK(ui_component_restore_columns(table,saved,bytes)==UI_STATUS_INVALID_ARGUMENT);record[33]=prior;memcpy(record,"missing",8);CHECK(ui_component_restore_columns(table,saved,bytes)==UI_STATUS_OK);CHECK(ui_component_get_column_width(table,"col0",&width)==UI_STATUS_OK&&width==100);}
 drag(table,"resize-col63",24,3);free(saved);if(shell)ui_native_shell_destroy(shell);ui_host_destroy(h);
#ifdef UI_EXPERIENCE_WEBVIEW2
 if(backend){ui_webview2_backend_destroy(backend);pump(2000);CoUninitialize();}
#endif
 if(root)DestroyWindow(root);printf("Column experience %s queries%d: %d failures\n",runtime?"actual Runtime":"Light",queries,failures);return failures?1:0;}
