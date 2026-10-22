#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <ole2.h>
#include <commctrl.h>
#include "ui_framework/light_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int failures,navigations,submits,messages;static char target[4096];static ui_document_field_t saved_fields[64];static char first_value[4096];
static unsigned downs[256],ups[256],characters,mouse_ups;
#define T(x) do{if(!(x)){fprintf(stderr,"desktop line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void pump(unsigned ms){ULONGLONG end=GetTickCount64()+ms;do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}Sleep(1);}while(GetTickCount64()<end);}
static LRESULT CALLBACK observed_input(HWND h,UINT m,WPARAM w,LPARAM l,UINT_PTR id,DWORD_PTR data)
{LRESULT result=DefSubclassProc(h,m,w,l);(void)id;(void)data;if(w<256){if(m==WM_KEYDOWN||m==WM_SYSKEYDOWN)++downs[w];if(m==WM_KEYUP||m==WM_SYSKEYUP)++ups[w];}if(m==WM_CHAR)++characters;if(m==WM_LBUTTONUP)++mouse_ups;return result;}
static void wait_change(unsigned *counter,unsigned before)
{ULONGLONG end=GetTickCount64()+3000;while(*counter==before&&GetTickCount64()<end)pump(1);T(*counter!=before);}
static void key(WORD vk,int up){INPUT input={0};unsigned *counter=up?&ups[vk]:&downs[vk],before=*counter;input.type=INPUT_KEYBOARD;input.ki.wVk=vk;input.ki.dwFlags=up?KEYEVENTF_KEYUP:0;T(SendInput(1,&input,sizeof(input))==1);wait_change(counter,before);}
static void chord(WORD vk){key(VK_CONTROL,0);key(vk,0);key(vk,1);key(VK_CONTROL,1);pump(20);}
static void text(const wchar_t *s){for(;*s;++s){INPUT in[2]={{0}};unsigned before=characters;in[0].type=in[1].type=INPUT_KEYBOARD;in[0].ki.wScan=in[1].ki.wScan=*s;in[0].ki.dwFlags=KEYEVENTF_UNICODE;in[1].ki.dwFlags=KEYEVENTF_UNICODE|KEYEVENTF_KEYUP;T(SendInput(2,in,sizeof(INPUT))==2);wait_change(&characters,before);}pump(20);}
static void click(ui_web_view_t *view,const char *id,uint32_t dpi)
{
    ui_element_presentation_t p={0};POINT point;INPUT mouse[2]={{0}};p.size=sizeof(p);T(ui_web_view_get_presentation(view,id,&p)==UI_STATUS_OK);T(p.visible);
    point=(POINT){MulDiv(p.clip.x+p.clip.width/2,(int)dpi,96),MulDiv(p.clip.y+p.clip.height/2,(int)dpi,96)};ClientToScreen((HWND)ui_web_view_native_handle(view),&point);
    T(SetCursorPos(point.x,point.y));pump(20);T(WindowFromPoint(point)==(HWND)ui_web_view_native_handle(view));
    mouse[0].type=mouse[1].type=INPUT_MOUSE;mouse[0].mi.dwFlags=MOUSEEVENTF_LEFTDOWN;mouse[1].mi.dwFlags=MOUSEEVENTF_LEFTUP;{unsigned before=mouse_ups;T(SendInput(2,mouse,sizeof(INPUT))==2);wait_change(&mouse_ups,before);}pump(30);
}
static void doc_event(ui_web_view_t *view,const ui_document_event_t *e,void *data)
{(void)view;(void)data;if(e->kind==UI_DOCUMENT_NAVIGATE){++navigations;strcpy_s(target,sizeof(target),e->url_utf8);}if(e->kind==UI_DOCUMENT_SUBMIT){++submits;T(e->field_count==2);if(e->field_count)strcpy_s(first_value,sizeof(first_value),e->fields[0].value_utf8);}}
static void chrome_message(ui_web_view_t *view,const char *json,void *data){(void)view;(void)data;printf("chrome event: %s\n",json);if(strstr(json,"go"))++messages;}
static char *read_file(const wchar_t *path){FILE *f=NULL;char *data;long size;_wfopen_s(&f,path,L"rb");if(!f)return NULL;fseek(f,0,SEEK_END);size=ftell(f);rewind(f);data=(char *)malloc((size_t)size+1);if(data){if(fread(data,1,(size_t)size,f)!=(size_t)size){free(data);data=NULL;}else data[size]=0;}fclose(f);return data;}
static void save_capture(ui_web_view_t *v,const wchar_t *path,int width,int height,uint32_t dpi)
{
    ui_pixel_buffer_t p={0};BITMAPFILEHEADER file={0};BITMAPINFOHEADER info={0};FILE *out=NULL;p.size=sizeof(p);T(ui_web_view_capture_rgba(v,width,height,dpi,&p)==UI_STATUS_OK);p.capacity=p.stride*p.height;p.pixels=(uint8_t *)malloc(p.capacity);T(p.pixels!=NULL);if(!p.pixels)return;T(ui_web_view_capture_rgba(v,width,height,dpi,&p)==UI_STATUS_OK);
    for(size_t y=0;y<p.height;++y)for(size_t x=0;x<p.width;++x){uint8_t *pixel=p.pixels+y*p.stride+x*4;uint8_t swap=pixel[0];pixel[0]=pixel[2];pixel[2]=swap;}
    file.bfType=0x4d42;file.bfOffBits=sizeof(file)+sizeof(info);file.bfSize=file.bfOffBits+(DWORD)p.capacity;info.biSize=sizeof(info);info.biWidth=(LONG)p.width;info.biHeight=-(LONG)p.height;info.biPlanes=1;info.biBitCount=32;info.biCompression=BI_RGB;
    _wfopen_s(&out,path,L"wb");if(out){fwrite(&file,sizeof(file),1,out);fwrite(&info,sizeof(info),1,out);fwrite(p.pixels,p.capacity,1,out);fclose(out);}free(p.pixels);
}
int wmain(int argc,wchar_t **argv)
{
    HWND previous=GetForegroundWindow(),root;POINT cursor;IDataObject *clipboard=NULL;HRESULT ole;ui_host_config_t hc={0};ui_light_web_config_t bc={0};ui_host_t *host;ui_web_backend_t *db,*cb;ui_web_view_t *doc,*chrome;char *html,*chrome_html;ui_document_load_t load={0};ui_document_state_t state={0};ui_rect_t dr={0,132,800,480},cr={0,0,800,132};
    (void)saved_fields;if(argc<3)return 2;setvbuf(stdout,NULL,_IONBF,0);GetCursorPos(&cursor);ole=OleInitialize(NULL);if(SUCCEEDED(ole))(void)OleGetClipboard(&clipboard);T(ui_framework_initialize()==UI_STATUS_OK);
    root=CreateWindowW(L"STATIC",L"Restricted browser desktop verification",WS_OVERLAPPEDWINDOW|WS_VISIBLE,60,60,840,680,NULL,NULL,NULL,NULL);T(root!=NULL);SetForegroundWindow(root);pump(80);T(GetForegroundWindow()==root);
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;host=ui_host_create(&hc);bc.size=sizeof(bc);bc.parent_hwnd=root;db=ui_light_document_backend_create(&bc);cb=ui_light_web_backend_create(&bc);doc=ui_web_view_create(host,db);chrome=ui_web_view_create(host,cb);T(doc&&chrome);
    SetWindowSubclass((HWND)ui_web_view_native_handle(doc),observed_input,1,0);SetWindowSubclass((HWND)ui_web_view_native_handle(chrome),observed_input,1,0);
    html=read_file(argv[1]);chrome_html=read_file(argv[2]);T(html&&chrome_html);if(!html||!chrome_html)return 1;
    T(ui_web_view_set_rect(chrome,&cr,96)==UI_STATUS_OK);T(ui_web_view_set_rect(doc,&dr,96)==UI_STATUS_OK);T(ui_web_view_set_message_callback(chrome,chrome_message,NULL)==UI_STATUS_OK);T(ui_web_view_load_html(chrome,chrome_html)==UI_STATUS_OK);
    T(ui_light_document_set_event_callback(doc,doc_event,NULL)==UI_STATUS_OK);load.size=sizeof(load);load.navigation_id=1;load.source_url="http://127.0.0.1:8765/page";load.html_utf8=html;
    {LARGE_INTEGER a,b,f;QueryPerformanceFrequency(&f);QueryPerformanceCounter(&a);T(ui_light_document_load(doc,&load)==UI_STATUS_OK);pump(100);UpdateWindow((HWND)ui_web_view_native_handle(doc));QueryPerformanceCounter(&b);state.size=sizeof(state);T(ui_light_document_get_state(doc,&state)==UI_STATUS_OK);T(state.paint_count>0);printf("visible endpoint=first completed WM_PAINT+100ms pump; elapsed_ms=%.3f parse_ms=%.3f layout_ms=%.3f paint_ms=%.3f paints=%llu\n",(double)(b.QuadPart-a.QuadPart)*1000.0/f.QuadPart,state.parse_ms,state.layout_ms,state.paint_ms,(unsigned long long)state.paint_count);}
    click(chrome,"address",96);chord('A');text(L"http://127.0.0.1:8765/page");key(VK_RETURN,0);key(VK_RETURN,1);pump(30);printf("go events=%d\n",messages);T(messages==1);
    T(ui_light_document_fragment(doc,"next")==UI_STATUS_OK);pump(30);click(doc,"next",96);T(navigations==1&&strstr(target,"/next")!=NULL);
    T(ui_light_document_fragment(doc,"query")==UI_STATUS_OK);pump(30);click(doc,"query",96);chord('A');text(L"actual GET 中文");T(ui_light_document_fragment(doc,"submit")==UI_STATUS_OK);pump(30);click(doc,"submit",96);T(submits==1&&!strcmp(first_value,"actual GET 中文"));
    T(ui_light_document_scroll(doc,0,0)==UI_STATUS_OK);click(doc,"mixed",96);key(VK_TAB,0);key(VK_TAB,1);pump(20);
    {ui_element_presentation_t p={0};p.size=sizeof(p);T(ui_web_view_get_presentation(doc,"next",&p)==UI_STATUS_OK&&p.focused);}
    T(ui_light_document_find(doc,"find-target-24",0)==UI_STATUS_OK);pump(20);chord('C');T(OpenClipboard(root));{HANDLE data=GetClipboardData(CF_UNICODETEXT);const wchar_t *s=data?(const wchar_t *)GlobalLock(data):NULL;T(s&&!wcscmp(s,L"find-target-24"));if(s)GlobalUnlock(data);}CloseClipboard();
    T(ui_light_document_scroll(doc,0,0)==UI_STATUS_OK);{ui_element_presentation_t p={0};POINT point;INPUT in={0};p.size=sizeof(p);T(ui_web_view_get_presentation(doc,"mixed",&p)==UI_STATUS_OK);point=(POINT){p.clip.x+10,p.clip.y+5};ClientToScreen((HWND)ui_web_view_native_handle(doc),&point);SetCursorPos(point.x,point.y);in.type=INPUT_MOUSE;in.mi.dwFlags=MOUSEEVENTF_WHEEL;in.mi.mouseData=(DWORD)-120;T(SendInput(1,&in,sizeof(in))==1);pump(30);state.size=sizeof(state);ui_light_document_get_state(doc,&state);T(state.scroll_y>0);}
    SetFocus((HWND)ui_web_view_native_handle(doc));chord(VK_END);state.size=sizeof(state);ui_light_document_get_state(doc,&state);printf("Ctrl+End scroll=%d,%d\n",state.scroll_x,state.scroll_y);T(state.scroll_y>0&&state.scroll_x>0);chord(VK_HOME);ui_light_document_get_state(doc,&state);T(state.scroll_y==0&&state.scroll_x==0);
    T(ui_light_document_zoom(doc,150)==UI_STATUS_OK);T(ui_light_document_zoom(doc,100)==UI_STATUS_OK);save_capture(doc,L".work/reading-96.bmp",800,480,96);
    for(uint32_t dpi=144;dpi<=192;dpi+=48){cr.width=320;dr.width=320;dr.height=180;T(ui_web_view_set_rect(chrome,&cr,dpi)==UI_STATUS_OK);T(ui_web_view_set_rect(doc,&dr,dpi)==UI_STATUS_OK);T(ui_light_document_fragment(doc,"query")==UI_STATUS_OK);pump(30);click(doc,"query",dpi);text(L" DPI");T(ui_light_document_scroll(doc,0,0)==UI_STATUS_OK);}
    T(ui_web_view_post_json(chrome,"{\"theme\":\"dark\"}")==UI_STATUS_OK);pump(30);save_capture(chrome,L".work/chrome-dark-192.bmp",320,132,192);
    ui_web_view_destroy(doc);ui_web_view_destroy(chrome);ui_light_web_backend_destroy(db);ui_light_web_backend_destroy(cb);ui_host_destroy(host);DestroyWindow(root);free(html);free(chrome_html);
    if(SUCCEEDED(ole)){(void)OleSetClipboard(clipboard);if(clipboard){(void)OleFlushClipboard();IDataObject_Release(clipboard);}OleUninitialize();}if(previous&&IsWindow(previous))SetForegroundWindow(previous);SetCursorPos(cursor.x,cursor.y);
    printf("real desktop input failures=%d; desktop restored\n",failures);return failures?1:0;
}
