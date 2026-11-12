/* Isolated window driver: public view/slot APIs, no human authorization. */
#define UHA_MAIN inherited_uha_main
#include "uha_host.c"
#undef wmain
static void bitmap(const wchar_t *folder,const char *name,const unsigned char *pixels,int width,int height)
{
 wchar_t file[4096];_snwprintf_s(file,4096,_TRUNCATE,L"%s/%S.bmp",folder,name);
 FILE *f=_wfopen(file,L"wb");CHECK(f!=NULL);if(!f)return;
 BITMAPFILEHEADER b={0};BITMAPINFOHEADER h={0};h.biSize=sizeof(h);h.biWidth=width;h.biHeight=-height;h.biPlanes=1;h.biBitCount=32;h.biCompression=BI_RGB;
 b.bfType=0x4d42;b.bfOffBits=sizeof(b)+sizeof(h);b.bfSize=b.bfOffBits+(DWORD)(width*height*4);
 fwrite(&b,sizeof(b),1,f);fwrite(&h,sizeof(h),1,f);fwrite(pixels,4,(size_t)width*height,f);fclose(f);
}
static void capture(ui_web_view_t *v,const wchar_t *folder,const char *name,int width,int height)
{
 ui_rect_t current={0};CHECK(ui_web_view_get_rect(v,&current)==UI_STATUS_OK);width=current.width;height=current.height;
 ui_pixel_buffer_t b={0};b.size=sizeof(b);CHECK(ui_web_view_capture_rgba(v,width,height,96,&b)==UI_STATUS_OK);
 b.capacity=b.stride*b.height;b.pixels=malloc(b.capacity);CHECK(b.pixels!=NULL);
 if(b.pixels){CHECK(ui_web_view_capture_rgba(v,width,height,96,&b)==UI_STATUS_OK);
  if(!strncmp(name,"canvas",6))CHECK(b.pixels[0]==32&&b.pixels[1]==128&&b.pixels[2]==96);
  for(size_t i=0;i<b.capacity;i+=4){unsigned char red=b.pixels[i];b.pixels[i]=b.pixels[i+2];b.pixels[i+2]=red;}
  bitmap(folder,name,b.pixels,b.width,b.height);free(b.pixels);}
}
static void empty_paint(HWND window,const wchar_t *folder,const char *name)
{
 RECT r;GetClientRect(window,&r);int width=r.right,height=r.bottom;
 BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;void *bytes=NULL;
 HDC dc=CreateCompatibleDC(NULL);HBITMAP bm=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bytes,NULL,0);HGDIOBJ old=SelectObject(dc,bm);FillRect(dc,&r,(HBRUSH)GetStockObject(BLACK_BRUSH));
 LRESULT erased=SendMessageW(window,WM_ERASEBKGND,(WPARAM)dc,0);GdiFlush();COLORREF pixel=GetPixel(dc,8,8);
 CHECK(erased!=0);CHECK(pixel!=RGB(0,0,0));bitmap(folder,name,bytes,width,height);
 SendMessageW(window,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();unsigned ink=0;
 for(int y=24;y<110;++y)for(int x=24;x<300;++x)if(GetPixel(dc,x,y)!=pixel)++ink;
 CHECK(ink>20);bitmap(folder,"empty-hint",bytes,width,height);
 printf("empty paint=%lld corner=%06lx\n",(long long)erased,(unsigned long)pixel);
 SelectObject(dc,old);DeleteObject(bm);DeleteDC(dc);
}
static host_window_t *closing_host;static int capture_close,callback_guard;
static LRESULT CALLBACK capture_proc(HWND h,UINT m,WPARAM w,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
 (void)id;(void)data;if(m==WM_CAPTURECHANGED&&capture_close){capture_close=0;host_window_t *s=closing_host;
  callback_guard=s->chrome_host->dispatch_depth>0;SendMessageW(s->hwnd,WM_CLOSE,0,0);CHECK(IsWindow(s->hwnd)&&s->ai!=NULL);}
 return DefSubclassProc(h,m,w,l);
}
static void canvas_click(ui_web_view_t *v)
{
 ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(v,"mark",&p)==UI_STATUS_OK&&p.visible);
 ui_input_event_t e={0};e.size=sizeof(e);e.x=p.clip.x+8;e.y=p.clip.y+8;e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);
}
static void reveal(host_window_t *s,const char *id,ui_element_presentation_t *p)
{
 DWORD begin=GetTickCount();RECT r;GetClientRect(s->hwnd,&r);
 do{CHECK(ui_web_view_get_presentation(s->view,id,p)==UI_STATUS_OK);if(p->visible&&p->clip.height==p->rect.height)return;
 ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_WHEEL;ui_rect_t assistant={0};CHECK(ui_web_view_get_element_rect(s->view,"assistant",&assistant)==UI_STATUS_OK);e.x=assistant.x+assistant.width-24;e.y=r.bottom-90;e.wheel_delta=p->rect.y<70?480:-480;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);pump(s);
 }while(GetTickCount()-begin<5000);CHECK(p->visible&&p->clip.height==p->rect.height);
}
static void collapsed(host_window_t *s)
{
 ui_element_presentation_t p={0};p.size=sizeof(p);
 CHECK(!assistant_visible(s));CHECK(ui_web_view_get_presentation(s->view,"ai-input",&p)==UI_STATUS_OK&&!p.visible);
 CHECK(!s->ai_preview_ready);HRGN mask=CreateRectRgn(0,0,0,0);CHECK(GetWindowRgn(s->web_hwnd,mask)!=ERROR);
 CHECK(!PtInRegion(mask,s->workspace_rect.x+s->workspace_rect.width-10,s->workspace_rect.y+100));DeleteObject(mask);
}
int wmain(int argc,wchar_t **argv)
{
 if(argc!=4)return 2;host_window_t s={0};host_action_t a={0};GUID g;wchar_t unique[40],db[4096];char instance[32];
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CoCreateGuid(&g);StringFromGUID2(&g,unique,40);_snwprintf_s(db,4096,_TRUNCATE,L"%s/visibility-%s.db",argv[2],unique);CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));
 CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.ai)return 1;
 CHECK(model_wait(&s,NULL));MoveWindow(s.hwnd,-16000,-16000,1200,900,FALSE);ShowWindow(s.hwnd,SW_SHOWNOACTIVATE);
 open_path(&s,argv[1]);pump(&s);ui_app_instance_info_t app={0};app.size=sizeof(app);CHECK(get_instance(&s,ui_workspace_active(s.workspace),&app));
 int empty=!wcscmp(argv[3],L"empty"),labels=!wcscmp(argv[3],L"labels"),focus=!wcscmp(argv[3],L"focus"),reentry=!wcscmp(argv[3],L"reentry"),held=!wcscmp(argv[3],L"capture")||reentry;
 ui_web_backend_t *backend=NULL;ui_web_view_t *view=NULL;
 if(!empty&&!reentry){
  ui_content_slot_t *slot=ui_shell_get_content_slot(ui_host_get_shell(app.host),NULL);ui_light_web_config_t c={0};c.size=sizeof(c);c.parent_hwnd=ui_content_slot_native_handle(slot);backend=ui_light_web_backend_create(&c);CHECK(backend!=NULL);view=ui_web_view_create(app.host,backend);CHECK(view!=NULL);
  CHECK(ui_web_view_load_html(view,"<body style='background:#208060;padding:30px'><div id='content'>VISIBLE BUSINESS FIXTURE</div><button id='mark'>hit 0</button><script>var n=0;document.getElementById('mark').addEventListener('click',function(){document.getElementById('mark').textContent='hit '+(++n)})</script></body>")==UI_STATUS_OK);
  CHECK(ui_content_slot_attach_web_view(slot,view)==UI_STATUS_OK);layout(&s);pump(&s);capture(view,argv[2],"canvas-before",500,300);canvas_click(view);
 }
 if(empty)empty_paint((HWND)app.native_container,argv[2],"empty-before");
 a.action="assistant";process_action(&s,&a);pump(&s);CHECK(assistant_visible(&s));capture(s.view,argv[2],"expanded",1200,900);
 click(&s,"ai-model-toggle");click(&s,"ai-draft-range-toggle");text(&s,"ai-model-name","offline-fixture");click(&s,"ai-model-save");CHECK(model_wait(&s,NULL));response(&s,0);
 click(&s,"ai-model-catalog");click(&s,"ai-resource-select-0");int selected=cap_index(&s);CHECK(selected>=0);char node[64];snprintf(node,64,"ai-cap-select-%d",selected);click(&s,node);snprintf(node,64,"ai-field-select-%d-0",selected);click(&s,node);snprintf(node,64,"ai-field-select-%d-1",selected);click(&s,node);
 if(labels){
  const char *ids[]={"ai-input-label","ai-answer-label","ai-resource-label","ai-plan-label"};ui_element_presentation_t p={0};p.size=sizeof(p);
  for(unsigned i=0;i<4;++i)CHECK(ui_web_view_get_presentation(s.view,ids[i],&p)==UI_STATUS_OK&&p.text_utf8[0]);
 }else{
  snprintf(instance,32,"%llu",(unsigned long long)app.instance_id);a.action="ai-model-propose";a.token="visibility-proposal";a.answer="synthetic local proposal";a.id=instance;a.panel="memory.document";a.params="{\"history\":\"[]\",\"projection\":\"{\\\"capabilities\\\":[],\\\"fields\\\":[]}\"}";process_action(&s,&a);CHECK(model_wait(&s,"\"phase\":3"));click(&s,"ai-view-scope");CHECK(bound(&s));
 }
 text(&s,"ai-input","保留草稿 draft");char run[65];strcpy_s(run,65,s.ai_run);char *draft=_strdup(s.ai_draft);CHECK(draft&&strstr(draft,"memory.read")&&strstr(draft,"revision"));ui_element_presentation_t input={0};input.size=sizeof(input);CHECK(ui_web_view_get_presentation(s.view,"ai-input",&input)==UI_STATUS_OK&&input.focused);
 if(held){ui_element_presentation_t p={0};p.size=sizeof(p);reveal(&s,"ai-model-toggle",&p);ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.x=p.clip.x+5;e.y=p.clip.y+5;CHECK(ui_web_view_dispatch_input(s.view,&e)==UI_STATUS_OK);CHECK(GetCapture()==s.web_hwnd);}
 if(held){for(unsigned i=0;i<3;++i){char *state=NULL;send_state(&s,&state);free(state);}CHECK(GetCapture()==s.web_hwnd);}
 if(reentry){closing_host=&s;CHECK(SetWindowSubclass(s.web_hwnd,capture_proc,1,0));capture_close=1;}
 if(focus||held){a.action="assistant";process_action(&s,&a);if(reentry){char *state=NULL;send_state(&s,&state);free(state);}else pump(&s);}else click(&s,"assistant-toggle");
 if(reentry){CHECK(callback_guard&&!capture_close);DWORD begin=GetTickCount();HWND window=s.hwnd;while(IsWindow(window)&&GetTickCount()-begin<5000)pump(&s);CHECK(!IsWindow(window)&&!s.ai&&!s.workspace);free(draft);printf("visibility reentry checks=%u failures=%u\n",checks,failures);SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);return failures?1:0;}
 collapsed(&s);if(!labels)CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);capture(s.view,argv[2],"collapsed",1200,900);CHECK(GetCapture()!=s.web_hwnd);
 if(focus){ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_TEXT;e.text_utf8="BAD-HIDDEN";CHECK(ui_web_view_dispatch_input(s.view,&e)!=UI_STATUS_OK);CHECK(ui_web_view_get_presentation(s.view,"ai-input",&input)==UI_STATUS_OK&&!input.focused&&!strcmp(input.text_utf8,"保留草稿 draft"));}
 if(empty)empty_paint((HWND)app.native_container,argv[2],"empty-collapsed");
 for(unsigned i=0;i<3;++i){char *state=NULL;send_state(&s,&state);free(state);}CHECK(!strcmp(run,s.ai_run));CHECK(s.ai->model_config->requests==0);
 click(&s,"assistant-toggle");CHECK(assistant_visible(&s));CHECK(ui_web_view_get_presentation(s.view,"ai-input",&input)==UI_STATUS_OK&&!strcmp(input.text_utf8,"保留草稿 draft"));
 if(!labels){CHECK(bound(&s));CHECK(s.ai_preview_ready);}
 for(unsigned i=0;i<3;++i){click(&s,"assistant-toggle");collapsed(&s);click(&s,"assistant-toggle");CHECK(assistant_visible(&s));}
 MoveWindow(s.hwnd,-16000,-16000,1000,1050,FALSE);pump(&s);CHECK(ui_web_view_get_presentation(s.view,"ai-input",&input)==UI_STATUS_OK&&!strcmp(input.text_utf8,"保留草稿 draft"));
 char *page=load_page();CHECK(page&&ui_web_view_load_html(s.view,page)==UI_STATUS_OK);free(page);schedule_refresh(&s);pump(&s);CHECK(!strcmp(run,s.ai_run));CHECK(ui_web_view_get_presentation(s.view,"ai-input",&input)==UI_STATUS_OK&&!strcmp(input.text_utf8,"保留草稿 draft"));CHECK(!s.ai_preview_ready);if(!labels)click(&s,"ai-view-scope");if(!labels){int ready=bound(&s);if(!ready){ui_element_presentation_t q={0};q.size=sizeof(q);ui_web_view_get_presentation(s.view,"ai-disclosure-0",&q);fprintf(stderr,"rebuild ready=%d valid=%d generation=%llu/%llu visible=%d rect=%d,%d,%d,%d binding=%llu error=%s\n",s.ai_preview_ready,s.ai_display_valid,(unsigned long long)s.ai_projection_generation,(unsigned long long)s.ai_projection_rendered,q.visible,q.rect.x,q.rect.y,q.rect.width,q.rect.height,(unsigned long long)s.ai_model_bindings[1],s.ai_error?s.ai_error:"");}CHECK(ready);}
 if(view){canvas_click(view);ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(view,"mark",&p)==UI_STATUS_OK&&!strcmp(p.text_utf8,"hit 2"));capture(view,argv[2],"canvas-after",500,300);ui_web_view_destroy(view);ui_light_web_backend_destroy(backend);}
 CHECK(s.ai_draft&&!strcmp(s.ai_draft,draft));free(draft);capture(s.view,argv[2],"reexpanded",1200,900);CHECK(s.ai->model_config->requests==0);CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);
 printf("visibility %ls checks=%u failures=%u; simulated input only\n",argv[3],checks,failures);return failures?1:0;
}
