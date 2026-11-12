/* Formal host page; isolated credentials, transport and synthetic data only. */
#include <stdlib.h>
#include <string.h>
#include "ui_framework/light_web.h"
static int summary_interrupt,summary_cache;
/* Instrument only the isolated page copy. No production message or export. */
static char *summary_replace(char *text,const char *needle,const char *insert)
{
 if(!text)return NULL;char *at=strstr(text,needle);if(!at){free(text);return NULL;}size_t prefix=(size_t)(at-text),n=strlen(text)+strlen(insert)+1;char *out=malloc(n);if(out){memcpy(out,text,prefix);strcpy(out+prefix,insert);strcpy(out+prefix+strlen(insert),at+strlen(needle));}free(text);return out;
}
static ui_status_t summary_load(ui_web_view_t *view,const char *html)
{
 if(summary_cache){char *copy=_strdup(html);
  copy=summary_replace(copy,"clear(element('ai-disclosure'));","clear(element('ai-disclosure'));cacheFault('clear','');");if(!copy)return UI_STATUS_VALIDATION_FAILED;
  copy=summary_replace(copy,"element('ai-disclosure').appendChild(t)","element('ai-disclosure').appendChild(t);cacheFault('append','')");if(!copy)return UI_STATUS_VALIDATION_FAILED;
  copy=summary_replace(copy,"p.text=text||'';","p.text=text||'';cacheFault('pending',id);");if(!copy)return UI_STATUS_VALIDATION_FAILED;
  copy=summary_replace(copy,"element(id).value=p.text.slice(at,end);","element(id).value=p.text.slice(at,end);cacheFault('text',id);");if(!copy)return UI_STATUS_VALIDATION_FAILED;
  copy=summary_replace(copy,"<script>","<span id=\"summary-fault-fired\" style=\"display:none\"></span><span id=\"summary-unexpected\" style=\"display:none\"></span><script>");if(!copy)return UI_STATUS_VALIDATION_FAILED;
  copy=summary_replace(copy," send('ready');","var cachePoint='',cacheNode='';function cacheFault(point,id){if(cachePoint===point&&(!cacheNode||cacheNode===id)){cachePoint='';element('summary-fault-fired').textContent=point+':'+id;throw Error('isolated cache interruption')}}send('ready');");if(!copy)return UI_STATUS_VALIDATION_FAILED;
  copy=summary_replace(copy,"ui.onmessage=function(state) {","ui.onmessage=function(state) {if(state.type==='cache-arm'){cachePoint=state.point;cacheNode=state.node;element('summary-fault-fired').textContent='armed';return;}");if(!copy)return UI_STATUS_VALIDATION_FAILED;
  copy=summary_replace(copy,"catch(e){element('ai-projection-proof').textContent='';}","catch(e){element('summary-unexpected').textContent=String(e);element('ai-projection-proof').textContent='';}");if(!copy)return UI_STATUS_VALIDATION_FAILED;
  ui_status_t st=ui_web_view_load_html(view,copy);free(copy);return st;
 }
 if(!summary_interrupt)return ui_web_view_load_html(view,html);
 const char *needle="element('ai-disclosure').appendChild(t)",*at=strstr(html,needle),*script=strstr(html,"<script>");if(!at||!script)return UI_STATUS_VALIDATION_FAILED;
 const char *mark="<span id=\"summary-fault-fired\" style=\"display:none\"></span>",*fault=";if(!globalThis.__summaryInterrupted){globalThis.__summaryInterrupted=true;element('summary-fault-fired').textContent='once';throw Error('isolated mid-detail interruption')}";
 size_t n=strlen(html)+strlen(mark)+strlen(fault)+1;char *copy=malloc(n);if(!copy)return UI_STATUS_OUT_OF_MEMORY;size_t first=(size_t)(script-html),second=(size_t)(at+strlen(needle)-script);memcpy(copy,html,first);strcpy(copy+first,mark);size_t used=first+strlen(mark);memcpy(copy+used,script,second);used+=second;strcpy(copy+used,fault);used+=strlen(fault);strcpy(copy+used,at+strlen(needle));ui_status_t st=ui_web_view_load_html(view,copy);free(copy);return st;
}
#define ui_web_view_load_html summary_load
#define UHA_MAIN previous_uha_main
#include "uha_host.c"
#undef ui_web_view_load_html

static int shown(host_window_t *s,const char *id,const char *part)
{ui_element_presentation_t p={0};p.size=sizeof(p);ui_status_t st=ui_web_view_get_presentation(s->view,id,&p);if(st||!p.text_utf8||!strstr(p.text_utf8,part)){fprintf(stderr,"summary node %s status=%d expected=%s actual=%.1200s\n",id,st,part,st?"":p.text_utf8?p.text_utf8:"");return 0;}return 1;}
/* Compare every displayed page with the authoritative full text, including
 * the existing surrogate-safe overlap at a UTF-16 page boundary. */
