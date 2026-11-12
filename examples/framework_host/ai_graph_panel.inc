/* This adapter only proposes fixed targets and projects Coordinator data. */
static void ai_panel_graph_bindings(host_window_t *s)
{
 const char *nodes[]={"ai-graph-confirm","ai-graph-approve","ai-graph-check","ai-graph-migrate"};char *root=ai_panel_edit_string(s->ai_graph,"binding_hash"),*selected=ai_panel_edit_string(s->ai_graph,"run_id"),*migration=ai_panel_edit_string(s->ai_coordinator,"binding_hash");char pending[65]="",proof[65]="",pending_hash[65]="",proof_hash[65]="",key[1024];int ready=s->ai_projection_generation==s->ai_projection_rendered&&!(s->ai_coordinator&&strstr(s->ai_coordinator,"\"pending\":true"));int schema=ai_panel_edit_number(s->ai_coordinator,"schema"),confirmed=s->ai_graph&&strstr(s->ai_graph,"\"confirmed\":true"),terminal=s->ai_graph&&strstr(s->ai_graph,"\"terminal\":true");
 {char *a=ai_panel_edit_string(s->ai_graph,"approval_node"),*h=ai_panel_edit_string(s->ai_graph,"approval_binding"),*u=ai_panel_edit_string(s->ai_graph,"unknown_node"),*v=ai_panel_edit_string(s->ai_graph,"unknown_binding");if(a)strcpy_s(pending,65,a);if(h)strcpy_s(pending_hash,65,h);if(u)strcpy_s(proof,65,u);if(v)strcpy_s(proof_hash,65,v);free(a);free(h);free(u);free(v);}
 snprintf(key,sizeof(key),"%s:%d:%s:%s:%d:%d:%s:%s:%s:%s:%d",migration?migration:"",schema,selected?selected:"",root?root:"",confirmed,terminal,pending,pending_hash,proof,proof_hash,ready);
 if(!strcmp(key,s->ai_graph_bound))goto done;
 for(unsigned i=0;i<4;++i)if(s->ai_graph_bindings[i]){ui_status_t status=ui_ai_graph_unbind(s->ai,s->ai_graph_bindings[i]);if(status==UI_STATUS_PENDING)goto done;if(!status||status==UI_STATUS_NOT_FOUND)s->ai_graph_bindings[i]=0;else goto done;}
 if(s->view&&ready)for(unsigned i=0;i<4;++i){int enabled=i==0?root&&!confirmed&&!terminal:i==1?pending[0]!=0:i==2?proof[0]!=0:schema==2;if(enabled){ui_ai_graph_light_binding_t d={0};d.size=sizeof(d);d.version=1;d.view=s->view;d.control_id=nodes[i];d.run_id=selected?selected:"";d.node_id=i==1?pending:i==2?proof:"";d.binding_hash=i==0?root:i==1?pending_hash:i==2?proof_hash:migration;d.action=(ui_ai_graph_action_t)(i+1);ui_status_t status=ui_ai_graph_bind_light(s->ai,&d,&s->ai_graph_bindings[i]);if(status){replace_text(&s->ai_error,"Coordinator control binding unavailable; authority not accepted");goto done;}}}
 strcpy_s(s->ai_graph_bound,sizeof(s->ai_graph_bound),key);
 done:free(root);free(selected);free(migration);
}
static int ai_panel_graph_action(host_window_t *s,host_action_t *action)
{
 ui_status_t status=UI_STATUS_PERMISSION_DENIED;if(strncmp(action->action,"ai-graph-",9))return 0;
 if(!strcmp(action->action,"ai-graph-propose")||!strcmp(action->action,"ai-graph-continue")){
  ui_ai_graph_request_t q={0};ui_cap_target_v2_t targets[16];char *rows=_strdup(action->answer?action->answer:""),*cursor=rows;size_t count=0;char id[65];
  if(!rows){status=UI_STATUS_OUT_OF_MEMORY;goto finish;}
  while(cursor&&*cursor&&count<16){char *end=strchr(cursor,'\n'),*colon;if(end)*end=0;colon=strchr(cursor,':');if(!colon){status=UI_STATUS_INVALID_ARGUMENT;break;}*colon++=0;if(*colon&&colon[strlen(colon)-1]=='\r')colon[strlen(colon)-1]=0;memset(&targets[count],0,sizeof(targets[count]));targets[count].size=sizeof(targets[count]);targets[count].version=2;status=ui_ai_edit_bind_target(s->ai,read_id(cursor),colon,&targets[count]);if(status)break;++count;cursor=end?end+1:NULL;}
  if(!status&&cursor&&*cursor)status=UI_STATUS_LIMIT_EXCEEDED;
  if(!status&&count){q.size=sizeof(q);q.version=1;q.client_request_id=action->token;q.input_utf8=action->command;q.graph_json=action->params;q.targets=targets;q.target_count=count;status=!strcmp(action->action,"ai-graph-continue")?ui_ai_graph_continue(s->ai,action->id,&q,id):ui_ai_graph_submit(s->ai,&q,id);if(!status||(status==UI_STATUS_PENDING&&id[0])){strcpy_s(s->ai_run,65,id);status=UI_STATUS_OK;}}free(rows);
 }else if(!strcmp(action->action,"ai-graph-answer"))status=ui_ai_graph_answer(s->ai,action->id,action->panel,action->command,action->token,action->answer);
 else if(!strcmp(action->action,"ai-graph-cancel"))status=ui_ai_graph_cancel(s->ai,action->id);
 /* confirm/approve/reconcile/migrate page messages are deliberately refused. */
 finish:if(status){char error[128];snprintf(error,sizeof(error),"Coordinator request rejected (%d); input retained",status);replace_text(&s->ai_error,error);}else replace_text(&s->ai_error,"");schedule_refresh(s);return 1;
}
static void ai_panel_graph_tick(host_window_t *s,ui_ai_service_phase_t phase)
{
 char *json=NULL;ui_status_t status=ui_ai_coordinator_get_state(s->ai,&json);if(status==UI_STATUS_NOT_FOUND&&phase==UI_AI_READY)(void)ui_ai_coordinator_enable(s->ai);if(!status){if(phase==UI_AI_READY&&ai_panel_edit_number(json,"schema")==1&&ai_panel_edit_number(s->ai_edit,"schema")>=2)(void)ui_ai_coordinator_enable(s->ai);if(!s->ai_coordinator||strcmp(s->ai_coordinator,json)){replace_text(&s->ai_coordinator,json);++s->ai_projection_generation;schedule_refresh(s);}}ui_ai_release(json);json=NULL;
 if(s->ai_run[0]&&ui_ai_graph_get(s->ai,s->ai_run,&json)==UI_STATUS_OK){if(!s->ai_graph||strcmp(s->ai_graph,json)){replace_text(&s->ai_graph,json);++s->ai_projection_generation;schedule_refresh(s);}}else if(s->ai_graph){free(s->ai_graph);s->ai_graph=NULL;++s->ai_projection_generation;schedule_refresh(s);}ui_ai_release(json);if(phase==UI_AI_READY)ai_panel_graph_bindings(s);
}
