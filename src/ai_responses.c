#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "ai_responses.h"
#include "capability_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* Allocation owner travels with the buffer, including cross-thread frees.
 * A job installs its prepaid 2MiB pool; no fallback when that pool is full. */
typedef struct am_allocation {yyjson_alc owner;size_t bytes;} am_allocation_t;
__declspec(thread) static yyjson_alc *am_memory;
yyjson_alc *am_memory_enter(yyjson_alc *a){yyjson_alc *old=am_memory;am_memory=a;return old;}
void *am_allocate(size_t n)
{if(n>SIZE_MAX-sizeof(am_allocation_t))return NULL;am_allocation_t *p=am_memory?am_memory->malloc(am_memory->ctx,n+sizeof(*p)):malloc(n+sizeof(*p));if(!p)return NULL;memset(p,0,sizeof(*p));if(am_memory)p->owner=*am_memory;p->bytes=n;return p+1;}
void am_delete(void *p){if(p){am_allocation_t *h=(am_allocation_t *)p-1;if(h->owner.free)h->owner.free(h->owner.ctx,h);else free(h);}}
void *am_resize(void *p,size_t n)
{
 if(!p)return am_allocate(n);if(n>SIZE_MAX-sizeof(am_allocation_t))return NULL;am_allocation_t *h=(am_allocation_t *)p-1;yyjson_alc a=h->owner;size_t old=h->bytes;
 /* yyjson's bounded pool realloc supports growth only. Shrink within the
  * original owner without copying the old capacity into a smaller block. */
 if(a.realloc&&n<old){am_allocation_t *v=a.malloc(a.ctx,sizeof(*h)+n);if(!v)return NULL;memcpy(v,h,sizeof(*h)+n);v->bytes=n;a.free(a.ctx,h);return v+1;}
 am_allocation_t *v=a.realloc?a.realloc(a.ctx,h,sizeof(*h)+old,sizeof(*h)+n):realloc(h,sizeof(*h)+n);if(!v)return NULL;v->bytes=n;return v+1;
}
static void *am_malloc_cb(void *ctx,size_t n){(void)ctx;return am_allocate(n);}
static void *am_realloc_cb(void *ctx,void *p,size_t old,size_t n){(void)ctx;(void)old;return am_resize(p,n);}
static void am_free_cb(void *ctx,void *p){(void)ctx;am_delete(p);}
static const yyjson_alc am_writer={am_malloc_cb,am_realloc_cb,am_free_cb,NULL};
char *am_value_write(yyjson_val *v){return yyjson_val_write_opts(v,0,&am_writer,NULL,NULL);}
char *am_mutable_write(yyjson_mut_doc *v){return yyjson_mut_write_opts(v,0,&am_writer,NULL,NULL);}
void am_json_free(ui_cap_json_doc_t *p){if(p->doc)yyjson_doc_free(p->doc);am_delete(p->pool);memset(p,0,sizeof(*p));}
static int am_bad(am_stream_t *s,const char *code){s->failed=1;snprintf(s->error,sizeof(s->error),"%s",code);return 0;}
static int am_inspect(yyjson_val *v,unsigned depth,unsigned *nodes)
{
 if(depth>16||++*nodes>4096)return 0;
 if(yyjson_is_str(v))return ui_cap_text(yyjson_get_str(v),AM_EVENT,1)&&strlen(yyjson_get_str(v))==yyjson_get_len(v);
 if(yyjson_is_arr(v)){size_t i,n;yyjson_val *x;if(yyjson_arr_size(v)>512)return 0;yyjson_arr_foreach(v,i,n,x)if(!am_inspect(x,depth+1,nodes))return 0;}
 if(yyjson_is_obj(v)){yyjson_obj_iter it=yyjson_obj_iter_with(v);yyjson_val *key;const char *keys[128];unsigned count=0;if(yyjson_obj_size(v)>128)return 0;while((key=yyjson_obj_iter_next(&it))){const char *name=yyjson_get_str(key);if(!name||strlen(name)!=yyjson_get_len(key))return 0;for(unsigned i=0;i<count;++i)if(!strcmp(keys[i],name))return 0;keys[count++]=name;if(!am_inspect(yyjson_obj_iter_get_val(key),depth+1,nodes))return 0;}}
 return 1;
}
int am_json_parse(const char *text,size_t n,ui_cap_json_doc_t *p)
{
 yyjson_alc a;unsigned nodes=0;size_t pool=yyjson_read_max_memory_usage(n,0);memset(p,0,sizeof(*p));if(!text||!n||n>AM_EVENT)return 0;if(pool>512u*1024u)pool=512u*1024u;p->pool=am_allocate(pool);if(!p->pool)return 0;
 if(!yyjson_alc_pool_init(&a,p->pool,pool)){am_json_free(p);return 0;}p->doc=yyjson_read_opts((char *)text,n,0,&a,NULL);if(!p->doc||!am_inspect(yyjson_doc_get_root(p->doc),0,&nodes)){am_json_free(p);return 0;}return 1;
}
static const char *am_str(yyjson_val *v,const char *k){return yyjson_get_str(yyjson_obj_get(v,k));}
static int am_text(const char *s,size_t n){return s&&ui_cap_text(s,n,1);}
static am_function_t *am_item(am_stream_t *s,const char *id)
{for(unsigned i=0;id&&i<s->count;++i)if(!strcmp(s->functions[i].item,id))return &s->functions[i];return NULL;}
static int am_final(am_stream_t *s,yyjson_val *r)
{
 yyjson_val *out=yyjson_obj_get(r,"output"),*item;size_t i,n,used=0;unsigned calls=0;unsigned char seen[64]={0};char *text=am_allocate(65537);int ok=text&&yyjson_is_arr(out)&&am_str(r,"id")&&!strcmp(am_str(r,"id"),s->response)&&am_str(r,"status")&&!strcmp(am_str(r,"status"),"completed");
 if(text)memset(text,0,65537);
 if(ok)yyjson_arr_foreach(out,i,n,item){const char *type=am_str(item,"type");if(!type){ok=0;break;}
  if(!strcmp(type,"function_call")){am_function_t *f=am_item(s,am_str(item,"id"));const char *a=am_str(item,"arguments"),*c=am_str(item,"call_id"),*name=am_str(item,"name");if(!f||!f->done||!a||!c||!name||strcmp(f->args?f->args:"",a)||strcmp(f->call,c)||strcmp(f->name,name)){ok=0;break;}if(seen[f-s->functions]++){ok=0;break;}++calls;}
  else if(!strcmp(type,"message")){yyjson_val *parts=yyjson_obj_get(item,"content"),*p;size_t j,m;if(!yyjson_is_arr(parts)){ok=0;break;}yyjson_arr_foreach(parts,j,m,p){const char *t=am_str(p,"type"),*v=am_str(p,"text");size_t z=v?strlen(v):0;if(!t||strcmp(t,"output_text")||!am_text(v,65536)||z>65536-used){ok=0;break;}memcpy(text+used,v,z);used+=z;}}
  else{ok=0;break;}
 }
 if(ok)ok=calls==s->count&&used==s->text_len&&!memcmp(text,s->text,used);am_delete(text);
 if(!ok)return am_bad(s,"invalid_final_response");s->final_json=am_value_write(r);if(!s->final_json||strlen(s->final_json)>AM_EVENT)return am_bad(s,"response_limit");s->complete=1;return 1;
}
static int am_event(am_stream_t *s)
{
 ui_cap_json_doc_t d={0};yyjson_val *o,*v;const char *type,*id;char hash[65];uint64_t seq;int ok=1;
 if(!s->data_len){s->event[0]=0;return 1;}
 if(!am_json_parse(s->data,s->data_len,&d))return am_bad(s,"invalid_event_json");o=yyjson_doc_get_root(d.doc);type=am_str(o,"type");v=yyjson_obj_get(o,"sequence_number");
 if(!type||!yyjson_is_uint(v)||(s->event[0]&&strcmp(s->event,type))||ui_cap_hash_bytes(s->data,s->data_len,hash)){ok=am_bad(s,"event_identity");goto done;}seq=yyjson_get_uint(v);
 if(s->sequence_seen&&seq==s->sequence){ok=!strcmp(hash,s->last_hash);if(!ok)am_bad(s,"duplicate_event_conflict");goto done;}
 if(s->complete){ok=am_bad(s,"event_after_terminal");goto done;}if(s->sequence_seen&&seq!=s->sequence+1){ok=am_bad(s,"event_order");goto done;}s->sequence_seen=1;s->sequence=seq;strcpy_s(s->last_hash,65,hash);
 if(!strcmp(type,"response.created")){id=am_str(yyjson_obj_get(o,"response"),"id");if(s->response[0]||!am_text(id,256)||!*id)ok=am_bad(s,"response_identity");else strcpy_s(s->response,257,id);}
 else if(!s->response[0])ok=am_bad(s,"missing_created");
 else if(am_str(o,"response_id")&&strcmp(am_str(o,"response_id"),s->response))ok=am_bad(s,"response_identity");
 else if(!strcmp(type,"response.output_text.delta")){const char *t=am_str(o,"delta");size_t z=t?strlen(t):0;if(!am_text(t,65536)||z>65536-s->text_len)ok=am_bad(s,"output_limit");else{memcpy(s->text+s->text_len,t,z);s->text_len+=z;s->text[s->text_len]=0;}}
 else if(!strcmp(type,"response.output_item.added")){v=yyjson_obj_get(o,"item");const char *t=am_str(v,"type");if(!t)ok=am_bad(s,"invalid_item");else if(!strcmp(t,"function_call")){am_function_t *f;const char *c=am_str(v,"call_id"),*name=am_str(v,"name");id=am_str(v,"id");if(s->count==64||!am_text(id,256)||!*id||!am_text(c,256)||!*c||!am_text(name,64)||am_item(s,id))ok=am_bad(s,"invalid_function");else{for(unsigned i=0;i<s->count;++i)if(!strcmp(c,s->functions[i].call))ok=0;if(!ok)am_bad(s,"duplicate_call_id");else{f=&s->functions[s->count++];strcpy_s(f->item,257,id);strcpy_s(f->call,257,c);strcpy_s(f->name,65,name);}}}else if(strcmp(t,"message"))ok=am_bad(s,"unsupported_item");}
 else if(!strcmp(type,"response.function_call_arguments.delta")){am_function_t *f=am_item(s,am_str(o,"item_id"));const char *a=am_str(o,"delta");size_t z=a?strlen(a):0;if(!f||f->done||!am_text(a,65536)||z>65536-s->args_bytes)ok=am_bad(s,"arguments_limit");else{char *q=am_resize(f->args,f->length+z+1);if(!q)ok=am_bad(s,"out_of_memory");else{f->args=q;memcpy(q+f->length,a,z);f->length+=z;q[f->length]=0;s->args_bytes+=z;}}}
 else if(!strcmp(type,"response.function_call_arguments.done")){am_function_t *f=am_item(s,am_str(o,"item_id"));const char *a=am_str(o,"arguments");if(!f||f->done||!a||strcmp(f->args?f->args:"",a))ok=am_bad(s,"arguments_conflict");else f->done=1;}
 else if(!strcmp(type,"response.completed"))ok=am_final(s,yyjson_obj_get(o,"response"));
 else if(!strcmp(type,"response.failed")||!strcmp(type,"response.incomplete")||!strcmp(type,"error"))ok=am_bad(s,"provider_failed_or_incomplete");
 else if(strcmp(type,"response.in_progress")&&strcmp(type,"response.content_part.added")&&strcmp(type,"response.content_part.done")&&strcmp(type,"response.output_text.done")&&strcmp(type,"response.output_item.done"))ok=am_bad(s,"unsupported_event");
done:am_json_free(&d);s->data_len=0;s->data[0]=0;s->event[0]=0;return ok;
}
static int am_line(am_stream_t *s)
{
 char *line=s->line;size_t n=s->line_len;s->line[n]=0;s->line_len=0;
 if(!n)return am_event(s);if(line[0]==':')return 1;
 if(!strncmp(line,"data:",5)){char *p=line+5;if(*p==' ')++p;size_t z=strlen(p);if((s->data_len&&s->data_len==AM_EVENT)||z>AM_EVENT-s->data_len-(s->data_len?1u:0u))return am_bad(s,"event_limit");if(s->data_len)s->data[s->data_len++]='\n';memcpy(s->data+s->data_len,p,z);s->data_len+=z;s->data[s->data_len]=0;}
 else if(!strncmp(line,"event:",6)){char *p=line+6;if(*p==' ')++p;if(!am_text(p,79))return am_bad(s,"event_name");strcpy_s(s->event,80,p);}
 else return am_bad(s,"unsupported_sse_field");return 1;
}
int am_stream_feed(am_stream_t *s,const void *bytes,size_t n)
{
 const unsigned char *p=bytes;if(s->failed||n>AM_WIRE-s->wire)return am_bad(s,"wire_limit");s->wire+=n;
 for(size_t i=0;i<n;++i){unsigned char c=p[i];if(!c)return am_bad(s,"nul_stream");if(s->cr){s->cr=0;if(c=='\n')continue;}if(c=='\r'||c=='\n'){if(!am_line(s))return 0;s->cr=c=='\r';}else{if(s->line_len==AM_EVENT)return am_bad(s,"line_limit");s->line[s->line_len++]=(char)c;}}return 1;
}
int am_stream_eof(am_stream_t *s){if(s->failed||s->line_len||s->data_len||!s->complete)return am_bad(s,"truncated_stream");return 1;}
void am_stream_clear(am_stream_t *s){for(unsigned i=0;i<s->count;++i)am_delete(s->functions[i].args);am_delete(s->final_json);memset(s,0,sizeof(*s));}
