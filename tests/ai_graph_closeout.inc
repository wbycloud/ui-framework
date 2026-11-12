/* P6R-1 controlled regressions. Only linked to the isolated fixture. */
#include "ai_edit_internal.h"
static const wchar_t *cr_package,*cr_directory;
static int cr_slot(const char *id)
{int slot=-1;EnterCriticalSection(&s->lock);for(int i=0;i<AI_RUNS;++i)if(!strcmp(s->runs[i].id,id)){slot=i;break;}LeaveCriticalSection(&s->lock);return slot;}
static void cr_idle(void)
{DWORD begin=GetTickCount();int idle;do{pump();EnterCriticalSection(&s->lock);idle=!s->cmd_count&&(!s->worker||WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0);LeaveCriticalSection(&s->lock);if(idle)return;}while(GetTickCount()-begin<5000);CHECK(0);}
static void cr_plain(unsigned i,int complete)
{char client[65];ui_ai_task_request_t q={0};snprintf(client,65,"closeout-%u",i);q.size=sizeof(q);q.version=1;q.client_request_id=client;q.input_utf8="Controlled ordinary cache record";q.stub_plan_json="{\"steps\":[]}";CHECK(ui_ai_submit(s,&q,run)==UI_STATUS_OK);DWORD begin=GetTickCount();do{int at; pump();at=cr_slot(run);if(at>=0){int terminal;EnterCriticalSection(&s->lock);terminal=s->runs[at].terminal;LeaveCriticalSection(&s->lock);if(terminal)break;}}while(GetTickCount()-begin<5000);if(complete){int at=cr_slot(run);CHECK(at>=0&&s->runs[at].terminal);}cr_idle();}
static void cr_fail_accept(const char *client)
{
 DWORD begin=GetTickCount();int exited;
 /* UI consumption is deliberately paused. Establish the idle-worker boundary
    before atomically publishing fault+command, rather than relying on poll to
    restart a worker whose exit was already decided. No worker waits on UI here. */
 do{EnterCriticalSection(&s->lock);exited=!s->worker||WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0;LeaveCriticalSection(&s->lock);if(exited)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);CHECK(exited);
 EnterCriticalSection(&s->lock);ui_ai_edit_fixture(s,5,SQLITE_IOERR_WRITE);
 {ui_ai_task_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id=client;q.input_utf8="Controlled isolated accept failure";q.stub_plan_json="{\"steps\":[]}";CHECK(ui_ai_submit(s,&q,run)==UI_STATUS_OK);}
 LeaveCriticalSection(&s->lock);
}
static void cr_pressure(void)
{
 char root[65];int at,done=0;unsigned occupied=0;DWORD begin;
 /* Persist real records beyond the production cache, then rehydrate only
    evicted identities while the older graph waits for a correlated answer. */
 for(unsigned i=0;i<64;++i)cr_plain(i,1);
 single("memory.read",",\"question\":\"hold for real cache pressure\"");CHECK(graph("\"state\":2"));strcpy_s(root,65,run);at=cr_slot(root);CHECK(at>=0);
 for(unsigned i=0;i<63;++i)cr_plain(i,1);
 strcpy_s(run,65,root);CHECK(ui_ai_graph_answer(s,root,"step","q-operation","pressure-answer","continue")==UI_STATUS_OK);
 begin=GetTickCount();do{pump();EnterCriticalSection(&s->lock);ai_graph_t *g=s->runs[at].graph;int queued=s->coordinator->job==4||s->coordinator->done==4||(s->coordinator->busy&&g&&g->persisted==g->version&&!g->dirty);int finishing=s->runs[at].graph&&s->runs[at].graph->finishing;LeaveCriticalSection(&s->lock);if(queued&&finishing){/* No more UI consumption once the final job is queued. */
  do{EnterCriticalSection(&s->lock);done=s->coordinator->done==4;LeaveCriticalSection(&s->lock);if(done)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);if(done)break;}
 }while(GetTickCount()-begin<5000);
 CHECK(done);EnterCriticalSection(&s->lock);for(int i=0;i<AI_RUNS;++i)if(s->runs[i].id[0])++occupied;int retained=s->runs[at].refs;LeaveCriticalSection(&s->lock);CHECK(occupied==AI_RUNS);CHECK(retained==1);
 /* Submit without polling: the done result must still own the root. */
 {ui_ai_task_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="pressure-new";q.input_utf8="force real eviction";q.stub_plan_json="{\"steps\":[]}";CHECK(ui_ai_submit(s,&q,run)==UI_STATUS_OK);}
 CHECK(cr_slot(root)==at);printf("P6R1 pressure occupied=%u root_refs=%d identity_retained=%d\n",occupied,retained,cr_slot(root)==at);fflush(stdout);
 if(cr_slot(root)!=at||!retained){printf("bounded red: skip unsafe stale result dereference\n");ExitProcess(1);}
 strcpy_s(run,65,root);CHECK(settled());cr_idle();CHECK(s->runs[at].refs==0&&s->runs[at].graph->finished);pump();CHECK(s->runs[at].refs==0);cr_plain(130,1);CHECK(value(0)==10);
}
static int cr_sql(const char *query)
{sqlite3 *db=NULL;sqlite3_stmt *q=NULL;int result=-1;CHECK(sqlite3_open_v2(dbpath,&db,SQLITE_OPEN_READONLY,NULL)==SQLITE_OK);CHECK(sqlite3_prepare_v2(db,query,-1,&q,NULL)==SQLITE_OK);if(sqlite3_step(q)==SQLITE_ROW)result=sqlite3_column_int(q,0);sqlite3_finalize(q);CHECK(sqlite3_close(db)==SQLITE_OK);return result;}
static void cr_checkpoint(void)
{
 unsigned next=0,before;DWORD begin;int failed=0;
 /* Real successful transactions, unchanged production period32. */
 while(s->store_commits%32u!=30u&&next<32)cr_plain(next++,1);
 CHECK(s->store_commits%32u==30u);before=s->store_commits;ui_ai_edit_fixture(s,7,SQLITE_IOERR_WRITE);
 {ui_ai_task_request_t q={0};q.size=sizeof(q);q.version=1;q.client_request_id="running-checkpoint";q.input_utf8="must not progress after running commit";q.stub_plan_json="{\"steps\":[]}";CHECK(ui_ai_submit(s,&q,run)==UI_STATUS_OK);}
 begin=GetTickCount();do{pump();EnterCriticalSection(&s->lock);failed=s->store_failed;int exited=s->worker&&WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0;LeaveCriticalSection(&s->lock);if(failed&&exited)break;}while(GetTickCount()-begin<5000);
 CHECK(failed&&strstr(s->error,"passive_checkpoint"));CHECK(s->store_commits==before+2);CHECK(value(0)==10);close_service();
 CHECK(cr_sql("SELECT count(*) FROM events e JOIN runs r ON e.run_id=r.id WHERE r.client='running-checkpoint' AND e.kind='running'")==1);
 CHECK(cr_sql("SELECT count(*) FROM events e JOIN runs r ON e.run_id=r.id WHERE r.client='running-checkpoint' AND e.terminal=1")==0);
 CHECK(cr_sql("SELECT count(*) FROM runs WHERE client='running-checkpoint' AND terminal=0")==1);
 {wchar_t wide[4096];CHECK(MultiByteToWideChar(CP_UTF8,0,dbpath,-1,wide,4096)>0);CHECK(child(cr_package,cr_directory,L"cr-cold",wide));}
}
static void cr_checkpoint_graph(int child_edit)
{
 unsigned i=0,before;int at,ready=0;ai_edit_t *e=NULL;DWORD begin;
 if(child_edit){for(i=0;i<18;++i)cr_plain(i,1);}
 else while(s->store_commits%32u!=30u&&i<32)cr_plain(i++,1);
 if(!child_edit)ui_ai_edit_fixture(s,7,SQLITE_IOERR_WRITE);
 single("memory.set",NULL);at=cr_slot(run);CHECK(at>=0);
 if(child_edit){CHECK(confirm());begin=GetTickCount();do{pump();e=s->runs[at].graph->nodes[0].edit;if(e){EnterCriticalSection(&s->lock);if(e->phase==AE_GATE||e->job==AE_INTENT)e->fixture_hold=1;ready=e->job==AE_INTENT&&!s->coordinator->job&&!s->coordinator->busy&&!s->coordinator->done&&!s->runs[at].graph->dirty;LeaveCriticalSection(&s->lock);if(ready)break;}}while(GetTickCount()-begin<5000);CHECK(ready);CHECK(s->store_commits%32u==31u);before=s->store_commits;ui_ai_edit_fixture(s,7,SQLITE_IOERR_WRITE);EnterCriticalSection(&s->lock);e->fixture_hold=0;LeaveCriticalSection(&s->lock);ui_ai_edit_fixture(s,5,0);
  begin=GetTickCount();do{EnterCriticalSection(&s->lock);int failed=s->store_failed&&s->worker&&WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0;LeaveCriticalSection(&s->lock);if(failed)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);CHECK(s->store_commits==before+1);CHECK(e->io_done==AE_INTENT&&e->io_ok==1);CHECK(!s->coordinator->failed);
 }
 begin=GetTickCount();do{pump();EnterCriticalSection(&s->lock);int drained=s->store_failed&&!s->coordinator->job&&!s->coordinator->busy&&!s->coordinator->done&&!s->runs[at].refs&&(!e||(!e->holding&&!e->live));LeaveCriticalSection(&s->lock);if(drained)break;}while(GetTickCount()-begin<5000);
 CHECK(s->store_failed&&strstr(s->error,"passive_checkpoint"));CHECK(value(0)==10);CHECK(!e||!e->started);CHECK(s->runs[at].refs==0);close_service();
 CHECK(cr_sql("SELECT count(*) FROM ai_graphs")==1);CHECK(cr_sql("SELECT count(*) FROM ai_edit_consumptions")==child_edit);CHECK(cr_sql("SELECT count(*) FROM events WHERE terminal=1")==(int)i);
 {wchar_t wide[4096];CHECK(MultiByteToWideChar(CP_UTF8,0,dbpath,-1,wide,4096)>0);CHECK(child(cr_package,cr_directory,L"cr-cold",wide));}
}
static void cr_storage(int window)
{
 ai_edit_t *e=NULL;int at,ready=0,failed=0;DWORD begin;char root[65];unsigned before;
 if(window==1)control(0,AE_MODE_ASYNC);
 single("memory.set",NULL);strcpy_s(root,65,run);CHECK(confirm());at=cr_slot(root);CHECK(at>=0);
 begin=GetTickCount();do{pump();e=s->runs[at].graph->nodes[0].edit;if(e){EnterCriticalSection(&s->lock);
  if(window==0&&(e->phase==AE_GATE||e->job==AE_INTENT))e->fixture_hold=1;
  else if(window==2&&e->phase>=AE_BEGIN)e->fixture_hold=1;
  ready=window==0?e->job==AE_INTENT:window==1?e->stage==UI_EDIT_EXECUTE:e->job==AE_FINAL;
  /* Do not hold earlier persistence needed to reach final. */
  if(window==2&&e->phase<AE_BEGIN)e->fixture_hold=0;
  LeaveCriticalSection(&s->lock);if(ready)break;}
 }while(GetTickCount()-begin<5000);CHECK(ready&&e);if(!ready)ExitProcess(1);
 /* Drain only the coordinator snapshot before faulting the independent
    accept, so c.failed cannot accidentally mask the active-slot window. */
 begin=GetTickCount();do{pump();EnterCriticalSection(&s->lock);int stable=!s->coordinator->job&&!s->coordinator->busy&&!s->coordinator->done&&!s->runs[at].graph->dirty;LeaveCriticalSection(&s->lock);if(stable)break;}while(GetTickCount()-begin<5000);
 CHECK(!s->coordinator->failed);before=s->store_commits;cr_fail_accept("failure-during-child");
 begin=GetTickCount();do{EnterCriticalSection(&s->lock);failed=s->store_failed;int exited=s->worker&&WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0;LeaveCriticalSection(&s->lock);if(failed&&exited)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);CHECK(failed);CHECK(!s->coordinator->failed);ui_ai_edit_fixture(s,5,0);begin=GetTickCount();do{EnterCriticalSection(&s->lock);int exited=s->worker&&WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0;LeaveCriticalSection(&s->lock);if(exited)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);CHECK(s->active<0);
 EnterCriticalSection(&s->lock);e->fixture_hold=0;LeaveCriticalSection(&s->lock);
 CHECK(ui_ai_service_request_close(s)==UI_STATUS_OK);CHECK(ui_ai_service_destroy(s)==UI_STATUS_PENDING);
 if(window==1){CHECK(GetModuleHandleW(L"ui_ai_edit_fixture.dll")!=NULL);CHECK(ui_workspace_close(w,apps[0].instance_id,UI_APP_CLOSE_TAB)==UI_STATUS_OK);pump();CHECK(ui_workspace_count(w)==2);CHECK(ui_workspace_post_app_data(w,apps[0].instance_id,AE_APP_WORKER_EXIT,NULL,0)==UI_STATUS_OK);pump();CHECK(ui_ai_service_destroy(s)==UI_STATUS_PENDING);CHECK(ui_workspace_post_app_data(w,apps[0].instance_id,AE_APP_CALLBACK_EXIT,NULL,0)==UI_STATUS_OK);}
 begin=GetTickCount();do{pump();if(!e->job&&!e->busy&&!e->holding&&!e->live)break;}while(GetTickCount()-begin<5000);
 CHECK(!e->job&&!e->busy&&!e->holding&&!e->live);CHECK(e->effect==UI_CAP2_EFFECT_UNKNOWN&&e->phase==AE_UNKNOWN);CHECK(s->store_commits==before);CHECK(!e->committed||window==2);
 printf("P6R1 storage window=%d active=%d job=%d busy=%d live=%d holding=%d phase=%d committed=%d store_commits=%u/%u\n",window,s->active,e->job,e->busy,e->live,e->holding,e->phase,e->committed,s->store_commits,before);fflush(stdout);
 if(e->job||e->busy||e->holding||e->live)ExitProcess(1);
 CHECK(s->runs[at].refs==0);if(window!=1)CHECK(value(0)==(window==2?42:10));close_service();CHECK(cr_sql("SELECT count(*) FROM (SELECT run_id FROM events WHERE terminal=1 GROUP BY run_id HAVING count(*)>1)")==0);
}

