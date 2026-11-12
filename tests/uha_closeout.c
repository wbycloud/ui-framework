/* Isolated controlled timing; no production source or network qualification. */
#include <string.h>
#include <stdlib.h>
#include "ui_framework/light_web.h"
static int post_fail,post_mode;static size_t post_max;static unsigned post_count;static char *last_fragment;
static ui_status_t closeout_post(ui_web_view_t *v,const char *j)
{size_t n=strlen(j);if(n>post_max)post_max=n;++post_count;if(post_fail)return UI_STATUS_LIMIT_EXCEEDED;
 if(strstr(j,"\"type\":\"projection\"")){
  int fragment=strstr(j,"\"index\":1,")!=NULL;
  if(post_mode==1&&fragment)return UI_STATUS_OK;
  if(post_mode==2&&fragment){ui_status_t st=ui_web_view_post_json(v,j);return st?st:ui_web_view_post_json(v,j);}
  if(post_mode==3&&strstr(j,"\"index\":0,")){char *copy=_strdup(j),*index=strstr(copy,"\"index\":0,");if(index)index[8]='1';ui_status_t st=ui_web_view_post_json(v,copy);free(copy);return st;}
  free(last_fragment);last_fragment=_strdup(j);
 }
 return ui_web_view_post_json(v,j);}
