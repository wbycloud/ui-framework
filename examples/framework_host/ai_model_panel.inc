/* Host-owned display proof. Page messages never assert that a preview was shown. */
static void ai_model_preview_clear(host_window_t *s)
{free(s->ai_preview);free(s->ai_chunks);s->ai_preview=s->ai_chunks=NULL;s->ai_preview_hash[0]=s->ai_preview_run[0]=0;s->ai_preview_ready=0;ai_projection_invalidate(s);}
static int ai_model_preview_set(host_window_t *s,const char *preview)
{
 if(UI_UHA_PROJECTION_PROBE(1)){ai_model_preview_clear(s);return 0;}
 yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *a=d?yyjson_mut_arr(d):NULL;if(!a){yyjson_mut_doc_free(d);return 0;}yyjson_mut_doc_set_root(d,a);
 size_t length=strlen(preview),at=0;int ok=length<=256*1024;
 while(ok&&at<length){size_t n=length-at;if(n>3000)n=3000;while(at+n<length&&((unsigned char)preview[at+n]&0xc0)==0x80)--n;ok=n&&yyjson_mut_arr_add_strncpy(d,a,preview+at,n);at+=n;}
 char *chunks=ok?yyjson_mut_write(d,0,NULL):NULL;yyjson_mut_doc_free(d);if(!chunks){ai_model_preview_clear(s);return 0;}
 ai_model_preview_clear(s);s->ai_preview=_strdup(preview);if(!s->ai_preview){free(chunks);return 0;}s->ai_chunks=chunks;return 1;
}
static int ai_model_preview_displayed(host_window_t *s)
{
 if(UI_UHA_PROJECTION_PROBE(4)||!s->view||!s->ai_chunks)return 0;yyjson_doc *d=yyjson_read(s->ai_chunks,strlen(s->ai_chunks),0);if(!d)return 0;
 size_t i,n;yyjson_val *v;int ok=1;yyjson_arr_foreach(yyjson_doc_get_root(d),i,n,v){char node[64];ui_element_presentation_t p={0};p.size=sizeof(p);snprintf(node,sizeof(node),"ai-disclosure-%u",(unsigned)i);if(ui_web_view_get_presentation(s->view,node,&p)||p.rect.width<=0||p.rect.height<=0||strcmp(p.text_utf8,yyjson_get_str(v))){ok=0;break;}}
 yyjson_doc_free(d);return ok;
}
static void ai_model_panel_projection(host_window_t *s,json_buffer_t *json)
{
 char *preview=NULL;ui_status_t status=s->ai_run[0]?(s->ai_general?ui_ai_assistant_get_disclosure(s->ai,s->ai_run,&preview):ui_ai_model_get_disclosure(s->ai,s->ai_run,&preview)):UI_STATUS_NOT_FOUND;
 if(UI_UHA_PROJECTION_PROBE(2))status=UI_STATUS_OUT_OF_MEMORY;
 if(!status&&preview){if(!s->ai_preview||strcmp(preview,s->ai_preview)){ai_projection_invalidate(s);if(ai_model_preview_set(s,preview)){yyjson_doc *d=yyjson_read(preview,strlen(preview),0);const char *h=d?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"binding_hash")):NULL;if(h)strcpy_s(s->ai_preview_hash,65,h);yyjson_doc_free(d);strcpy_s(s->ai_preview_run,65,s->ai_run);}++s->ai_projection_generation;}}
 else if(s->ai_preview){ai_model_preview_clear(s);++s->ai_projection_generation;}
 ui_ai_release(preview);json_append(json,",\"choices\":");json_append(json,s->ai_choices?s->ai_choices:"null");json_append(json,",\"disclosureChunks\":");json_append(json,s->ai_chunks?s->ai_chunks:"[]");json_append(json,",\"configLocation\":");json_string(json,s->ai_config_location?s->ai_config_location:"");json_append(json,",\"copyNotice\":");json_string(json,s->ai_copy_notice);json_append(json,",\"draft\":");json_append(json,s->ai_draft?s->ai_draft:"null");
}
static ui_status_t ai_model_catalog(host_window_t *s,uint64_t instance,const char *resource,size_t offset,size_t resource_offset)
{
 ui_app_instance_info_t app={0};ui_cap_target_t targets[32];size_t count=0,next=0;uint64_t generation=0;char *page=NULL;app.size=sizeof(app);
 if(!get_instance(s,instance,&app)||app.closing)return UI_STATUS_NOT_FOUND;
 for(unsigned i=0;i<32;++i){memset(&targets[i],0,sizeof(targets[i]));targets[i].size=sizeof(targets[i]);targets[i].version=1;}
 ui_status_t status=ui_capability_resources(app.host,resource_offset,targets,32,&count,&next,&generation);if(status)return status;
 ui_cap_target_t selected={0};selected.size=sizeof(selected);selected.version=1;if(resource&&*resource){status=ui_capability_bind(app.host,resource,&selected);if(status)return status;}
 generation=0;status=ui_capability_discover(app.host,NULL,offset,32,&generation,&page);if(status)return status;
 yyjson_doc *pd=yyjson_read(page,strlen(page),0);yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=d?yyjson_mut_obj(d):NULL,*rs=d?yyjson_mut_arr(d):NULL,*caps=d?yyjson_mut_arr(d):NULL;int ok=o&&rs&&caps&&pd;
 if(ok){yyjson_mut_doc_set_root(d,o);char instance_text[32];snprintf(instance_text,32,"%llu",(unsigned long long)instance);yyjson_mut_obj_add_strcpy(d,o,"instance",instance_text);yyjson_mut_obj_add_strcpy(d,o,"resource",resource?resource:"");yyjson_mut_obj_add_val(d,o,"resources",rs);yyjson_mut_obj_add_val(d,o,"capabilities",caps);yyjson_mut_obj_add_uint(d,o,"next",yyjson_get_uint(yyjson_obj_get(yyjson_doc_get_root(pd),"next")));yyjson_mut_obj_add_uint(d,o,"resource_next",next);yyjson_mut_obj_add_uint(d,o,"resource_offset",resource_offset);
  for(size_t i=0;i<count;++i)yyjson_mut_arr_add_strcpy(d,rs,targets[i].resource_id);
  size_t i,n;yyjson_val *v;yyjson_arr_foreach(yyjson_obj_get(yyjson_doc_get_root(pd),"items"),i,n,v){const char *id=yyjson_get_str(yyjson_obj_get(v,"id"));char *desc=NULL;if(!id||ui_capability_describe(app.host,id,&desc))continue;
   yyjson_doc *dd=yyjson_read(desc,strlen(desc),0);yyjson_val *root=dd?yyjson_doc_get_root(dd):NULL;const char *effect=yyjson_get_str(yyjson_obj_get(root,"effect"));
   if(effect&&!strcmp(effect,"read")&&(!selected.kind||yyjson_get_uint(yyjson_obj_get(root,"target_kind"))==(uint64_t)selected.kind)){yyjson_mut_val *cap=yyjson_mut_obj(d),*fields=yyjson_mut_arr(d);const char *name=yyjson_get_str(yyjson_obj_get(root,"name"));yyjson_mut_obj_add_strcpy(d,cap,"id",id);yyjson_mut_obj_add_strcpy(d,cap,"name",name?name:id);yyjson_mut_obj_add_val(d,cap,"fields",fields);yyjson_val *props=yyjson_obj_get(yyjson_obj_get(root,"output_schema"),"properties");size_t x,z;yyjson_val *key,*val;yyjson_obj_foreach(props,x,z,key,val){(void)val;yyjson_mut_arr_add_strcpy(d,fields,yyjson_get_str(key));}yyjson_mut_arr_add_val(caps,cap);}
   yyjson_doc_free(dd);ui_capability_release(desc);
  }
 }
 char *out=ok?yyjson_mut_write(d,0,NULL):NULL;if(out&&strlen(out)>65536){free(out);out=NULL;status=UI_STATUS_LIMIT_EXCEEDED;}if(out){replace_text(&s->ai_choices,out);free(out);status=UI_STATUS_OK;}else if(!status)status=UI_STATUS_OUT_OF_MEMORY;
 yyjson_doc_free(pd);yyjson_mut_doc_free(d);ui_capability_release(page);return status;
}
static int ai_model_snapshot_matches(host_window_t *s,const char *json)
{yyjson_doc *d=json?yyjson_read(json,strlen(json),0):NULL;const char *id=d?yyjson_get_str(yyjson_obj_get(yyjson_doc_get_root(d),"run_id")):NULL;int ok=id&&!strcmp(id,s->ai_run);yyjson_doc_free(d);return ok;}
/* Read-only selection shared by the page and copied diagnostic. Recheck after
 * disclosure callbacks; a borrowed old snapshot must not follow a new selection. */
