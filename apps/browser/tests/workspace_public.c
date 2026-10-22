#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>
#include <commctrl.h>
#include <ole2.h>
#include "ui_framework/application.h"
#include "ui_framework/theme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clipboard_snapshot.h"

static ui_workspace_t *workspace;static int failures;static char latest[49152];
static unsigned key_down[256],key_up[256],chars;
static LRESULT CALLBACK observed(HWND h,UINT m,WPARAM w,LPARAM l,UINT_PTR id,DWORD_PTR data)
{LRESULT result=DefSubclassProc(h,m,w,l);(void)id;(void)data;if(w<256){if(m==WM_KEYDOWN||m==WM_SYSKEYDOWN)++key_down[w];if(m==WM_KEYUP||m==WM_SYSKEYUP)++key_up[w];}if(m==WM_CHAR)++chars;return result;}
static BOOL CALLBACK observe_child(HWND h,LPARAM data)
{HWND *first=(HWND *)data;if(!GetWindow(h,GW_CHILD)){SetWindowSubclass(h,observed,1,0);if(!*first&&IsWindowVisible(h))*first=h;}return TRUE;}
static LONG WINAPI crash(EXCEPTION_POINTERS *exception)
{
    HANDLE process=GetCurrentProcess();CONTEXT context=*exception->ContextRecord;STACKFRAME64 frame={0};
    frame.AddrPC.Offset=context.Rip;frame.AddrPC.Mode=AddrModeFlat;frame.AddrStack.Offset=context.Rsp;frame.AddrStack.Mode=AddrModeFlat;frame.AddrFrame.Offset=context.Rbp;frame.AddrFrame.Mode=AddrModeFlat;
    SymSetOptions(SYMOPT_LOAD_LINES|SYMOPT_UNDNAME);SymInitialize(process,NULL,TRUE);
    fprintf(stderr,"exception %08lx at %llx\n",exception->ExceptionRecord->ExceptionCode,(unsigned long long)context.Rip);
    for(int i=0;i<24;++i){char symbol_buffer[sizeof(SYMBOL_INFO)+256];SYMBOL_INFO *symbol=(SYMBOL_INFO *)symbol_buffer;DWORD64 offset=0;IMAGEHLP_LINE64 line={0};DWORD delta=0;
        memset(symbol_buffer,0,sizeof(symbol_buffer));symbol->SizeOfStruct=sizeof(SYMBOL_INFO);symbol->MaxNameLen=255;line.SizeOfStruct=sizeof(line);
        if(SymFromAddr(process,frame.AddrPC.Offset,&offset,symbol))fprintf(stderr,"  %s+%llu",symbol->Name,(unsigned long long)offset);else fprintf(stderr,"  %llx",(unsigned long long)frame.AddrPC.Offset);
        if(SymGetLineFromAddr64(process,frame.AddrPC.Offset,&delta,&line))fprintf(stderr," %s:%lu",line.FileName,line.LineNumber);fputc('\n',stderr);
        if(!StackWalk64(IMAGE_FILE_MACHINE_AMD64,process,GetCurrentThread(),&frame,&context,NULL,SymFunctionTableAccess64,SymGetModuleBase64,NULL))break;
    }return EXCEPTION_EXECUTE_HANDLER;
}
#define T(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
static wchar_t profile_base[MAX_PATH],old_local[32768];
static HWND input_root;static int input_aborted;static DWORD clipboard_written_sequence;
static int foreground(HWND root)
{HWND current=GetForegroundWindow();DWORD other=current?GetWindowThreadProcessId(current,NULL):0,self=GetCurrentThreadId();int attached=other&&other!=self&&AttachThreadInput(self,other,TRUE);SetForegroundWindow(root);SetActiveWindow(root);if(attached)AttachThreadInput(self,other,FALSE);return GetForegroundWindow()==root;}
static int input_allowed(void)
{if(input_aborted)return 0;if(input_root&&GetForegroundWindow()!=input_root){DWORD owner=0;GetWindowThreadProcessId(GetForegroundWindow(),&owner);input_aborted=1;fprintf(stderr,"foreground loss: owner_pid=%lu test_pid=%lu; all further input stopped\n",(unsigned long)owner,(unsigned long)GetCurrentProcessId());T(0&&"Foreground ownership lost: no input sent");return 0;}return 1;}
static void profile_begin(void)
{wchar_t path[MAX_PATH];GetEnvironmentVariableW(L"LOCALAPPDATA",old_local,32768);T(GetTempPathW(MAX_PATH,path));T(GetTempFileNameW(path,L"ubw",0,profile_base));T(DeleteFileW(profile_base));T(CreateDirectoryW(profile_base,NULL));T(SetEnvironmentVariableW(L"LOCALAPPDATA",profile_base));}
static void profile_end(void)
{wchar_t path[32768];swprintf(path,32768,L"%s\\ui-framework\\browser\\origins-v1.txt",profile_base);if(GetFileAttributesW(path)!=INVALID_FILE_ATTRIBUTES)T(DeleteFileW(path));swprintf(path,32768,L"%s\\ui-framework\\browser",profile_base);if(GetFileAttributesW(path)!=INVALID_FILE_ATTRIBUTES)T(RemoveDirectoryW(path));swprintf(path,32768,L"%s\\ui-framework",profile_base);if(GetFileAttributesW(path)!=INVALID_FILE_ATTRIBUTES)T(RemoveDirectoryW(path));T(RemoveDirectoryW(profile_base));T(SetEnvironmentVariableW(L"LOCALAPPDATA",*old_local?old_local:NULL));}
static void save_native(HWND root,const wchar_t *path)
{RECT r;BITMAPINFO bitmap={0};BITMAPFILEHEADER header={0};HDC source,memory;HBITMAP image;HGDIOBJ previous;void *pixels=NULL;FILE *file=NULL;GetClientRect(root,&r);bitmap.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bitmap.bmiHeader.biWidth=r.right;bitmap.bmiHeader.biHeight=-r.bottom;bitmap.bmiHeader.biPlanes=1;bitmap.bmiHeader.biBitCount=32;bitmap.bmiHeader.biCompression=BI_RGB;
 source=GetDC(root);memory=CreateCompatibleDC(source);image=CreateDIBSection(source,&bitmap,DIB_RGB_COLORS,&pixels,NULL,0);T(image&&pixels);if(image&&pixels){previous=SelectObject(memory,image);T(BitBlt(memory,0,0,r.right,r.bottom,source,0,0,SRCCOPY));header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(BITMAPINFOHEADER);header.bfSize=header.bfOffBits+(DWORD)(r.right*r.bottom*4);T(!_wfopen_s(&file,path,L"wb"));if(file){fwrite(&header,sizeof(header),1,file);fwrite(&bitmap.bmiHeader,sizeof(BITMAPINFOHEADER),1,file);fwrite(pixels,(size_t)r.right*r.bottom*4,1,file);fclose(file);}SelectObject(memory,previous);DeleteObject(image);}DeleteDC(memory);ReleaseDC(root,source);}
