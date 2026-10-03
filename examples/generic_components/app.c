#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/components.h"
#include "ui_framework/opengl.h"
#include "command_json.h"

typedef struct job { ui_component_query_t query; char component[64]; struct job *next; } job_t;
typedef struct sample {
    ui_app_context_t context;
    ui_component_t *table,*tree,*form,*dialog,*status;
    ui_surface_t *canvas;
    HANDLE worker,wake,stop;
    CRITICAL_SECTION lock;
    job_t *head,*tail;
    int active,closing;
    unsigned operations,version;
    struct {uint64_t id;char column[33],text[256];} changes[64];
    size_t change_count;
    char snapshot[512];
} sample_t;
typedef struct sample_row { ui_row_t row; ui_cell_t cells[16]; char title[256],text[16][80]; } sample_row_t;
static const char *commands[]={"demo.dialog","demo.refresh","demo.submit","demo.edit","demo.select","demo.rename","demo.context","demo.cancel"};
static const char *titles[]={"Web 对话框","刷新图片","提交表单","编辑单元格","选择项目","重命名","节点菜单","取消"};
static const char *column_ids[]={"checked","name","image","style","color","value","c6","c7","c8","c9","c10","c11","c12","c13","c14","c15"};
static DWORD WINAPI worker(void *data)
{
    sample_t *s=(sample_t *)data;HANDLE events[2]={s->stop,s->wake};
    while(WaitForMultipleObjects(2,events,FALSE,INFINITE)!=WAIT_OBJECT_0){job_t *j;
        EnterCriticalSection(&s->lock);j=s->head;if(j){s->head=j->next;if(!s->head)s->tail=NULL;}LeaveCriticalSection(&s->lock);
        if(j){ui_thumbnail_result_t result={0};uint32_t x,y,w=j->query.pixel_width,h=j->query.pixel_height;uint8_t *pixels;
            if(w>256)w=256;if(h>256)h=256;pixels=(uint8_t *)malloc((size_t)w*h*4);
            result.size=sizeof(result);result.component_generation=j->query.component_generation;result.request_id=j->query.request_id;
            result.item_id=j->query.item_id;result.content_version=j->query.content_version;result.dpi=j->query.dpi;
            if(pixels){for(y=0;y<h;++y)for(x=0;x<w;++x){uint8_t *p=pixels+((size_t)y*w+x)*4;
                p[0]=(uint8_t)(40+x*150/(w?w:1));p[1]=(uint8_t)(70+j->query.item_id%120);p[2]=(uint8_t)(90+y*150/(h?h:1));p[3]=(uint8_t)(x<3||y<3?80:255);}
                result.rgba.size=sizeof(result.rgba);result.rgba.width=w;result.rgba.height=h;result.rgba.stride=(size_t)w*4;result.rgba.bytes=result.rgba.stride*h;result.rgba.pixels=pixels;
            }else result.failed=1;
            (void)ui_workspace_post_thumbnail(s->context.workspace,s->context.instance_id,j->component,&result);free(pixels);free(j);
            SetEvent(s->wake);}
    }
    (void)ui_workspace_post_close_complete(s->context.workspace,s->context.instance_id,UI_APP_CLOSE_ALLOW);return 0;
}
static void source(ui_component_t *component,const ui_component_query_t *q,void *data)
{
    sample_t *s=(sample_t *)data;size_t i,k,count;sample_row_t *storage;ui_row_t *rows;ui_component_batch_t batch={0};uint64_t total;
    if(q->kind==UI_QUERY_THUMBNAIL){job_t *j=(job_t *)calloc(1,sizeof(*j));if(!j)return;j->query=*q;
        strcpy_s(j->component,sizeof(j->component),component==s->tree?"tree":"table");EnterCriticalSection(&s->lock);
        if(s->tail)s->tail->next=j;else s->head=j;s->tail=j;LeaveCriticalSection(&s->lock);SetEvent(s->wake);return;}
    total=component==s->tree?(q->parent_id?1000:100):100000;count=q->first<total?q->count:0;if(count>total-q->first&&q->first<total)count=(size_t)(total-q->first);
    storage=(sample_row_t *)calloc(count?count:1,sizeof(*storage));rows=(ui_row_t *)calloc(count?count:1,sizeof(*rows));if(!storage||!rows){free(storage);free(rows);return;}
    for(i=0;i<count;++i){sample_row_t *r=&storage[i];uint64_t index=q->first+i;
        r->row.size=sizeof(r->row);r->row.id=component==s->tree?(q->parent_id?q->parent_id*1000+index+1:index+1):9007199254741000ULL+index;
        r->row.parent_id=q->parent_id;r->row.content_version=s->version;r->row.title=r->title;r->row.has_children=component==s->tree&&!q->parent_id;
        snprintf(r->title,sizeof(r->title),"%s %llu",component==s->tree?(q->parent_id?"节点":"分组"):"项目",(unsigned long long)index);
        for(k=0;component==s->table&&k<q->column_count;++k){size_t col=q->first_column+k;ui_cell_t *cell=&r->cells[k];
            cell->size=sizeof(*cell);cell->column_id=column_ids[col];cell->kind=col==0?UI_VALUE_BOOLEAN:col==2?UI_VALUE_IMAGE:col==3?UI_VALUE_STYLE:col==4?UI_VALUE_COLOR:UI_VALUE_TEXT;
            snprintf(r->text[k],sizeof(r->text[k]),col==0?"false":"值 %llu / %zu",(unsigned long long)index,col);cell->text=r->text[k];cell->color_rgba=0x3b82f6ff;
            cell->style.size=sizeof(cell->style);cell->style.fill_rgba=0x234971c0;cell->style.border_rgba=0x70c4ffff;cell->style.border_width=1;cell->style.line_style=UI_LINE_DASH;cell->style.dot_rgba=0xaaddffff;cell->style.dot_spacing=8;cell->style.dot_radius=2;}
        {size_t at;for(at=0;at<s->change_count;++at)if(s->changes[at].id==r->row.id){
            if(component==s->tree&&!*s->changes[at].column)strcpy_s(r->title,sizeof(r->title),s->changes[at].text);
            for(k=0;component==s->table&&k<q->column_count;++k)if(!strcmp(r->cells[k].column_id,s->changes[at].column))r->cells[k].text=s->changes[at].text;}}
        r->row.cells=r->cells;r->row.cell_count=component==s->table?q->column_count:0;rows[i]=r->row;
    }
    batch.size=sizeof(batch);batch.component_generation=q->component_generation;batch.request_id=q->request_id;batch.parent_id=q->parent_id;batch.first=q->first;batch.total_count=total;batch.rows=rows;batch.row_count=count;
    (void)ui_component_submit(component,&batch);free(rows);free(storage);
}
static void run(ui_host_t *host,uint64_t request,const char *command,const char *params,const char *origin,void *data)
{
    sample_t *s=(sample_t *)data;char status[128];(void)origin;++s->operations;
    if(!strcmp(command,"demo.dialog"))(void)ui_component_show_dialog(s->dialog);
    if(!strcmp(command,"demo.context")){uint64_t id=uj_u64(params,"id");(void)ui_component_show_menu(id>=9007199254741000ULL?s->table:s->tree,id,"节点");}
    if(!strcmp(command,"demo.cancel"))(void)ui_component_close_dialog(s->dialog);
    if(!strcmp(command,"demo.submit")){ui_component_state_t state={0};ui_cell_t size={0};ui_component_t *target=s->form;state.size=sizeof(state);size.size=sizeof(size);
        (void)ui_component_get_state(s->dialog,&state);if(state.modal)target=s->dialog;
        if(ui_component_get_field(target,"size",&size)==UI_STATUS_OK&&*size.text&&strtod(size.text,NULL)<=0){
            (void)ui_component_set_error(target,"size","尺寸必须大于零");(void)ui_host_reply(host,request,0,"{\"error\":\"size must be positive\"}");return;}
        (void)ui_component_accept_fields(target);if(target==s->dialog)(void)ui_component_close_dialog(target);}
    if(!strcmp(command,"demo.edit")||!strcmp(command,"demo.rename")){char column[33]={0},value[256];uint64_t id=uj_u64(params,"id");size_t at;
        if(!id||!uj_get(params,"text",value,sizeof(value))||(!strcmp(command,"demo.edit")&&!uj_get(params,"column",column,sizeof(column)))){
            (void)ui_host_reply(host,request,0,"{\"error\":\"invalid edit payload\"}");return;}
        for(at=0;at<s->change_count;++at)if(s->changes[at].id==id&&!strcmp(s->changes[at].column,column))break;
        if(at==64){(void)ui_host_reply(host,request,0,"{\"error\":\"demo edit capacity reached\"}");return;}
        if(at==s->change_count)++s->change_count;s->changes[at].id=id;strcpy_s(s->changes[at].column,33,column);strcpy_s(s->changes[at].text,256,value);
        if(!*column){ui_row_t row={0};row.size=sizeof(row);row.id=id;row.title=value;row.has_children=id<=100;row.content_version=s->version;
            (void)ui_component_update_rows(s->tree,&row,1);}
        else{ui_component_state_t state={0};state.size=sizeof(state);
            if(ui_component_get_state(s->table,&state)==UI_STATUS_OK)(void)ui_component_query(s->table,0,state.first,20,state.first_column,state.column_count);}}
    if(!strcmp(command,"demo.refresh")){ui_component_state_t state={0};state.size=sizeof(state);++s->version;
        if(ui_component_get_state(s->table,&state)==UI_STATUS_OK)(void)ui_component_query(s->table,0,state.first,20,state.first_column,state.column_count);}
    snprintf(status,sizeof(status),"实例 %llu · 操作 %u · 图片版本 %u",(unsigned long long)s->context.instance_id,s->operations,s->version);
    (void)ui_component_set_text(s->status,status);(void)ui_host_reply(host,request,1,"{\"ok\":true}");
}
static const char *snapshot(void *data)
{
    sample_t *s=(sample_t *)data;ui_component_state_t t={0},tree={0};ui_image_stats_t images={0};t.size=tree.size=sizeof(t);images.size=sizeof(images);
    (void)ui_component_get_state(s->table,&t);(void)ui_component_get_state(s->tree,&tree);(void)ui_image_get_stats(s->context.host,&images);
    snprintf(s->snapshot,sizeof(s->snapshot),"{\"instance\":\"%llu\",\"operations\":%u,\"rows\":\"%llu\",\"dom\":%zu,\"tree_dom\":%zu,\"first\":\"%llu\",\"cached_rows\":%zu,\"columns\":%zu,\"image_bytes\":%zu,\"image_limit\":%zu}",
        (unsigned long long)s->context.instance_id,s->operations,(unsigned long long)t.total_count,t.rendered_nodes,tree.rendered_nodes,(unsigned long long)t.first,t.row_count,t.column_count,images.bytes,images.limit);return s->snapshot;
}
static ui_status_t register_form(sample_t *s,const char *id,ui_component_kind_t kind,ui_component_t **out)
{
    const char *options[]={"自动","手动","混合"};ui_field_desc_t fields[5]={0};ui_component_desc_t d={0};size_t i;
    const char *ids[]={"name","size","enabled","mode","notes"},*labels[]={"名称","尺寸","启用","模式","说明"};
    for(i=0;i<5;++i){fields[i].size=sizeof(fields[i]);fields[i].id=ids[i];fields[i].title=labels[i];fields[i].group=i<2?"基本":"选项";fields[i].kind=i==1?UI_VALUE_NUMBER:i==2?UI_VALUE_BOOLEAN:i==3?UI_VALUE_ENUM:UI_VALUE_TEXT;}
    fields[0].flags=UI_VALUE_REQUIRED;fields[1].unit="mm";fields[1].flags=UI_VALUE_MIXED;fields[3].options=options;fields[3].option_count=3;fields[4].flags=UI_VALUE_MULTILINE;
    d.size=sizeof(d);d.id=id;d.title=kind==UI_COMPONENT_DIALOG?"Web 参数与确认":"属性表单";d.kind=kind;d.fields=fields;d.field_count=5;d.commands.submit="demo.submit";d.commands.cancel="demo.cancel";
    return ui_component_register(s->context.host,&d,out);
}
static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,void **out,ui_assistant_config_t *assistant)
{
    sample_t *s=(sample_t *)calloc(1,sizeof(*s));ui_component_desc_t d={0};ui_column_desc_t columns[16]={0};ui_panel_desc_t panel={0};ui_toolbar_desc_t toolbar={0};ui_layout_desc_t layout={0};ui_status_t status;size_t i;
    if(!s)return UI_STATUS_OUT_OF_MEMORY;s->context=*context;s->version=1;*out=s;InitializeCriticalSection(&s->lock);s->wake=CreateEventW(NULL,FALSE,FALSE,NULL);s->stop=CreateEventW(NULL,TRUE,FALSE,NULL);
    if(!s->wake||!s->stop)return UI_STATUS_PLATFORM_ERROR;
    layout.size=sizeof(layout);layout.menu_bar_height=28;layout.toolbar_height=36;layout.status_bar_height=24;
    layout.left_sidebar_preferred_width=layout.right_sidebar_preferred_width=250;
    layout.left_sidebar_min_width=layout.right_sidebar_min_width=220;
    layout.left_sidebar_max_width=layout.right_sidebar_max_width=300;layout.collapsed_tag_width=32;
    status=ui_host_set_layout(context->host,&layout);if(status!=UI_STATUS_OK)return status;
    for(i=0;i<8;++i){ui_command_desc_t c={0};ui_menu_item_desc_t m={0};c.size=sizeof(c);c.id=commands[i];c.title=titles[i];c.params_schema_json="{\"type\":\"object\"}";c.handler=run;c.user_data=s;
        if(i==0){c.shortcut_key='D';c.shortcut_modifiers=UI_INPUT_MODIFIER_CONTROL;}status=ui_host_register_command(context->host,&c);if(status!=UI_STATUS_OK)return status;
        m.size=sizeof(m);m.id=commands[i];m.title=titles[i];m.menu_path="组件";m.command_id=commands[i];m.order=(int)i;status=ui_host_register_menu_item(context->host,&m);if(status!=UI_STATUS_OK)return status;}
    for(i=0;i<2;++i){ui_menu_item_desc_t item={0};item.size=sizeof(item);item.id=i?"node.refresh":"node.dialog";item.title=titles[i];item.menu_path="节点";item.command_id=commands[i];item.order=(int)i;
        status=ui_host_register_menu_item(context->host,&item);if(status!=UI_STATUS_OK)return status;}
    toolbar.size=sizeof(toolbar);toolbar.id="demo.tools";toolbar.title="组件";toolbar.visible=1;status=ui_host_register_toolbar(context->host,&toolbar);if(status!=UI_STATUS_OK)return status;
    for(i=0;i<2;++i){ui_toolbar_item_desc_t item={0};item.size=sizeof(item);item.id=commands[i];item.title=titles[i];item.toolbar_id=toolbar.id;item.command_id=commands[i];item.order=(int)i;status=ui_host_register_toolbar_item(context->host,&item);if(status!=UI_STATUS_OK)return status;}
    for(i=0;i<3;++i){panel.size=sizeof(panel);panel.entry_url="";panel.id=i==0?"tree":i==1?"form":"drawing";panel.title=i==0?"按需树":i==1?"属性":"绘图内容槽";
        panel.kind=i==2?UI_PANEL_FLOATING:UI_PANEL_SIDEBAR;panel.dock_region=i==0?UI_LAYOUT_REGION_LEFT_SIDEBAR:UI_LAYOUT_REGION_RIGHT_SIDEBAR;panel.preferred_width=i==2?360:250;
        status=ui_host_register_panel(context->host,&panel);if(status!=UI_STATUS_OK)return status;}
    for(i=0;i<16;++i){columns[i].size=sizeof(columns[i]);columns[i].id=column_ids[i];columns[i].title=column_ids[i];columns[i].kind=UI_VALUE_TEXT;columns[i].width=110;}
    d.size=sizeof(d);d.id="table";d.title="十万行 × 16 列 · 按需表格";d.kind=UI_COMPONENT_TABLE;d.columns=columns;d.column_count=16;d.source=source;d.user_data=s;
    d.commands.select="demo.select";d.commands.edit="demo.edit";d.commands.context_menu="demo.context";status=ui_component_register(context->host,&d,&s->table);if(status!=UI_STATUS_OK)return status;
    d.id="tree";d.title="十万节点 · 按展开分页";d.kind=UI_COMPONENT_TREE;d.columns=NULL;d.column_count=0;d.commands.rename="demo.rename";
    status=ui_component_register(context->host,&d,&s->tree);if(status!=UI_STATUS_OK)return status;
    status=register_form(s,"form",UI_COMPONENT_FORM,&s->form);if(status!=UI_STATUS_OK)return status;status=register_form(s,"dialog",UI_COMPONENT_DIALOG,&s->dialog);if(status!=UI_STATUS_OK)return status;
    memset(&d,0,sizeof(d));d.size=sizeof(d);d.id="status";d.title="API 3 · Web 组件";d.kind=UI_COMPONENT_STATUS;status=ui_component_register(context->host,&d,&s->status);if(status!=UI_STATUS_OK)return status;
    assistant->user_data=s;assistant->snapshot=snapshot;assistant->max_permission=UI_ASSISTANT_PERMISSION_EDIT;assistant->source_id="demo.assistant";return UI_STATUS_OK;
}
static void frame(ui_surface_t *surface,void *data)
{
    sample_t *s=(sample_t *)data;ui_rect_t pixels;if(!s->active||ui_surface_make_current(surface)!=UI_STATUS_OK)return;
    (void)ui_surface_get_pixel_rect(surface,&pixels);glViewport(0,0,pixels.width,pixels.height);glClearColor(.06f,.10f,.16f,1);glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_TRIANGLES);glColor3f(.3f,.7f,.9f);glVertex2f(-.7f,-.6f);glColor3f(.7f,.4f,.9f);glVertex2f(.7f,-.6f);glColor3f(.4f,.9f,.6f);glVertex2f(0,.7f);glEnd();(void)ui_surface_swap_buffers(surface);
}
static ui_status_t UI_APP_CALL mount(void *data,const ui_app_context_t *context)
{
    sample_t *s=(sample_t *)data;ui_shell_t *shell=ui_host_get_shell(context->host);ui_surface_desc_t surface={0};ui_opengl_config_t gl={0};ui_status_t status;size_t i;s->context=*context;
    s->worker=CreateThread(NULL,0,worker,s,0,NULL);if(!s->worker)return UI_STATUS_PLATFORM_ERROR;
    {ui_image_id_t icon;status=ui_image_load_resource(context,"icon.png",&icon);if(status!=UI_STATUS_OK)return status;
     (void)ui_host_set_item_image(context->host,"demo.dialog",icon);(void)ui_host_set_item_image(context->host,"demo.refresh",icon);}
    for(i=0;i<8;++i){ui_assistant_command_desc_t c={0};c.size=sizeof(c);c.id=commands[i];c.permission=UI_ASSISTANT_PERMISSION_EDIT;c.params_schema_json="{\"type\":\"object\"}";status=ui_assistant_register_command(context->assistant,&c);if(status!=UI_STATUS_OK)return status;}
    status=ui_component_mount(s->table,ui_shell_get_content_slot(shell,NULL));if(status!=UI_STATUS_OK)return status;
    status=ui_component_mount(s->tree,ui_shell_get_content_slot(shell,"tree"));if(status!=UI_STATUS_OK)return status;
    status=ui_component_mount(s->form,ui_shell_get_content_slot(shell,"form"));if(status!=UI_STATUS_OK)return status;
    surface.size=sizeof(surface);surface.id="drawing";surface.kind=UI_SURFACE_OPENGL;surface.visible=1;gl.size=sizeof(gl);gl.legacy_context=1;
    s->canvas=ui_opengl_surface_create(context->host,&surface,&gl,&status);if(!s->canvas)return status;
    (void)ui_surface_set_callbacks(s->canvas,NULL,frame,s);return ui_content_slot_attach_surface(ui_shell_get_content_slot(shell,"drawing"),s->canvas);
}
static void UI_APP_CALL active(void *data,int active_value)
{sample_t *s=(sample_t *)data;s->active=active_value;if(s->active&&s->canvas)(void)ui_surface_invalidate(s->canvas);}
static ui_app_close_decision_t UI_APP_CALL close_app(void *data,ui_app_close_reason_t reason)
{sample_t *s=(sample_t *)data;(void)reason;s->closing=1;SetEvent(s->stop);return s->worker?UI_APP_CLOSE_WAIT:UI_APP_CLOSE_ALLOW;}
static void UI_APP_CALL unmount(void *data)
{sample_t *s=(sample_t *)data;SetEvent(s->stop);if(s->worker){WaitForSingleObject(s->worker,INFINITE);CloseHandle(s->worker);s->worker=NULL;}if(s->canvas){ui_surface_destroy(s->canvas);s->canvas=NULL;}}
static void UI_APP_CALL destroy(void *data)
{sample_t *s=(sample_t *)data;job_t *j;while(s->head){j=s->head;s->head=j->next;free(j);}if(s->wake)CloseHandle(s->wake);if(s->stop)CloseHandle(s->stop);DeleteCriticalSection(&s->lock);free(s);}
static ui_status_t UI_APP_CALL shutdown_module(void){return UI_STATUS_OK;}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t d={sizeof(d),1,3,create,mount,active,close_app,unmount,destroy,shutdown_module};return &d;}
