#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/ui_internal.h"
#include "ui_framework/webview2.h"
static int failures;static ui_workspace_t *workspace;
static int created_windows;
static LRESULT CALLBACK window_hook(int code,WPARAM w,LPARAM p)
{if(code==HCBT_CREATEWND)++created_windows;return CallNextHookEx(NULL,code,w,p);}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"integration line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(DWORD ms)
{ULONGLONG start=GetTickCount64();do{MSG m;unsigned n=0;while(n++<1000&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}ui_workspace_poll(workspace);Sleep(1);}while(GetTickCount64()-start<ms);}
static ui_host_t *host(uint64_t id)
{ui_app_instance_info_t info={0};info.size=sizeof(info);CHECK(ui_workspace_get_instance(workspace,id,&info)==UI_STATUS_OK);return info.host;}
static ui_status_t query(ui_component_t *c,const char *id,ui_element_presentation_t *p)
{ULONGLONG start=GetTickCount64();ui_status_t status;do{status=ui_component_get_presentation(c,id,p);if(status!=UI_STATUS_PENDING)return status;pump(1);}while(GetTickCount64()-start<10000);return status;}
static int click(ui_component_t *c,const char *id,unsigned modifiers)
{ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);if(query(c,id,&p)!=UI_STATUS_OK||!p.visible||!p.enabled)return 0;
 e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.x=p.clip.x+p.clip.width/2;e.y=p.clip.y+p.clip.height/2;e.modifiers=modifiers;if(ui_component_dispatch_input(c,&e)!=UI_STATUS_OK)return 0;pump(60);e.kind=UI_INPUT_POINTER_UP;if(ui_component_dispatch_input(c,&e)!=UI_STATUS_OK)return 0;pump(60);return 1;}
