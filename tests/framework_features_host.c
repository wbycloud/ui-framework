/* Real host chrome + common popup + dynamically loaded API4 sample. */
#define UI_HOST_TEST 1
#include "../examples/framework_host/main.c"
static int failures,group_count,forbidden;
static void trace_step(int line,const char *text)
{if(GetEnvironmentVariableW(L"UI_FEATURE_TRACE",NULL,0)){fprintf(stderr,"FEATURE_TRACE %llu line%d: %s\n",(unsigned long long)GetTickCount64(),line,text);fflush(stderr);}}
#define CHECK(x) do{trace_step(__LINE__,#x);if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;}}while(0)
/* A live rendering queue need not become empty. Check bounds before removal so
 * every retrieved message is dispatched, including when a paint stays pending. */
static void pump(void){MSG m;unsigned n=0,timers=0,paints=0;ULONGLONG start=GetTickCount64();while(n<3000&&GetTickCount64()-start<100&&PeekMessageW(&m,NULL,0,0,PM_REMOVE)){++n;if(m.message!=WM_QUIT){timers+=m.message==WM_TIMER;paints+=m.message==WM_PAINT;TranslateMessage(&m);DispatchMessageW(&m);}}if(GetEnvironmentVariableW(L"UI_FEATURE_TRACE",NULL,0)){fprintf(stderr,"FEATURE_PUMP %llu ms messages%u timers%u paints%u\n",(unsigned long long)(GetTickCount64()-start),n,timers,paints);fflush(stderr);}}
static int click(ui_web_view_t *v,const char *id)
{ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);if(ui_web_view_get_presentation(v,id,&p)!=UI_STATUS_OK||!p.visible||!p.enabled)return 0;
 e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.x=p.clip.x+3;e.y=p.clip.y+3;e.pointer_button=1;if(ui_web_view_dispatch_input(v,&e)!=UI_STATUS_OK)return 0;
 e.kind=UI_INPUT_POINTER_UP;return ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK;}
static void group(const ui_menu_model_entry_t *entry,void *data)
{host_window_t *host=(host_window_t *)data;menu_state_writer_t writer={0};json_buffer_t json={0};char id[80];const char *key;size_t n;ui_element_presentation_t p={0};
 const char *names[]={"File","Edit","View","Tools","Cell","Layer","Help"};CHECK(group_count<7);if(group_count>=7)return;
 CHECK(!strcmp(entry->path,names[group_count++]));writer.json=&json;state_menu_group(entry,&writer);key=strstr(json.text,"\"key\":\"")+7;n=strcspn(key,"\"");snprintf(id,sizeof(id),"app-menu-%.*s",(int)n,key);
 p.size=sizeof(p);{ui_status_t status=ui_web_view_get_presentation(host->view,id,&p);if(status!=UI_STATUS_OK||!p.visible||p.text_overflow||strcmp(p.text_utf8,entry->title))fprintf(stderr,"Menu presentation id=%s dpi=%u status=%d visible=%d overflow=%d rect=%d,%d,%d,%d text=%s expected=%s\n",id,host->dpi,status,p.visible,p.text_overflow,p.rect.x,p.rect.y,p.rect.width,p.rect.height,p.text_utf8,entry->title);CHECK(status==UI_STATUS_OK&&p.visible&&!p.text_overflow&&!strcmp(p.text_utf8,entry->title));}free(json.text);}
static int menu_click(ui_host_t *host,const char *id)
{ui_element_presentation_t p={0};ui_input_event_t e={0};p.size=sizeof(p);if(ui_host_menu_get_item_presentation(host,id,&p)!=UI_STATUS_OK||!p.visible||!p.enabled)return 0;
 e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.x=p.clip.x+3;e.y=p.clip.y+3;e.pointer_button=1;if(ui_host_menu_dispatch_input(host,&e)!=UI_STATUS_OK)return 0;e.kind=UI_INPUT_POINTER_UP;return ui_host_menu_dispatch_input(host,&e)==UI_STATUS_OK;}
static BOOL CALLBACK controls(HWND w,LPARAM data){wchar_t name[80];(void)data;GetClassNameW(w,name,80);
 if(!_wcsicmp(name,L"Edit")||!_wcsicmp(name,L"Button")||!_wcsicmp(name,L"SysTreeView32")||!_wcsicmp(name,L"SysListView32"))++forbidden;return TRUE;}
static unsigned operations(ui_app_instance_info_t *info){const char *json=NULL,*p;unsigned n=0;
 CHECK(ui_assistant_get_state_snapshot(info->assistant,&json)==UI_STATUS_OK);p=strstr(json,"\"operations\":");if(p)sscanf(p+13,"%u",&n);return n;}
static void screenshot(HWND window,const wchar_t *path)
{RECT r;HDC dc;HBITMAP bitmap;HGDIOBJ old;BITMAPINFO info={0};BITMAPFILEHEADER header={0};void *bits;FILE *file=NULL;
 GetClientRect(window,&r);dc=CreateCompatibleDC(NULL);info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=r.right;info.bmiHeader.biHeight=-r.bottom;
 info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);CHECK(bitmap!=NULL);if(!bitmap){DeleteDC(dc);return;}
 old=SelectObject(dc,bitmap);CHECK(PrintWindow(window,dc,PW_CLIENTONLY));GdiFlush();header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);header.bfSize=header.bfOffBits+(DWORD)r.right*(DWORD)r.bottom*4;
 if(!_wfopen_s(&file,path,L"wb")){fwrite(&header,sizeof(header),1,file);fwrite(&info.bmiHeader,sizeof(info.bmiHeader),1,file);fwrite(bits,4,(size_t)r.right*r.bottom,file);fclose(file);}
 SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);}
