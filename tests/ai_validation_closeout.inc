/* Isolated CTest processes; no production authorization or transport changes. */
static unsigned vr_mode,vr_received,vr_attempts;
static int vr_refs(void)
{int total=0;for(unsigned i=0;i<AI_RUNS;++i)total+=s->runs[i].refs;return total;}
static void vr_call(const ui_ai_event_t *e,const char *client,int continuing,ui_status_t expected)
{
 ui_ai_real_task_t q={0};char out[65];q.size=sizeof(q);q.version=1;q.client_request_id=client;q.input_utf8="Invalid nested request must not execute";q.profile_id="deepseek";q.profile_revision=s->model_config->revision;q.history_selection_json="[]";q.disclosure_projection_json="{\"capabilities\":[],\"fields\":[]}";
 EnterCriticalSection(&s->lock);int methods=s->methods,refs=vr_refs();CHECK(s->validation!=NULL);CHECK(methods>0);CHECK(s->callbacks>0);
 printf("received nested %s client=%s; expecting=%d; before comparison\n",continuing?"continue":"propose",client?client:"<NULL>",expected);fflush(stdout);++vr_attempts;
 ui_status_t status=continuing?ui_ai_real_continue(s,e->run_id,&q,out):ui_ai_real_propose(s,&q,out);
 CHECK(status==expected);CHECK(!out[0]);CHECK(s->methods==methods);CHECK(vr_refs()==refs);CHECK(ui_ai_service_destroy(s)==UI_STATUS_PENDING);LeaveCriticalSection(&s->lock);
}
static void vr_event(const ui_ai_event_t *e,void *data)
{
 model_event(e,data);if(strcmp(e->type,"received")||strcmp(e->client_request_id,"vr-outer"))return;++vr_received;CHECK(vr_received==1);
 if(vr_mode==1)vr_call(e,NULL,0,UI_STATUS_INVALID_ARGUMENT);
 else if(vr_mode==2)vr_call(e,NULL,1,UI_STATUS_INVALID_ARGUMENT);
 else{char long_id[66];memset(long_id,'x',65);long_id[65]=0;const char *bad[]={"",long_id,"invalid client"};for(unsigned i=0;i<3;++i)for(int next=0;next<2;++next)vr_call(e,bad[i],next,UI_STATUS_INVALID_ARGUMENT);}
 for(int next=0;next<2;++next)vr_call(e,"valid-but-not-admitted",next,UI_STATUS_PERMISSION_DENIED);
}
static void vr_case(const wchar_t *mode)
{
 /* A broken implementation may fault: CTest owns this process and its 20s bound. */
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
 vr_mode=!wcscmp(mode,L"vr-null-propose")?1u:!wcscmp(mode,L"vr-null-continue")?2u:3u;vc_prepare(300,1200);vc_wires(rp_text,NULL,NULL,NULL);s->config.on_event=vr_event;
 CHECK(vc_submit(UI_AI_VALIDATION_TEXT,"vr-outer")==UI_STATUS_OK);CHECK(vr_received==1);CHECK(vr_attempts==(vr_mode==3?8u:3u));CHECK(run[0]);CHECK(s->methods==0);
 char original[65];strcpy_s(original,65,run);CHECK(real_wait("\"phase\":3"));CHECK(s->model_config->requests==0);CHECK(vc_state("\"reserved\":0"));CHECK(vc_submit(UI_AI_VALIDATION_TEXT,"vr-outer")==UI_STATUS_OK);CHECK(!strcmp(run,original));CHECK(vr_received==1);
 authorize();CHECK(real_wait("\"terminal\":true"));CHECK(strstr(state,"\"outcome\":\"completed\""));CHECK(s->model_config->requests==1);CHECK(vc_submit(UI_AI_VALIDATION_TEXT,"vr-outer")==UI_STATUS_OK);CHECK(!strcmp(run,original));CHECK(vr_received==1);CHECK(s->model_config->requests==1);
 vc_wires(rp_discover,rp_read,rp_text,NULL);CHECK(vc_submit(UI_AI_VALIDATION_READ,"vr-read")==UI_STATUS_OK);authorize();CHECK(real_wait("\"terminal\":true"));CHECK(strstr(state,"\"outcome\":\"completed\""));CHECK(s->model_config->requests==4);CHECK(value(0)==10&&vc_value==10);CHECK(s->methods==0);CHECK(!secret_leaked);
 close_service();CHECK(cr_sql("SELECT count(*) FROM runs")==2);CHECK(cr_sql("SELECT count(*) FROM events WHERE terminal=1")==2);CHECK(cr_sql("SELECT count(*) FROM tools WHERE capability='memory.read' AND status=0")==1);
 printf("validation reentry attempts=%u received=%u; external sends=0; safe close complete\n",vr_attempts,vr_received);
}