static LRESULT CALLBACK window_proc(HWND h,UINT m,WPARAM w,LPARAM l)
{intptr_t result=0;if(workspace&&ui_workspace_handle_message(workspace,h,m,w,l,&result)==UI_STATUS_OK)return (LRESULT)result;return DefWindowProcW(h,m,w,l);}
static void pump(unsigned ms)
{ULONGLONG end=GetTickCount64()+ms;do{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}if(workspace)(void)ui_workspace_flush(workspace,64);Sleep(1);}while(GetTickCount64()<end);}
static void send_key(WORD vk,int up)
{INPUT in={0};unsigned *counter=up?key_up+vk:key_down+vk,before=*counter;ULONGLONG end=GetTickCount64()+3000;if(!input_allowed())return;in.type=INPUT_KEYBOARD;in.ki.wVk=vk;in.ki.dwFlags=up?KEYEVENTF_KEYUP:0;T(SendInput(1,&in,sizeof(in))==1);
 /* Windows menu handling may consume a standalone Alt transition. Verify its
    physical state, then verify the actual back/forward command result below. */
 if(vk==VK_MENU){while(((GetAsyncKeyState(vk)&0x8000)!=0)==(up!=0)&&GetTickCount64()<end)pump(1);T(((GetAsyncKeyState(vk)&0x8000)!=0)==!up);pump(10);return;}
 while(*counter==before&&GetTickCount64()<end)pump(1);if(*counter==before)fprintf(stderr,"native input missing vk=%u up=%d focus=%p\n",vk,up,(void *)GetFocus());T(*counter!=before);}