int wmain(int argc,wchar_t **argv)
{
 host_window_t host={0};HWND root;uint64_t first,second;ui_app_instance_info_t a={0},b={0};ui_menu_popup_desc_t menu={0};ui_input_event_t e={0};ui_element_presentation_t p={0};int i,j;
 const int sizes[]={1920,1280,800,640},heights[]={1080,720,600,480};const uint32_t dpis[]={96,144,192};if(argc<2)return 2;CHECK(ui_framework_initialize()==UI_STATUS_OK);
 root=create_host(&host,GetModuleHandleW(NULL));CHECK(root!=NULL);if(!root)return 1;ShowWindow(root,SW_SHOW);
 open_path(&host,argv[1]);pump();first=ui_workspace_active(host.workspace);open_path(&host,argv[1]);pump();second=ui_workspace_active(host.workspace);
 CHECK(first&&second&&first!=second);if(!first||!second){fprintf(stderr,"%s\n",ui_workspace_last_error(host.workspace));return 1;}
 CHECK(get_instance(&host,first,&a)&&get_instance(&host,second,&b));MoveWindow(root,30,30,1920,1080,TRUE);pump();group_count=0;CHECK(ui_host_visit_menu(b.host,"",group,&host)==UI_STATUS_OK&&group_count==7);
 menu.size=sizeof(menu);menu.path="Edit";menu.anchor.size=sizeof(menu.anchor);CHECK(ui_host_show_menu(b.host,&menu)==UI_STATUS_OK);CHECK(menu_click(b.host,"feature.menu.1"));CHECK(operations(&b)==1&&operations(&a)==0);
 CHECK(ui_host_show_menu(b.host,&menu)==UI_STATUS_OK);CHECK(ui_workspace_activate(host.workspace,first)==UI_STATUS_OK);layout(&host);CHECK(ui_web_view_flush(host.view,64)==UI_STATUS_OK);pump();p.size=sizeof(p);CHECK(ui_host_menu_get_presentation(b.host,"close",&p)==UI_STATUS_NOT_FOUND);
 /* Normal Web dispatch reserves Ctrl+O for the application and executes once. */
 e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code='O';e.modifiers=UI_INPUT_MODIFIER_CONTROL;CHECK(ui_web_view_dispatch_input(host.view,&e)==UI_STATUS_OK);pump();CHECK(operations(&a)==1);
 {MSG key={0};key.hwnd=root;key.wParam=VK_MENU;key.message=WM_SYSKEYDOWN;CHECK(host_menu_key(&host,&key));key.message=WM_SYSKEYUP;CHECK(host_menu_key(&host,&key));p.size=sizeof(p);CHECK(ui_host_menu_get_presentation(a.host,"close",&p)==UI_STATUS_OK);key.message=WM_KEYDOWN;key.wParam=VK_ESCAPE;CHECK(host_menu_key(&host,&key));key.message=WM_KEYUP;CHECK(host_menu_key(&host,&key));}
 for(i=0;i<4;++i)for(j=0;j<3;++j){MoveWindow(root,30,30,sizes[i],heights[i],TRUE);pump();host.dpi=dpis[j];layout(&host);pump();
  p.size=sizeof(p);CHECK(ui_web_view_get_presentation(host.view,"tool-more",&p)==UI_STATUS_OK);if(sizes[i]<=800)CHECK(p.visible&&p.enabled);
  if(p.visible){CHECK(click(host.view,"tool-more"));pump();for(int n=0;n<6;++n){ui_element_presentation_t down={0};down.size=sizeof(down);if(ui_host_menu_get_presentation(a.host,"down",&down)!=UI_STATUS_OK||!down.enabled)break;
    e.kind=UI_INPUT_POINTER_DOWN;e.x=down.clip.x+3;e.y=down.clip.y+3;e.pointer_button=1;e.modifiers=0;CHECK(ui_host_menu_dispatch_input(a.host,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_host_menu_dispatch_input(a.host,&e)==UI_STATUS_OK);}
   CHECK(ui_host_menu_get_item_presentation(a.host,"feature.tool.12",&p)==UI_STATUS_OK&&p.visible&&p.enabled);CHECK(ui_host_close_menu(a.host)==UI_STATUS_OK);}}
 host.dpi=96;MoveWindow(root,30,30,1600,1000,TRUE);pump();layout(&host);pump();if(argc>2)screenshot(root,argv[2]);
 EnumChildWindows(root,controls,0);CHECK(forbidden==0);CHECK(ui_workspace_close_all(host.workspace)==UI_STATUS_OK);
 {ULONGLONG end=GetTickCount64()+5000;while(ui_workspace_count(host.workspace)&&GetTickCount64()<end){pump();ui_workspace_poll(host.workspace);SwitchToThread();}}
 CHECK(ui_workspace_count(host.workspace)==0);DestroyWindow(root);pump();printf("API4 actual host menus/overflow/instances: %d failures\n",failures);return failures?1:0;
}
