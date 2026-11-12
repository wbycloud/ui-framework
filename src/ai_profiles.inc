/* Data lives with the existing host registry; no executor or DLL callback. */
ui_status_t ui_ai_agent_profile_register(ui_host_t *host,const ui_ai_agent_profile_t *d)
{
 registry_t *r;agent_profile_t *p;char *ids=NULL,*json;size_t len;ui_cap_json_doc_t parsed={0};ui_status_t status=registry(host,&r);
 if(status)return status;if(!d||d->size<sizeof(*d)||d->version!=1||!ui_cap_text(d->id,64,0)||!ui_cap_text(d->instructions_utf8,16384,1)||!d->allowed_capability_ids_json)return UI_STATUS_INVALID_ARGUMENT;
 if(!host->capability_read_entries||is_closing(r)||host->dispatch_blocked)return UI_STATUS_PERMISSION_DENIED;
 for(p=r->profiles;p;p=p->next)if(!strcmp(p->id,d->id))return UI_STATUS_ALREADY_EXISTS;
 status=ui_cap_json_canonicalize(d->allowed_capability_ids_json,strlen(d->allowed_capability_ids_json),&ids,&len);if(status)return status;
 status=ui_cap_parse(ids,len,65536,&parsed);if(status){free(ids);return status;}
 {yyjson_val *root=yyjson_doc_get_root(parsed.doc),*v;size_t i,n;status=UI_STATUS_OK;
  if(!yyjson_is_arr(root)||yyjson_arr_size(root)>256)status=UI_STATUS_VALIDATION_FAILED;
  else{yyjson_arr_foreach(root,i,n,v){const char *id=yyjson_get_str(v);capability_t *cap=id?find_cap(r,id):NULL;if(!cap||cap->retiring){status=UI_STATUS_NOT_FOUND;break;}for(size_t j=0;j<i;++j)if(!strcmp(id,yyjson_get_str(yyjson_arr_get(root,j))))status=UI_STATUS_ALREADY_EXISTS;}}
 }
 ui_cap_doc_free(&parsed);if(status){free(ids);return status;}
 {yyjson_mut_doc *m=yyjson_mut_doc_new(NULL);yyjson_mut_val *o=m?yyjson_mut_obj(m):NULL;yyjson_doc *a=yyjson_read(ids,len,0);
  if(!o||!a){yyjson_mut_doc_free(m);yyjson_doc_free(a);free(ids);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(m,o);yyjson_mut_obj_add_strcpy(m,o,"id",d->id);yyjson_mut_obj_add_strcpy(m,o,"instructions",d->instructions_utf8);yyjson_mut_obj_add_val(m,o,"capabilities",yyjson_val_mut_copy(m,yyjson_doc_get_root(a)));json=yyjson_mut_write(m,0,NULL);yyjson_mut_doc_free(m);yyjson_doc_free(a);}
 free(ids);if(!json)return UI_STATUS_OUT_OF_MEMORY;len=sizeof(*p)+strlen(d->id)+1+strlen(json)+1;
 if(len>65536||r->profile_count==8||len>262144-r->profile_bytes){free(json);return UI_STATUS_LIMIT_EXCEEDED;}
 p=calloc(1,sizeof(*p));if(!p){free(json);return UI_STATUS_OUT_OF_MEMORY;}p->id=ui_strdup(d->id);if(!p->id){free(json);free(p);return UI_STATUS_OUT_OF_MEMORY;}p->json=json;p->bytes=len;p->next=r->profiles;r->profiles=p;r->profile_bytes+=len;++r->profile_count;return UI_STATUS_OK;
}
ui_status_t ui_ai_agent_profile_unregister(ui_host_t *host,const char *id)
{registry_t *r;agent_profile_t **at,*p;ui_status_t status=registry(host,&r);if(status)return status;if(!ui_cap_text(id,64,0))return UI_STATUS_INVALID_ARGUMENT;for(at=&r->profiles;*at&&strcmp((*at)->id,id);at=&(*at)->next){}if(!*at)return UI_STATUS_NOT_FOUND;p=*at;*at=p->next;r->profile_bytes-=p->bytes;--r->profile_count;free(p->id);free(p->json);free(p);return UI_STATUS_OK;}
ui_status_t ui_ai_agent_profile_get(ui_host_t *host,const char *id,char **out)
{registry_t *r;agent_profile_t *p;ui_status_t status=registry(host,&r);if(out)*out=NULL;if(status)return status;if(!out||!ui_cap_text(id,64,0))return UI_STATUS_INVALID_ARGUMENT;if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;for(p=r->profiles;p&&strcmp(p->id,id);p=p->next){}if(!p)return UI_STATUS_NOT_FOUND;*out=ui_strdup(p->json);return *out?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;}

ui_status_t ui_ai_agent_profile_list(ui_host_t *host,char **out)
{
 registry_t *r;ui_status_t status=registry(host,&r);if(out)*out=NULL;
 if(status)return status;if(!out)return UI_STATUS_INVALID_ARGUMENT;
 if(is_closing(r)||host->dispatch_blocked)return UI_STATUS_CANCELLED;
 yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *a=d?yyjson_mut_arr(d):NULL;
 if(!a){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,a);
 for(agent_profile_t *p=r->profiles;p;p=p->next){
  yyjson_doc *v=yyjson_read(p->json,strlen(p->json),0);
  int ok=v&&yyjson_mut_arr_add_val(a,yyjson_val_mut_copy(d,yyjson_doc_get_root(v)));
  yyjson_doc_free(v);if(!ok){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}
 }
 *out=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);return *out?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;
}