static const char *ai_model_task_snapshot(host_window_t *s,const char *run,const char **source)
{
 *source="unavailable";
 if(ai_model_snapshot_matches(s,run)){*source="memory";return run;}
 if(!strcmp(s->ai_stored_id,s->ai_run)&&ai_model_snapshot_matches(s,s->ai_stored_run)){*source="persisted";return s->ai_stored_run;}
 if(s->ai_query_pending)*source="pending";else if(s->ai_query_error&&*s->ai_query_error)*source="error";
 return NULL;
}
static char *ai_model_diagnostic_json(host_window_t *s)
{
 char *run=NULL;const char *source=NULL;
 if(s->ai_run[0])(void)ui_ai_get_run(s->ai,s->ai_run,&run);
 const char *task=ai_model_task_snapshot(s,run,&source);
 json_buffer_t j={0};json_append(&j,"{\"model\":");json_append(&j,s->ai_model?s->ai_model:"null");json_append(&j,",\"selected_run_id\":");json_string(&j,s->ai_run);json_append(&j,",\"task_query_id\":");json_string(&j,s->ai_query_id);json_append(&j,",\"task_source\":");json_string(&j,source);json_append(&j,",\"task\":");json_append(&j,task?task:"null");json_format(&j,",\"display_valid\":%s,\"display_generation\":\"%llu\"",s->ai_display_valid?"true":"false",(unsigned long long)s->ai_display_generation);json_append(&j,",\"query_error\":");json_string(&j,s->ai_query_error);json_append(&j,",\"error\":");json_string(&j,s->ai_error);json_append(&j,",\"config_location\":");json_string(&j,s->ai_config_location);json_append(&j,"}");ui_ai_release(run);if(j.failed){free(j.text);return NULL;}return j.text;
}
static ui_status_t ai_model_copy_diagnostic(host_window_t *s)
{
 json_buffer_t j={0};j.text=ai_model_diagnostic_json(s);if(!j.text)return UI_STATUS_OUT_OF_MEMORY;
 s->ai_copy_error=0;int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,j.text,-1,NULL,0);
 if(!n){s->ai_copy_error=GetLastError();free(j.text);fprintf(stderr,"UH-A clipboard UTF8 error=%lu\n",s->ai_copy_error);return UI_STATUS_PLATFORM_ERROR;}
 HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,(size_t)n*sizeof(wchar_t));wchar_t *p=memory?GlobalLock(memory):NULL;ui_status_t st=UI_STATUS_PLATFORM_ERROR;
 if(!p){s->ai_copy_error=GetLastError();goto done;}
 if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,j.text,-1,p,n)){s->ai_copy_error=GetLastError();GlobalUnlock(memory);goto done;}GlobalUnlock(memory);
 if(!OpenClipboard(s->hwnd)){s->ai_copy_error=GetLastError();goto done;}
 if(!EmptyClipboard()){s->ai_copy_error=GetLastError();CloseClipboard();goto done;}
 if(!SetClipboardData(CF_UNICODETEXT,memory)){s->ai_copy_error=GetLastError();CloseClipboard();goto done;}
 memory=NULL;st=UI_STATUS_OK;CloseClipboard();
 done:if(memory)GlobalFree(memory);free(j.text);if(st)fprintf(stderr,"UH-A clipboard Win32 error=%lu\n",s->ai_copy_error);return st;
}
/* Host settings and state projection. Secret text never enters this bridge. */
static void ai_model_panel_tick(host_window_t *s)
{char *json=NULL;if(ui_ai_model_get_state(s->ai,&json)==UI_STATUS_OK){if(!s->ai_model||strcmp(s->ai_model,json)){replace_text(&s->ai_model,json);++s->ai_projection_generation;schedule_refresh(s);}ui_ai_release(json);}}
static ui_status_t ai_model_display_check(void *data,const char *run,const char *hash,uint64_t generation)
{
 host_window_t *s=data;
  return !s->exiting&&assistant_visible(s)&&s->ai_display_valid&&s->ai_preview_ready&&generation==s->ai_display_generation&&s->ai_projection_rendered==s->ai_projection_generation&&!strcmp(run,s->ai_run)&&!strcmp(run,s->ai_preview_run)&&!strcmp(hash,s->ai_preview_hash)&&ai_projection_marker(s)&&ai_model_preview_displayed(s)?UI_STATUS_OK:UI_STATUS_PERMISSION_DENIED;
}
static void ai_model_panel_bindings(host_window_t *s)
{
 char *run=NULL,*hash[2]={NULL,NULL};int real=0,confirmed=0,phase=-1;
 if(!s->ai||!s->view)return;
  int displayed=s->ai_display_valid&&!s->exiting&&assistant_visible(s)&&s->ai_projection_generation==s->ai_projection_rendered&&s->ai_preview&&ai_projection_marker(s)&&ai_model_preview_displayed(s);s->ai_preview_ready=displayed;
 if(s->ai_model_bound_view!=s->view){for(unsigned i=0;i<2;++i)if(s->ai_model_bindings[i]){if(ui_ai_model_unbind(s->ai,s->ai_model_bindings[i])==UI_STATUS_OK)s->ai_model_bindings[i]=0;else return;}s->ai_model_bound_view=s->view;}
 if(s->ai_run[0]&&ui_ai_get_run(s->ai,s->ai_run,&run)==UI_STATUS_OK){real=strstr(run,"\"real_provider\":true")!=NULL;confirmed=strstr(run,"\"disclosure_confirmed\":true")!=NULL;phase=ai_panel_edit_number(run,"phase");}
 if(s->ai_model&&strstr(s->ai_model,"\"ready\":true")&&!strstr(s->ai_model,"\"pending\":true"))hash[0]=ai_panel_edit_string(s->ai_model,"binding_hash");
 if(displayed&&real&&!confirmed&&phase==UI_AI_WAITING_INPUT&&strstr(run,"\"committed\":true")&&!strcmp(s->ai_preview_run,s->ai_run)){hash[1]=ai_panel_edit_string(run,"binding_hash");if(hash[1]&&strcmp(hash[1],s->ai_preview_hash)){free(hash[1]);hash[1]=NULL;}}
 const char *nodes[2]={"ai-model-key","ai-model-confirm"};
 for(unsigned i=0;i<2;++i){int same=hash[i]&&!strcmp(hash[i],s->ai_model_binding_hash[i])&&(i==0||(!strcmp(s->ai_run,s->ai_model_binding_run)&&s->ai_binding_display_generation==s->ai_display_generation));
  if(s->ai_model_bindings[i]&&!same){ui_status_t status=ui_ai_model_unbind(s->ai,s->ai_model_bindings[i]);if(status==UI_STATUS_OK||status==UI_STATUS_NOT_FOUND){s->ai_model_bindings[i]=0;s->ai_model_binding_hash[i][0]=0;}else {free(hash[i]);continue;}}
  if(hash[i]&&!s->ai_model_bindings[i]){ui_ai_model_light_binding_t d={0};d.size=sizeof(d);d.version=1;d.view=s->view;d.control_id=nodes[i];d.binding_hash=hash[i];d.run_id=i?s->ai_run:NULL;d.action=i?UI_AI_MODEL_CONFIRM:UI_AI_MODEL_KEY;ui_ai_model_display_binding_t display={0};display.size=sizeof(display);display.version=1;display.binding=d;display.generation=s->ai_display_generation;display.check=ai_model_display_check;display.user_data=s;if((i?ui_ai_model_bind_light_display(s->ai,&display,&s->ai_model_bindings[i]):ui_ai_model_bind_light(s->ai,&d,&s->ai_model_bindings[i]))==UI_STATUS_OK){strcpy_s(s->ai_model_binding_hash[i],65,hash[i]);if(i){strcpy_s(s->ai_model_binding_run,65,s->ai_run);s->ai_binding_display_generation=s->ai_display_generation;}}}
  free(hash[i]);
 }ui_ai_release(run);
}
static int ai_model_panel_action(host_window_t *s,host_action_t *a)
{
#if defined(UI_HOST_TEST) && defined(UI_HOST_MODEL_ALLOW)
 if((!strcmp(a->action,"ai-model-save")||!strcmp(a->action,"ai-model-propose")||!strcmp(a->action,"ai-model-continue"))&&!UI_HOST_MODEL_ALLOW(s))return 1;
#endif
 if(strncmp(a->action,"ai-model-",9))return 0;ui_status_t status=UI_STATUS_PERMISSION_DENIED;char *config=NULL,*history=NULL,*projection=NULL;char ref[128],id[65];uint64_t revision=0;
 if(ui_ai_model_get_state(s->ai,&config)==UI_STATUS_OK){char *value=ai_panel_edit_string(config,"credential_ref");if(value){strcpy_s(ref,128,value);free(value);}else ref[0]=0;revision=(uint64_t)ai_panel_edit_number(config,"revision");}
 if(!strcmp(a->action,"ai-model-display")){ai_projection_invalidate(s);schedule_refresh(s);ui_ai_release(config);return 1;}
 if(!strcmp(a->action,"ai-model-draft")){yyjson_doc *d=a->params&&strlen(a->params)<=65536?yyjson_read(a->params,strlen(a->params),0):NULL;if(d&&yyjson_is_obj(yyjson_doc_get_root(d)))replace_text(&s->ai_draft,a->params);yyjson_doc_free(d);ui_ai_release(config);return 1;}
 else if(!strcmp(a->action,"ai-model-catalog")){status=ai_model_catalog(s,read_id(a->id),a->panel,(size_t)read_id(a->command),(size_t)read_id(a->params));}
 else if(!strcmp(a->action,"ai-model-copy")){status=ai_model_copy_diagnostic(s);snprintf(s->ai_copy_notice,sizeof(s->ai_copy_notice),status?"诊断复制失败：status=%d Win32=%lu；任务和凭据错误保留":"已复制脱敏诊断；未授权、未提交、未发送",status,s->ai_copy_error);schedule_refresh(s);ui_ai_release(config);return 1;}
 else if(config&&!strcmp(a->action,"ai-model-save")){ui_ai_model_profile_t p={0};p.size=sizeof(p);p.version=1;p.profile_id="deepseek";p.protocol_profile="deepseek-responses-v1";p.origin="https://api.deepseek.com";p.model_id=a->answer;
#if defined(UI_HOST_TEST) && defined(UI_HOST_MODEL_TRACE)
 UI_HOST_MODEL_TRACE("host-configure",p.model_id,a->token);
#endif
 p.credential_ref=ref;p.revision=revision+1;status=ui_ai_model_configure(s->ai,&p,a->token);}
 else if(config&&(!strcmp(a->action,"ai-model-propose")||!strcmp(a->action,"ai-model-continue"))){ui_ai_real_task_t q={0};ui_app_instance_info_t app={0};ui_cap_target_t target={0};q.size=sizeof(q);q.version=1;q.client_request_id=a->token;q.input_utf8=a->answer;q.profile_id="deepseek";q.profile_revision=revision;history=ai_panel_edit_string(a->params,"history");projection=ai_panel_edit_string(a->params,"projection");q.history_selection_json=history;q.disclosure_projection_json=projection;
  if(a->panel&&*a->panel){if(!get_instance(s,read_id(a->id),&app)||app.closing)goto done;target.size=sizeof(target);target.version=1;status=ui_capability_bind(app.host,a->panel,&target);if(status)goto done;q.targets=&target;q.target_count=1;}
  status=!strcmp(a->action,"ai-model-continue")?ui_ai_real_continue(s->ai,a->command,&q,id):ui_ai_real_propose(s->ai,&q,id);if(status==UI_STATUS_OK){strcpy_s(s->ai_run,65,id);free(s->ai_stored_run);s->ai_stored_run=NULL;}
 }
 /* confirm/key are not page messages; only framework source bindings handle them. */
done:free(history);free(projection);ui_ai_release(config);if(status){char message[128];snprintf(message,sizeof(message),"Model request rejected (%d); input retained; no automatic retry",status);replace_text(&s->ai_error,message);}else replace_text(&s->ai_error,"");schedule_refresh(s);return 1;
}
