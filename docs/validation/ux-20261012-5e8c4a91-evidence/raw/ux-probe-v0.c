/* Diagnostic only: real maintained host, package and native input. */
#include <windows.h>
#include <stdio.h>
#include "ui_framework/light_web.h"
#include "ui_framework/webview2.h"
static int mirror_runtime;
static ui_web_backend_t *ux_backend_create(const ui_light_web_config_t *);
static void ux_backend_destroy(ui_web_backend_t *);
#define ui_light_web_backend_create ux_backend_create
#define ui_light_web_backend_destroy ux_backend_destroy
#define wmain previous_stateful_main
#include "tests/stateful_components.c"
#undef wmain
#undef ui_light_web_backend_create
#undef ui_light_web_backend_destroy
static host_window_t *ux_host;
static const wchar_t *output;
static unsigned frame_number;
static LARGE_INTEGER frequency;
static ui_web_backend_t *ux_backend_create(const ui_light_web_config_t *cfg)
{
    if(mirror_runtime){ui_webview2_backend_config_t config={0};ui_status_t status;
        config.size=sizeof(config);config.parent_window=cfg->parent_hwnd;config.framework_components=1;
        return ui_webview2_backend_create(&config,&status);}
    return ui_light_web_backend_create(cfg);
}
static void ux_backend_destroy(ui_web_backend_t *backend)
{if(mirror_runtime)ui_webview2_backend_destroy(backend);else ui_light_web_backend_destroy(backend);}
static double clock_ms(void)
{LARGE_INTEGER now;QueryPerformanceCounter(&now);return (double)now.QuadPart*1000.0/(double)frequency.QuadPart;}
static void drain(DWORD duration)
{
    double end=clock_ms()+duration;
    do{MSG message;unsigned count=0;while(count++<3000&&PeekMessageW(&message,NULL,0,0,PM_REMOVE)){
        if(message.message!=WM_QUIT&&(!ux_host||!host_menu_key(ux_host,&message))){TranslateMessage(&message);DispatchMessageW(&message);++message_count;}}
        if(!count)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);
    }while(clock_ms()<end);
}
static ui_status_t chrome_presentation(const char *id,ui_element_presentation_t *p)
{
    double end=clock_ms()+10000;ui_status_t status;
    do{p->size=sizeof(*p);status=ui_web_view_get_presentation(ux_host->view,id,p);
        if(status!=UI_STATUS_PENDING)return status;drain(1);}while(clock_ms()<end);
    return status;
}
static POINT chrome_point(const ui_element_presentation_t *p)
{POINT point={MulDiv(p->clip.x+p->clip.width/2,(int)ux_host->dpi,96),MulDiv(p->clip.y+p->clip.height/2,(int)ux_host->dpi,96)};
 CHECK(ClientToScreen(ux_host->web_hwnd,&point));return point;}