static void paged_equal(host_window_t *s,const char *id,const char *expected)
{int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,expected,-1,NULL,0);CHECK(n>0);if(n<=0)return;wchar_t *w=malloc((size_t)n*sizeof(*w));CHECK(w!=NULL);if(!w)return;CHECK(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,expected,-1,w,n)==n);unsigned length=(unsigned)n-1,pages=(length+899)/900;char next[80];snprintf(next,sizeof(next),"%s-next",id);
 for(unsigned page=0;page<pages;++page){unsigned at=page*900,end=at+900;if(end>length)end=length;if(at&&w[at]>=0xdc00&&w[at]<=0xdfff)--at;if(end<length&&w[end-1]>=0xd800&&w[end-1]<=0xdbff)++end;char slice[4096];int bytes=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,w+at,(int)(end-at),slice,sizeof(slice)-1,NULL,NULL);CHECK(bytes>0);if(bytes>0){slice[bytes]=0;ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s->view,id,&p)==UI_STATUS_OK&&p.text_utf8&&!strcmp(p.text_utf8,slice));}if(page+1<pages){if(!page)click(s,next);else{ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s->view,next,&p)==UI_STATUS_OK&&p.visible&&p.enabled);ui_input_event_t e={0};e.size=sizeof(e);e.x=p.clip.x+p.clip.width/2;e.y=p.clip.y+p.clip.height/2;e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);}}}
 printf("summary full pages=%u UTF16=%u\n",pages,length);free(w);
}
static void summary_capture(host_window_t *s,const wchar_t *folder,const char *name)
{ui_rect_t rect={0};CHECK(ui_web_view_get_rect(s->view,&rect)==UI_STATUS_OK);ui_pixel_buffer_t b={0};b.size=sizeof(b);CHECK(ui_web_view_capture_rgba(s->view,rect.width,rect.height,96,&b)==UI_STATUS_OK);b.capacity=b.stride*b.height;b.pixels=malloc(b.capacity);CHECK(b.pixels!=NULL);if(!b.pixels)return;CHECK(ui_web_view_capture_rgba(s->view,rect.width,rect.height,96,&b)==UI_STATUS_OK);for(size_t i=0;i<b.capacity;i+=4){unsigned char red=b.pixels[i];b.pixels[i]=b.pixels[i+2];b.pixels[i+2]=red;}wchar_t path[4096];_snwprintf_s(path,4096,_TRUNCATE,L"%s/%S.bmp",folder,name);FILE *f=_wfopen(path,L"wb");CHECK(f!=NULL);if(f){BITMAPFILEHEADER bh={0};BITMAPINFOHEADER ih={0};ih.biSize=sizeof(ih);ih.biWidth=b.width;ih.biHeight=-(LONG)b.height;ih.biPlanes=1;ih.biBitCount=32;ih.biCompression=BI_RGB;bh.bfType=0x4d42;bh.bfOffBits=sizeof(bh)+sizeof(ih);bh.bfSize=bh.bfOffBits+(DWORD)b.capacity;fwrite(&bh,sizeof(bh),1,f);fwrite(&ih,sizeof(ih),1,f);fwrite(b.pixels,1,b.capacity,f);fclose(f);}free(b.pixels);}
static void cache_stage(host_window_t *s,const char *stage)
{ui_element_presentation_t p={0};p.size=sizeof(p);ui_status_t st=ui_web_view_get_presentation(s->view,"summary-unexpected",&p);printf("cache stage=%s valid=%d error=%s page=%s\n",stage,s->ai_display_valid,s->ai_error?s->ai_error:"",!st&&p.text_utf8?p.text_utf8:"");fflush(stdout);}
static void start_page(host_window_t *s)
{host_action_t a={0};CHECK(create_host(s,GetModuleHandleW(NULL))!=NULL);CHECK(model_wait(s,NULL));CHECK(MoveWindow(s->hwnd,-16000,-16000,1200,4000,FALSE));ShowWindow(s->hwnd,SW_HIDE);a.action="assistant";process_action(s,&a);pump(s);click(s,"ai-model-toggle");click(s,"ai-draft-range-toggle");}
static void seed_summary(host_window_t *s,int large,int utf8)
{if(!UI_MODEL_DISPATCH_GUARD(s))return;
 /* Paging fault coverage begins after both history records exist. Coalesce
    unrelated seed projections, keeping the original tasks and all 50000 bytes. */
 int coalesced=summary_cache&&large;if(coalesced)s->refreshing=1;
 char *value=malloc(50001);CHECK(value!=NULL);if(!value)return;memset(value,'A',50000);value[50000]=0;if(utf8)for(unsigned i=0;i<7142;++i)memcpy(value+i*7,"汉😀",7);
 for(unsigned i=0;i<2;++i){ui_ai_task_request_t q={0};json_buffer_t j={0};q.size=sizeof(q);q.version=1;q.client_request_id=i?"summary-seed-b":"summary-seed-a";q.input_utf8=i?"UNSELECTED-HISTORY-B":"SELECTED-HISTORY-A";json_append(&j,"{\"steps\":[{\"type\":\"text\",\"text\":");json_string(&j,large?value:"synthetic response");json_append(&j,"}]}");q.stub_plan_json=j.text;if(!UI_MODEL_DISPATCH_GUARD(s)){free(j.text);free(value);return;}CHECK(ui_ai_submit(s->ai,&q,s->ai_run)==UI_STATUS_OK);free(j.text);CHECK(model_wait(s,"\"terminal\":true"));}
 free(value);if(coalesced){s->refreshing=0;schedule_refresh(s);}DWORD begin=GetTickCount();do{pump(s);if(s->ai_messages&&strstr(s->ai_messages,"UNSELECTED-HISTORY-B"))break;}while(GetTickCount()-begin<5000);CHECK(s->ai_messages&&strstr(s->ai_messages,"UNSELECTED-HISTORY-B"));}