static void shortcut_key(WORD modifier,WORD key){send_key(modifier,0);send_key(key,0);send_key(key,1);send_key(modifier,1);pump(20);if(!input_aborted&&modifier==VK_CONTROL&&key=='C')clipboard_written_sequence=GetClipboardSequenceNumber();}
static void toolbar_click(HWND chrome,int offset,unsigned dpi)
{RECT r;POINT point;INPUT mouse[2]={{0}};if(!input_allowed())return;GetClientRect(chrome,&r);point=(POINT){r.right-MulDiv(offset,(int)dpi,96),MulDiv(85,(int)dpi,96)};ClientToScreen(chrome,&point);SetCursorPos(point.x,point.y);pump(20);if(!input_allowed())return;if(!IsChild(chrome,WindowFromPoint(point))&&WindowFromPoint(point)!=chrome){T(0&&"Click target is not the owned test view");return;}mouse[0].type=mouse[1].type=INPUT_MOUSE;mouse[0].mi.dwFlags=MOUSEEVENTF_LEFTDOWN;mouse[1].mi.dwFlags=MOUSEEVENTF_LEFTUP;T(SendInput(2,mouse,sizeof(INPUT))==2);pump(80);}
static void type_text(const wchar_t *text)
{for(;*text;++text){INPUT in[2]={{0}};unsigned before=chars;ULONGLONG end=GetTickCount64()+3000;if(!input_allowed())return;in[0].type=in[1].type=INPUT_KEYBOARD;in[0].ki.wScan=in[1].ki.wScan=*text;in[0].ki.dwFlags=KEYEVENTF_UNICODE;in[1].ki.dwFlags=KEYEVENTF_UNICODE|KEYEVENTF_KEYUP;T(SendInput(2,in,sizeof(INPUT))==2);while(chars==before&&GetTickCount64()<end)pump(1);T(chars!=before);}}
static const char *state(uint64_t id)
{ui_app_instance_info_t info={0};const char *json="{}";info.size=sizeof(info);if(ui_workspace_get_instance(workspace,id,&info)==UI_STATUS_OK)(void)ui_assistant_get_state_snapshot(info.assistant,&json);strcpy_s(latest,sizeof(latest),json);return latest;}
static int wait_ready(uint64_t id,unsigned timeout)
{ULONGLONG end=GetTickCount64()+timeout;do{pump(10);state(id);if(strstr(latest,"\"loading\":0")&&strstr(latest,"\"idle\":1")&&(strstr(latest,"\"pending\":0")||strstr(latest,"\"error\":1")))return 1;}while(GetTickCount64()<end);fprintf(stderr,"wait_ready: %s\n",state(id));return 0;}
static void invoke(uint64_t id,const char *cmd,const char *params)
{uint64_t request=0;T(ui_workspace_invoke(workspace,id,cmd,params?params:"{}",&request)==UI_STATUS_OK);T(request!=0);pump(10);}
static void go(uint64_t id,const char *path)
{char params[512];snprintf(params,sizeof(params),"{\"url\":\"http://127.0.0.1:8765%s\"}",path);invoke(id,"browser.navigate",params);T(wait_ready(id,8000));printf("%s %s\n",path,state(id));}
static long native_number(const char *name)
{char key[96];const char *at;snprintf(key,sizeof(key),"\"%s\":",name);at=strstr(latest,key);T(at!=NULL);return at?strtol(at+strlen(key),NULL,10):-1;}
static void m2_desktop(HWND root,uint64_t id,ui_app_instance_info_t *info)
{
    (void)root;
    shortcut_key(VK_CONTROL,'L');type_text(L"http://127.0.0.1:8765/m2-reading");send_key(VK_RETURN,0);send_key(VK_RETURN,1);if(input_aborted)return;T(wait_ready(id,8000));pump(80);T(strstr(state(id),"M2 阅读兼容固定集")!=NULL);
    shortcut_key(VK_CONTROL,'F');shortcut_key(VK_CONTROL,'A');type_text(L"12. 编号十二");send_key(VK_RETURN,0);send_key(VK_RETURN,1);if(input_aborted)return;T(strstr(state(id),"已定位")!=NULL);shortcut_key(VK_CONTROL,'C');
    T(OpenClipboard(root));{HANDLE data=GetClipboardData(CF_UNICODETEXT);const wchar_t *text=data?GlobalLock(data):NULL;T(text&&!wcscmp(text,L"12. 编号十二"));if(text)GlobalUnlock(data);}CloseClipboard();
    shortcut_key(VK_CONTROL,VK_HOME);if(input_aborted)return;
    {POINT at={100,220};INPUT wheel={0};HWND view=GetFocus();ClientToScreen((HWND)info->native_container,&at);SetCursorPos(at.x,at.y);pump(20);if(!input_allowed()||WindowFromPoint(at)!=view){T(0&&"Wheel target is not the owned document");return;}wheel.type=INPUT_MOUSE;wheel.mi.dwFlags=MOUSEEVENTF_WHEEL;wheel.mi.mouseData=(DWORD)-WHEEL_DELTA;T(SendInput(1,&wheel,sizeof(wheel))==1);pump(50);T(native_number("scroll_y")>=0);state(id);T(native_number("scroll_y")>0);}
    send_key(VK_END,0);send_key(VK_END,1);state(id);T(native_number("scroll_x")>0);send_key(VK_HOME,0);send_key(VK_HOME,1);state(id);T(native_number("scroll_x")==0);
    shortcut_key(VK_CONTROL,VK_END);state(id);T(native_number("scroll_y")>0);shortcut_key(VK_CONTROL,VK_HOME);
    shortcut_key(VK_CONTROL,'F');shortcut_key(VK_CONTROL,'A');type_text(L"收入 / Revenue");send_key(VK_RETURN,0);send_key(VK_RETURN,1);if(input_aborted)return;T(strstr(state(id),"已定位")!=NULL);pump(80);save_native((HWND)info->native_container,L".work/m2/merged-table-96.bmp");
    printf("M2_VISIBLE_ENDPOINT %s\n",state(id));T(native_number("paint_count")>0);
    T(ui_host_set_theme(info->host,UI_THEME_DARK)==UI_STATUS_OK);
    for(unsigned dpi=96;dpi<=192;dpi+=48){ui_rect_t narrow={0,0,320,300};T(ui_workspace_set_rect(workspace,&narrow,dpi)==UI_STATUS_OK);pump(80);if(dpi==192)save_native((HWND)info->native_container,L".work/m2/reading-dark-192.bmp");}
    {ui_rect_t normal={0,0,860,640};T(ui_workspace_set_rect(workspace,&normal,96)==UI_STATUS_OK);}T(ui_host_set_theme(info->host,UI_THEME_LIGHT)==UI_STATUS_OK);
    shortcut_key(VK_CONTROL,VK_HOME);send_key(VK_TAB,0);send_key(VK_TAB,1);shortcut_key(VK_CONTROL,'A');type_text(L"中文 & + English");send_key(VK_TAB,0);send_key(VK_TAB,1);send_key(VK_RETURN,0);send_key(VK_RETURN,1);if(input_aborted)return;T(wait_ready(id,8000));T(strstr(state(id),"M2 GET Passed")!=NULL);
    shortcut_key(VK_MENU,VK_LEFT);T(wait_ready(id,8000));T(strstr(state(id),"M2 阅读兼容固定集")!=NULL);shortcut_key(VK_MENU,VK_RIGHT);T(wait_ready(id,8000));T(strstr(state(id),"M2 GET Passed")!=NULL);
    printf("M2 actual reading/select-copy/wheel/Home-End/Tab-GET/Alt/DPI failures=%d\n",failures);
}
static void desktop(HWND root,uint64_t id)
{
    HWND previous=GetForegroundWindow(),first=NULL;POINT cursor;clipboard_snapshot saved={0};ui_app_instance_info_t info={0};GetCursorPos(&cursor);if(!clipboard_capture(root,&saved)){T(0&&"Cannot preserve clipboard: no input sent");return;}
    T(ui_workspace_activate(workspace,id)==UI_STATUS_OK);ShowWindow(root,SW_SHOW);input_root=root;input_aborted=0;clipboard_written_sequence=0;if(!foreground(root)){T(0&&"Cannot acquire foreground; desktop test aborted before SendInput");ShowWindow(root,SW_HIDE);clipboard_discard(&saved);input_root=NULL;return;}pump(50);info.size=sizeof(info);T(ui_workspace_get_instance(workspace,id,&info)==UI_STATUS_OK);EnumChildWindows((HWND)info.native_container,observe_child,(LPARAM)&first);SetWindowSubclass(root,observed,1,0);T(first!=NULL);SetFocus(first);
    shortcut_key(VK_CONTROL,'L');{POINT caret;HWND edit=GetFocus();T(GetCaretPos(&caret));caret.y+=4;ClientToScreen(edit,&caret);SetCursorPos(caret.x,caret.y);pump(10);if(!input_allowed()||WindowFromPoint(caret)!=edit){T(0&&"Click target is not the owned edit");goto restore_desktop;}INPUT mouse[2]={{0}};mouse[0].type=mouse[1].type=INPUT_MOUSE;mouse[0].mi.dwFlags=MOUSEEVENTF_LEFTDOWN;mouse[1].mi.dwFlags=MOUSEEVENTF_LEFTUP;T(SendInput(2,mouse,sizeof(INPUT))==2);pump(30);}
    shortcut_key(VK_CONTROL,'A');type_text(L"http://127.0.0.1:8765/page");send_key(VK_RETURN,0);send_key(VK_RETURN,1);T(wait_ready(id,8000));T(strstr(state(id),"固定测试页面")!=NULL);
    shortcut_key(VK_CONTROL,'F');shortcut_key(VK_CONTROL,'A');type_text(L"find-target-24");send_key(VK_RETURN,0);send_key(VK_RETURN,1);pump(30);T(strstr(state(id),"已定位")!=NULL);
    shortcut_key(VK_CONTROL,'C');T(OpenClipboard(root));{HANDLE data=GetClipboardData(CF_UNICODETEXT);const wchar_t *value=data?(const wchar_t *)GlobalLock(data):NULL;T(value&&!wcscmp(value,L"find-target-24"));if(value)GlobalUnlock(data);}CloseClipboard();
    shortcut_key(VK_CONTROL,VK_OEM_PLUS);T(strstr(state(id),"\"zoom\":110")!=NULL);shortcut_key(VK_CONTROL,VK_OEM_MINUS);shortcut_key(VK_CONTROL,'0');T(strstr(state(id),"\"zoom\":100")!=NULL);
    shortcut_key(VK_CONTROL,'L');type_text(L"http://127.0.0.1:8765/next");send_key(VK_RETURN,0);send_key(VK_RETURN,1);T(wait_ready(id,8000));T(strstr(state(id),"Next 第二页")!=NULL);
    shortcut_key(VK_MENU,VK_LEFT);T(wait_ready(id,8000));T(strstr(state(id),"固定测试页面")!=NULL);shortcut_key(VK_MENU,VK_RIGHT);T(wait_ready(id,8000));if(!strstr(state(id),"Next 第二页"))fprintf(stderr,"after actual forward: %s\n",state(id));T(strstr(latest,"Next 第二页")!=NULL);
    send_key(VK_F5,0);send_key(VK_F5,1);T(wait_ready(id,8000));shortcut_key(VK_CONTROL,'R');T(wait_ready(id,8000));
    shortcut_key(VK_CONTROL,'L');type_text(L"http://127.0.0.1:8765/slow");send_key(VK_RETURN,0);send_key(VK_RETURN,1);pump(30);send_key(VK_ESCAPE,0);send_key(VK_ESCAPE,1);pump(100);T(strstr(state(id),"\"idle\":1")!=NULL);
    m2_desktop(root,id,&info);if(input_aborted)goto restore_desktop;
    {HWND chrome=(HWND)info.native_container;toolbar_click(chrome,55,96);T(strstr(state(id),"\"panel\":1")!=NULL);save_native((HWND)info.native_container,L".work/m2/settings-96.bmp");
     shortcut_key(VK_CONTROL,'A');type_text(L"http://127.0.0.1:8765");send_key(VK_RETURN,0);send_key(VK_RETURN,1);type_text(L"http://localhost:8765");send_key(VK_TAB,0);send_key(VK_TAB,1);send_key(VK_RETURN,0);send_key(VK_RETURN,1);pump(700);T(strstr(state(id),"\"policy_count\":2")!=NULL);
     send_key(VK_TAB,0);send_key(VK_TAB,1);send_key(VK_RETURN,0);send_key(VK_RETURN,1);pump(700);T(strstr(state(id),"\"policy_count\":3")!=NULL);
     shortcut_key(VK_CONTROL,'L');type_text(L"http://127.0.0.1:8765/diag-resources");send_key(VK_RETURN,0);send_key(VK_RETURN,1);T(wait_ready(id,8000));toolbar_click(chrome,20,96);T(strstr(state(id),"\"panel\":2")!=NULL);
     shortcut_key(VK_CONTROL,'A');shortcut_key(VK_CONTROL,'C');T(OpenClipboard(root));{HANDLE value=GetClipboardData(CF_UNICODETEXT);const wchar_t *text=value?(const wchar_t *)GlobalLock(value):NULL;T(text&&wcsstr(text,L"http")&&!wcsstr(text,L"SuperSecret"));if(text)GlobalUnlock(value);}CloseClipboard();
     T(ui_host_set_theme(info.host,UI_THEME_DARK)==UI_STATUS_OK);
     for(unsigned dpi=96;dpi<=192;dpi+=48){ui_rect_t narrow={0,0,320,300};T(ui_workspace_set_rect(workspace,&narrow,dpi)==UI_STATUS_OK);pump(80);if(dpi==192)save_native((HWND)info.native_container,L".work/m2/diagnostics-dark-192.bmp");}
     send_key(VK_TAB,0);send_key(VK_TAB,1);send_key(VK_RETURN,0);send_key(VK_RETURN,1);pump(50);T(strstr(state(id),"\"panel\":0")!=NULL);{ui_rect_t normal={0,0,860,640};T(ui_workspace_set_rect(workspace,&normal,96)==UI_STATUS_OK);}T(ui_host_set_theme(info.host,UI_THEME_LIGHT)==UI_STATUS_OK);
     printf("M01 actual settings save/reset, diagnostic copy, Tab exit, narrow DPI96/144/192 failures=%d\n",failures);}
restore_desktop:
    printf("desktop restore: hide\n");ShowWindow(root,SW_HIDE);if(clipboard_written_sequence&&GetClipboardSequenceNumber()==clipboard_written_sequence)T(clipboard_restore(root,&saved));else{if(clipboard_written_sequence)printf("clipboard changed outside test; current clipboard preserved\n");clipboard_discard(&saved);}printf("desktop restore: foreground/cursor\n");if(!input_aborted){if(previous&&IsWindow(previous))SetForegroundWindow(previous);SetCursorPos(cursor.x,cursor.y);}input_root=NULL;printf("official uapp real shortcuts/click/copy complete; desktop restored\n");
}
int wmain(int argc,wchar_t **argv)
{
    WNDCLASSW wc={0};HWND root;ui_workspace_config_t cfg={0};ui_rect_t rect={0,0,860,640};uint64_t first=0,second=0;char path[32768],title[4096];size_t required;ui_app_instance_info_t info={0};
    SetUnhandledExceptionFilter(crash);setvbuf(stdout,NULL,_IONBF,0);if(argc<2)return 2;profile_begin();T(ui_framework_initialize()==UI_STATUS_OK);wc.lpfnWndProc=window_proc;wc.hInstance=GetModuleHandleW(NULL);wc.lpszClassName=L"BrowserPublicWorkspaceCheck";RegisterClassW(&wc);
    root=CreateWindowW(wc.lpszClassName,L"Browser verification",WS_OVERLAPPEDWINDOW,100,100,900,700,NULL,NULL,wc.hInstance,NULL);T(root!=NULL);
    cfg.size=sizeof(cfg);cfg.native_parent=root;cfg.shell_mode=UI_WORKSPACE_SHELL_WEB;workspace=ui_workspace_create(&cfg);T(workspace!=NULL);T(ui_workspace_set_rect(workspace,&rect,96)==UI_STATUS_OK);
    T(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,path,sizeof(path),NULL,NULL)>0);
    T(ui_workspace_open(workspace,path,&first)==UI_STATUS_OK);
    if(!first){fprintf(stderr,"open failed: %s\n",ui_workspace_last_error(workspace));ui_workspace_destroy(workspace);DestroyWindow(root);profile_end();return 1;}
    T(ui_workspace_open(workspace,path,&second)==UI_STATUS_OK);T(first!=second);pump(50);
    T(strstr(state(first),"browser:start")!=NULL);T(strstr(latest,"\"images\":0")!=NULL);T(strstr(state(second),"browser:start")!=NULL);
    if(argc>2&&!wcscmp(argv[2],L"--desktop"))desktop(root,first);
    if(argc>2&&!wcscmp(argv[2],L"--https")){invoke(first,"browser.navigate","{\"url\":\"https://example.com/\"}");T(wait_ready(first,8000));printf("SUPPLEMENTAL_TLS %s\n",state(first));T(strstr(latest,"\"error\":0")!=NULL);}
    go(first,"/page");T(strstr(latest,"固定测试页面")!=NULL);T(strstr(latest,"\"error\":0")!=NULL);T(strstr(latest,"\"script_runtime\":0")!=NULL);T(strstr(latest,"\"images\":1")!=NULL);
    T(ui_workspace_get_instance_title(workspace,first,title,sizeof(title),&required)==UI_STATUS_OK);T(strstr(title,"固定测试")!=NULL);T(strstr(state(second),"browser:start")!=NULL);
    go(first,"/redirect");T(strstr(latest,"/page")!=NULL);
    go(first,"/formats");T(strstr(latest,"\"images\":4")!=NULL);T(strstr(latest,"\"failed_resources\":0")!=NULL);T(strstr(latest,"\"restrictions\":1")!=NULL);
    go(first,"/long-title");T(ui_workspace_get_instance_title(workspace,first,title,sizeof(title),&required)==UI_STATUS_OK);T(strlen(title)>4080&&required==strlen(title)+1);
    {char url[3500],params[3600];strcpy(url,"http://127.0.0.1:8765/next?long=");size_t prefix=strlen(url);memset(url+prefix,'a',3000);url[prefix+3000]=0;snprintf(params,sizeof(params),"{\"url\":\"%s\"}",url);invoke(first,"browser.navigate",params);T(wait_ready(first,8000));T(strstr(state(first),"http://127.0.0.1:8765/next")!=NULL);T(strstr(latest,"?long=")==NULL);}
    go(first,"/404");T(strstr(latest,"\"error\":1")!=NULL);go(first,"/500");T(strstr(latest,"\"error\":1")!=NULL);
    go(first,"/disconnect");T(strstr(latest,"\"error\":1")!=NULL);
    go(first,"/invalid-encoding");T(strstr(latest,"\"error\":1")!=NULL);
    go(first,"/font-limit");T(strstr(latest,"\"error\":1")!=NULL);
    go(first,"/gzip");T(strstr(latest,"Gzip 页面")!=NULL);T(strstr(latest,"\"error\":0")!=NULL);
    go(first,"/cross");T(strstr(latest,"\"error\":1")!=NULL);go(first,"/protocol");T(strstr(latest,"\"error\":1")!=NULL);go(first,"/loop");T(strstr(latest,"\"error\":1")!=NULL);
    go(first,"/big");T(strstr(latest,"\"error\":1")!=NULL);go(first,"/nodes");T(strstr(latest,"\"error\":1")!=NULL);go(first,"/longtext");T(strstr(latest,"\"error\":1")!=NULL);go(first,"/deep");T(strstr(latest,"\"error\":1")!=NULL);
    go(first,"/image-limit");T(strstr(latest,"\"failed_resources\":1")!=NULL);go(first,"/resource-limit");T(strstr(latest,"\"error\":1")!=NULL);
    go(first,"/gbk");T(strstr(latest,"编码页面")!=NULL);T(strstr(latest,"\"error\":0")!=NULL);
    go(first,"/utf16");T(strstr(latest,"UTF16 页面")!=NULL);T(strstr(latest,"\"error\":0")!=NULL);
    invoke(first,"browser.navigate","{\"url\":\"https://localhost:8766/page\"}");T(wait_ready(first,8000));
    if(argc>2&&!wcscmp(argv[2],L"--trusted-local")){T(strstr(state(first),"\"error\":0")!=NULL);T(strstr(latest,"固定测试页面")!=NULL);T(strstr(latest,"\"images\":1")!=NULL);}
    else T(strstr(state(first),"\"error\":1")!=NULL);
    printf("TLS %s\n",latest);
    invoke(first,"browser.navigate","{\"url\":\"http://127.0.0.1:1/offline\"}");T(strstr(state(first),"\"error\":1")!=NULL);
    invoke(first,"browser.navigate","{\"url\":\"http://127.0.0.1:8765/slow\"}");pump(50);go(first,"/next");pump(2200);T(strstr(state(first),"Next 第二页")!=NULL);
    invoke(first,"browser.navigate","{\"url\":\"http://127.0.0.1:8765/slow\"}");pump(30);invoke(first,"browser.stop",NULL);pump(100);T(strstr(state(first),"\"idle\":1")!=NULL);
    go(first,"/page");invoke(first,"browser.find-next","{\"text\":\"find-target-24\"}");T(strstr(state(first),"已定位")!=NULL);
    invoke(first,"browser.plus",NULL);T(strstr(state(first),"\"zoom\":110")!=NULL);invoke(first,"browser.reset",NULL);T(strstr(state(first),"\"zoom\":100")!=NULL);
    info.size=sizeof(info);T(ui_workspace_get_instance(workspace,first,&info)==UI_STATUS_OK);T(ui_host_set_theme(info.host,UI_THEME_DARK)==UI_STATUS_OK);T(ui_host_set_theme(info.host,UI_THEME_LIGHT)==UI_STATUS_OK);
    for(unsigned dpi=96;dpi<=192;dpi+=48){rect.width=320;rect.height=300;T(ui_workspace_set_rect(workspace,&rect,dpi)==UI_STATUS_OK);pump(30);T(strstr(state(first),"\"error\":0")!=NULL);}rect.width=860;rect.height=640;T(ui_workspace_set_rect(workspace,&rect,96)==UI_STATUS_OK);
    {PROCESS_MEMORY_COUNTERS_EX before={0},after={0};before.cb=sizeof(before);after.cb=sizeof(after);GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS *)&before,sizeof(before));
     for(int i=0;i<16;++i){go(first,"/page");go(first,"/next");}GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS *)&after,sizeof(after));
     printf("memory private_before=%zu private_after=%zu delta=%lld\n",before.PrivateUsage,after.PrivateUsage,(long long)after.PrivateUsage-(long long)before.PrivateUsage);
     T(after.PrivateUsage<=before.PrivateUsage+32u*1024u*1024u);T(after.PrivateUsage<=128u*1024u*1024u);}
    invoke(first,"browser.navigate","{\"url\":\"http://127.0.0.1:8765/slow-headers\"}");pump(20);
    T(ui_workspace_close(workspace,first,UI_APP_CLOSE_TAB)==UI_STATUS_OK);T(ui_workspace_close(workspace,second,UI_APP_CLOSE_TAB)==UI_STATUS_OK);
    {ULONGLONG end=GetTickCount64()+10000;while(ui_workspace_count(workspace)&&GetTickCount64()<end)pump(10);}
    T(ui_workspace_count(workspace)==0);T(GetModuleHandleW(L"browser_app.dll")==NULL);
    {uint64_t decoding_id=0;int observed_decode=0;T(ui_workspace_open(workspace,path,&decoding_id)==UI_STATUS_OK);
        invoke(decoding_id,"browser.navigate","{\"url\":\"http://127.0.0.1:8765/decode-page\"}");ULONGLONG end=GetTickCount64()+8000;
        while(GetTickCount64()<end){MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}ui_workspace_flush(workspace,64);
            if(strstr(state(decoding_id),"\"decoding\":1")){observed_decode=1;break;}SwitchToThread();}
        if(!observed_decode)fprintf(stderr,"decode observation end: %s\n",state(decoding_id));
        T(observed_decode);T(ui_workspace_close(workspace,decoding_id,UI_APP_CLOSE_TAB)==UI_STATUS_OK);end=GetTickCount64()+10000;while(ui_workspace_count(workspace)&&GetTickCount64()<end)pump(1);
        T(!ui_workspace_count(workspace));T(GetModuleHandleW(L"browser_app.dll")==NULL);printf("close during observed WIC decode=%d; DLL unloaded=%d\n",observed_decode,GetModuleHandleW(L"browser_app.dll")==NULL);}
    T(ui_workspace_destroy(workspace)==UI_STATUS_OK);workspace=NULL;DestroyWindow(root);
    profile_end();printf("actual DLL unload=%s; public workspace failures=%d\n",GetModuleHandleW(L"browser_app.dll")==NULL?"yes":"no",failures);return failures?1:0;
}