static void cr_read_close(int removed)
{
 int at,ready=0;DWORD begin;ag_node_t *n;
 single("memory.read",NULL);at=cr_slot(run);CHECK(at>=0);n=&s->runs[at].graph->nodes[0];
 begin=GetTickCount();do{pump();if(n->call&&n->tool_phase==1){ready=1;break;}}while(GetTickCount()-begin<5000);CHECK(ready);if(!ready)ExitProcess(1);
 ui_ai_graph_fixture(s,1,1);cr_fail_accept("read-cleanup-fault");
 begin=GetTickCount();do{EnterCriticalSection(&s->lock);int failed=s->store_failed&&s->worker&&WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0;LeaveCriticalSection(&s->lock);if(failed)break;(void)MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}while(GetTickCount()-begin<5000);CHECK(s->store_failed);
 CHECK(ui_workspace_close(w,apps[0].instance_id,UI_APP_CLOSE_TAB)==UI_STATUS_OK);if(removed)ui_workspace_poll(w);CHECK(ui_workspace_count(w)==(removed?1u:2u));ui_ai_edit_fixture(s,5,0);ui_ai_graph_fixture(s,1,0);
 begin=GetTickCount();do{pump();if(!n->call&&!s->runs[at].refs)break;}while(GetTickCount()-begin<5000);CHECK(!n->call&&!s->runs[at].refs);printf("P6R1 closed read call=%llu refs=%d\n",(unsigned long long)n->call,s->runs[at].refs);fflush(stdout);if(n->call)ExitProcess(1);CHECK(value(1)==10);close_service();CHECK(cr_sql("SELECT count(*) FROM events WHERE terminal=1")==0);
}
