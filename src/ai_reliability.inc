/* P6R-1: read-only logical audit BEFORE changing recovered records. Worker only. */
static int ar_zero(ui_ai_service_t *s,const char *sql)
{sqlite3_stmt *q=NULL;int ok=ai_statement(s,sql,&q)&&sqlite3_step(q)==SQLITE_ROW&&sqlite3_column_int64(q,0)==0;sqlite3_finalize(q);return ok;}
static int ar_graphs(ui_ai_service_t *s)
{
 sqlite3_stmt *q=NULL;int rc,ok=1;if(!ai_statement(s,"SELECT run_id,payload,version FROM ai_graphs",&q))return 0;
 while((rc=sqlite3_step(q))==SQLITE_ROW){
  const char *text=(const char *)sqlite3_column_text(q,1);size_t len=(size_t)sqlite3_column_bytes(q,1);yyjson_doc *d=len<=AI_PAGE?yyjson_read(text,len,0):NULL;yyjson_val *o=d?yyjson_doc_get_root(d):NULL,*b=yyjson_obj_get(o,"budget"),*a=yyjson_obj_get(o,"nodes"),*v;size_t i,n;unsigned charged=0,agents=0;const char *id=yyjson_get_str(yyjson_obj_get(o,"run_id"));
  ok=o&&id&&!strcmp(id,(const char *)sqlite3_column_text(q,0))&&yyjson_get_uint(yyjson_obj_get(o,"version"))==(uint64_t)sqlite3_column_int64(q,2)&&yyjson_is_arr(a)&&yyjson_arr_size(a)>0&&yyjson_arr_size(a)<=32;
  {const char *reserved[]={"reserved_steps","reserved_models","reserved_tools","reserved_agents"},*used[]={"charged_steps","charged_models","charged_tools","charged_agents"};const unsigned max[]={64,16,64,32};for(unsigned k=0;k<4;++k){yyjson_val *r=yyjson_obj_get(b,reserved[k]),*u=yyjson_obj_get(b,used[k]);if(!yyjson_is_uint(r)||!yyjson_is_uint(u)||yyjson_get_uint(r)>max[k]||yyjson_get_uint(u)>yyjson_get_uint(r))ok=0;}}
  yyjson_arr_foreach(a,i,n,v){const char *node=yyjson_get_str(yyjson_obj_get(v,"id")),*op=yyjson_get_str(yyjson_obj_get(v,"operation_id")),*profile=yyjson_get_str(yyjson_obj_get(v,"agent_profile"));if(!node||!op||!profile||!ai_id(node)||!ai_id(op))ok=0;if(yyjson_is_true(yyjson_obj_get(v,"charged"))){++charged;if(profile&&*profile)++agents;}if(yyjson_is_true(yyjson_obj_get(v,"settled"))&&yyjson_get_int(yyjson_obj_get(v,"state"))!=AG_COLLECTED)ok=0;for(size_t j=0;j<i;++j){yyjson_val *other=yyjson_arr_get(a,j);const char *x=yyjson_get_str(yyjson_obj_get(other,"id")),*y=yyjson_get_str(yyjson_obj_get(other,"operation_id"));if((node&&x&&!strcmp(node,x))||(op&&y&&!strcmp(op,y)))ok=0;}}
  if(yyjson_get_uint(yyjson_obj_get(b,"charged_tools"))!=charged||yyjson_get_uint(yyjson_obj_get(b,"charged_agents"))!=agents||yyjson_get_uint(yyjson_obj_get(b,"charged_models"))!=1u+agents||yyjson_get_uint(yyjson_obj_get(b,"charged_steps"))!=1u+charged+agents)ok=0;
  yyjson_doc_free(d);if(!ok)break;
 }
 sqlite3_finalize(q);return ok&&rc==SQLITE_DONE;
}
static int ar_consumptions(ui_ai_service_t *s)
{
 sqlite3_stmt *q=NULL;int rc,ok=1;if(!ai_statement(s,"SELECT c.binding,e.payload FROM ai_edit_consumptions c LEFT JOIN events e ON e.op_id=c.edit_id||':edit_intent'",&q))return 0;
 while((rc=sqlite3_step(q))==SQLITE_ROW){const char *text=(const char *)sqlite3_column_text(q,1);size_t len=(size_t)sqlite3_column_bytes(q,1);yyjson_doc *d=text&&len<=AI_PAGE?yyjson_read(text,len,0):NULL;const char *binding=d?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"binding_hash")):NULL;ok=binding&&!strcmp(binding,(const char *)sqlite3_column_text(q,0));yyjson_doc_free(d);if(!ok)break;}sqlite3_finalize(q);return ok&&rc==SQLITE_DONE;
}
static int ai_store_consistent(ui_ai_service_t *s,int version)
{
 if(!ar_zero(s,"SELECT abs(count(*)-1) FROM conversation")||!ar_zero(s,"SELECT count(*) FROM conversation WHERE length(id)<>36")||
    !ar_zero(s,"SELECT count(*) FROM runs WHERE terminal NOT IN(0,1) OR phase NOT BETWEEN 0 AND 9 OR (terminal=1 AND phase<>7) OR terminal<>(SELECT count(*) FROM events e WHERE e.run_id=runs.id AND e.terminal=1)")||
    !ar_zero(s,"SELECT count(*) FROM events WHERE terminal NOT IN(0,1) OR (run_id<>'' AND NOT EXISTS(SELECT 1 FROM runs WHERE id=run_id))")||
    !ar_zero(s,"SELECT count(*) FROM messages WHERE NOT EXISTS(SELECT 1 FROM runs WHERE id=run_id)")||
    !ar_zero(s,"SELECT count(*) FROM tools WHERE NOT EXISTS(SELECT 1 FROM runs WHERE id=run_id)")||
    !ar_zero(s,"SELECT abs(count(*)-1) FROM sqlite_master WHERE type='index' AND name='one_terminal'"))return 0;
 if(version>=2){
  sqlite3_stmt *q=NULL;int rc,ok=1;
  if(!ar_zero(s,"SELECT count(*) FROM ai_edits e LEFT JOIN runs r ON r.id=e.id WHERE r.id IS NULL OR e.terminal NOT IN(0,1) OR e.unknown NOT IN(0,1) OR e.terminal<>r.terminal")||
     !ar_zero(s,"SELECT count(*) FROM ai_edit_consumptions c LEFT JOIN ai_edits e ON e.id=c.edit_id WHERE e.id IS NULL OR c.kind NOT IN('scope','approval') OR c.binding=''")||!ar_consumptions(s))return 0;
  if(!ai_statement(s,"SELECT id,payload FROM ai_edits",&q))return 0;
  while((rc=sqlite3_step(q))==SQLITE_ROW){size_t len=(size_t)sqlite3_column_bytes(q,1);yyjson_doc *d=len<=AI_PAGE?yyjson_read((const char *)sqlite3_column_text(q,1),len,0):NULL;yyjson_val *o=d?yyjson_doc_get_root(d):NULL;const char *id=yyjson_get_str(yyjson_obj_get(o,"id"));ok=o&&id&&!strcmp(id,(const char *)sqlite3_column_text(q,0));yyjson_doc_free(d);if(!ok)break;}sqlite3_finalize(q);if(!ok||rc!=SQLITE_DONE)return 0;
 }
 if(version>=3&&(!ar_zero(s,"SELECT count(*) FROM ai_graphs g LEFT JOIN runs r ON r.id=g.run_id WHERE r.id IS NULL OR g.version<1")||!ar_zero(s,"SELECT count(*) FROM ai_graphs g WHERE NOT EXISTS(SELECT 1 FROM events e WHERE e.op_id=g.run_id||':graph:'||g.version AND e.kind='graph' AND e.payload=g.payload)")||!ar_graphs(s)))return 0;
 return 1;
}
