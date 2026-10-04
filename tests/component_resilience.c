/* Real host/DLL/template checks; program input/DPI do not prove manual UV-01..04. */
#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"
#include "ui_framework/components.h"
#include <psapi.h>

static int failures;
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while (0)

static void pump_for(DWORD duration)
{
    ULONGLONG end=GetTickCount64()+duration;MSG m;
    do { unsigned n=0;
        while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)&&n++<3000)
            if(m.message!=WM_QUIT){TranslateMessage(&m);DispatchMessageW(&m);}
        Sleep(1);
    } while(GetTickCount64()<end);
}
static void key(ui_component_t *c,uint32_t code,uint32_t modifiers)
{
    ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;
    e.key_code=code;e.modifiers=modifiers;CHECK(ui_component_dispatch_input(c,&e)==UI_STATUS_OK);
}
static void insert(ui_component_t *c,const char *value)
{
    ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_TEXT;e.text_utf8=value;
    CHECK(ui_component_dispatch_input(c,&e)==UI_STATUS_OK);
}
static void click(ui_component_t *c,const char *id)
{
    ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);
    CHECK(ui_component_get_presentation(c,id,&p)==UI_STATUS_OK&&p.visible&&p.enabled);
    e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.x=p.clip.x+5;e.y=p.clip.y+5;
    CHECK(ui_component_dispatch_input(c,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;
    CHECK(ui_component_dispatch_input(c,&e)==UI_STATUS_OK);
}
static void field_equals(ui_component_t *c,const char *expected)
{
    ui_cell_t value={0};ui_component_state_t state={0};value.size=sizeof(value);state.size=sizeof(state);
    CHECK(ui_component_get_field(c,"name",&value)==UI_STATUS_OK&&!strcmp(value.text,expected));
    CHECK(ui_component_get_state(c,&state)==UI_STATUS_OK&&state.dirty);
}
static void layout_transition(host_window_t *window,ui_app_instance_info_t *a,uint64_t other,int mode)
{
    ui_component_t *dialog=ui_component_find(a->host,"dialog");
    if(mode==0||mode==1){
        CHECK(ui_native_shell_set_panel_floating(a->shell,"form",mode==0)==UI_STATUS_OK);
    } else if(mode==2){
        int visible=1;CHECK(ui_host_resize(a->host,400,420)==UI_STATUS_OK);
        CHECK(ui_shell_get_slot_state(ui_host_get_shell(a->host),"form",NULL,NULL,&visible,NULL)==UI_STATUS_OK&&!visible);
        CHECK(ui_host_resize(a->host,1400,820)==UI_STATUS_OK);
    } else if(mode==3){
        CHECK(ui_host_set_dpi(a->host,192)==UI_STATUS_OK);CHECK(ui_host_set_dpi(a->host,96)==UI_STATUS_OK);
    } else if(mode==4){
        printf("focus before refloat=%p\n",(void *)GetFocus());
        CHECK(ui_native_shell_set_panel_floating(a->shell,"form",1)==UI_STATUS_OK);
        printf("focus after refloat=%p\n",(void *)GetFocus());
        CHECK(ui_component_show_dialog(dialog)==UI_STATUS_OK);pump_for(5);
        CHECK(!IsWindowEnabled((HWND)a->native_container));
        {ui_component_t *form=ui_component_find(a->host,"form");ui_input_event_t e={0};
            ui_web_view_t *view;ui_element_presentation_t before={0},after={0};before.size=after.size=sizeof(before);
            e.size=sizeof(e);e.kind=UI_INPUT_TEXT;e.text_utf8="blocked";
            CHECK(ui_component_dispatch_input(form,&e)==UI_STATUS_CANCELLED);
            CHECK(ui_component_get_presentation(form,"field-name",&before)==UI_STATUS_OK);
            for(view=a->host->web_views;view;view=view->host_next)
                if(GetParent((HWND)ui_web_view_native_handle(view))==(HWND)ui_native_shell_panel_handle(a->shell,"form"))break;
            CHECK(view!=NULL);if(view){
                CHECK(ui_web_view_dispatch_input(view,&e)==UI_STATUS_CANCELLED);
                SendMessageW((HWND)ui_web_view_native_handle(view),WM_CHAR,'!',0);
                SendMessageW((HWND)ui_web_view_native_handle(view),WM_KEYDOWN,VK_BACK,0);}
            CHECK(ui_component_get_presentation(form,"field-name",&after)==UI_STATUS_OK&&!strcmp(before.text_utf8,after.text_utf8));}
        click(dialog,"field-name");insert(dialog,"allowed");field_equals(dialog,"allowed");
        CHECK(ui_workspace_activate(window->workspace,other)==UI_STATUS_OK);
        CHECK(ui_workspace_activate(window->workspace,a->instance_id)==UI_STATUS_OK);
        CHECK(ui_component_close_dialog(dialog)==UI_STATUS_OK);
        printf("focus after modal close=%p\n",(void *)GetFocus());
        CHECK(IsWindowEnabled((HWND)a->native_container));
        CHECK(ui_native_shell_set_panel_floating(a->shell,"form",0)==UI_STATUS_OK);
        printf("focus after redock=%p\n",(void *)GetFocus());
    } else {
        CHECK(ui_workspace_activate(window->workspace,other)==UI_STATUS_OK);
        CHECK(ui_workspace_activate(window->workspace,a->instance_id)==UI_STATUS_OK);
    }
    pump_for(5);
}
static void edit_layout(host_window_t *window,ui_app_instance_info_t *a,uint64_t other)
{
    ui_component_t *form=ui_component_find(a->host,"form");
    const char *draft="ABC中文😀尾部";int mode;
    CHECK(ui_host_resize(a->host,1400,820)==UI_STATUS_OK);
    for(mode=0;mode<6&&!failures;++mode){
        ui_element_presentation_t p={0};p.size=sizeof(p);
        click(form,"field-name");key(form,'A',UI_INPUT_MODIFIER_CONTROL);insert(form,draft);
        key(form,VK_HOME,0);key(form,VK_RIGHT,UI_INPUT_MODIFIER_SHIFT);
        key(form,VK_RIGHT,UI_INPUT_MODIFIER_SHIFT);key(form,VK_RIGHT,UI_INPUT_MODIFIER_SHIFT);
        layout_transition(window,a,other,mode);field_equals(form,draft);
        CHECK(ui_component_get_presentation(form,"field-name",&p)==UI_STATUS_OK&&p.focused);
        {ui_web_view_t *view;for(view=a->host->web_views;view;view=view->host_next)
            if(GetParent((HWND)ui_web_view_native_handle(view))==(HWND)ui_native_shell_panel_handle(a->shell,"form"))break;
            CHECK(view&&GetFocus()==(HWND)ui_web_view_native_handle(view));}
        insert(form,"新");field_equals(form,"新中文😀尾部");
        key(form,'Z',UI_INPUT_MODIFIER_CONTROL);field_equals(form,draft);
        printf("edit/layout mode %d: %d failures\n",mode,failures);
    }
    CHECK(ui_native_shell_set_panel_floating(a->shell,"form",0)==UI_STATUS_OK);
}
static void sample_resources(ui_app_instance_info_t *a,ULONGLONG elapsed,unsigned iterations)
{
    PROCESS_MEMORY_COUNTERS_EX memory={0};DWORD handles=0;ui_image_stats_t images={0};
    ui_component_state_t table={0},tree={0};memory.cb=sizeof(memory);images.size=sizeof(images);table.size=tree.size=sizeof(table);
    CHECK(GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS *)&memory,sizeof(memory)));
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&handles));
    if(a){
        CHECK(ui_image_get_stats(a->host,&images)==UI_STATUS_OK&&images.bytes<=images.limit);
        CHECK(ui_component_get_state(ui_component_find(a->host,"table"),&table)==UI_STATUS_OK&&table.presentation_status==UI_STATUS_OK&&table.rendered_nodes<1024&&table.cached_bytes<=2u*1024u*1024u);
        CHECK(ui_component_get_state(ui_component_find(a->host,"tree"),&tree)==UI_STATUS_OK&&tree.presentation_status==UI_STATUS_OK&&tree.rendered_nodes<1024&&tree.cached_bytes<=2u*1024u*1024u);
    }
    printf("resource,%llu,%u,%llu,%zu,%zu,%lu,%lu,%lu,%zu,%zu,%zu,%zu,%zu,%zu,%zu\n",
        (unsigned long long)elapsed,iterations,(unsigned long long)(a?a->instance_id:0),
        (size_t)memory.WorkingSetSize,(size_t)memory.PrivateUsage,(unsigned long)handles,
        (unsigned long)GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),
        (unsigned long)GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS),
        table.rendered_nodes,table.cached_rows,table.cached_bytes,tree.rendered_nodes,tree.cached_bytes,images.bytes,images.count);
    fflush(stdout);
}
static void close_one(host_window_t *window,uint64_t id)
{
    ui_app_instance_info_t info={0};ULONGLONG end=GetTickCount64()+5000;
    CHECK(ui_workspace_close(window->workspace,id,UI_APP_CLOSE_TAB)==UI_STATUS_OK);
    while(get_instance(window,id,&info)&&GetTickCount64()<end){pump_for(2);ui_workspace_poll(window->workspace);}
    CHECK(!get_instance(window,id,&info));
}
static void style_source(ui_component_t *c,const ui_component_query_t *q,void *data)
{
    ui_cell_t cells[3]={0};ui_row_t row={0};ui_component_batch_t batch={0};size_t i;
    const char *ids[]={"generated","cell-image","style-image"};ui_image_id_t supplied=*(ui_image_id_t *)data;
    for(i=0;i<3;++i){cells[i].size=sizeof(cells[i]);cells[i].column_id=ids[i];cells[i].kind=UI_VALUE_STYLE;
        cells[i].style.size=sizeof(cells[i].style);cells[i].style.fill_rgba=0x234971c0;}
    cells[1].image_id=supplied;cells[2].style.image_id=supplied;
    row.size=sizeof(row);row.id=q->first+1;row.title="style";row.cells=cells;row.cell_count=3;
    batch.size=sizeof(batch);batch.component_generation=q->component_generation;batch.request_id=q->request_id;
    batch.first=q->first;batch.total_count=1000;batch.rows=&row;batch.row_count=1;
    CHECK(ui_component_submit(c,&batch)==UI_STATUS_OK);
}
static void preview_lifetime(void)
{
    ui_host_config_t hc={0};ui_component_desc_t desc={0};ui_column_desc_t columns[3]={0};ui_field_desc_t field={0};
    ui_cell_t value={0};ui_rgba_desc_t rgba={0};ui_image_id_t supplied=0;ui_image_stats_t stats={0};ui_image_info_t info={0};
    ui_host_t *host;ui_component_t *table=NULL,*form=NULL;uint8_t pixel[4]={0,100,200,255};unsigned i;
    hc.size=sizeof(hc);hc.api_version=4;host=ui_host_create(&hc);CHECK(host!=NULL);if(!host)return;
    rgba.size=sizeof(rgba);rgba.width=rgba.height=1;rgba.stride=rgba.bytes=4;rgba.pixels=pixel;
    CHECK(ui_image_create(host,&rgba,&supplied)==UI_STATUS_OK);
    for(i=0;i<3;++i){columns[i].size=sizeof(columns[i]);columns[i].id=i==0?"generated":i==1?"cell-image":"style-image";columns[i].title=columns[i].id;columns[i].kind=UI_VALUE_STYLE;columns[i].width=80;}
    desc.size=sizeof(desc);desc.id="styles";desc.kind=UI_COMPONENT_TABLE;desc.columns=columns;desc.column_count=3;desc.source=style_source;desc.user_data=&supplied;
    CHECK(ui_component_register(host,&desc,&table)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(table,400,300,96)==UI_STATUS_OK);
    field.size=sizeof(field);field.id="preview";field.kind=UI_VALUE_STYLE;field.title="Preview";
    memset(&desc,0,sizeof(desc));desc.size=sizeof(desc);desc.id="style-form";desc.kind=UI_COMPONENT_FORM;desc.fields=&field;desc.field_count=1;
    CHECK(ui_component_register(host,&desc,&form)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(form,300,300,96)==UI_STATUS_OK);
    value.size=sizeof(value);value.kind=UI_VALUE_STYLE;value.style.size=sizeof(value.style);stats.size=sizeof(stats);info.size=sizeof(info);
    for(i=0;i<200&&!failures;++i){value.style.fill_rgba=0x23497100u|i;
        CHECK(ui_component_set_field(form,"preview",&value)==UI_STATUS_OK);
        CHECK(ui_component_query(table,0,i,1,0,3)==UI_STATUS_OK);
        CHECK(ui_image_get_stats(host,&stats)==UI_STATUS_OK&&stats.count<=3);
        CHECK(ui_image_get_info(host,supplied,&info)==UI_STATUS_OK);}
    {uint64_t removed=200;CHECK(ui_component_remove_rows(table,&removed,1)==UI_STATUS_OK);
        CHECK(ui_image_get_stats(host,&stats)==UI_STATUS_OK&&stats.count==2);}
    CHECK(ui_component_query(table,0,500,1,0,3)==UI_STATUS_OK);
    CHECK(ui_component_set_source(table,style_source,&supplied)==UI_STATUS_OK);
    CHECK(ui_image_get_stats(host,&stats)==UI_STATUS_OK&&stats.count==2);
    CHECK(ui_component_query(table,0,501,1,0,3)==UI_STATUS_OK);
    value.style.image_id=supplied;CHECK(ui_component_set_field(form,"preview",&value)==UI_STATUS_OK);
    CHECK(ui_image_get_stats(host,&stats)==UI_STATUS_OK&&stats.count==2);
    value.style.image_id=0;value.image_id=supplied;CHECK(ui_component_set_field(form,"preview",&value)==UI_STATUS_OK);
    CHECK(ui_image_get_stats(host,&stats)==UI_STATUS_OK&&stats.count==2);
    CHECK(ui_component_unregister(table)==UI_STATUS_OK);CHECK(ui_component_unregister(form)==UI_STATUS_OK);
    CHECK(ui_image_get_stats(host,&stats)==UI_STATUS_OK&&stats.count==1);CHECK(ui_image_get_info(host,supplied,&info)==UI_STATUS_OK);
    CHECK(ui_image_release(host,supplied)==UI_STATUS_OK);ui_host_destroy(host);
    printf("automatic style preview lifetime: %d failures\n",failures);
}
int wmain(int argc,wchar_t **argv)
{
    host_window_t window={0};HWND root;ui_app_instance_info_t a={0},b={0};uint64_t first,second;
    ULONGLONG started,next_sample;unsigned iterations=0,seconds=argc>2?(unsigned)_wtoi(argv[2]):2;
    if(argc<2||!seconds||seconds>3600)return 2;
    CHECK(ui_framework_initialize()==UI_STATUS_OK);preview_lifetime();if(failures)return 1;
    root=create_host(&window,GetModuleHandleW(NULL));
    CHECK(root!=NULL);if(!root)return 1;ShowWindow(root,SW_SHOWNOACTIVATE);
    sample_resources(NULL,0,0);
    open_path(&window,argv[1]);pump_for(50);first=ui_workspace_active(window.workspace);
    open_path(&window,argv[1]);pump_for(50);second=ui_workspace_active(window.workspace);
    CHECK(first&&second&&first!=second);CHECK(get_instance(&window,first,&a)&&get_instance(&window,second,&b));
    if(!first||!second){DestroyWindow(root);return 1;}
    CHECK(ui_workspace_activate(window.workspace,first)==UI_STATUS_OK);pump_for(10);edit_layout(&window,&a,second);
    CHECK(ui_component_query(ui_component_find(a.host,"tree"),0,0,100,0,0)==UI_STATUS_OK);
    CHECK(ui_component_query(ui_component_find(b.host,"tree"),0,0,100,0,0)==UI_STATUS_OK);
    puts("record,elapsed_ms,iterations,instance,working_set,private_bytes,handles,gdi,user,table_dom,table_rows,table_bytes,tree_dom,tree_bytes,image_bytes,image_count");
    started=GetTickCount64();next_sample=started;
    while(!failures&&GetTickCount64()-started<(ULONGLONG)seconds*1000){
        ui_app_instance_info_t *active=iterations%2?&b:&a;ui_component_t *table=ui_component_find(active->host,"table"),*tree=ui_component_find(active->host,"tree");uint64_t request;
        CHECK(ui_workspace_activate(window.workspace,active->instance_id)==UI_STATUS_OK);
        CHECK(ui_component_query(table,0,(iterations*997u)%99988u,12,iterations%13,4)==UI_STATUS_OK);
        {ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);
            CHECK(ui_component_get_presentation(table,"rows",&p)==UI_STATUS_OK&&p.visible);
            e.size=sizeof(e);e.kind=UI_INPUT_WHEEL;e.x=p.clip.x+5;e.y=p.clip.y+5;e.wheel_delta=-120;
            CHECK(ui_component_dispatch_input(table,&e)==UI_STATUS_OK);e.modifiers=UI_INPUT_MODIFIER_SHIFT;
            CHECK(ui_component_dispatch_input(table,&e)==UI_STATUS_OK);}
        CHECK(ui_component_expand(tree,iterations%100+1,(iterations/100)%2==0)==UI_STATUS_OK);
        if(iterations%8==0)CHECK(ui_workspace_invoke(window.workspace,active->instance_id,"demo.refresh","{}",&request)==UI_STATUS_OK);
        pump_for(5);ui_workspace_poll(window.workspace);++iterations;
        if(GetTickCount64()>=next_sample){sample_resources(&a,GetTickCount64()-started,iterations);sample_resources(&b,GetTickCount64()-started,iterations);next_sample=GetTickCount64()+1000;}
        if(iterations%200==0){uint64_t old=b.instance_id;close_one(&window,old);open_path(&window,argv[1]);pump_for(20);
            second=ui_workspace_active(window.workspace);CHECK(second&&second!=old&&get_instance(&window,second,&b));CHECK(ui_workspace_count(window.workspace)==2);
            CHECK(ui_component_query(ui_component_find(b.host,"tree"),0,0,100,0,0)==UI_STATUS_OK);}
    }
    sample_resources(&a,GetTickCount64()-started,iterations);sample_resources(&b,GetTickCount64()-started,iterations);
    close_one(&window,a.instance_id);close_one(&window,b.instance_id);CHECK(ui_workspace_count(window.workspace)==0);
    CHECK(GetModuleHandleW(L"framework_features_app.dll")==NULL);sample_resources(NULL,GetTickCount64()-started,iterations);
    DestroyWindow(root);pump_for(5);printf("resilience: %d failures; %u iterations; physical/manual checks remain open\n",failures,iterations);
    return failures?1:0;
}