static void input(ui_component_t *c,unsigned key,unsigned modifiers,const char *text)
{ui_input_event_t e={0};e.size=sizeof(e);e.kind=text?UI_INPUT_TEXT:UI_INPUT_KEY_DOWN;e.key_code=key;e.modifiers=modifiers;e.text_utf8=text;CHECK(ui_component_dispatch_input(c,&e)==UI_STATUS_OK);pump(60);}
static void close_instance(uint64_t id)
{ui_status_t status=ui_workspace_close(workspace,id,UI_APP_CLOSE_TAB);CHECK(status==UI_STATUS_OK||status==UI_STATUS_PENDING);for(int i=0;i<100&&ui_workspace_count(workspace);++i){ui_app_instance_info_t info={0};info.size=sizeof(info);if(ui_workspace_get_instance(workspace,id,&info)==UI_STATUS_NOT_FOUND)break;pump(10);}}
static size_t captured_white(ui_component_t *c)
{
    ui_pixel_buffer_t p={0};ui_status_t status;ULONGLONG start;size_t white=0;p.size=sizeof(p);
    start=GetTickCount64();do{status=ui_component_capture_rgba(c,320,240,96,&p);if(status!=UI_STATUS_PENDING)break;pump(1);}while(GetTickCount64()-start<10000);CHECK(status==UI_STATUS_OK);if(status!=UI_STATUS_OK)return 0;
    p.capacity=p.stride*p.height;p.pixels=(uint8_t *)malloc(p.capacity);CHECK(p.pixels!=NULL);if(!p.pixels)return 0;
    start=GetTickCount64();do{status=ui_component_capture_rgba(c,320,240,96,&p);if(status!=UI_STATUS_PENDING)break;pump(1);}while(GetTickCount64()-start<10000);CHECK(status==UI_STATUS_OK);
    if(status==UI_STATUS_OK)for(size_t at=0;at<p.capacity;at+=4)if(p.pixels[at]==255&&p.pixels[at+1]==255&&p.pixels[at+2]==255)++white;free(p.pixels);return white;
}
int wmain(int argc,wchar_t **argv)
{
    ui_workspace_config_t config={0};HWND root=NULL;HHOOK hook=NULL;ui_host_t *h,*other;ui_component_t *form,*table,*tree,*dialog,*viewport;
    uint64_t a=0,b=0,ids[512];ui_image_id_t viewport_image;size_t n=0,bytes;char package[16384];wchar_t provider_path[4096];int runtime;ui_rect_t root_rect={0,0,1280,900};ui_panel_layout_t layout={0};
    ui_element_presentation_t p={0};ui_cell_t field={0};ui_opengl_info_t info={0};ui_pixel_buffer_t pixels={0};ui_image_info_t image={0};ui_image_stats_t images={0};ui_component_state_t state={0};unsigned char *saved;DWORD handles,gdi,users;
    setvbuf(stdout,NULL,_IONBF,0);setvbuf(stderr,NULL,_IONBF,0);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    puts("API6 integration: initialize");if(argc!=4||!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,package,sizeof(package),NULL,NULL))return 2;
    runtime=!wcscmp(argv[3],L"webview2");CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);
    if(!runtime){hook=SetWindowsHookExW(WH_CBT,window_hook,NULL,GetCurrentThreadId());CHECK(hook!=NULL);}
    CHECK(GetFullPathNameW(argv[2],4096,provider_path,NULL)>0);SetEnvironmentVariableW(L"UI_API6_BACKEND",argv[3]);SetEnvironmentVariableW(L"UI_API6_OSMESA_DLL",provider_path);config.size=sizeof(config);config.shell_mode=UI_WORKSPACE_SHELL_WEB;config.max_permission=UI_ASSISTANT_PERMISSION_EDIT;
    if(runtime){root=CreateWindowW(L"STATIC",L"API6 independent actual Runtime DLL",WS_OVERLAPPEDWINDOW,0,0,1300,940,NULL,NULL,NULL,NULL);config.native_parent=root;ShowWindow(root,SW_SHOWNOACTIVATE);}else config.run_mode=UI_RUN_OFFSCREEN;
    workspace=ui_workspace_create(&config);CHECK(workspace!=NULL);if(!workspace)return 1;CHECK(ui_workspace_set_rect(workspace,&root_rect,96)==UI_STATUS_OK);
    SetEnvironmentVariableW(L"UI_API6_FAIL_CREATE",L"1");CHECK(ui_workspace_open(workspace,package,&a)==UI_STATUS_PLATFORM_ERROR&&a==0);SetEnvironmentVariableW(L"UI_API6_FAIL_CREATE",NULL);pump(50);
    SetEnvironmentVariableW(L"UI_API6_FAIL_MOUNT",L"1");CHECK(ui_workspace_open(workspace,package,&a)==UI_STATUS_PLATFORM_ERROR&&a==0);SetEnvironmentVariableW(L"UI_API6_FAIL_MOUNT",NULL);pump(100);
    puts("API6 integration: init-failure cleanup complete");CHECK(GetModuleHandleW(L"ui_api6_fixture.dll")==NULL);CHECK(ui_workspace_open(workspace,package,&a)==UI_STATUS_OK);if(!a){fprintf(stderr,"open: %s\n",ui_workspace_last_error(workspace));return 1;}
    h=host(a);form=ui_component_find(h,"form");table=ui_component_find(h,"table");tree=ui_component_find(h,"tree");dialog=ui_component_find(h,"dialog");viewport=ui_component_find(h,"viewport");CHECK(form&&table&&tree&&dialog&&viewport);
    CHECK(click(form,"field-name",0));input(form,0,0,"layout draft 中文");field.size=sizeof(field);CHECK(ui_component_get_field(form,"name",&field)==UI_STATUS_OK&&!strcmp(field.text,"layout draft 中文"));
    CHECK(click(form,"choice-mode",0));CHECK(click(form,"option-mode-2",0));CHECK(click(form,"color-color",0));CHECK(click(form,"swatch-color-1",0));
    CHECK(ui_component_set_sort(table,"name",-1)==UI_STATUS_OK);CHECK(ui_component_select_range(table,90000,5,0)==UI_STATUS_OK);pump(100);CHECK(ui_component_get_selection(table,ids,512,&n)==UI_STATUS_OK&&n==5&&ids[0]==9007199254750999ULL);
    CHECK(ui_component_set_sort(table,"name",0)==UI_STATUS_OK);CHECK(click(table,"cell-9007199254741000-name",0));input(table,40,0,NULL);input(table,13,0,NULL);input(table,'A',UI_INPUT_MODIFIER_CONTROL,NULL);input(table,0,0,"committed");input(table,13,0,NULL);
    CHECK(strstr(ui_host_status_text(h),"\"edits\":1"));input(table,113,0,NULL);input(table,27,0,NULL);CHECK(strstr(ui_host_status_text(h),"\"edits\":1"));
    CHECK(click(form,"field-name",0));p.size=sizeof(p);CHECK(query(form,"field-name",&p)==UI_STATUS_OK&&p.focused);info.size=sizeof(info);CHECK(ui_opengl_surface_get_info(h->surfaces,&info)==UI_STATUS_OK&&info.samples==4&&!ui_surface_native_handle(h->surfaces));
    puts("API6 integration: component editing complete");CHECK(ui_shell_save_layout(ui_host_get_shell(h),NULL,0,&bytes)==UI_STATUS_OK);saved=(unsigned char *)malloc(bytes);CHECK(saved!=NULL);if(!saved)return 1;CHECK(ui_shell_save_layout(ui_host_get_shell(h),saved,bytes,&bytes)==UI_STATUS_OK);
    layout.size=sizeof(layout);CHECK(ui_shell_get_panel_layout(ui_host_get_shell(h),"form",&layout)==UI_STATUS_OK);layout.floating=1;layout.floating_rect=(ui_rect_t){-10000,10000,320,650};CHECK(ui_shell_set_panel_layout(ui_host_get_shell(h),"form",&layout)==UI_STATUS_OK);pump(100);
    CHECK(query(form,"field-name",&p)==UI_STATUS_OK&&p.focused&&!strcmp(p.text_utf8,"layout draft 中文"));CHECK(ui_opengl_surface_get_info(h->surfaces,&info)==UI_STATUS_OK&&info.samples==4);
    CHECK(ui_shell_resize_splitter(ui_host_get_shell(h),UI_LAYOUT_REGION_LEFT_SIDEBAR,NULL,300)==UI_STATUS_OK);CHECK(ui_shell_restore_layout(ui_host_get_shell(h),saved,bytes)==UI_STATUS_OK);free(saved);pump(100);
    CHECK(ui_component_get_field(form,"name",&field)==UI_STATUS_OK&&!strcmp(field.text,"layout draft 中文"));CHECK(ui_component_get_selection(table,ids,512,&n)==UI_STATUS_OK&&n==1&&ids[0]==9007199254741001ULL);
    CHECK(ui_shell_begin_panel_drag(ui_host_get_shell(h),"tree")==UI_STATUS_OK);{ui_rect_t preview;ui_layout_region_t region;CHECK(ui_shell_update_panel_drag(ui_host_get_shell(h),1275,400,&preview,&region)==UI_STATUS_OK&&region==UI_LAYOUT_REGION_RIGHT_SIDEBAR);}CHECK(ui_shell_end_panel_drag(ui_host_get_shell(h),1)==UI_STATUS_OK);pump(100);
    puts("API6 integration: layout complete");CHECK(ui_host_invoke(h,"test.render","{}","test")!=0);pump(100);field.size=sizeof(field);CHECK(ui_component_get_field(viewport,"pixels",&field)==UI_STATUS_OK&&field.image_id);viewport_image=field.image_id;image.size=sizeof(image);CHECK(ui_image_get_info(h,viewport_image,&image)==UI_STATUS_OK&&image.version>=2);
    {size_t white=captured_white(viewport);printf("Actual GL C-image component capture white pixels=%zu\n",white);CHECK(white>20);}
    pixels.size=sizeof(pixels);CHECK(ui_opengl_offscreen_render(h->surfaces,&pixels)==UI_STATUS_OK);pixels.capacity=pixels.stride*pixels.height;pixels.pixels=(uint8_t *)malloc(pixels.capacity);CHECK(pixels.pixels!=NULL);CHECK(ui_opengl_offscreen_render(h->surfaces,&pixels)==UI_STATUS_OK);{size_t partial=0;FILE *frame_file=NULL;const char *path=runtime?"api6-runtime-frame.ppm":"api6-light-frame.ppm";
        for(size_t at=0;at<pixels.capacity;at+=4)if(pixels.pixels[at]>0&&pixels.pixels[at]<255)++partial;CHECK(partial>20);CHECK(!fopen_s(&frame_file,path,"wb"));if(frame_file){fprintf(frame_file,"P6\n%u %u\n255\n",pixels.width,pixels.height);for(size_t at=0;at<pixels.capacity;at+=4)fwrite(pixels.pixels+at,1,3,frame_file);CHECK(fclose(frame_file)==0);}
        printf("Actual API6 DLL %s / %s samples=%d edge_pixels=%zu frame=%s\n",info.renderer,info.version,info.samples,partial,path);}free(pixels.pixels);
    CHECK(ui_host_invoke(h,"test.late","{}","test")!=0);CHECK(strstr(ui_host_status_text(h),"\"status\":0"));CHECK(ui_host_invoke(h,"test.modal","{}","test")!=0);pump(150);{ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code=13;CHECK(ui_component_dispatch_input(form,&e)==UI_STATUS_CANCELLED);CHECK(ui_shell_reset_layout(ui_host_get_shell(h))==UI_STATUS_CANCELLED);}
    CHECK(click(dialog,"submit",0));pump(150);CHECK(ui_component_get_field(form,"name",&field)==UI_STATUS_OK&&!strcmp(field.text,"layout draft 中文"));
    puts("API6 integration: modal complete");CHECK(ui_workspace_open(workspace,package,&b)==UI_STATUS_OK&&b!=a);other=host(b);CHECK(ui_component_get_field(ui_component_find(other,"form"),"name",&field)==UI_STATUS_OK&&!strcmp(field.text,""));CHECK(ui_workspace_activate(workspace,a)==UI_STATUS_OK);pump(150);CHECK(ui_component_get_field(form,"name",&field)==UI_STATUS_OK&&!strcmp(field.text,"layout draft 中文"));close_instance(b);
    root_rect.width=800;root_rect.height=600;CHECK(ui_workspace_set_rect(workspace,&root_rect,144)==UI_STATUS_OK);pump(150);state.size=sizeof(state);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.cached_bytes<=2097152&&state.rendered_nodes<=1024);images.size=sizeof(images);CHECK(ui_image_get_stats(h,&images)==UI_STATUS_OK&&images.bytes<=33554432);
    puts("API6 integration: instances/resize complete");CHECK(ui_host_invoke(h,"test.image.release","{}","test")!=0);CHECK(ui_image_get_info(h,viewport_image,&image)==UI_STATUS_NOT_FOUND);close_instance(a);pump(2000);CHECK(ui_workspace_count(workspace)==0&&GetModuleHandleW(L"ui_api6_fixture.dll")==NULL);
    GetProcessHandleCount(GetCurrentProcess(),&handles);gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);users=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);
    for(int cycle=0;cycle<8;++cycle){printf("API6 integration: reopen %d\n",cycle);CHECK(ui_workspace_open(workspace,package,&a)==UI_STATUS_OK);if(a){h=host(a);CHECK(ui_host_invoke(h,"test.render","{}","test")!=0);close_instance(a);pump(runtime?500:20);CHECK(GetModuleHandleW(L"ui_api6_fixture.dll")==NULL);}}
    {DWORD end=0;ULONGLONG start=GetTickCount64();do{GetProcessHandleCount(GetCurrentProcess(),&end);if(end<=handles+12)break;pump(10);}while(GetTickCount64()-start<15000);printf("API6 reopen8 handles %lu->%lu GDI %lu->%lu USER %lu->%lu\n",handles,end,gdi,GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),users,GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS));CHECK(end<=handles+12&&GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=gdi+4&&GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS)<=users+4);}
    CHECK(ui_workspace_destroy(workspace)==UI_STATUS_OK);workspace=NULL;if(root)DestroyWindow(root);if(hook){CHECK(created_windows==0);UnhookWindowsHookEx(hook);printf("API6 complete offscreen lifecycle created HWNDs=%d\n",created_windows);}SetEnvironmentVariableW(L"UI_API6_OSMESA_DLL",NULL);SetEnvironmentVariableW(L"UI_API6_BACKEND",NULL);CoUninitialize();printf("API6 independent %s DLL integration: %d failures\n",runtime?"WebView2":"light offscreen",failures);return failures?1:0;
}