static void real_key(WORD key,int shift)
{
    INPUT input[4]={0};unsigned count=shift?4:2;
    for(unsigned i=0;i<count;++i){input[i].type=INPUT_KEYBOARD;input[i].ki.wVk=shift&&(i==0||i==3)?VK_SHIFT:key;
        input[i].ki.dwFlags=i>=count/2?KEYEVENTF_KEYUP:0;}
    CHECK(SendInput(count,input,sizeof(INPUT))==count);
}
static void frame(const char *id,const char *stage)
{
    RECT rect;POINT cursor;DWORD pid=0;wchar_t klass[128],path[4096];ui_element_presentation_t p={0};
    HDC screen=GetDC(NULL),memory=CreateCompatibleDC(screen);BITMAPINFO bitmap_info={0};BITMAPFILEHEADER header={0};
    HBITMAP bitmap;HGDIOBJ previous;void *pixels;FILE *file=NULL;HWND hit;
    CHECK(chrome_presentation(id,&p)==UI_STATUS_OK);GetWindowRect(ux_host->hwnd,&rect);GetCursorPos(&cursor);
    hit=WindowFromPoint(cursor);GetWindowThreadProcessId(hit,&pid);GetClassNameW(hit,klass,128);
    printf("FRAME n%u qpcMs%.3f id%s stage%s rect%d,%d,%d,%d clip%d,%d,%d,%d focused%d enabled%d visible%d dpi%u windowDpi%u cursor%ld,%ld hit%p pid%lu class%ls capture%p focus%p dispatch%u\n",
        frame_number,clock_ms(),id,stage,p.rect.x,p.rect.y,p.rect.width,p.rect.height,p.clip.x,p.clip.y,p.clip.width,p.clip.height,p.focused,p.enabled,p.visible,
        ux_host->dpi,GetDpiForWindow(ux_host->hwnd),cursor.x,cursor.y,(void *)hit,pid,klass,(void *)GetCapture(),(void *)GetFocus(),message_count);
    bitmap_info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bitmap_info.bmiHeader.biWidth=rect.right-rect.left;bitmap_info.bmiHeader.biHeight=rect.top-rect.bottom;
    bitmap_info.bmiHeader.biPlanes=1;bitmap_info.bmiHeader.biBitCount=32;
    bitmap=CreateDIBSection(memory,&bitmap_info,DIB_RGB_COLORS,&pixels,NULL,0);CHECK(bitmap!=NULL);if(!bitmap){DeleteDC(memory);ReleaseDC(NULL,screen);return;}
    previous=SelectObject(memory,bitmap);CHECK(BitBlt(memory,0,0,rect.right-rect.left,rect.bottom-rect.top,screen,rect.left,rect.top,SRCCOPY|CAPTUREBLT));GdiFlush();
    header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(bitmap_info.bmiHeader);header.bfSize=header.bfOffBits+(DWORD)(rect.right-rect.left)*(DWORD)(rect.bottom-rect.top)*4;
    swprintf(path,4096,L"%s\\%03u-%hs-%hs.bmp",output,frame_number++,id,stage);CHECK(!_wfopen_s(&file,path,L"wb"));
    if(file){fwrite(&header,sizeof(header),1,file);fwrite(&bitmap_info.bmiHeader,sizeof(bitmap_info.bmiHeader),1,file);fwrite(pixels,4,(size_t)(rect.right-rect.left)*(size_t)(rect.bottom-rect.top),file);fclose(file);}
    SelectObject(memory,previous);DeleteObject(bitmap);DeleteDC(memory);ReleaseDC(NULL,screen);
}
static void button_states(const char *id)
{
    ui_element_presentation_t p={0},before={0},after={0};POINT point,outside;HWND root=ux_host->hwnd;
    CHECK(chrome_presentation(id,&p)==UI_STATUS_OK&&p.visible&&p.enabled);if(!p.visible)return;
    outside=(POINT){60,1100};CHECK(mouse(outside,0));drain(70);frame(id,"before");before=p;
    point=chrome_point(&p);CHECK(GetAncestor(WindowFromPoint(point),GA_ROOT)==root);
    CHECK(mouse(point,0));drain(80);frame(id,"hover");
    CHECK(mouse(point,MOUSEEVENTF_LEFTDOWN));drain(80);frame(id,"down");
    CHECK(mouse(outside,0));drain(40);frame(id,"held-outside");CHECK(mouse(outside,MOUSEEVENTF_LEFTUP));drain(80);frame(id,"release-outside");
    CHECK(chrome_presentation(id,&after)==UI_STATUS_OK);
    CHECK(!memcmp(&before.rect,&after.rect,sizeof(before.rect)));CHECK(GetCapture()==NULL);
    CHECK(IsWindow(root)&&!IsIconic(root)&&!IsZoomed(root));
    if(strcmp(id,"window-max")){real_key(VK_TAB,0);drain(70);real_key(VK_TAB,1);drain(70);frame(id,"keyboard");}
}
int wmain(int argc,wchar_t **argv)
{
    host_window_t host={0};ui_app_instance_info_t info={0};wchar_t provider[4096],home[4096];HWND root;
    const char *ids[]={"open","assistant-toggle","window-min","window-max","window-close","panel-form","fold-form"};
    if(argc<8)return 2;setvbuf(stdout,NULL,_IONBF,0);QueryPerformanceFrequency(&frequency);output=argv[4];mirror_runtime=!wcscmp(argv[7],L"runtime");
    CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);
    CHECK(GetFullPathNameW(argv[2],4096,provider,NULL)>0);SetEnvironmentVariableW(L"UI_API7_OSMESA_DLL",provider);
    SetEnvironmentVariableW(L"UI_API7_BACKEND",argv[3]);swprintf(home,4096,L"%s\\profiles",output);CHECK(CreateDirectoryW(home,NULL));SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_HOME",home);
    swprintf(host.recent_file,4096,L"%s\\recent.bin",home);ux_host=&host;
    printf("IDENTITY pid%lu backend%ls chrome%s dpi%ls theme%ls input=real-SendInput capture=desktop-BitBlt\n",GetCurrentProcessId(),argv[3],mirror_runtime?"Runtime-mirror":"actual-Light",argv[5],argv[6]);
    double started=clock_ms();root=create_host(&host,GetModuleHandleW(NULL));CHECK(root!=NULL);if(!root)return 2;
    host.dpi=(uint32_t)_wtoi(argv[5]);host.dark=!wcscmp(argv[6],L"dark");ShowWindow(root,SW_SHOW);
    SetWindowPos(root,HWND_TOPMOST,40,40,1200,800,0);SetForegroundWindow(root);layout(&host);drain(400);
    open_path(&host,argv[1]);drain(650);CHECK(get_instance(&host,ui_workspace_active(host.workspace),&info));
    printf("READY qpcMs%.3f startupMs%.3f hwnd%p chrome%p\n",clock_ms(),clock_ms()-started,(void *)root,(void *)host.web_hwnd);
    for(size_t i=0;i<sizeof(ids)/sizeof(ids[0]);++i)button_states(ids[i]);
    {char tab[80];snprintf(tab,80,"close-%llu",(unsigned long long)info.instance_id);button_states(tab);}
    {ui_element_presentation_t p={0};CHECK(chrome_presentation("assistant-toggle",&p)==UI_STATUS_OK);POINT point=chrome_point(&p);
        CHECK(mouse(point,MOUSEEVENTF_LEFTDOWN));drain(20);CHECK(mouse(point,MOUSEEVENTF_LEFTUP));drain(100);frame("assistant-toggle","clicked");
        CHECK(chrome_presentation("assistant-toggle",&p)==UI_STATUS_OK&&p.focused);CHECK(host.assistant_expanded);
        real_key(VK_SPACE,0);drain(100);CHECK(!host.assistant_expanded);frame("assistant-toggle","space");
        real_key(VK_RETURN,0);drain(100);CHECK(host.assistant_expanded);frame("assistant-toggle","enter");}
    SendMessageW(root,WM_CLOSE,0,0);drain(800);CHECK(!IsWindow(root));ux_host=NULL;CoUninitialize();
    printf("END dispatch%u failures%d actual-main-return-only\n",message_count,failures);return failures?1:0;
}
