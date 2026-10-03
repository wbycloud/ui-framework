/* Real shell, JavaScript, components, worker queue and dynamically loaded DLLs. */
#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"
#include "ui_framework/components.h"
#include "ui_framework/opengl.h"
static int failed,forbidden;
#define CHECK(x) do{if(!(x)){printf("FAIL %d: %s\n",__LINE__,#x);++failed;}}while(0)
static void pump_for(DWORD duration)
{ULONGLONG end=GetTickCount64()+duration;MSG m;do{while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){if(m.message!=WM_QUIT){TranslateMessage(&m);DispatchMessageW(&m);}}Sleep(1);}while(GetTickCount64()<end);}
static BOOL CALLBACK controls(HWND hwnd,LPARAM parameter)
{wchar_t name[80];(void)parameter;GetClassNameW(hwnd,name,80);if(!_wcsicmp(name,L"Edit")||!_wcsicmp(name,L"Button")||!_wcsicmp(name,L"SysTreeView32")||!_wcsicmp(name,L"SysListView32")||!_wcsicmp(name,L"ComboBox")){++forbidden;printf("Native control: %ls\n",name);}return TRUE;}
static BOOL CALLBACK windows(HWND hwnd,LPARAM pid)
{DWORD owner;GetWindowThreadProcessId(hwnd,&owner);if(owner==(DWORD)pid)EnumChildWindows(hwnd,controls,0);return TRUE;}
static ui_web_view_t *content_view(ui_host_t *host,HWND parent)
{ui_web_view_t *v;for(v=host->web_views;v;v=v->host_next)if(GetParent((HWND)ui_web_view_native_handle(v))==parent)return v;return NULL;}
static int press(ui_web_view_t *v,const char *id)
{ui_rect_t r;ui_input_event_t e={0};if(!v||ui_web_view_get_element_rect(v,id,&r)!=UI_STATUS_OK||!r.width||!r.height)return 0;
 e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.x=r.x+6;e.y=r.y+8;(void)ui_web_view_dispatch_input(v,&e);e.kind=UI_INPUT_POINTER_UP;return ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK;}