/* Direct interruption timing: no message pump between arming and inspecting the
 * failed projection. All task proposals and authoritative previews are real. */
static void cache_project(host_window_t *s,const char *id)
{if(id){host_action_t a={0};a.action="ai-select";a.id=(char *)id;process_action(s,&a);}char *layout=NULL;send_state(s,&layout);free(layout);}
static void cache_arm(host_window_t *s,const char *point,const char *node)
{char j[180];snprintf(j,sizeof(j),"{\"type\":\"cache-arm\",\"point\":\"%s\",\"node\":\"%s\"}",point,node);CHECK(ui_web_view_post_json(s->view,j)==UI_STATUS_OK);}
static void cache_zero(host_window_t *s)
{CHECK(s->ai->model_config->requests==0);EnterCriticalSection(&s->ai->lock);for(unsigned i=0;i<AI_RUNS;++i){ai_run_t *r=&s->ai->runs[i];if(r->model){CHECK(!r->model_calls&&!r->tool_calls&&!r->model->ordinal&&!r->model->confirmed&&!r->model->request_saved);}}LeaveCriticalSection(&s->ai->lock);}
static void cache_same(host_window_t *s,const char *id,const char *expected)
{ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s->view,id,&p)==UI_STATUS_OK&&p.text_utf8&&!strcmp(p.text_utf8,expected));}
static char *cache_text(host_window_t *s,const char *id)
{ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s->view,id,&p)==UI_STATUS_OK);return _strdup(p.text_utf8?p.text_utf8:"");}
static void cache_details(host_window_t *s,const char *expected)
{yyjson_doc *d=yyjson_read(s->ai_chunks,strlen(s->ai_chunks),0);size_t i,n;yyjson_val *v;json_buffer_t all={0};yyjson_arr_foreach(yyjson_doc_get_root(d),i,n,v){char node[64];snprintf(node,sizeof(node),"ai-disclosure-%u",(unsigned)i);cache_same(s,node,yyjson_get_str(v));json_append(&all,yyjson_get_str(v));}CHECK(all.text&&!strcmp(all.text,expected));char label[100];snprintf(label,sizeof(label),"共 %u 块",(unsigned)yyjson_arr_size(yyjson_doc_get_root(d)));CHECK(shown(s,"ai-detail-count",label));free(all.text);yyjson_doc_free(d);}
static int cache_replay(host_window_t *s,const char *state)
{int rendered=ai_projection_send(s,state);if(rendered)s->ai_projection_rendered=s->ai_projection_generation;ai_model_panel_bindings(s);return rendered;}
/* A permitted script-budget interruption is distinct from the armed fault.
 * Replay display only, without pumping Service or accepting any confirmation. */
static void cache_complete(host_window_t *s,const char *state)
{
 for(unsigned attempt=0;attempt<3;++attempt){
  if(cache_replay(s,state))return;
  CHECK(!s->ai_display_valid);
  CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
  cache_zero(s);
 }
 CHECK(0);
}
static void cache_state_details(host_window_t *s,const char *state)
{yyjson_doc *d=yyjson_read(state,strlen(state),0);yyjson_val *chunks=yyjson_obj_get(yyjson_obj_get(yyjson_doc_get_root(d),"ai"),"disclosureChunks"),*v;size_t i,n;yyjson_arr_foreach(chunks,i,n,v){char node[64];snprintf(node,sizeof(node),"ai-disclosure-%u",(unsigned)i);cache_same(s,node,yyjson_get_str(v));}char label[100];snprintf(label,sizeof(label),"共 %u 块",(unsigned)yyjson_arr_size(chunks));CHECK(shown(s,"ai-detail-count",label));yyjson_doc_free(d);}
static void cache_page_value(host_window_t *s,const char *full,unsigned page)
{int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,full,-1,NULL,0);CHECK(n>0);if(n<=0)return;wchar_t *w=malloc((size_t)n*sizeof(*w));CHECK(w!=NULL);if(!w)return;MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,full,-1,w,n);unsigned length=(unsigned)n-1,at=page*900,end=at+900;if(end>length)end=length;CHECK(at<length);if(at&&w[at]>=0xdc00&&w[at]<=0xdfff)--at;if(end<length&&w[end-1]>=0xd800&&w[end-1]<=0xdbff)++end;char out[4096],label[80];int bytes=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,w+at,(int)(end-at),out,sizeof(out)-1,NULL,NULL);CHECK(bytes>0);if(bytes>0){out[bytes]=0;cache_same(s,"ai-summary-history",out);}snprintf(label,sizeof(label),"展示 %u / %u",page+1,(length+899)/900);cache_same(s,"ai-summary-history-page",label);free(w);}
/* Paging interruption does not require a second waiting task. Keep the full
 * two 50000-byte histories, but isolate only this synchronous page transition. */
