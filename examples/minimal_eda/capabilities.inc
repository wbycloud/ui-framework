#include "ui_framework/ai_operation.h"
#define EDA_NOTE_INPUT "{\"type\":\"object\",\"properties\":{\"text\":{\"type\":\"string\"},\"base_revision\":{\"type\":\"string\",\"maxLength\":127},\"generation\":{\"type\":\"string\",\"maxLength\":20}},\"required\":[\"text\",\"base_revision\",\"generation\"],\"additionalProperties\":false}"
#define EDA_SELECT_INPUT "{\"type\":\"object\",\"properties\":{\"ids\":{\"type\":\"array\",\"items\":{\"type\":\"string\",\"maxLength\":20},\"maxItems\":1}},\"required\":[\"ids\"],\"additionalProperties\":false}"
static const char eda_document_output[]="{\"type\":\"object\",\"properties\":{\"revision\":{\"type\":\"string\"},\"note\":{\"type\":\"string\"},\"objects\":{\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"string\"},\"x\":{\"type\":\"number\"},\"y\":{\"type\":\"number\"},\"width\":{\"type\":\"number\"},\"height\":{\"type\":\"number\"}},\"required\":[\"id\",\"x\",\"y\",\"width\",\"height\"],\"additionalProperties\":false},\"maxItems\":64},\"zoom\":{\"type\":\"number\"},\"selection\":{\"type\":\"array\",\"items\":{\"type\":\"string\"},\"maxItems\":1},\"draft_conflict\":{\"type\":\"boolean\"},\"draft_dirty\":{\"type\":\"boolean\"}},\"required\":[\"revision\",\"note\",\"objects\"],\"additionalProperties\":false}";
static const char eda_view_output[]="{\"type\":\"object\",\"properties\":{\"revision\":{\"type\":\"string\"},\"note\":{\"type\":\"string\"},\"objects\":{\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"string\"},\"x\":{\"type\":\"number\"},\"y\":{\"type\":\"number\"},\"width\":{\"type\":\"number\"},\"height\":{\"type\":\"number\"}},\"required\":[\"id\",\"x\",\"y\",\"width\",\"height\"],\"additionalProperties\":false},\"maxItems\":64},\"zoom\":{\"type\":\"number\"},\"selection\":{\"type\":\"array\",\"items\":{\"type\":\"string\"},\"maxItems\":1},\"draft_conflict\":{\"type\":\"boolean\"},\"draft_dirty\":{\"type\":\"boolean\"}},\"required\":[\"revision\",\"zoom\",\"selection\",\"draft_conflict\",\"draft_dirty\"],\"additionalProperties\":false}";
static const p2_spec_t eda_specs[]={
 {"eda.document.read","Read all blocks and committed note","read",p2_empty_schema,eda_document_output,"document",0},
 {"eda.view.read","Read zoom and object selection","read",p2_empty_schema,eda_view_output,"view",1},
 {"eda.add_block","Add fixed rectangle (A)","write",p2_empty_schema,eda_document_output,"document",0},
 {"eda.clear","Clear blocks and reset zoom (C)","write",p2_empty_schema,eda_document_output,"document",0},
 {"eda.zoom_in","Zoom in (Z)","view",p2_empty_schema,eda_view_output,"view",1},
 {"eda.select","Select existing object","view",EDA_SELECT_INPUT,eda_view_output,"view",1},
 {"eda.note.submit","Commit note draft","write",EDA_NOTE_INPUT,eda_document_output,"document",0},
 {"eda.note.cancel","Discard note draft and reload","draft",p2_empty_schema,eda_view_output,"draft",1},
 {"eda.note.keep","Keep conflicted draft explicitly","draft",p2_empty_schema,eda_view_output,"draft",1}
};
static ui_status_t eda_revision(void *data,unsigned resource,char out[128])
{eda_state_t *e=data;if(!resource)snprintf(out,128,"doc-%llu",(unsigned long long)e->revision);else snprintf(out,128,"view-%llu-draft-%llu-%d-%d",(unsigned long long)e->view_revision,(unsigned long long)e->note_generation,e->note_dirty,e->note_conflict);return UI_STATUS_OK;}
static ui_status_t eda_describe(void *data,unsigned resource,yyjson_mut_doc *doc,yyjson_mut_val *root)
{
 eda_state_t *e=data;char revision[128];(void)eda_revision(e,resource,revision);yyjson_mut_obj_add_strcpy(doc,root,"revision",revision);
 if(!resource){yyjson_mut_val *objects=yyjson_mut_arr(doc);yyjson_mut_obj_add_strcpy(doc,root,"note",e->committed_note?e->committed_note:"");
   for(size_t i=0;i<e->block_count;++i){eda_block_t *b=&e->blocks[i];char id[24];yyjson_mut_val *object=yyjson_mut_obj(doc);snprintf(id,sizeof(id),"%llu",(unsigned long long)b->id);yyjson_mut_obj_add_strcpy(doc,object,"id",id);yyjson_mut_obj_add_real(doc,object,"x",b->x);yyjson_mut_obj_add_real(doc,object,"y",b->y);yyjson_mut_obj_add_real(doc,object,"width",b->width);yyjson_mut_obj_add_real(doc,object,"height",b->height);yyjson_mut_arr_add_val(objects,object);}yyjson_mut_obj_add_val(doc,root,"objects",objects);
 }else{yyjson_mut_val *selection=yyjson_mut_arr(doc);if(e->selected){char id[24];snprintf(id,sizeof(id),"%llu",(unsigned long long)e->selected);yyjson_mut_arr_add_strcpy(doc,selection,id);}yyjson_mut_obj_add_val(doc,root,"selection",selection);yyjson_mut_obj_add_real(doc,root,"zoom",e->zoom);yyjson_mut_obj_add_bool(doc,root,"draft_dirty",e->note_dirty!=0);yyjson_mut_obj_add_bool(doc,root,"draft_conflict",e->note_conflict!=0);}
 return UI_STATUS_OK;
}
static void eda_draft_project(eda_state_t *e)
{if(e->capabilities.staging)return;if(e->note_dirty){e->note_conflict=e->note_base!=e->revision;return;}e->note_base=e->revision;e->note_conflict=0;}
static ui_status_t eda_note_reload(eda_state_t *e)
{const char *text=e->committed_note?e->committed_note:"";int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,NULL,0);wchar_t *wide;if(!count)return UI_STATUS_VALIDATION_FAILED;wide=malloc((size_t)count*sizeof(*wide));if(!wide)return UI_STATUS_OUT_OF_MEMORY;(void)MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,wide,count);e->projecting=1;if(e->note)SetWindowTextW(e->note,wide);e->projecting=0;free(wide);e->note_dirty=e->note_conflict=0;e->note_base=e->revision;++e->note_generation;return UI_STATUS_OK;}
static ui_status_t eda_run(void *data,const char *id,yyjson_val *params,yyjson_mut_doc *out,yyjson_mut_val *root,uint64_t call)
{
 eda_state_t *e=data;p2_cap_t *cap=p2_find(&e->capabilities,id);unsigned resource=cap->spec->resource;(void)call;
 if(!strcmp(id,"eda.add_block")){eda_block_t *b;if(e->block_count==64||e->next_id==UINT64_MAX)return UI_STATUS_LIMIT_EXCEEDED;b=&e->blocks[e->block_count];b->id=++e->next_id;b->x=-.8f+(float)(e->block_count%5u)*.35f;b->y=.6f-(float)(e->block_count/5u)*.25f;b->width=.22f;b->height=.12f;++e->block_count;++e->revision;eda_draft_project(e);}
 else if(!strcmp(id,"eda.clear")){if(e->block_count){e->block_count=0;++e->revision;eda_draft_project(e);}if(e->zoom!=1.0f||e->selected){if(e->capabilities.staging)e->capabilities.staging_effect=1;else{e->zoom=1.0f;e->selected=0;++e->view_revision;}}}
 else if(!strcmp(id,"eda.zoom_in")){float zoom=e->zoom*1.1f;if(zoom>4)zoom=4;if(zoom!=e->zoom){e->zoom=zoom;++e->view_revision;}}
 else if(!strcmp(id,"eda.select")){yyjson_val *v=yyjson_arr_get(yyjson_obj_get(params,"ids"),0);uint64_t selected=0;if(v){int found=0;if(!p2_id(yyjson_get_str(v),&selected))return UI_STATUS_VALIDATION_FAILED;for(size_t i=0;i<e->block_count;++i)if(e->blocks[i].id==selected)found=1;if(!found)return UI_STATUS_NOT_FOUND;}if(selected!=e->selected){e->selected=selected;++e->view_revision;}}
 else if(!strcmp(id,"eda.note.submit")){char revision[128];uint64_t generation;const char *text=p2_text(params,"text");char *copy;(void)eda_revision(e,0,revision);
   if(strcmp(revision,p2_text(params,"base_revision"))){e->note_conflict=e->note_dirty=1;update_properties(e);return UI_STATUS_VALIDATION_FAILED;}
   if(!p2_id(p2_text(params,"generation"),&generation))return UI_STATUS_VALIDATION_FAILED;copy=_strdup(text);if(!copy)return UI_STATUS_OUT_OF_MEMORY;
   if(!e->committed_note||strcmp(e->committed_note,copy)){free(e->committed_note);e->committed_note=copy;++e->revision;}else free(copy);
   if(!e->capabilities.staging&&e->note_generation==generation){e->note_dirty=e->note_conflict=0;e->note_base=e->revision;}else eda_draft_project(e);
 }else if(!strcmp(id,"eda.note.cancel")){ui_status_t status=eda_note_reload(e);if(status)return status;}
 else if(!strcmp(id,"eda.note.keep")){e->note_base=e->revision;e->note_conflict=0;++e->note_generation;}
 if(e->capabilities.staging)return eda_describe(e,resource,out,root);update_properties(e);if(e->active&&e->canvas)(void)ui_surface_invalidate(e->canvas);return eda_describe(e,resource,out,root);
}
typedef struct eda_business {uint64_t revision;size_t count;eda_block_t blocks[64];char note[1];} eda_business_t;
static ui_status_t eda_capture(void *data,void **out,size_t *bytes)
{eda_state_t *e=data;size_t length=strlen(e->committed_note?e->committed_note:"");eda_business_t *b=calloc(1,sizeof(*b)+length);if(!b)return UI_STATUS_OUT_OF_MEMORY;b->revision=e->revision;b->count=e->block_count;memcpy(b->blocks,e->blocks,sizeof(b->blocks));memcpy(b->note,e->committed_note?e->committed_note:"",length+1);*out=b;*bytes=sizeof(*b)+length;return UI_STATUS_OK;}
static ui_status_t eda_restore(void *data,const void *copy,size_t bytes,int publish)
{eda_state_t *e=data;const eda_business_t *b=copy;char *note;uint64_t revision=e->revision;if(!b||bytes<sizeof(*b)||b->count>64||!memchr(b->note,0,bytes-(size_t)((const char *)b->note-(const char *)b)))return UI_STATUS_INVALID_ARGUMENT;if(publish&&revision==UINT64_MAX)return UI_STATUS_LIMIT_EXCEEDED;note=_strdup(b->note);if(!note)return UI_STATUS_OUT_OF_MEMORY;free(e->committed_note);e->committed_note=note;e->block_count=b->count;memcpy(e->blocks,b->blocks,sizeof(e->blocks));e->revision=publish?revision+1:b->revision;return UI_STATUS_OK;}
static ui_status_t eda_publish(void *data)
{eda_state_t *e=data;ui_status_t status=UI_STATUS_OK;if(e->capabilities.transaction_committed&&!e->capabilities.transaction_is_undo&&!strcmp(e->capabilities.transaction_capability,"eda.clear")){if(e->zoom!=1||e->selected)++e->view_revision;e->zoom=1;e->selected=0;}
 eda_draft_project(e);if(!e->note_dirty)status=eda_note_reload(e);update_properties(e);if(e->active&&e->canvas)(void)ui_surface_invalidate(e->canvas);return status;}