static void text(ui_web_view_t *v,const char *value)
{ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_TEXT;e.text_utf8=value;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);}
static ui_component_query_t thumbnail_query;static ui_row_t image_row;static ui_cell_t image_cell;
static void image_source(ui_component_t *c,const ui_component_query_t *q,void *data)
{
    ui_component_batch_t batch={0};(void)data;if(q->kind==UI_QUERY_THUMBNAIL){thumbnail_query=*q;return;}
    image_cell.size=sizeof(image_cell);image_cell.column_id="image";image_cell.kind=UI_VALUE_IMAGE;
    image_row.size=sizeof(image_row);image_row.id=42;image_row.content_version=1;image_row.title="async image";image_row.cells=&image_cell;image_row.cell_count=1;
    batch.size=sizeof(batch);batch.component_generation=q->component_generation;batch.request_id=q->request_id;batch.parent_id=q->parent_id;batch.first=q->first;batch.total_count=1;batch.rows=&image_row;batch.row_count=1;
    CHECK(ui_component_submit(c,&batch)==UI_STATUS_OK);
}
int wmain(int argc,wchar_t **argv)
{
    host_window_t window;HWND root;ui_app_instance_info_t a={0},b={0};uint64_t first,second,request;ui_component_t *table,*form,*tree;ui_component_state_t state={0};ui_web_view_t *table_view,*form_view;ui_rect_t r;ui_image_stats_t images={0};int i,j;
    const int sizes[][2]={{1920,1080},{1280,720},{800,600},{640,480}};const uint32_t dpi[]={96,144,192};
    if(argc<2)return 2;CHECK(ui_framework_initialize()==UI_STATUS_OK);memset(&window,0,sizeof(window));root=create_host(&window,GetModuleHandleW(NULL));CHECK(root!=NULL);if(!root)return 1;
    ShowWindow(root,SW_SHOW);open_path(&window,argv[1]);pump_for(100);first=ui_workspace_active(window.workspace);
    CHECK(first!=0);if(!first){printf("Load: %s\n",ui_workspace_last_error(window.workspace));DestroyWindow(root);return 1;}
    open_path(&window,argv[1]);pump_for(100);second=ui_workspace_active(window.workspace);CHECK(second&&first!=second);
    CHECK(get_instance(&window,first,&a)&&get_instance(&window,second,&b));table=ui_component_find(a.host,"table");form=ui_component_find(a.host,"form");tree=ui_component_find(a.host,"tree");
    CHECK(table&&form&&tree);state.size=sizeof(state);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.total_count==100000&&state.rendered_nodes>30&&state.rendered_nodes<1024);
    printf("initial actual DOM=%zu rows=%zu columns=%zu\n",state.rendered_nodes,state.row_count,state.column_count);
    CHECK(ui_component_select(table,9007199254741000ULL)==UI_STATUS_OK);CHECK(ui_component_get_state(ui_component_find(b.host,"table"),&state)==UI_STATUS_OK&&state.selected_id==0);
    CHECK(ui_workspace_activate(window.workspace,first)==UI_STATUS_OK);pump_for(30);
    table_view=content_view(a.host,(HWND)a.native_container);form_view=content_view(a.host,(HWND)ui_native_shell_panel_handle(a.shell,"form"));CHECK(table_view&&form_view);
    CHECK(press(form_view,"field-name"));text(form_view,"未提交中文 draft");CHECK(ui_component_get_state(form,&state)==UI_STATUS_OK&&state.dirty);
    CHECK(ui_workspace_activate(window.workspace,second)==UI_STATUS_OK);CHECK(ui_workspace_activate(window.workspace,first)==UI_STATUS_OK);pump_for(30);
    CHECK(ui_component_get_state(form,&state)==UI_STATUS_OK&&state.dirty);CHECK(ui_web_view_get_element_rect(form_view,"field-name",&r)==UI_STATUS_OK);CHECK(GetFocus()==(HWND)ui_web_view_native_handle(form_view));
    {ui_cell_t value={0};value.size=sizeof(value);CHECK(ui_component_get_field(form,"name",&value)==UI_STATUS_OK&&strstr(value.text,"未提交中文 draft")!=NULL);}
    {ui_cell_t value={0};value.size=sizeof(value);value.column_id="size";value.kind=UI_VALUE_NUMBER;value.text="-2";
        CHECK(ui_component_set_field(form,"size",&value)==UI_STATUS_OK);CHECK(press(form_view,"submit"));
        CHECK(ui_component_get_field(form,"size",&value)==UI_STATUS_OK&&strstr(value.error,"尺寸必须大于零")!=NULL);
        value.flags=0;value.text="2";value.error="";CHECK(ui_component_set_field(form,"size",&value)==UI_STATUS_OK);CHECK(press(form_view,"submit"));
        CHECK(ui_component_get_state(form,&state)==UI_STATUS_OK&&!state.dirty);}
    CHECK(ui_component_query(table,0,99900,12,12,4)==UI_STATUS_OK);pump_for(50);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.first==99900&&state.row_count==12&&state.column_count==4&&state.rendered_nodes<1024);
    printf("100k tail actual DOM=%zu rows=%zu columns=%zu\n",state.rendered_nodes,state.row_count,state.column_count);
    CHECK(ui_component_expand(tree,1,1)==UI_STATUS_OK);pump_for(30);CHECK(ui_component_get_state(tree,&state)==UI_STATUS_OK&&state.total_count==1100&&state.rendered_nodes<1024);
    {const char *before,*after;unsigned count;ui_web_view_t *tree_view=content_view(a.host,(HWND)ui_native_shell_panel_handle(a.shell,"tree"));
        CHECK(ui_assistant_get_state_snapshot(a.assistant,&before)==UI_STATUS_OK);before=strstr(before,"\"operations\":");count=before?(unsigned)strtoul(before+13,NULL,10):0;
        CHECK(press(tree_view,"tree-label-1001"));CHECK(ui_assistant_get_state_snapshot(a.assistant,&after)==UI_STATUS_OK);
        after=strstr(after,"\"operations\":");CHECK(after&&(unsigned)strtoul(after+13,NULL,10)==count+1);}
    CHECK(ui_component_query(tree,0,0,100,0,0)==UI_STATUS_OK);
    for(i=1;i<=100;++i)CHECK(ui_component_expand(tree,(uint64_t)i,1)==UI_STATUS_OK);
    CHECK(ui_component_get_state(tree,&state)==UI_STATUS_OK&&state.total_count==100100&&state.cached_rows<5000&&state.rendered_nodes<1024);
    printf("100k expanded tree actual DOM=%zu cached rows=%zu total=%llu\n",state.rendered_nodes,state.cached_rows,(unsigned long long)state.total_count);
    CHECK(ui_workspace_invoke(window.workspace,first,"demo.context","{\"id\":\"1001\"}",&request)==UI_STATUS_OK);pump_for(10);
    CHECK(press(content_view(a.host,(HWND)ui_native_shell_panel_handle(a.shell,"tree")),"node-menu-0"));pump_for(30);
    CHECK(ui_component_get_state(ui_component_find(a.host,"dialog"),&state)==UI_STATUS_OK&&state.modal);
    CHECK(!IsWindowEnabled((HWND)a.native_container)&&IsWindowEnabled((HWND)b.native_container));
    CHECK(ui_workspace_activate(window.workspace,second)==UI_STATUS_OK);CHECK(ui_workspace_activate(window.workspace,first)==UI_STATUS_OK);
    CHECK(ui_component_close_dialog(ui_component_find(a.host,"dialog"))==UI_STATUS_OK&&IsWindowEnabled((HWND)a.native_container));
    EnumWindows(windows,(LPARAM)GetCurrentProcessId());CHECK(forbidden==0);
    {ui_surface_t *canvas=a.host->surfaces;void *native;ui_opengl_info_t info={0};ui_rect_t pixels;
        while(canvas&&canvas->kind!=UI_SURFACE_OPENGL)canvas=canvas->next;CHECK(canvas!=NULL);
        if(canvas){native=ui_surface_native_handle(canvas);info.size=sizeof(info);CHECK(ui_opengl_surface_get_info(canvas,&info)==UI_STATUS_OK);
            ShowWindow(root,SW_MINIMIZE);pump_for(10);ShowWindow(root,SW_RESTORE);pump_for(10);
            CHECK(ui_surface_native_handle(canvas)==native&&ui_surface_get_pixel_rect(canvas,&pixels)==UI_STATUS_OK&&pixels.width>0);
            CHECK(ui_surface_make_current(canvas)==UI_STATUS_OK&&ui_surface_swap_buffers(canvas)==UI_STATUS_OK);}}
    for(i=0;i<4;++i)for(j=0;j<3;++j){SetWindowPos(root,NULL,0,0,sizes[i][0],sizes[i][1],SWP_NOZORDER|SWP_NOACTIVATE);window.dpi=dpi[j];layout(&window);pump_for(5);
        CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.rendered_nodes<1024);}
    images.size=sizeof(images);CHECK(ui_image_get_stats(a.host,&images)==UI_STATUS_OK&&images.bytes<=images.limit);printf("image bytes=%zu limit=%zu evictions=%zu\n",images.bytes,images.limit,images.evictions);
    CHECK(ui_component_set_source(table,image_source,NULL)==UI_STATUS_OK);CHECK(ui_component_query(table,0,0,1,2,1)==UI_STATUS_OK);
    {ui_thumbnail_result_t old={0},fresh;uint8_t pixel[4]={255,0,0,128};size_t nodes,mounts;
        CHECK(thumbnail_query.kind==UI_QUERY_THUMBNAIL);old.size=sizeof(old);old.component_generation=thumbnail_query.component_generation;old.request_id=thumbnail_query.request_id;
        old.item_id=42;old.content_version=1;old.dpi=thumbnail_query.dpi;old.rgba.size=sizeof(old.rgba);old.rgba.width=old.rgba.height=1;old.rgba.stride=old.rgba.bytes=4;old.rgba.pixels=pixel;
        CHECK(ui_component_thumbnail(table,&old)==UI_STATUS_OK);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK);nodes=state.rendered_nodes;mounts=state.row_nodes_created;
        pixel[1]=80;CHECK(ui_component_thumbnail(table,&old)==UI_STATUS_OK);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.rendered_nodes==nodes&&state.row_nodes_created==mounts);
        image_row.content_version=2;CHECK(ui_component_update_rows(table,&image_row,1)==UI_STATUS_OK);CHECK(ui_component_thumbnail(table,&old)==UI_STATUS_CANCELLED);
        fresh=old;fresh.content_version=2;fresh.request_id=thumbnail_query.request_id;CHECK(ui_component_thumbnail(table,&fresh)==UI_STATUS_OK);
        {uint64_t id=42;CHECK(ui_component_remove_rows(table,&id,1)==UI_STATUS_OK);}CHECK(ui_component_thumbnail(table,&fresh)==UI_STATUS_CANCELLED);
    }
    CHECK(ui_workspace_set_component_queue_limit(window.workspace,first,64)==UI_STATUS_OK);
    {ui_thumbnail_result_t result={0};uint8_t pixel[4]={0};result.size=sizeof(result);result.rgba.size=sizeof(result.rgba);result.rgba.width=result.rgba.height=1;result.rgba.stride=result.rgba.bytes=4;result.rgba.pixels=pixel;
     CHECK(ui_workspace_post_thumbnail(window.workspace,first,"table",&result)==UI_STATUS_LIMIT_EXCEEDED);}
    CHECK(ui_workspace_close(window.workspace,first,UI_APP_CLOSE_TAB)==UI_STATUS_OK);pump_for(100);CHECK(!get_instance(&window,first,&a));
    CHECK(ui_workspace_post_component_batch(window.workspace,first,"table",NULL)==UI_STATUS_INVALID_ARGUMENT);
    open_path(&window,argv[1]);pump_for(50);CHECK(ui_workspace_active(window.workspace)!=first&&ui_workspace_count(window.workspace)==2);
    if(argc>2){open_path(&window,argv[2]);pump_for(20);CHECK(ui_workspace_count(window.workspace)==3);}
    if(argc>3){open_path(&window,argv[3]);pump_for(20);CHECK(ui_workspace_count(window.workspace)==4);}
    CHECK(ui_workspace_close_all(window.workspace)==UI_STATUS_OK);pump_for(150);CHECK(ui_workspace_count(window.workspace)==0);DestroyWindow(root);pump_for(10);
    printf("generic Web real host: %d failures; physical IME/multiple monitors not tested\n",failed);return failed?1:0;
}