static void cache_page_case(host_window_t *s,const char *id,const char *preview)
{
 text(s,"ai-input","CACHE-DRAFT-未提交");char *draft=_strdup(s->ai_draft);uint64_t old=s->ai_model_bindings[1];CHECK(old);++s->ai->methods;click(s,"ai-detail-toggle");click(s,"ai-summary-history-next");cache_arm(s,"text","ai-summary-history");
 ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s->view,"ai-summary-history-next",&p)==UI_STATUS_OK&&p.visible&&p.enabled);ui_input_event_t e={0};e.size=sizeof(e);e.x=p.clip.x+p.clip.width/2;e.y=p.clip.y+p.clip.height/2;e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;CHECK(ui_web_view_dispatch_input(s->view,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;(void)ui_web_view_dispatch_input(s->view,&e);
 CHECK(shown(s,"summary-fault-fired","text:ai-summary-history"));CHECK(!s->ai_preview_ready);ai_model_panel_bindings(s);CHECK(s->ai_model_bindings[1]==old);CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);cache_zero(s);
 cache_project(s,NULL);yyjson_doc *d=yyjson_read(preview,strlen(preview),0);yyjson_val *input=yyjson_obj_get(yyjson_obj_get(yyjson_doc_get_root(d),"request"),"input");json_buffer_t history={0};json_append(&history,"所选历史序号：2、4\n以下按实际请求顺序列出所选历史全文：\n");for(unsigned i=0;i<2;++i){yyjson_val *v=yyjson_arr_get(input,i);CHECK(strlen(yyjson_get_str(yyjson_obj_get(v,"content")))==50000);json_format(&history,"片段 %u · %s\n",i+1,yyjson_get_str(yyjson_obj_get(v,"role")));json_append(&history,yyjson_get_str(yyjson_obj_get(v,"content")));json_append(&history,"\n");}cache_page_value(s,history.text,2);free(history.text);yyjson_doc_free(d);CHECK(shown(s,"ai-task-state",id));cache_zero(s);--s->ai->methods;
 click(s,"ai-detail-toggle");CHECK(bound(s));cache_details(s,preview);CHECK(s->ai_preview_ready&&s->ai_draft&&draft&&!strcmp(s->ai_draft,draft));CHECK(shown(s,"ai-input","CACHE-DRAFT-未提交"));cache_zero(s);if(!failures){CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);CHECK(model_wait(s,"\"terminal\":true"));CHECK(s->ai->model_config->requests==1);}free(draft);
}
static void cache_cases(host_window_t *s,const wchar_t *mode,const char *aid,const char *apreview)
{
 if(wcsstr(mode,L"page")){cache_page_case(s,aid,apreview);return;}
 const char *ids[]={"ai-summary-input","ai-summary-history","ai-summary-fixed","ai-summary-policy"};char *as[4],*bs[4];int detail=wcsstr(mode,L"clear")!=NULL||wcsstr(mode,L"append")!=NULL,back=wcsstr(mode,L"back")!=NULL,status=wcsstr(mode,L"status")!=NULL;
 text(s,"ai-input","CACHE-DRAFT-未提交");cache_project(s,NULL);char *astate=_strdup(s->ai_state_sent),*draft=_strdup(s->ai_draft);for(unsigned i=0;i<4;++i)as[i]=cache_text(s,ids[i]);
 /* Service serializes waiting tasks. Retain the actual A display, cancel A,
  * then obtain B. Replays below test only page rendering, never old authority. */
 CHECK(ui_ai_cancel(s->ai,aid)==UI_STATUS_OK);CHECK(model_wait(s,"\"outcome\":\"cancelled\""));cache_project(s,NULL);char *cancelled=_strdup(s->ai_state_sent),*cancelstate=cache_text(s,"ai-task-state");
 char long_input[1401];memset(long_input,'B',1400);memcpy(long_input,"CACHE-TASK-B:",13);long_input[1400]=0;text(s,"ai-input",long_input);click(s,"ai-model-propose");CHECK(model_wait(s,"\"phase\":3"));CHECK(bound(s));char bid[65];strcpy_s(bid,65,s->ai_run);CHECK(strcmp(aid,bid)!=0);char *bp=_strdup(s->ai_preview);CHECK(bp!=NULL);if(!bp){free(astate);free(draft);free(cancelled);free(cancelstate);for(unsigned i=0;i<4;++i)free(as[i]);return;}
 text(s,"ai-input","CACHE-DRAFT-未提交");cache_project(s,NULL);char *bstate=_strdup(s->ai_state_sent);for(unsigned i=0;i<4;++i)bs[i]=cache_text(s,ids[i]);
 if(wcsstr(mode,L"parent")){
  /* Two actual terminal records have no preview object. This isolates the
   * parent summary key rather than relying on detail reparsing to refresh it. */
  cache_project(s,aid);free(astate);astate=_strdup(s->ai_state_sent);for(unsigned i=0;i<4;++i){free(as[i]);as[i]=cache_text(s,ids[i]);}
  char seed[65]={0};EnterCriticalSection(&s->ai->lock);for(unsigned i=0;i<AI_RUNS;++i)if(!strcmp(s->ai->runs[i].client,"summary-seed-a"))strcpy_s(seed,65,s->ai->runs[i].id);LeaveCriticalSection(&s->ai->lock);CHECK(seed[0]);cache_project(s,seed);free(bstate);bstate=_strdup(s->ai_state_sent);for(unsigned i=0;i<4;++i){free(bs[i]);bs[i]=cache_text(s,ids[i]);}cache_project(s,bid);CHECK(s->ai_preview_ready);
 }
 const char *point=detail?(wcsstr(mode,L"clear")?"clear":"append"):(wcsstr(mode,L"pending")||status)?"pending":"text";
 const char *node=detail?"":status?"ai-task-state":"ai-summary-input";
 uint64_t old=s->ai_model_bindings[1];CHECK(old!=0);++s->ai->methods;cache_complete(s,astate);cache_state_details(s,astate);
 cache_arm(s,point,node);
 for(unsigned attempt=0;attempt<3;++attempt){
  int rendered=cache_replay(s,status?cancelled:bstate);
  if(shown(s,"summary-fault-fired",point))break;
  CHECK(!rendered&&!s->ai_display_valid);
  CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);
  cache_zero(s);
 }
 CHECK(shown(s,"summary-fault-fired",point));CHECK(!s->ai_display_valid);ai_model_panel_bindings(s);CHECK(s->ai_model_bindings[1]==old);CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);cache_zero(s);
 const char *want=back?astate:status?cancelled:bstate;cache_complete(s,want);
 if(status)cache_same(s,"ai-task-state",cancelstate);
 else{for(unsigned i=0;i<4;++i)cache_same(s,ids[i],back?as[i]:bs[i]);cache_state_details(s,want);CHECK(shown(s,"ai-task-state",back?aid:bid));cache_same(s,"ai-summary-input-page",back?"展示 1 / 1":"展示 1 / 2");}
 if(back){cache_complete(s,bstate);for(unsigned i=0;i<4;++i)cache_same(s,ids[i],bs[i]);cache_state_details(s,bstate);}
 cache_zero(s);--s->ai->methods;
 /* Return to a fresh authoritative B projection before any positive fixture.
  * Captured A and cancelled records can never revive A's authority. */
 ai_projection_invalidate(s);cache_project(s,NULL);CHECK(bound(s));cache_details(s,bp);CHECK(shown(s,"ai-task-state",bid));CHECK(shown(s,"ai-input","CACHE-DRAFT-未提交"));CHECK(s->ai_draft&&draft&&!strcmp(s->ai_draft,draft));cache_zero(s);cache_project(s,NULL);cache_details(s,bp);CHECK(s->ai_preview_ready);
 if(!failures){CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")==UI_STATUS_OK);CHECK(ui_ai_model_fixture_light(s->ai,s->view,"ai-model-confirm")!=UI_STATUS_OK);CHECK(model_wait(s,"\"terminal\":true"));CHECK(s->ai->model_config->requests==1);}
 for(unsigned i=0;i<4;++i){free(as[i]);free(bs[i]);}free(astate);free(bstate);free(cancelled);free(cancelstate);free(bp);free(draft);
}