static int projection_fault;
#define UI_UHA_PROJECTION_PROBE(stage) (projection_fault==(stage))
#define ui_web_view_post_json closeout_post
#define UHA_MAIN previous_uha_main
#include "uha_host.c"
#undef wmain
#undef ui_web_view_post_json
static int wait_stored(host_window_t *s)
{DWORD begin=GetTickCount();do{pump(s);if(s->ai_stored_run&&strstr(s->ai_stored_run,s->ai_run))return 1;}while(GetTickCount()-begin<5000);return 0;}
static void seed(host_window_t *s,unsigned count,int large,char first[65])
{int was_refreshing=s->refreshing;s->refreshing=1;DWORD seed_begin=GetTickCount();char *value=malloc(50001);memset(value,'A',50000);value[50000]=0;if(large==2){for(unsigned i=0;i<7142;++i)memcpy(value+7*i,"汉😀",7);}
 for(unsigned i=0;i<count;i++){json_buffer_t plan={0};char client[65];snprintf(client,65,"seed-%u",i);json_append(&plan,"{\"steps\":[{\"type\":\"text\",\"text\":");json_string(&plan,large?value:"saved synthetic response");json_append(&plan,"}]}");
  ui_ai_task_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id=client;q.input_utf8="synthetic seed";q.stub_plan_json=plan.text;CHECK(ui_ai_submit(s->ai,&q,s->ai_run)==UI_STATUS_OK);free(plan.text);if(!i)strcpy_s(first,65,s->ai_run);CHECK(model_wait(s,"\"terminal\":true"));}
 free(value);s->refreshing=was_refreshing;schedule_refresh(s);printf("seeded=%u elapsed_ms=%lu; real cache limit=%d\n",count,GetTickCount()-seed_begin,AI_RUNS);fflush(stdout);
}
static host_window_t *callback_host;static int revision_mode,callback_pending,callback_methods;
static ui_status_t reentrant_revision(char *out,size_t capacity,void *data)
{
 (void)data;host_window_t *h=callback_host;int mode=revision_mode;revision_mode=0;
 if(mode){callback_methods=h->ai->methods>0;if(mode==1){post_fail=1;char *state=NULL;send_state(h,&state);free(state);callback_pending=ui_ai_model_unbind(h->ai,h->ai_model_bindings[1])==UI_STATUS_PENDING;post_fail=0;}else{CHECK(ui_ai_service_request_close(h->ai)==UI_STATUS_OK);callback_pending=ui_ai_service_destroy(h->ai)==UI_STATUS_PENDING;}}
 return strcpy_s(out,capacity,"r1")?UI_STATUS_INVALID_ARGUMENT:UI_STATUS_OK;
}
static char *diagnostic(host_window_t *h,const char *source)
{char *j=ai_model_diagnostic_json(h);CHECK(j!=NULL);yyjson_doc *d=j?yyjson_read(j,strlen(j),0):NULL;const char *v=d?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"task_source")):NULL;CHECK(v&&!strcmp(v,source));yyjson_doc_free(d);return j;}
static void request_equal(host_window_t *h,const char *expected)
{yyjson_doc *p=yyjson_read(expected,strlen(expected),0),*b=yyjson_read(h->ai->model_config->fixture_body,strlen(h->ai->model_config->fixture_body),0);char *x=p?yyjson_val_write(yyjson_obj_get(yyjson_doc_get_root(p),"request"),0,NULL):NULL,*y=b?yyjson_val_write(yyjson_doc_get_root(b),0,NULL):NULL;CHECK(x&&y&&!strcmp(x,y));free(x);free(y);yyjson_doc_free(p);yyjson_doc_free(b);}
int wmain(int argc,wchar_t **argv)
{
 if(argc!=4)return 2;host_window_t s={0};host_action_t a={0};GUID g;wchar_t db[4096],u[40];char first[65]={0};
 CHECK(CreateDirectoryW(argv[2],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CoCreateGuid(&g);StringFromGUID2(&g,u,40);_snwprintf_s(db,4096,_TRUNCATE,L"%s/closeout-%s.db",argv[2],u);CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.ai)return 1;
 CHECK(model_wait(&s,NULL));MoveWindow(s.hwnd,-16000,-16000,1200,3200,FALSE);ShowWindow(s.hwnd,SW_HIDE);a.action="assistant";process_action(&s,&a);pump(&s);click(&s,"ai-model-toggle");click(&s,"ai-draft-range-toggle");text(&s,"ai-model-name","offline-fixture");click(&s,"ai-model-save");CHECK(model_wait(&s,NULL));response(&s,0);
 if(!wcscmp(argv[3],L"stored")||!wcscmp(argv[3],L"restart")||!wcscmp(argv[3],L"failed")){
  int failed=!wcscmp(argv[3],L"failed"),restart=failed||!wcscmp(argv[3],L"restart");seed(&s,restart?1:65,0,first);if(failed){ui_ai_task_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="failed-snapshot";q.input_utf8="synthetic failure";q.stub_plan_json="{\"steps\":[{\"type\":\"fail\"}]}";CHECK(ui_ai_submit(s.ai,&q,s.ai_run)==UI_STATUS_OK);strcpy_s(first,65,s.ai_run);CHECK(model_wait(&s,"\"outcome\":\"failed\""));}char *live=NULL;
  if(restart){CHECK(close_host(&s));memset(&s,0,sizeof(s));CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);CHECK(model_wait(&s,NULL));MoveWindow(s.hwnd,-16000,-16000,1200,3200,FALSE);ShowWindow(s.hwnd,SW_HIDE);}
  if(!restart){unsigned occupied=0;EnterCriticalSection(&s.ai->lock);for(unsigned ix=0;ix<AI_RUNS;++ix)if(s.ai->runs[ix].id[0])++occupied;LeaveCriticalSection(&s.ai->lock);CHECK(occupied==64&&AI_RUNS==64);}
  CHECK(ui_ai_get_run(s.ai,first,&live)==UI_STATUS_NOT_FOUND);ui_ai_release(live);a.action="ai-select";a.id=first;process_action(&s,&a);char first_query[65];strcpy_s(first_query,65,s.ai_query_id);char *j=diagnostic(&s,"pending");free(j);CHECK(wait_stored(&s));j=diagnostic(&s,"persisted");CHECK(j&&strstr(j,failed?"stub_disconnect":"completed")&&strstr(j,"\"committed\":true")&&strstr(j,"\"error_recorded\":true"));free(j);
  char *saved=_strdup(s.ai_stored_run);char other[65];memset(other,'b',64);other[64]=0;a.id=other;process_action(&s,&a);j=diagnostic(&s,"pending");CHECK(j&&!strstr(j,first));free(j);
  ui_ai_event_t event={0};event.type="query";event.query_id=first_query;event.payload=saved;ai_panel_event(&event,&s);CHECK(!s.ai_stored_run);j=diagnostic(&s,"pending");free(j);event.type="query_error";event.query_id=s.ai_query_id;event.payload="isolated query failure";ai_panel_event(&event,&s);j=diagnostic(&s,"error");CHECK(j&&strstr(j,"isolated query failure"));free(j);
  event.type="query";event.payload=saved;ai_panel_event(&event,&s);CHECK(!s.ai_stored_run&&strstr(s.ai_query_error,"身份"));
  a.id=first;process_action(&s,&a);CHECK(strcmp(first_query,s.ai_query_id)!=0);event.type="query";event.query_id=first_query;event.payload=saved;ai_panel_event(&event,&s);CHECK(!s.ai_stored_run&&s.ai_query_pending);CHECK(wait_stored(&s));free(saved);CHECK(s.ai->model_config->requests==0);
 }else{
  char selection[100]="[]";
  if(!wcscmp(argv[3],L"long")||!wcscmp(argv[3],L"utf8")){seed(&s,2,!wcscmp(argv[3],L"utf8")?2:1,first);DWORD start=GetTickCount();do{pump(&s);if(s.ai_messages&&strlen(s.ai_messages)>100000)break;}while(GetTickCount()-start<5000);CHECK(s.ai_messages&&strlen(s.ai_messages)>100000);yyjson_doc *d=yyjson_read(s.ai_messages,strlen(s.ai_messages),0);size_t i,n;yyjson_val *v;uint64_t ids[2]={0};unsigned at=0;yyjson_arr_foreach(yyjson_obj_get(yyjson_doc_get_root(d),"items"),i,n,v){const char *c=yyjson_get_str(yyjson_obj_get(v,"content"));if(c&&strlen(c)==50000&&at<2)ids[at++]=yyjson_get_uint(yyjson_obj_get(v,"sequence"));}CHECK(at==2);snprintf(selection,100,"[%llu,%llu]",(unsigned long long)ids[0],(unsigned long long)ids[1]);yyjson_doc_free(d);}
  if(!wcscmp(argv[3],L"reentry")){open_path(&s,argv[1]);pump(&s);ui_app_instance_info_t app={0};app.size=sizeof(app);CHECK(ui_workspace_get_instance(s.workspace,ui_workspace_active(s.workspace),&app)==UI_STATUS_OK);ui_cap_resource_desc_t resource={0};resource.size=sizeof(resource);resource.version=1;resource.kind=UI_CAP_DOCUMENT;resource.id="closeout.resource";resource.get_revision=reentrant_revision;CHECK(ui_capability_register_resource(app.host,&resource)==UI_STATUS_OK);callback_host=&s;}
  char params[256];snprintf(params,sizeof(params),"{\"history\":\"%s\",\"projection\":\"{\\\"capabilities\\\":[],\\\"fields\\\":[]}\"}",selection);a.action="ai-model-propose";a.token="closeout-task";a.answer="Synthetic current input";a.params=params;if(!wcscmp(argv[3],L"reentry")){static char instance[32];snprintf(instance,32,"%llu",(unsigned long long)ui_workspace_active(s.workspace));a.id=instance;a.panel="closeout.resource";}process_action(&s,&a);CHECK(model_wait(&s,"\"phase\":3"));click(&s,"ai-view-scope");CHECK(bound(&s));
  if(!wcscmp(argv[3],L"faults")){
   for(projection_fault=1;projection_fault<=4;++projection_fault){char *expected=_strdup(s.ai_preview);++s.ai->methods;if(projection_fault==3)ai_projection_invalidate(&s);if(projection_fault==1)CHECK(!ai_model_preview_set(&s,expected));else{char *state=NULL;send_state(&s,&state);free(state);}ai_model_panel_bindings(&s);CHECK(!s.ai_preview_ready);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);--s.ai->methods;free(expected);CHECK(s.ai->model_config->requests==0);int saved=projection_fault;projection_fault=0;schedule_refresh(&s);pump(&s);CHECK(bound(&s)&&s.ai_preview_ready);projection_fault=saved;}
   projection_fault=0;CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_OK);CHECK(model_wait(&s,"\"outcome\":\"completed\""));CHECK(s.ai->model_config->requests==1);CHECK(s.ai->bytes<=UI_AI_MAX_BYTES);
  }else if(!wcscmp(argv[3],L"reentry")){
   revision_mode=1;CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);CHECK(callback_pending&&callback_methods&&!s.ai_preview_ready&&s.ai->model_config->requests==0);pump(&s);CHECK(bound(&s));revision_mode=2;CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);CHECK(callback_pending&&callback_methods&&s.ai->model_config->requests==0);
  }else if(!wcscmp(argv[3],L"post")){
   uint64_t b=s.ai_model_bindings[1],generation=s.ai_display_generation;CHECK(b&&s.ai_preview_ready);++s.ai->methods;post_fail=1;char *state=NULL;send_state(&s,&state);free(state);CHECK(!s.ai_preview_ready);CHECK(s.ai_model_bindings[1]==b);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);--s.ai->methods;CHECK(s.ai->model_config->requests==0);post_fail=0;pump(&s);CHECK(bound(&s)&&s.ai_preview_ready);CHECK(ai_model_display_check(&s,s.ai_run,s.ai_preview_hash,generation)==UI_STATUS_PERMISSION_DENIED);
   for(post_mode=1;post_mode<=3;++post_mode){ai_projection_invalidate(&s);char *state2=NULL;send_state(&s,&state2);free(state2);CHECK(!s.ai_preview_ready&&s.ai->model_config->requests==0);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);}post_mode=0;schedule_refresh(&s);pump(&s);CHECK(bound(&s)&&s.ai_preview_ready);
   char *stale=last_fragment?_strdup(last_fragment):NULL;uint64_t stable=s.ai_display_generation;for(unsigned i=0;i<5;++i){char *state3=NULL;send_state(&s,&state3);free(state3);}CHECK(s.ai_display_generation==stable&&s.ai_preview_ready);if(stale){CHECK(ui_web_view_post_json(s.view,stale)==UI_STATUS_OK);free(stale);}CHECK(ai_model_preview_displayed(&s));CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_OK);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);CHECK(model_wait(&s,"\"outcome\":\"completed\""));CHECK(s.ai->model_config->requests==1);CHECK(s.ai->bytes<=UI_AI_MAX_BYTES);
  }else{
   CHECK(post_max<=256u*1024u);CHECK(s.ai_preview_ready);char *p=NULL;CHECK(ui_ai_model_get_disclosure(s.ai,s.ai_run,&p)==UI_STATUS_OK);CHECK(p&&strlen(p)>100000&&strlen(p)<128u*1024u);CHECK(s.ai->model_config->requests==0);
   yyjson_doc *d=yyjson_read(p,strlen(p),0);yyjson_val *input=yyjson_obj_get(yyjson_obj_get(yyjson_doc_get_root(d),"request"),"input");CHECK(yyjson_arr_size(input)==4);for(unsigned i=0;i<2;++i){yyjson_val *v=yyjson_arr_get(input,i);const char *c=yyjson_get_str(yyjson_obj_get(v,"content"));CHECK(c&&strlen(c)==50000);if(c){CHECK(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,c,-1,NULL,0)>0);CHECK(!wcscmp(argv[3],L"long")?strspn(c,"A")==50000:strstr(c,"汉😀")!=NULL);}}yyjson_doc_free(d);
   char *page=load_page();CHECK(page&&ui_web_view_load_html(s.view,page)==UI_STATUS_OK);free(page);schedule_refresh(&s);pump(&s);CHECK(!s.ai_preview_ready);CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")!=UI_STATUS_OK);CHECK(s.ai->model_config->requests==0);click(&s,"ai-view-scope");CHECK(bound(&s)&&s.ai_preview_ready&&ai_model_preview_displayed(&s));CHECK(ui_ai_model_fixture_light(s.ai,s.view,"ai-model-confirm")==UI_STATUS_OK);CHECK(model_wait(&s,"\"outcome\":\"completed\""));request_equal(&s,p);ui_ai_release(p);CHECK(s.ai->model_config->requests==1);CHECK(s.ai->bytes<=UI_AI_MAX_BYTES);
  }
 }
 printf("controlled case=%ls checks=%u failures=%u max_packet=%zu posts=%u\n",argv[3],checks,failures,post_max,post_count);CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);free(last_fragment);last_fragment=NULL;return failures?1:0;
}