static ui_status_t eda_capabilities(eda_state_t *e,const ui_app_context_t *context)
{const char *resources[]={"eda.document","eda.view"};ui_cap_resource_kind_t kinds[]={UI_CAP_DOCUMENT,UI_CAP_INSTANCE_VIEW};e->revision=e->view_revision=e->note_generation=1;e->capabilities.capture_business=eda_capture;e->capabilities.restore_business=eda_restore;e->capabilities.project_business=eda_publish;e->capabilities.run=eda_run;e->capabilities.revision=eda_revision;e->capabilities.describe=eda_describe;
 ui_status_t status=p2_register(&e->capabilities,context,e,eda_specs,sizeof(eda_specs)/sizeof(eda_specs[0]),resources,kinds,2);
 if(status)return status;
 return ui_ai_operation_register(context->host,
  "{\"version\":1,\"id\":\"drawing.blocks\",\"title\":\"图块\",\"resource\":\"eda.document\","
  "\"observe\":{\"capability\":\"eda.document.read\",\"fields\":{\"objects\":\"count\"}},"
  "\"actions\":[{\"capability\":\"eda.add_block\",\"identity\":[],\"sources\":{}}],\"max_actions\":8}");}
static ui_status_t eda_collect(const ui_cap_input_context_t *context,char *out,size_t capacity,size_t *length,void *data)
{
 p2_cap_t *cap=data;eda_state_t *e=cap->app->model;yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *root=d?yyjson_mut_obj(d):NULL;ui_status_t status=UI_STATUS_OK;
 if(!root){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,root);
 if(!strcmp(cap->spec->id,"eda.note.submit")){int count=GetWindowTextLengthW(e->note);wchar_t *wide;char *text;int bytes;char revision[128],generation[24];if(count<0||(size_t)count>UI_CAPABILITY_INPUT_BYTES){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}wide=malloc(((size_t)count+1)*sizeof(*wide));if(!wide){status=UI_STATUS_OUT_OF_MEMORY;goto done;}GetWindowTextW(e->note,wide,count+1);bytes=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wide,-1,NULL,0,NULL,NULL);text=bytes?malloc((size_t)bytes):NULL;if(!text){free(wide);status=bytes?UI_STATUS_OUT_OF_MEMORY:UI_STATUS_VALIDATION_FAILED;goto done;}WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,wide,-1,text,bytes,NULL,NULL);free(wide);
   snprintf(revision,sizeof(revision),"doc-%llu",(unsigned long long)e->note_base);snprintf(generation,sizeof(generation),"%llu",(unsigned long long)e->note_generation);yyjson_mut_obj_add_strcpy(d,root,"text",text);yyjson_mut_obj_add_strcpy(d,root,"base_revision",revision);yyjson_mut_obj_add_strcpy(d,root,"generation",generation);free(text);
 }else if(!strcmp(cap->spec->id,"eda.select")){ui_rect_t rect={0};yyjson_mut_val *ids=yyjson_mut_arr(d);uint64_t selected=0;(void)ui_surface_get_rect(e->canvas,&rect);if(rect.width<=0||rect.height<=0){status=UI_STATUS_VALIDATION_FAILED;goto done;}
   {double aspect=(double)rect.width/rect.height,x=((double)context->event.x/rect.width*2-1)*aspect/e->zoom,y=(1-(double)context->event.y/rect.height*2)/e->zoom;
    for(size_t i=e->block_count;i>0;--i){eda_block_t *b=&e->blocks[i-1];if(x>=b->x&&x<=b->x+b->width&&y>=b->y&&y<=b->y+b->height){selected=b->id;break;}}}
   if(selected){char id[24];snprintf(id,sizeof(id),"%llu",(unsigned long long)selected);yyjson_mut_arr_add_strcpy(d,ids,id);}yyjson_mut_obj_add_val(d,root,"ids",ids);}
 status=p2_json_copy(d,out,capacity,length);done:yyjson_mut_doc_free(d);return status;
}
static ui_status_t eda_bindings(eda_state_t *e)
{
 const char *ids[]={"eda.add_block","eda.zoom_in","eda.clear","eda.select","eda.note.submit","eda.note.cancel","eda.note.keep"};
 for(unsigned i=0;i<7;++i){ui_cap_input_binding_desc_t d={0};uint64_t binding;p2_cap_t *cap=p2_find(&e->capabilities,ids[i]);d.size=sizeof(d);d.version=1;d.capability_id=ids[i];d.resource=e->capabilities.resources[cap->spec->resource].ref;d.purpose=UI_CAP_UI_INVOKE;d.collect=eda_collect;d.observe=p2_notice;d.user_data=cap;
   if(i<4){d.kind=UI_CAP_INPUT_SURFACE;d.surface=e->canvas;d.trigger=i<3?UI_INPUT_KEY_DOWN:UI_INPUT_POINTER_UP;if(i<3){const unsigned keys[]={'A','Z','C'};d.key=keys[i];}else d.pointer_button=1;}
   else{d.kind=UI_CAP_INPUT_NATIVE_BUTTON;d.native_button=i==4?e->submit:i==5?e->cancel:e->keep;d.trigger=UI_INPUT_POINTER_UP;d.pointer_button=1;}
   {ui_status_t status=ui_capability_input_bind(e->context.host,&d,&binding);if(status)return status;}}
 return UI_STATUS_OK;
}