#ifndef SUMMARY_MAIN
#define SUMMARY_MAIN wmain
#endif
int SUMMARY_MAIN(int argc,wchar_t **argv)
{
 if(argc!=4)return 2;DWORD test_begin=GetTickCount();host_window_t s={0};host_action_t a={0};GUID g;wchar_t db[4096],u[40];int large=!wcscmp(argv[3],L"long")||!wcscmp(argv[3],L"utf8")||!wcscmp(argv[3],L"cache-page"),persisted=!wcscmp(argv[3],L"persisted");
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CoCreateGuid(&g);StringFromGUID2(&g,u,40);_snwprintf_s(db,4096,_TRUNCATE,L"%s/summary-%s.db",argv[2],u);CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);summary_interrupt=!wcscmp(argv[3],L"interrupted");summary_cache=!wcsncmp(argv[3],L"cache-",6);start_page(&s);text(&s,"ai-model-name","offline-fixture");if(!UI_MODEL_DISPATCH_GUARD(&s)){CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);return 1;}click(&s,"ai-model-save");if(!UI_MODEL_DISPATCH_GUARD(&s)){CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);return 1;}CHECK(model_wait(&s,NULL));{yyjson_doc *d=yyjson_read(s.ai_model,strlen(s.ai_model),0);const char *model=yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"model_id"));model_trace("config-readback",model,"initial");CHECK(model&&!strcmp(model,"offline-fixture"));yyjson_doc_free(d);}response(&s,0);seed_summary(&s,large,!wcscmp(argv[3],L"utf8")||!wcscmp(argv[3],L"cache-page"));if(!UI_MODEL_DISPATCH_GUARD(&s)){CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);return 1;}
 open_path(&s,argv[1]);pump(&s);uint64_t first=ui_workspace_active(s.workspace);open_path(&s,argv[1]);pump(&s);uint64_t second=ui_workspace_active(s.workspace);CHECK(first&&second&&first!=second);CHECK(ui_workspace_activate(s.workspace,first)==UI_STATUS_OK);pump(&s);
 if(summary_cache)cache_stage(&s,"before-catalog");click(&s,"ai-model-catalog");if(!s.ai_choices){CHECK(s.ai_choices!=NULL);if(summary_cache)cache_stage(&s,"catalog-failed");CHECK(close_host(&s));return 1;}click(&s,"ai-resource-select-0");int ix=cap_index(&s);CHECK(ix>=0);char node[64];snprintf(node,64,"ai-cap-select-%d",ix);click(&s,node);snprintf(node,64,"ai-field-select-%d-0",ix);click(&s,node);snprintf(node,64,"ai-field-select-%d-1",ix);click(&s,node);
 if(large){click(&s,"ai-history-select-1");click(&s,"ai-history-select-3");}else click(&s,"ai-history-select-0");text(&s,"ai-input","当前任务输入：仅所选只读范围。");click(&s,"ai-model-propose");CHECK(model_wait(&s,"\"phase\":3"));CHECK(bound(&s));char id[65];strcpy_s(id,65,s.ai_run);char *expected=_strdup(s.ai_preview);CHECK(expected!=NULL);{yyjson_doc *d=yyjson_read(expected,strlen(expected),0);const char *model=yyjson_get_str(yyjson_obj_get(yyjson_obj_get(yyjson_doc_get_root(d),"request"),"model"));model_trace("task-fixed",model,id);CHECK(model&&!strcmp(model,"offline-fixture"));yyjson_doc_free(d);}
 CHECK(shown(&s,"ai-task-title","当前任务与处理状态"));CHECK(shown(&s,"ai-scope-title","本次允许发送的内容"));CHECK(shown(&s,"ai-fixed-title","已接纳任务的固定目标"));CHECK(shown(&s,"ai-detail-title","完整请求详情"));CHECK(shown(&s,"ai-diagnostic-title","脱敏诊断"));
 /* Fail boundedly on the old page; do not drive missing controls. */
 if(failures){free(expected);CHECK(close_host(&s));printf("summary %ls checks=%u failures=%u\n",argv[3],checks,failures);return 1;}
 if(summary_cache){cache_cases(&s,argv[3],id,expected);free(expected);CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);printf("cache %ls checks=%u failures=%u; isolated only\n",argv[3],checks,failures);return failures?1:0;}
 if(summary_interrupt)CHECK(shown(&s,"summary-fault-fired","once"));CHECK(shown(&s,"ai-task-state","来源：内存"));CHECK(shown(&s,"ai-task-state",id));CHECK(shown(&s,"ai-task-state","等待外发确认"));CHECK(shown(&s,"ai-task-state","持久化接纳：是"));CHECK(shown(&s,"ai-task-state","模型请求保存：否"));CHECK(shown(&s,"ai-summary-input","当前任务输入：仅所选只读范围。"));
 CHECK(shown(&s,"ai-summary-fixed","memory.document"));CHECK(shown(&s,"ai-summary-fixed","r1"));CHECK(shown(&s,"ai-summary-fixed","read"));CHECK(shown(&s,"ai-summary-policy","memory.read"));CHECK(shown(&s,"ai-summary-policy","revision"));CHECK(shown(&s,"ai-summary-policy","value"));CHECK(shown(&s,"ai-summary-policy","offline-fixture"));CHECK(shown(&s,"ai-summary-policy","https://api.deepseek.com"));CHECK(shown(&s,"ai-summary-policy","配置代次：1"));
 yyjson_doc *d=yyjson_read(expected,strlen(expected),0);yyjson_val *request=yyjson_obj_get(yyjson_doc_get_root(d),"request"),*input=yyjson_obj_get(request,"input");CHECK(yyjson_arr_size(input)==(large?4u:3u));CHECK(yyjson_arr_size(yyjson_obj_get(request,"tools"))==2);CHECK(!strstr(expected,"UNSELECTED-HISTORY-B"));
 json_buffer_t history_text={0};json_append(&history_text,large?"所选历史序号：2、4\n以下按实际请求顺序列出所选历史全文：\n":"所选历史序号：1\n以下按实际请求顺序列出所选历史全文：\n");for(unsigned i=0;i<(large?2u:1u);++i){yyjson_val *v=yyjson_arr_get(input,i);json_format(&history_text,"片段 %u · %s\n",i+1,yyjson_get_str(yyjson_obj_get(v,"role")));json_append(&history_text,yyjson_get_str(yyjson_obj_get(v,"content")));json_append(&history_text,"\n");}CHECK(!history_text.failed);printf("summary prepared elapsed=%lu ms\n",GetTickCount()-test_begin);fflush(stdout);click(&s,"ai-detail-toggle");CHECK(!s.ai_preview_ready&&s.ai->model_config->requests==0);paged_equal(&s,"ai-summary-history",history_text.text);free(history_text.text);printf("summary pages elapsed=%lu ms\n",GetTickCount()-test_begin);fflush(stdout);click(&s,"ai-detail-toggle");CHECK(bound(&s)&&s.ai_preview_ready);
 if(large){CHECK(strlen(expected)>100000&&strlen(expected)<128u*1024u);for(unsigned i=0;i<2;++i)CHECK(strlen(yyjson_get_str(yyjson_obj_get(yyjson_arr_get(input,i),"content")))==50000);CHECK(shown(&s,"ai-summary-history",!wcscmp(argv[3],L"utf8")?"汉😀":"AAAA"));CHECK(shown(&s,"ai-summary-history",!wcscmp(argv[3],L"utf8")?"汉😀":"AAAA"));CHECK(s.ai->bytes<=UI_AI_MAX_BYTES);}
 else{CHECK(shown(&s,"ai-summary-history","SELECTED-HISTORY-A"));ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s.view,"ai-summary-history",&p)==UI_STATUS_OK&&!strstr(p.text_utf8,"UNSELECTED-HISTORY-B"));}
 /* Every target component and the schema selection comes from this request. */
 yyjson_doc *td=yyjson_read(yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"fixed_targets")),strlen(yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"fixed_targets"))),0);yyjson_val *t=yyjson_arr_get(yyjson_doc_get_root(td),0);const char *keys[]={"instance_id","resource_id","host_epoch","open_epoch","revision","scope"};for(unsigned i=0;i<6;++i)CHECK(shown(&s,"ai-summary-fixed",yyjson_get_str(yyjson_obj_get(t,keys[i]))));yyjson_doc_free(td);
 yyjson_doc_free(d);
 /* An active tab or an edited draft cannot change the admitted task. */
 text(&s,"ai-input","NEW-DRAFT-NOT-TASK");CHECK(ui_workspace_activate(s.workspace,second)==UI_STATUS_OK);pump(&s);CHECK(shown(&s,"ai-summary-input","当前任务输入：仅所选只读范围。"));CHECK(shown(&s,"ai-summary-fixed","实例：1"));CHECK(shown(&s,"ai-task-state",id));CHECK(shown(&s,"ai-input","NEW-DRAFT-NOT-TASK"));
 if(!wcscmp(argv[3],L"summary")){CHECK(MoveWindow(s.hwnd,-16000,-16000,1200,1100,FALSE));pump(&s);click(&s,"ai-task-title");summary_capture(&s,argv[2],"task-state");click(&s,"ai-view-scope");ui_element_presentation_t title={0};title.size=sizeof(title);CHECK(ui_web_view_get_presentation(s.view,"ai-scope-title",&title)==UI_STATUS_OK&&title.visible);CHECK(ui_web_view_get_presentation(s.view,"ai-summary-input",&title)==UI_STATUS_OK&&title.visible);CHECK(ui_web_view_get_presentation(s.view,"ai-summary-fixed",&title)==UI_STATUS_OK&&title.visible);CHECK(s.ai->model_config->requests==0);summary_capture(&s,argv[2],"scope-summary");CHECK(bound(&s));}
 if(!wcscmp(argv[3],L"gate")){
  uint64_t old=s.ai_model_bindings[1];++s.ai->methods;click(&s,"ai-detail-toggle");CHECK(!s.ai_preview_ready&&s.ai_model_bindings[1]==old);CHECK(shown(&s,"ai-scope-title","本次允许发送的内容"));ai_model_panel_bindings(&s);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);--s.ai->methods;CHECK(s.ai->model_config->requests==0);click(&s,"ai-detail-toggle");CHECK(bound(&s)&&s.ai_preview_ready);
  char *page=load_page();CHECK(ui_web_view_load_html(s.view,page)==UI_STATUS_OK);free(page);schedule_refresh(&s);pump(&s);CHECK(!s.ai_preview_ready);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);click(&s,"ai-view-scope");CHECK(bound(&s));
 }
 if(persisted){CHECK(ui_ai_cancel(s.ai,id)==UI_STATUS_OK);CHECK(model_wait(&s,"\"outcome\":\"cancelled\""));CHECK(close_host(&s));memset(&s,0,sizeof(s));start_page(&s);a.action="ai-select";a.id=id;process_action(&s,&a);DWORD begin=GetTickCount();do{pump(&s);if(s.ai_stored_run)break;}while(GetTickCount()-begin<5000);CHECK(s.ai_stored_run!=NULL);{yyjson_doc *trace_doc=yyjson_read(s.ai_stored_run,strlen(s.ai_stored_run),0);const char *plan=yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(trace_doc),"stub_plan"));yyjson_doc *p=plan?yyjson_read(plan,strlen(plan),0):NULL;const char *model=yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(p),"model"));model_trace("restored-task",model,id);CHECK(model&&!strcmp(model,"offline-fixture"));yyjson_doc_free(p);yyjson_doc_free(trace_doc);}pump(&s);CHECK(shown(&s,"ai-task-state","来源：持久化快照"));CHECK(shown(&s,"ai-task-state","cancelled"));CHECK(shown(&s,"ai-task-state","终态保存：是"));CHECK(shown(&s,"ai-task-state","模型请求保存：未记录"));CHECK(shown(&s,"ai-summary-fixed","memory.document"));CHECK(shown(&s,"ai-summary-policy","offline-fixture"));CHECK(shown(&s,"ai-summary-history","未随任务快照恢复"));CHECK(!s.ai_preview_ready&&s.ai->model_config->requests==0);
  /* The page and copy diagnostic share the same asynchronous query identity. */
  char *saved=_strdup(s.ai_stored_run),old_query[65],other[65];strcpy_s(old_query,65,s.ai_query_id);memset(other,'b',64);other[64]=0;a.id=other;process_action(&s,&a);char *state=NULL;send_state(&s,&state);free(state);CHECK(shown(&s,"ai-task-state","来源：正在读取"));CHECK(shown(&s,"ai-summary-fixed","暂无匹配任务"));
  ui_ai_event_t event={0};event.type="query";event.query_id=old_query;event.payload=saved;ai_panel_event(&event,&s);CHECK(s.ai_query_pending&&!s.ai_stored_run);event.type="query_error";event.query_id=s.ai_query_id;event.payload="isolated summary query failure";ai_panel_event(&event,&s);state=NULL;send_state(&s,&state);free(state);CHECK(shown(&s,"ai-task-state","来源：读取失败"));CHECK(shown(&s,"ai-task-state","isolated summary query failure"));
  a.id=id;process_action(&s,&a);event.type="query";event.query_id=old_query;event.payload=saved;ai_panel_event(&event,&s);event.type="query_error";event.payload="stale failure";ai_panel_event(&event,&s);CHECK(s.ai_query_pending&&!s.ai_stored_run);state=NULL;send_state(&s,&state);free(state);CHECK(shown(&s,"ai-task-state","来源：正在读取"));begin=GetTickCount();do{pump(&s);if(s.ai_stored_run)break;}while(GetTickCount()-begin<5000);CHECK(s.ai_stored_run!=NULL);{yyjson_doc *trace_doc=yyjson_read(s.ai_stored_run,strlen(s.ai_stored_run),0);const char *plan=yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(trace_doc),"stub_plan"));yyjson_doc *p=plan?yyjson_read(plan,strlen(plan),0):NULL;const char *model=yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(p),"model"));model_trace("restored-task",model,id);CHECK(model&&!strcmp(model,"offline-fixture"));yyjson_doc_free(p);yyjson_doc_free(trace_doc);}pump(&s);CHECK(shown(&s,"ai-task-state","来源：持久化快照"));free(saved);
  text(&s,"ai-model-name","new-config-not-task");click(&s,"ai-model-save");CHECK(model_wait(&s,NULL));CHECK(shown(&s,"ai-summary-policy","offline-fixture"));CHECK(shown(&s,"ai-summary-policy","配置代次：1"));begin=GetTickCount();ui_element_presentation_t cp={0};cp.size=sizeof(cp);do{pump(&s);if(ui_web_view_get_presentation(s.view,"ai-model-state",&cp)==UI_STATUS_OK&&strstr(cp.text_utf8,"new-config-not-task"))break;}while(GetTickCount()-begin<5000);CHECK(shown(&s,"ai-model-state","new-config-not-task"));char *diagnostic=ai_model_diagnostic_json(&s);CHECK(diagnostic&&strstr(diagnostic,"\"task_source\":\"persisted\"")&&strstr(diagnostic,id)&&!strstr(diagnostic,"uha-secret-sentinel"));free(diagnostic);
 }else{CHECK(s.ai->model_config->requests==0);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_OK);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);CHECK(model_wait(&s,"\"terminal\":true"));yyjson_doc *e=yyjson_read(expected,strlen(expected),0),*b=yyjson_read(s.ai->model_config->fixture_body,strlen(s.ai->model_config->fixture_body),0);char *x=yyjson_val_write(yyjson_obj_get(yyjson_doc_get_root(e),"request"),0,NULL),*y=yyjson_val_write(yyjson_doc_get_root(b),0,NULL);CHECK(x&&y&&!strcmp(x,y));free(x);free(y);yyjson_doc_free(e);yyjson_doc_free(b);CHECK(s.ai->model_config->requests==1);}
 free(expected);CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);printf("summary %ls checks=%u failures=%u; isolated only\n",argv[3],checks,failures);return failures?1:0;
}
