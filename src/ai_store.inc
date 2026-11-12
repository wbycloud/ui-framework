#ifdef UI_AI_FIXTURE
#include "ai_reliability_fixture.inc"
#else
#define ar_crash(service,point) ((void)0)
#endif
/* SQLite is only touched by the single, serial background execution channel. */
static int ai_sql(ui_ai_service_t *s,const char *sql)
{return sqlite3_exec(s->db,sql,NULL,NULL,NULL)==SQLITE_OK;}
static int ai_statement(ui_ai_service_t *s,const char *sql,sqlite3_stmt **q)
{return sqlite3_prepare_v2(s->db,sql,-1,q,NULL)==SQLITE_OK;}
static void ai_bind(sqlite3_stmt *q,int index,const char *text)
{sqlite3_bind_text(q,index,text?text:"",-1,SQLITE_TRANSIENT);}
static int ai_done(sqlite3_stmt *q)
{int ok=sqlite3_step(q)==SQLITE_DONE;sqlite3_finalize(q);return ok;}
static int ai_store_available(ui_ai_service_t *s)
{int available;EnterCriticalSection(&s->lock);available=!s->store_failed;LeaveCriticalSection(&s->lock);return available;}
static int ai_begin(ui_ai_service_t *s)
{int fail;EnterCriticalSection(&s->lock);fail=s->fixture_fail_write||s->store_failed;s->fixture_fail_write=0;LeaveCriticalSection(&s->lock);if(fail)return 0;return ai_sql(s,"BEGIN IMMEDIATE");}
static int ai_commit(ui_ai_service_t *s,int ok)
{if(ok&&ai_sql(s,"COMMIT")){if(++s->store_commits%32u==0){int rc;
#ifdef UI_AI_FIXTURE
LONG fault=InterlockedExchange(&s->fixture_checkpoint,0);if(fault)InterlockedExchange(&ar_disk_fault,fault);
#endif
rc=sqlite3_wal_checkpoint_v2(s->db,NULL,SQLITE_CHECKPOINT_PASSIVE,NULL,NULL);if(rc==SQLITE_OK)++s->store_checkpoints;else if(rc!=SQLITE_BUSY&&rc!=SQLITE_LOCKED)ai_store_error(s,NULL,"passive_checkpoint");}return 1;}(void)ai_sql(s,"ROLLBACK");return 0;}
static int ai_event_insert(ui_ai_service_t *s,ai_run_t *r,const char *suffix,
    const char *kind,const char *payload,int terminal,uint64_t *sequence)
{
 sqlite3_stmt *q=NULL;char op[192];snprintf(op,sizeof(op),"%s:%s",r?r->id:s->session,suffix);
 if(!ai_statement(s,"INSERT INTO events(op_id,run_id,kind,payload,terminal) VALUES(?,?,?,?,?) ON CONFLICT(op_id) DO NOTHING",&q))return 0;
 ai_bind(q,1,op);ai_bind(q,2,r?r->id:"");ai_bind(q,3,kind);ai_bind(q,4,payload);sqlite3_bind_int(q,5,terminal);if(!ai_done(q))return 0;
 if(!ai_statement(s,"SELECT seq,run_id,kind,payload,terminal FROM events WHERE op_id=?",&q))return 0;ai_bind(q,1,op);
 if(sqlite3_step(q)!=SQLITE_ROW||strcmp((const char *)sqlite3_column_text(q,1),r?r->id:"")||strcmp((const char *)sqlite3_column_text(q,2),kind)||strcmp((const char *)sqlite3_column_text(q,3),payload)||sqlite3_column_int(q,4)!=terminal){sqlite3_finalize(q);return 0;}
 *sequence=(uint64_t)sqlite3_column_int64(q,0);sqlite3_finalize(q);return 1;
}
static int ai_message(ui_ai_service_t *s,ai_run_t *r,const char *op,const char *role,const char *text)
{
 sqlite3_stmt *q=NULL;char id[192];snprintf(id,sizeof(id),"%s:%s",r->id,op);
 if(!ai_statement(s,"INSERT INTO messages(op_id,run_id,role,content) VALUES(?,?,?,?) ON CONFLICT(op_id) DO NOTHING",&q))return 0;
 ai_bind(q,1,id);ai_bind(q,2,r->id);ai_bind(q,3,role);ai_bind(q,4,text);return ai_done(q);
}
static int ai_change(ui_ai_service_t *s,ai_run_t *r,ui_ai_run_phase_t phase,const char *outcome,
    const char *suffix,const char *kind,const char *payload,const char *role,const char *text,uint64_t *sequence)
{
 sqlite3_stmt *q=NULL;int terminal=phase==UI_AI_FINISHED,ok=ai_begin(s);if(!ok)return 0;
 ok=ai_statement(s,"UPDATE runs SET phase=?,outcome=?,reply=?,question=? WHERE id=? AND terminal=0",&q);
 if(ok){sqlite3_bind_int(q,1,(int)phase);ai_bind(q,2,outcome);ai_bind(q,3,r->reply);ai_bind(q,4,r->question);ai_bind(q,5,r->id);ok=ai_done(q);}
 if(ok&&terminal){ok=ai_statement(s,"UPDATE runs SET terminal=1 WHERE id=?",&q);if(ok){ai_bind(q,1,r->id);ok=ai_done(q);}}
 if(ok&&role)ok=ai_message(s,r,suffix,role,text);
 if(ok)ok=ai_event_insert(s,r,suffix,kind,payload,terminal,sequence);
 return ai_commit(s,ok);
}
#include "ai_reliability.inc"
static int ai_store_open(ui_ai_service_t *s)
{
 sqlite3_stmt *q=NULL;wchar_t path[4096];DWORD attributes;int exists,version=0,app=0;
 if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s->path,-1,path,4096))return 0;
 attributes=GetFileAttributesW(path);exists=attributes!=INVALID_FILE_ATTRIBUTES;
 if(!exists){/* Only the background storage channel creates parent directories. */
  for(wchar_t *p=path+1;*p;++p)if((*p==L'/'||*p==L'\\')&&p[-1]!=L':'){
   wchar_t separator=*p;*p=0;if(GetFileAttributesW(path)==INVALID_FILE_ATTRIBUTES&&!CreateDirectoryW(path,NULL)&&GetLastError()!=ERROR_ALREADY_EXISTS){*p=separator;return 0;}*p=separator;
  }
 }
 if(sqlite3_open_v2(s->path,&s->db,SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_FULLMUTEX,
#ifdef UI_AI_FIXTURE
 ar_test_vfs()
#else
 NULL
#endif
 )!=SQLITE_OK)return 0;
 sqlite3_busy_timeout(s->db,50);sqlite3_limit(s->db,SQLITE_LIMIT_LENGTH,2*1024*1024);sqlite3_limit(s->db,SQLITE_LIMIT_SQL_LENGTH,65536);
 if(!ai_statement(s,"PRAGMA user_version",&q))return 0;if(sqlite3_step(q)==SQLITE_ROW)version=sqlite3_column_int(q,0);sqlite3_finalize(q);
 if(!ai_statement(s,"PRAGMA application_id",&q))return 0;if(sqlite3_step(q)==SQLITE_ROW)app=sqlite3_column_int(q,0);sqlite3_finalize(q);
 if(exists&&((version!=1&&version!=2&&version!=3)||app!=0x58535033)){sqlite3_close(s->db);s->db=NULL;return 0;}
 if(exists){if(!ai_statement(s,"PRAGMA quick_check(1)",&q))return 0;if(sqlite3_step(q)!=SQLITE_ROW||strcmp((const char *)sqlite3_column_text(q,0),"ok")){sqlite3_finalize(q);return 0;}sqlite3_finalize(q);}
 if(exists&&!ai_store_consistent(s,version)){EnterCriticalSection(&s->lock);strcpy_s(s->error,sizeof(s->error),"logical_consistency");LeaveCriticalSection(&s->lock);return 0;}
 if(!ai_sql(s,"PRAGMA foreign_keys=ON;PRAGMA synchronous=FULL;PRAGMA locking_mode=EXCLUSIVE"))return 0;
 if(!ai_statement(s,"PRAGMA journal_mode=WAL",&q))return 0;
 if(sqlite3_step(q)!=SQLITE_ROW||strcmp((const char *)sqlite3_column_text(q,0),"wal")){sqlite3_finalize(q);return 0;}sqlite3_finalize(q);
 if(!ai_begin(s))return 0;
 if(!exists){
  if(!ai_sql(s,"PRAGMA application_id=1481855027;PRAGMA user_version=1;"
   "CREATE TABLE conversation(id TEXT NOT NULL);"
   "CREATE TABLE runs(id TEXT PRIMARY KEY,client TEXT UNIQUE NOT NULL,hash TEXT NOT NULL,input TEXT NOT NULL,plan TEXT NOT NULL,targets TEXT NOT NULL,parent TEXT NOT NULL,phase INTEGER NOT NULL,outcome TEXT NOT NULL DEFAULT '',reply TEXT NOT NULL DEFAULT '',question TEXT NOT NULL DEFAULT '',terminal INTEGER NOT NULL DEFAULT 0);"
   "CREATE TABLE messages(seq INTEGER PRIMARY KEY AUTOINCREMENT,op_id TEXT UNIQUE NOT NULL,run_id TEXT NOT NULL,role TEXT NOT NULL,content TEXT NOT NULL);"
   "CREATE TABLE events(seq INTEGER PRIMARY KEY AUTOINCREMENT,op_id TEXT UNIQUE NOT NULL,run_id TEXT NOT NULL,kind TEXT NOT NULL,payload TEXT NOT NULL,terminal INTEGER NOT NULL);"
   "CREATE UNIQUE INDEX one_terminal ON events(run_id) WHERE terminal=1;"
   "CREATE TABLE tools(run_id TEXT NOT NULL,tool_id TEXT NOT NULL,target TEXT NOT NULL,capability TEXT NOT NULL,params TEXT NOT NULL,call_id TEXT NOT NULL DEFAULT '',result TEXT NOT NULL DEFAULT '',status INTEGER NOT NULL DEFAULT 1,PRIMARY KEY(run_id,tool_id));"))return ai_commit(s,0);
  if(!ai_statement(s,"INSERT INTO conversation(id) VALUES(?)",&q))return ai_commit(s,0);ai_bind(q,1,s->session);if(!ai_done(q))return ai_commit(s,0);
  /* Fresh daily Services use the existing P4/P5 schema. Existing files never
     take this branch and retain the original explicit migration contract. */
  if(s->assistant_policy&&!ai_sql(s,
   "CREATE TABLE ai_edits(id TEXT PRIMARY KEY,client TEXT UNIQUE NOT NULL,hash TEXT NOT NULL,resource TEXT NOT NULL,payload TEXT NOT NULL,terminal INTEGER NOT NULL,unknown INTEGER NOT NULL);"
   "CREATE TABLE ai_edit_consumptions(id TEXT PRIMARY KEY,edit_id TEXT NOT NULL,binding TEXT NOT NULL,kind TEXT NOT NULL);"
   "CREATE TABLE ai_graphs(run_id TEXT PRIMARY KEY,payload TEXT NOT NULL,version INTEGER NOT NULL);PRAGMA user_version=3;"))return ai_commit(s,0);
 }
 if(!ai_statement(s,"SELECT id FROM conversation",&q))return ai_commit(s,0);
 if(sqlite3_step(q)!=SQLITE_ROW||sqlite3_column_bytes(q,0)!=36){sqlite3_finalize(q);return ai_commit(s,0);}
 EnterCriticalSection(&s->lock);snprintf(s->conversation,sizeof(s->conversation),"%s",sqlite3_column_text(q,0));LeaveCriticalSection(&s->lock);sqlite3_finalize(q);
 /* Do not schedule or replay recovered tools. Keep the original request. */
 if(!ai_sql(s,"INSERT INTO events(op_id,run_id,kind,payload,terminal) SELECT id||':recovered',id,'needs_review','{}',0 FROM runs WHERE terminal=0 AND phase<>8 ON CONFLICT(op_id) DO NOTHING;UPDATE runs SET phase=8 WHERE terminal=0"))return ai_commit(s,0);
 return ai_commit(s,1);
}
static int ai_store_accept(ui_ai_service_t *s,ai_run_t *r,int *existing,uint64_t *seq)
{
 sqlite3_stmt *q=NULL;int ok;*existing=0;
 if(!ai_statement(s,"SELECT hash,phase,outcome,reply,question,terminal,(SELECT seq FROM events WHERE run_id=runs.id AND kind='accepted') FROM runs WHERE id=?",&q))return 0;ai_bind(q,1,r->id);
 if(sqlite3_step(q)==SQLITE_ROW){
  int mismatch=strcmp((const char *)sqlite3_column_text(q,0),r->hash)!=0;
  EnterCriticalSection(&s->lock);r->phase=(ui_ai_run_phase_t)sqlite3_column_int(q,1);snprintf(r->outcome,sizeof(r->outcome),"%s",sqlite3_column_text(q,2));
  strcpy_s(r->hash,sizeof(r->hash),(const char *)sqlite3_column_text(q,0));
  if(sqlite3_column_bytes(q,3)<=UI_AI_MAX_REPLY){strcpy_s(r->reply,UI_AI_MAX_REPLY+1,(const char *)sqlite3_column_text(q,3));r->reply_length=strlen(r->reply);}snprintf(r->question,sizeof(r->question),"%s",sqlite3_column_text(q,4));r->terminal=sqlite3_column_int(q,5);r->committed=1;LeaveCriticalSection(&s->lock);
  *seq=(uint64_t)sqlite3_column_int64(q,6);sqlite3_finalize(q);*existing=mismatch?-1:1;return 1;
 }sqlite3_finalize(q);
 if(r->parent[0]){if(!ai_statement(s,"SELECT id FROM runs WHERE id=? AND terminal=0 AND phase=8",&q))return 0;ai_bind(q,1,r->parent);ok=sqlite3_step(q)==SQLITE_ROW;sqlite3_finalize(q);if(!ok||!ai_graph_continue_valid(s,r)){*existing=-2;return 1;}}
 ar_crash(s,1);if(!ai_begin(s))return 0;
 ok=ai_statement(s,"INSERT INTO runs(id,client,hash,input,plan,targets,parent,phase) VALUES(?,?,?,?,?,?,?,?)",&q);
 if(ok){ai_bind(q,1,r->id);ai_bind(q,2,r->client);ai_bind(q,3,r->hash);ai_bind(q,4,r->input);ai_bind(q,5,r->graph?"{\"coordinator\":true}":r->plan);ai_bind(q,6,r->targets_json);ai_bind(q,7,r->parent);sqlite3_bind_int(q,8,UI_AI_QUEUED);ok=ai_done(q);}
 if(ok)ok=ai_message(s,r,"user","user",r->input);
 if(ok)ok=ai_event_insert(s,r,"accepted","accepted",r->graph?"{\"coordinator\":true}":r->request,0,seq);
 ok=ai_commit(s,ok);if(ok)ar_crash(s,2);return ok;
}
static int ai_store_tool(ui_ai_service_t *s,ai_run_t *r,ai_tool_t *t,int phase,uint64_t *seq)
{
 sqlite3_stmt *q=NULL;char suffix[128],payload[256];int ok=ai_begin(s);if(!ok)return 0;
 if(phase==0){ok=ai_statement(s,"INSERT INTO tools(run_id,tool_id,target,capability,params) VALUES(?,?,?,?,?)",&q);
  if(ok){ai_bind(q,1,r->id);ai_bind(q,2,t->id);ai_bind(q,3,t->target_json);ai_bind(q,4,t->capability);ai_bind(q,5,t->params);ok=ai_done(q);}}
 else if(phase==1){char call[32];snprintf(call,sizeof(call),"%llu",(unsigned long long)t->call);ok=ai_statement(s,"UPDATE tools SET call_id=? WHERE run_id=? AND tool_id=?",&q);if(ok){ai_bind(q,1,call);ai_bind(q,2,r->id);ai_bind(q,3,t->id);ok=ai_done(q);}}
 else {ok=ai_statement(s,"UPDATE tools SET result=?,status=? WHERE run_id=? AND tool_id=?",&q);if(ok){ai_bind(q,1,t->result);sqlite3_bind_int(q,2,t->status);ai_bind(q,3,r->id);ai_bind(q,4,t->id);ok=ai_done(q);}}
 if(ok){int state;EnterCriticalSection(&s->lock);state=r->cancel?(r->stalled?UI_AI_CANCELLATION_STALLED:UI_AI_CANCEL_REQUESTED):phase==2?UI_AI_RUNNING:UI_AI_WAITING_TOOL;LeaveCriticalSection(&s->lock);
  ok=ai_statement(s,"UPDATE runs SET phase=? WHERE id=? AND terminal=0",&q);if(ok){sqlite3_bind_int(q,1,state);ai_bind(q,2,r->id);ok=ai_done(q);}}
 snprintf(suffix,sizeof(suffix),"tool:%s:%d",t->id,phase);snprintf(payload,sizeof(payload),"{\"tool_call_id\":\"%s\",\"call_id\":\"%llu\",\"status\":%d}",t->id,(unsigned long long)t->call,t->status);
 if(ok)ok=ai_event_insert(s,r,suffix,phase==0?"tool_requested":phase==1?"tool_bound":"tool_result",payload,0,seq);
 return ai_commit(s,ok);
}
static char *ai_store_page(ui_ai_service_t *s,const ai_command_t *command)
{
 static const char *sql[]={"SELECT seq,run_id,role,content,op_id FROM messages WHERE seq>? ORDER BY seq LIMIT ?",
  "SELECT rowid,id,client,phase,outcome,input,terminal FROM runs WHERE rowid>? ORDER BY rowid LIMIT ?",
  "SELECT seq,run_id,kind,payload,terminal FROM events WHERE seq>? ORDER BY seq LIMIT ?"};
 sqlite3_stmt *q=NULL;yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *root=d?yyjson_mut_obj(d):NULL,*rows=d?yyjson_mut_arr(d):NULL;size_t used=0,length;uint64_t next=command->after;char *json=NULL;int more=0,rc;
 if(!root||!rows){yyjson_mut_doc_free(d);return NULL;}yyjson_mut_doc_set_root(d,root);yyjson_mut_obj_add_val(d,root,"items",rows);
 if(!ai_statement(s,sql[command->history],&q))goto done;sqlite3_bind_int64(q,1,(sqlite3_int64)command->after);sqlite3_bind_int(q,2,(int)command->limit+1);
 while((rc=sqlite3_step(q))==SQLITE_ROW){size_t bytes=0;char *encoded;yyjson_mut_val *row;uint64_t id=(uint64_t)sqlite3_column_int64(q,0);
  if(yyjson_mut_arr_size(rows)==command->limit){more=1;break;}
  row=yyjson_mut_obj(d);yyjson_mut_obj_add_uint(d,row,"sequence",id);yyjson_mut_obj_add_strcpy(d,row,"run_id",(const char *)sqlite3_column_text(q,1));
  if(command->history==UI_AI_MESSAGES){yyjson_mut_obj_add_strcpy(d,row,"role",(const char *)sqlite3_column_text(q,2));yyjson_mut_obj_add_strcpy(d,row,"content",(const char *)sqlite3_column_text(q,3));yyjson_mut_obj_add_strcpy(d,row,"operation_id",(const char *)sqlite3_column_text(q,4));}
  else if(command->history==UI_AI_RUNS){yyjson_mut_obj_add_strcpy(d,row,"client_request_id",(const char *)sqlite3_column_text(q,2));yyjson_mut_obj_add_int(d,row,"phase",sqlite3_column_int(q,3));yyjson_mut_obj_add_strcpy(d,row,"outcome",(const char *)sqlite3_column_text(q,4));yyjson_mut_obj_add_strcpy(d,row,"input",(const char *)sqlite3_column_text(q,5));yyjson_mut_obj_add_bool(d,row,"terminal",sqlite3_column_int(q,6)!=0);}
  else{yyjson_mut_obj_add_strcpy(d,row,"type",(const char *)sqlite3_column_text(q,2));yyjson_mut_obj_add_strcpy(d,row,"payload",(const char *)sqlite3_column_text(q,3));yyjson_mut_obj_add_bool(d,row,"terminal",sqlite3_column_int(q,4)!=0);}
  encoded=yyjson_mut_val_write(row,0,&bytes);if(!encoded)goto done;free(encoded);
  if(bytes+1>AI_PAGE-4096-used){if(!used)goto done;more=1;break;}
  yyjson_mut_arr_add_val(rows,row);used+=bytes+1;next=id;
 }
 if(!more&&rc!=SQLITE_DONE)goto done;
 yyjson_mut_obj_add_uint(d,root,"next",more?next:0);yyjson_mut_obj_add_uint(d,root,"last",next);yyjson_mut_obj_add_bool(d,root,"has_more",more!=0);yyjson_mut_obj_add_uint(d,root,"kind",command->history);json=yyjson_mut_write(d,0,&length);if(length>AI_PAGE){free(json);json=NULL;}
done:sqlite3_finalize(q);yyjson_mut_doc_free(d);return json;
}
static char *ai_store_query(ui_ai_service_t *s,const char *id)
{
 sqlite3_stmt *q=NULL;yyjson_mut_doc *d=NULL;yyjson_mut_val *root;char *json=NULL;size_t length;
 if(!ai_statement(s,"SELECT id,client,phase,outcome,input,plan,targets,parent,reply,question,terminal FROM runs WHERE id=?",&q))return NULL;ai_bind(q,1,id);
 if(sqlite3_step(q)!=SQLITE_ROW){sqlite3_finalize(q);return NULL;}d=yyjson_mut_doc_new(NULL);root=d?yyjson_mut_obj(d):NULL;if(!root)goto done;yyjson_mut_doc_set_root(d,root);
 {const char *names[]={"run_id","client_request_id","phase","outcome","input","stub_plan","targets","parent_run_id","reply","question_id","terminal"};for(int i=0;i<11;++i){if(i==2||i==10)yyjson_mut_obj_add_int(d,root,names[i],sqlite3_column_int(q,i));else yyjson_mut_obj_add_strcpy(d,root,names[i],(const char *)sqlite3_column_text(q,i));}}
 sqlite3_finalize(q);q=NULL;
 if(ai_statement(s,"SELECT payload FROM ai_edits WHERE id=?",&q)){ai_bind(q,1,id);if(sqlite3_step(q)==SQLITE_ROW){const char *payload=(const char *)sqlite3_column_text(q,0);yyjson_doc *record=yyjson_read(payload,strlen(payload),0);yyjson_mut_val *edit=record?yyjson_val_mut_copy(d,yyjson_doc_get_root(record)):NULL;
   if(edit){yyjson_mut_obj_remove_key(edit,"output");yyjson_mut_obj_remove_key(edit,"params");yyjson_mut_obj_remove_key(edit,"parameter_constraint");yyjson_mut_obj_add_bool(d,edit,"authorization_restored",false);yyjson_mut_obj_add_val(d,root,"edit_record",edit);}yyjson_doc_free(record);}}
 sqlite3_finalize(q);q=NULL;
 if(ai_statement(s,"SELECT payload FROM ai_graphs WHERE run_id=?",&q)){ai_bind(q,1,id);if(sqlite3_step(q)==SQLITE_ROW){const char *payload=(const char *)sqlite3_column_text(q,0);yyjson_doc *record=yyjson_read(payload,strlen(payload),0);yyjson_mut_val *graph=record?yyjson_val_mut_copy(d,yyjson_doc_get_root(record)):NULL;if(graph){yyjson_mut_obj_remove_key(graph,"confirmed");yyjson_mut_obj_add_bool(d,graph,"confirmed",false);yyjson_mut_obj_add_bool(d,graph,"authorization_restored",false);yyjson_mut_obj_add_val(d,root,"graph_record",graph);}yyjson_doc_free(record);}}
 sqlite3_finalize(q);q=NULL;
 if(!ai_statement(s,"SELECT payload FROM events WHERE run_id=? AND terminal=1 ORDER BY seq DESC LIMIT 1",&q))goto done;ai_bind(q,1,id);
 {int result=sqlite3_step(q),recorded=0;if(result==SQLITE_ROW){const char *payload=(const char *)sqlite3_column_text(q,0);yyjson_doc *event=yyjson_read(payload,strlen(payload),0);const char *error=event?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(event),"error")):NULL;if(error){yyjson_mut_obj_add_strcpy(d,root,"error",error);recorded=1;}yyjson_doc_free(event);}else if(result!=SQLITE_DONE)goto done;yyjson_mut_obj_add_bool(d,root,"error_recorded",recorded!=0);}
 yyjson_mut_obj_add_bool(d,root,"committed",true);json=yyjson_mut_write(d,0,&length);if(length>AI_PAGE){free(json);json=NULL;}
done:sqlite3_finalize(q);yyjson_mut_doc_free(d);return json;
}
