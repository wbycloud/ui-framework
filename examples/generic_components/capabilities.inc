typedef struct sample_form {sample_t *app;p2_cap_t *cap;ui_component_t *component;uint64_t binding;} sample_form_t;
#define SAMPLE_FIELDS "{\"type\":\"object\",\"properties\":{\"fields\":{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\",\"maxLength\":4096},\"size\":{\"type\":\"number\",\"minimum\":0},\"enabled\":{\"type\":\"boolean\"},\"mode\":{\"type\":\"string\",\"enum\":[\"自动\",\"手动\",\"混合\"]},\"notes\":{\"type\":\"string\",\"maxLength\":4096}},\"additionalProperties\":false}},\"required\":[\"fields\"],\"additionalProperties\":false}"
#define SAMPLE_CELL "{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"string\",\"maxLength\":20},\"column\":{\"type\":\"string\",\"maxLength\":32},\"text\":{\"type\":\"string\",\"maxLength\":255}},\"required\":[\"id\",\"column\",\"text\"],\"additionalProperties\":false}"
#define SAMPLE_RENAME "{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"string\",\"maxLength\":20},\"text\":{\"type\":\"string\",\"maxLength\":255}},\"required\":[\"id\",\"text\"],\"additionalProperties\":false}"
#define SAMPLE_TARGET "{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"string\",\"maxLength\":20},\"component\":{\"type\":\"string\",\"enum\":[\"table\",\"tree\"]}},\"required\":[\"id\",\"component\"],\"additionalProperties\":false}"
static const char sample_document_output[]="{\"type\":\"object\",\"properties\":{\"revision\":{\"type\":\"string\"},\"name\":{\"type\":\"string\"},\"size\":{\"type\":\"number\"},\"enabled\":{\"type\":\"boolean\"},\"mode\":{\"type\":\"string\"},\"notes\":{\"type\":\"string\"},\"changes\":{\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"string\"},\"column\":{\"type\":\"string\"},\"text\":{\"type\":\"string\"}},\"required\":[\"id\",\"column\",\"text\"],\"additionalProperties\":false},\"maxItems\":64},\"table_selected\":{\"type\":\"string\"},\"tree_selected\":{\"type\":\"string\"},\"image_version\":{\"type\":\"integer\"},\"refuse_close\":{\"type\":\"boolean\"}},\"required\":[\"revision\",\"name\",\"size\",\"enabled\",\"mode\",\"notes\",\"changes\"],\"additionalProperties\":false}";
static const char sample_view_output[]="{\"type\":\"object\",\"properties\":{\"revision\":{\"type\":\"string\"},\"name\":{\"type\":\"string\"},\"size\":{\"type\":\"number\"},\"enabled\":{\"type\":\"boolean\"},\"mode\":{\"type\":\"string\"},\"notes\":{\"type\":\"string\"},\"changes\":{\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"string\"},\"column\":{\"type\":\"string\"},\"text\":{\"type\":\"string\"}},\"required\":[\"id\",\"column\",\"text\"],\"additionalProperties\":false},\"maxItems\":64},\"table_selected\":{\"type\":\"string\"},\"tree_selected\":{\"type\":\"string\"},\"image_version\":{\"type\":\"integer\"},\"refuse_close\":{\"type\":\"boolean\"},\"dialog_open\":{\"type\":\"boolean\"}},\"required\":[\"revision\",\"table_selected\",\"tree_selected\",\"image_version\",\"refuse_close\",\"dialog_open\"],\"additionalProperties\":false}";
static const p2_spec_t sample_specs[]={
 {"demo.document.read","读取业务属性及实际修改","read",p2_empty_schema,sample_document_output,"document",0},
 {"demo.view.read","读取选择和图片版本","read",p2_empty_schema,sample_view_output,"view",1},
 {"demo.dialog","Web 对话框","view",p2_empty_schema,sample_view_output,"view",1},
 {"demo.refresh","刷新图片","view",p2_empty_schema,sample_view_output,"view",1},
 {"demo.submit","提交表单","write",SAMPLE_FIELDS,sample_document_output,"document",0},
 {"demo.edit","编辑单元格","write",SAMPLE_CELL,sample_document_output,"document",0},
 {"demo.select","选择项目","view",SAMPLE_TARGET,sample_view_output,"view",1},
 {"demo.rename","重命名","write",SAMPLE_RENAME,sample_document_output,"document",0},
 {"demo.context","节点菜单","view",SAMPLE_TARGET,sample_view_output,"view",1},
 {"demo.cancel","取消","view",p2_empty_schema,sample_view_output,"view",1},
#ifdef UI_SAMPLE_API4
 {"demo.close_refuse","拒绝关闭测试","view",p2_empty_schema,sample_view_output,"diagnostic",1},
 {"demo.close_allow","允许关闭测试","view",p2_empty_schema,sample_view_output,"diagnostic",1},
#endif
};
static int sample_row_valid(uint64_t id,int tree)
{return tree?((id>=1&&id<=100)||(id>=1001&&id<=101000)):(id>=9007199254741000ULL&&id<9007199254841000ULL);}
static const char *sample_value(sample_t *s,uint64_t id,const char *column,char fallback[256])
{for(size_t i=0;i<s->change_count;++i)if(s->changes[i].id==id&&!strcmp(s->changes[i].column,column))return s->changes[i].text;
 if(!*column)snprintf(fallback,256,"%s %llu",id<=100?"分组":"节点",(unsigned long long)(id<=100?id-1:(id-1)%1000));
 else if(!strcmp(column,"checked"))strcpy_s(fallback,256,"false");else{size_t col;for(col=0;col<16&&strcmp(column_ids[col],column);++col){}snprintf(fallback,256,"值 %llu / %zu",(unsigned long long)(id-9007199254741000ULL),col);}return fallback;}
static ui_status_t sample_revision(void *data,unsigned resource,char out[128])
{sample_t *s=data;if(resource){ui_component_state_t modal={0};modal.size=sizeof(modal);(void)ui_component_get_state(s->dialog,&modal);snprintf(out,128,"view-%llu-%d",(unsigned long long)s->view_revision,modal.modal);}else snprintf(out,128,"doc-%llu",(unsigned long long)s->revision);return UI_STATUS_OK;}
static ui_status_t sample_describe(void *data,unsigned resource,yyjson_mut_doc *doc,yyjson_mut_val *root)
{
 sample_t *s=data;char revision[128];(void)sample_revision(s,resource,revision);yyjson_mut_obj_add_strcpy(doc,root,"revision",revision);
 if(!resource){yyjson_mut_val *changes=yyjson_mut_arr(doc);yyjson_mut_obj_add_strcpy(doc,root,"name",s->name);yyjson_mut_obj_add_real(doc,root,"size",s->size);yyjson_mut_obj_add_bool(doc,root,"enabled",s->enabled!=0);yyjson_mut_obj_add_strcpy(doc,root,"mode",s->mode);yyjson_mut_obj_add_strcpy(doc,root,"notes",s->notes);
   for(size_t i=0;i<s->change_count;++i){char id[24];yyjson_mut_val *change=yyjson_mut_obj(doc);snprintf(id,sizeof(id),"%llu",(unsigned long long)s->changes[i].id);yyjson_mut_obj_add_strcpy(doc,change,"id",id);yyjson_mut_obj_add_strcpy(doc,change,"column",s->changes[i].column);yyjson_mut_obj_add_strcpy(doc,change,"text",s->changes[i].text);yyjson_mut_arr_add_val(changes,change);}yyjson_mut_obj_add_val(doc,root,"changes",changes);
 }else{ui_component_state_t table={0},tree={0},modal={0};char id[24];modal.size=sizeof(modal);(void)ui_component_get_state(s->dialog,&modal);yyjson_mut_obj_add_bool(doc,root,"dialog_open",modal.modal!=0);table.size=sizeof(table);tree.size=sizeof(tree);(void)ui_component_get_state(s->table,&table);(void)ui_component_get_state(s->tree,&tree);snprintf(id,sizeof(id),"%llu",(unsigned long long)table.selected_id);yyjson_mut_obj_add_strcpy(doc,root,"table_selected",id);snprintf(id,sizeof(id),"%llu",(unsigned long long)tree.selected_id);yyjson_mut_obj_add_strcpy(doc,root,"tree_selected",id);yyjson_mut_obj_add_uint(doc,root,"image_version",s->version);yyjson_mut_obj_add_bool(doc,root,"refuse_close",s->refuse_close!=0);}
 return UI_STATUS_OK;
}
static ui_status_t sample_form_read(char revision[128],char *out,size_t capacity,size_t *length,void *data)
{sample_form_t *f=data;sample_t *s=f->app;yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *r=d?yyjson_mut_obj(d):NULL;ui_status_t status;if(!r){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,r);yyjson_mut_obj_add_strcpy(d,r,"name",s->name);yyjson_mut_obj_add_real(d,r,"size",s->size);yyjson_mut_obj_add_bool(d,r,"enabled",s->enabled!=0);yyjson_mut_obj_add_strcpy(d,r,"mode",s->mode);yyjson_mut_obj_add_strcpy(d,r,"notes",s->notes);status=sample_revision(s,0,revision);if(!status)status=p2_json_copy(d,out,capacity,length);yyjson_mut_doc_free(d);return status;}
static ui_status_t sample_project(sample_t *s)
{if(s->capabilities.staging)return UI_STATUS_OK;for(unsigned i=0;i<2;++i){char revision[128],json[65537];size_t length;ui_cap_form_projection_t p={0};ui_status_t status=sample_form_read(revision,json,sizeof(json),&length,&s->forms[i]);if(status)return status;p.size=sizeof(p);p.version=1;p.revision=revision;p.fields_json=json;p.length=length;status=ui_capability_form_project(s->forms[i].component,&p);if(status)return status;}return UI_STATUS_OK;}
static void sample_form_notice(const ui_cap_ui_notice_t *n,void *data)
{sample_form_t *f=data;p2_notice(n,f->cap);if(f->component==f->app->dialog&&n->stage==UI_CAP_UI_FINISHED){ui_cap_form_state_t state={0};state.size=sizeof(state);state.version=1;if(ui_capability_form_get_state(f->component,&state)==UI_STATUS_OK&&!state.dirty_fields&&!state.conflict_fields)(void)ui_component_close_dialog(f->component);}}
static void sample_tick(void *data)
{sample_t *s=data;if(s->closing&&!s->close_posted&&(!s->worker||WaitForSingleObject(s->worker,0)==WAIT_OBJECT_0)){if(ui_workspace_post_close_complete(s->context.workspace,s->context.instance_id,UI_APP_CLOSE_ALLOW)==UI_STATUS_OK)s->close_posted=1;}}
static ui_status_t sample_run(void *data,const char *id,yyjson_val *params,yyjson_mut_doc *out,yyjson_mut_val *root,uint64_t call)
{
 sample_t *s=data;p2_cap_t *cap=p2_find(&s->capabilities,id);ui_status_t status=UI_STATUS_OK;(void)call;++s->operations;
 if(!strcmp(id,"demo.edit")||!strcmp(id,"demo.rename")){uint64_t row;size_t at,col=0;char fallback[256];const char *text=p2_text(params,"text"),*column=!strcmp(id,"demo.rename")?"":p2_text(params,"column");int tree=!strcmp(id,"demo.rename");
   if(strlen(text)>255)return UI_STATUS_LIMIT_EXCEEDED;
   if(!p2_id(p2_text(params,"id"),&row)||!sample_row_valid(row,tree))return UI_STATUS_VALIDATION_FAILED;
   if(!tree){for(col=0;col<16&&strcmp(column_ids[col],column);++col){}if(col==16)return UI_STATUS_VALIDATION_FAILED;if(col==0&&strcmp(text,"true")&&strcmp(text,"false"))return UI_STATUS_VALIDATION_FAILED;}
   if(strcmp(sample_value(s,row,column,fallback),text)){for(at=0;at<s->change_count;++at)if(s->changes[at].id==row&&!strcmp(s->changes[at].column,column))break;if(at==64)return UI_STATUS_LIMIT_EXCEEDED;if(at==s->change_count)++s->change_count;s->changes[at].id=row;strcpy_s(s->changes[at].column,33,column);strcpy_s(s->changes[at].text,256,text);++s->revision;
     if(!s->capabilities.staging&&tree){ui_row_t updated={0};updated.size=sizeof(updated);updated.id=row;updated.title=text;updated.has_children=row<=100;updated.content_version=s->version;status=ui_component_update_rows(s->tree,&updated,1);}
     else if(!s->capabilities.staging){ui_component_state_t state={0};state.size=sizeof(state);status=ui_component_get_state(s->table,&state);if(!status)status=ui_component_query(s->table,0,state.first,20,state.first_column,state.column_count);}if(!status)status=sample_project(s);}
 }else if(!strcmp(id,"demo.submit")){yyjson_val *fields=yyjson_obj_get(params,"fields"),*v;int changed=0;
   v=yyjson_obj_get(fields,"name");if(v&&strlen(yyjson_get_str(v))>4096)return UI_STATUS_LIMIT_EXCEEDED;v=yyjson_obj_get(fields,"notes");if(v&&strlen(yyjson_get_str(v))>4096)return UI_STATUS_LIMIT_EXCEEDED;

   v=yyjson_obj_get(fields,"size");if(v&&(!isfinite(yyjson_get_num(v))||yyjson_get_num(v)<=0))return UI_STATUS_VALIDATION_FAILED;
   v=yyjson_obj_get(fields,"name");if(v&&!*yyjson_get_str(v))return UI_STATUS_VALIDATION_FAILED;
   v=yyjson_obj_get(fields,"name");if(v&&strcmp(s->name,yyjson_get_str(v))){strcpy_s(s->name,sizeof(s->name),yyjson_get_str(v));changed=1;}
   v=yyjson_obj_get(fields,"size");if(v&&s->size!=yyjson_get_num(v)){s->size=yyjson_get_num(v);changed=1;}v=yyjson_obj_get(fields,"enabled");if(v&&s->enabled!=yyjson_get_bool(v)){s->enabled=yyjson_get_bool(v);changed=1;}
   v=yyjson_obj_get(fields,"mode");if(v&&strcmp(s->mode,yyjson_get_str(v))){strcpy_s(s->mode,sizeof(s->mode),yyjson_get_str(v));changed=1;}v=yyjson_obj_get(fields,"notes");if(v&&strcmp(s->notes,yyjson_get_str(v))){strcpy_s(s->notes,sizeof(s->notes),yyjson_get_str(v));changed=1;}if(changed)++s->revision;status=sample_project(s);
 }else if(!strcmp(id,"demo.select")||!strcmp(id,"demo.context")){uint64_t row;int tree=!strcmp(p2_text(params,"component"),"tree");ui_component_t *component=tree?s->tree:s->table;if(!p2_id(p2_text(params,"id"),&row)||!sample_row_valid(row,tree))return UI_STATUS_VALIDATION_FAILED;
   if(!strcmp(id,"demo.select")){ui_component_state_t prior={0};prior.size=sizeof(prior);(void)ui_component_get_state(component,&prior);status=ui_component_set_selection(component,&row,1);if(!status&&prior.selected_id!=row)++s->view_revision;}
   else status=ui_component_show_menu(component,row,
#ifdef UI_SAMPLE_API4
    "Tools/节点"
#else
    "节点"
#endif
   );
 }else if(!strcmp(id,"demo.dialog")){status=sample_project(s);if(!status)status=ui_component_show_dialog(s->dialog);}
 else if(!strcmp(id,"demo.cancel"))status=ui_component_close_dialog(s->dialog);
 else if(!strcmp(id,"demo.refresh")){ui_component_state_t state={0};state.size=sizeof(state);++s->version;++s->view_revision;status=ui_component_get_state(s->table,&state);if(!status)status=ui_component_query(s->table,0,state.first,20,state.first_column,state.column_count);}
 else if(!strcmp(id,"demo.close_refuse")){if(!s->refuse_close){s->refuse_close=1;++s->view_revision;}}
 else if(!strcmp(id,"demo.close_allow")){if(s->refuse_close){s->refuse_close=0;++s->view_revision;}}
 if(status)return status;if(s->capabilities.staging)return sample_describe(s,cap->spec->resource,out,root);{char text[160];snprintf(text,sizeof(text),"实例 %llu · 操作 %u · 业务revision %llu",(unsigned long long)s->context.instance_id,s->operations,(unsigned long long)s->revision);(void)ui_component_set_text(s->status,text);}return sample_describe(s,cap->spec->resource,out,root);
}
typedef struct sample_business {
 uint64_t revision;char name[4097],mode[32],notes[4097];double size;int enabled;
 unsigned char changes[sizeof(((sample_t *)0)->changes)];size_t change_count;
} sample_business_t;
static ui_status_t sample_capture(void *data,void **out,size_t *bytes)
{sample_t *s=data;sample_business_t *b=calloc(1,sizeof(*b));if(!b)return UI_STATUS_OUT_OF_MEMORY;b->revision=s->revision;memcpy(b->name,s->name,sizeof(b->name));memcpy(b->mode,s->mode,sizeof(b->mode));memcpy(b->notes,s->notes,sizeof(b->notes));b->size=s->size;b->enabled=s->enabled;memcpy(b->changes,s->changes,sizeof(b->changes));b->change_count=s->change_count;*out=b;*bytes=sizeof(*b);return UI_STATUS_OK;}
static ui_status_t sample_restore(void *data,const void *copy,size_t bytes,int publish)
{sample_t *s=data;const sample_business_t *b=copy;uint64_t revision=s->revision;if(!b||bytes!=sizeof(*b)||b->change_count>64)return UI_STATUS_INVALID_ARGUMENT;if(publish&&revision==UINT64_MAX)return UI_STATUS_LIMIT_EXCEEDED;
 memcpy(s->name,b->name,sizeof(s->name));memcpy(s->mode,b->mode,sizeof(s->mode));memcpy(s->notes,b->notes,sizeof(s->notes));s->size=b->size;s->enabled=b->enabled;memcpy(s->changes,b->changes,sizeof(s->changes));s->change_count=b->change_count;s->revision=publish?revision+1:b->revision;return UI_STATUS_OK;}
static ui_status_t sample_publish(void *data)
{sample_t *s=data;ui_component_state_t state={0};ui_status_t status;if(!s->capabilities.transaction_committed)return UI_STATUS_OK;status=sample_project(s);
 if(!strcmp(s->capabilities.transaction_capability,"demo.edit")){state.size=sizeof(state);if(!status)status=ui_component_get_state(s->table,&state);if(!status)status=ui_component_query(s->table,0,state.first,20,state.first_column,state.column_count);}
 if(strcmp(s->capabilities.transaction_capability,"demo.rename"))return status;
 for(unsigned pass=0;pass<2&&!status;++pass){sample_t prior;const sample_business_t *undo=s->capabilities.undo_business;size_t count=s->change_count;if(pass){if(!undo)continue;memset(&prior,0,sizeof(prior));memcpy(prior.changes,undo->changes,sizeof(prior.changes));count=undo->change_count;}
  for(size_t i=0;i<count&&!status;++i){uint64_t id=pass?prior.changes[i].id:s->changes[i].id;const char *column=pass?prior.changes[i].column:s->changes[i].column;if(!*column){char fallback[256];ui_row_t row={0};row.size=sizeof(row);row.id=id;row.title=sample_value(s,id,"",fallback);row.has_children=id<=100;row.content_version=s->version;status=ui_component_update_rows(s->tree,&row,1);}}}
 return status;}
static ui_status_t sample_capabilities(sample_t *s,const ui_app_context_t *context)
{const char *resources[]={"demo.document","demo.view"};ui_cap_resource_kind_t kinds[]={UI_CAP_DOCUMENT,UI_CAP_INSTANCE_VIEW};s->revision=s->view_revision=1;s->size=1;s->enabled=1;strcpy_s(s->name,sizeof(s->name),"Untitled");strcpy_s(s->mode,sizeof(s->mode),"自动");s->capabilities.capture_business=sample_capture;s->capabilities.restore_business=sample_restore;s->capabilities.project_business=sample_publish;s->capabilities.run=sample_run;s->capabilities.revision=sample_revision;s->capabilities.describe=sample_describe;s->capabilities.tick=sample_tick;return p2_register(&s->capabilities,context,s,sample_specs,sizeof(sample_specs)/sizeof(sample_specs[0]),resources,kinds,2);}
static ui_status_t sample_menu_params(const ui_cap_ui_context_t *context,char *out,size_t capacity,size_t *length,void *data)
{
 p2_cap_t *cap=data;sample_t *s=cap->app->model;const char *id=cap->spec->id;yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *r=d?yyjson_mut_obj(d):NULL;ui_status_t status=UI_STATUS_OK;(void)context;
 if(!r){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,r);
 if(!strcmp(id,"demo.submit")||!strcmp(id,"demo.edit")||!strcmp(id,"demo.rename")){
   ui_cap_form_state_t state={0};state.size=sizeof(state);state.version=1;status=ui_capability_form_get_state(s->form,&state);if(status)goto done;
   if(state.conflict_fields||(state.dirty_fields&&strcmp(state.base_revision,context->target.expected_revision))){status=UI_STATUS_VALIDATION_FAILED;goto done;}}
 if(!strcmp(id,"demo.submit")){yyjson_mut_val *fields=yyjson_mut_obj(d);const char *ids[]={"name","size","enabled","mode","notes"};for(unsigned i=0;i<5;++i){ui_cell_t cell={0};cell.size=sizeof(cell);status=ui_component_get_field(s->form,ids[i],&cell);if(status)goto done;
   if(i==1){char *end;double value=strtod(cell.text,&end);if(!*cell.text||*end||!isfinite(value)||value<=0){status=UI_STATUS_VALIDATION_FAILED;goto done;}yyjson_mut_obj_add_real(d,fields,ids[i],value);}else if(i==2)yyjson_mut_obj_add_bool(d,fields,ids[i],!strcmp(cell.text,"true"));else yyjson_mut_obj_add_strcpy(d,fields,ids[i],cell.text?cell.text:"");}yyjson_mut_obj_add_val(d,r,"fields",fields);
 }else if(!strcmp(id,"demo.edit")||!strcmp(id,"demo.rename")||!strcmp(id,"demo.context")||!strcmp(id,"demo.select")){ui_component_state_t state={0};char row[24];int tree=!strcmp(id,"demo.rename")||!strcmp(id,"demo.context");state.size=sizeof(state);status=ui_component_get_state(tree?s->tree:s->table,&state);if(status)goto done;if(!state.selected_id){status=UI_STATUS_VALIDATION_FAILED;goto done;}snprintf(row,sizeof(row),"%llu",(unsigned long long)state.selected_id);yyjson_mut_obj_add_strcpy(d,r,"id",row);
   if(!strcmp(id,"demo.edit")||!strcmp(id,"demo.rename")){ui_cell_t cell={0};cell.size=sizeof(cell);status=ui_component_get_field(s->form,"name",&cell);if(status)goto done;if(strlen(cell.text)>255){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}yyjson_mut_obj_add_strcpy(d,r,"text",cell.text);if(!tree)yyjson_mut_obj_add_strcpy(d,r,"column","name");}
   else yyjson_mut_obj_add_strcpy(d,r,"component",tree?"tree":"table");}
 status=p2_json_copy(d,out,capacity,length);done:yyjson_mut_doc_free(d);return status;
}
static ui_status_t sample_input_params(const ui_cap_input_context_t *context,char *out,size_t capacity,size_t *length,void *data)
{
 p2_cap_t *cap=data;sample_t *s=cap->app->model;const char *node=context->control_id,*start,*end;char row[24];yyjson_mut_doc *d;yyjson_mut_val *r;ui_status_t status;
 if(!node||!*node)return sample_menu_params(&context->invocation,out,capacity,length,data);
 if(context->control_id&&!strncmp(context->control_id,"feature.tool.",13))return sample_menu_params(&context->invocation,out,capacity,length,data);
 if(!strcmp(node,"demo.dialog")||!strcmp(node,"demo.refresh"))return sample_menu_params(&context->invocation,out,capacity,length,data);
 start=!strncmp(node,"tree-label-",11)?node+11:!strncmp(node,"rename-",7)?node+7:!strncmp(node,"check-",6)?node+6:!strncmp(node,"edit-",5)||!strncmp(node,"cell-",5)?node+5:!strncmp(node,"row-",4)?node+4:NULL;if(!start)return UI_STATUS_INVALID_ARGUMENT;end=strchr(start,'-');if(!end)end=start+strlen(start);if(end-start>20)return UI_STATUS_INVALID_ARGUMENT;memcpy(row,start,(size_t)(end-start));row[end-start]=0;
 d=yyjson_mut_doc_new(NULL);r=d?yyjson_mut_obj(d):NULL;if(!r){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,r);if(!strcmp(cap->spec->id,"demo.select")&&context->event.kind==UI_INPUT_KEY_DOWN&&!strncmp(node,"cell-",5)){uint64_t id;if(!p2_id(row,&id)){yyjson_mut_doc_free(d);return UI_STATUS_INVALID_ARGUMENT;}if(context->event.key_code==38&&id>9007199254741000ULL)--id;if(context->event.key_code==40&&id<9007199254840999ULL)++id;snprintf(row,sizeof(row),"%llu",(unsigned long long)id);}yyjson_mut_obj_add_strcpy(d,r,"id",row);
 if(!strcmp(cap->spec->id,"demo.edit")){const char *text=context->value_utf8?context->value_utf8:"";char fallback[256];uint64_t id;if(*end!='-'){yyjson_mut_doc_free(d);return UI_STATUS_INVALID_ARGUMENT;}if(!strncmp(node,"check-",6)){if(!p2_id(row,&id)){yyjson_mut_doc_free(d);return UI_STATUS_INVALID_ARGUMENT;}text=!strcmp(sample_value(s,id,end+1,fallback),"true")?"false":"true";}yyjson_mut_obj_add_strcpy(d,r,"column",end+1);yyjson_mut_obj_add_strcpy(d,r,"text",text);}
 else if(!strcmp(cap->spec->id,"demo.rename"))yyjson_mut_obj_add_strcpy(d,r,"text",context->value_utf8?context->value_utf8:"");
 else{uint64_t id;if(!p2_id(row,&id)){yyjson_mut_doc_free(d);return UI_STATUS_INVALID_ARGUMENT;}yyjson_mut_obj_add_strcpy(d,r,"component",id>=9007199254741000ULL?"table":"tree");}
 status=p2_json_copy(d,out,capacity,length);yyjson_mut_doc_free(d);return status;
}
static ui_status_t sample_bindings(sample_t *s)
{
 ui_status_t status;s->forms=calloc(2,sizeof(*s->forms));if(!s->forms)return UI_STATUS_OUT_OF_MEMORY;
 for(unsigned i=0;i<2;++i){sample_form_t *f=&s->forms[i];ui_cap_form_binding_desc_t d={0};f->app=s;f->cap=p2_find(&s->capabilities,"demo.submit");f->component=i?s->dialog:s->form;d.size=sizeof(d);d.version=1;d.component_id=i?"dialog":"form";d.capability_id="demo.submit";d.resource=s->capabilities.resources[0].ref;d.read_model=sample_form_read;d.observe=sample_form_notice;d.user_data=f;status=ui_capability_form_bind(s->context.host,&d,&f->binding);if(status)return status;}
 for(unsigned i=0;i<8;++i){status=p2_bind_menu(&s->capabilities,commands[i],commands[i],sample_menu_params);if(status)return status;}
 for(unsigned i=0;i<2;++i){const char *item=i?"node.refresh":"node.dialog";status=p2_bind_menu(&s->capabilities,item,commands[i],sample_menu_params);if(status)return status;}
#ifdef UI_SAMPLE_API4
 for(unsigned i=0;i<7;++i){char item[40];snprintf(item,sizeof(item),"feature.menu.%u",i);status=p2_bind_menu(&s->capabilities,item,commands[i],sample_menu_params);if(status)return status;}
 for(unsigned i=0;i<2;++i){const char *id=i?"demo.close_allow":"demo.close_refuse";status=p2_bind_menu(&s->capabilities,id,id,NULL);if(status)return status;}
#endif
 {const ui_cap_component_selector_t selectors[]={UI_CAP_CELL_EDIT,UI_CAP_CELL_TOGGLE,UI_CAP_ROW_SELECT,UI_CAP_ROW_CONTEXT,UI_CAP_ROW_RENAME,UI_CAP_ROW_SELECT,UI_CAP_ROW_CONTEXT};const char *ids[]={"demo.edit","demo.edit","demo.select","demo.context","demo.rename","demo.select","demo.context"};
 for(unsigned i=0;i<7;++i){ui_cap_component_input_desc_t d={0};uint64_t binding;p2_cap_t *cap=p2_find(&s->capabilities,ids[i]);d.size=sizeof(d);d.version=1;d.component=i<4?s->table:s->tree;d.selector=selectors[i];d.capability_id=ids[i];d.resource=s->capabilities.resources[cap->spec->resource].ref;d.collect=sample_input_params;d.observe=p2_notice;d.user_data=cap;status=ui_capability_component_input_bind(s->context.host,&d,&binding);if(status)return status;}}
 {unsigned count=2;
#ifdef UI_SAMPLE_API4
 count+=13;
#endif
 for(unsigned i=0;i<count;++i){ui_cap_input_binding_desc_t d={0};uint64_t binding;char item[40];const char *id=i<2?commands[i]:commands[(i-2)%8];p2_cap_t *cap=p2_find(&s->capabilities,id);if(i<2)strcpy_s(item,sizeof(item),id);else snprintf(item,sizeof(item),"feature.tool.%02u",i-2);d.size=sizeof(d);d.version=1;d.kind=UI_CAP_INPUT_TOOLBAR;d.control_id=item;d.trigger=UI_INPUT_POINTER_UP;d.pointer_button=1;d.capability_id=id;d.resource=s->capabilities.resources[cap->spec->resource].ref;d.purpose=UI_CAP_UI_INVOKE;d.collect=sample_input_params;d.observe=p2_notice;d.user_data=cap;status=ui_capability_input_bind(s->context.host,&d,&binding);if(status)return status;}}
 for(unsigned i=0;i<
#ifdef UI_SAMPLE_API4
 2
#else
 1
#endif
 ;++i){ui_cap_input_binding_desc_t d={0};uint64_t binding;const char *id=i?"demo.refresh":"demo.dialog";d.size=sizeof(d);d.version=1;d.kind=UI_CAP_INPUT_SHORTCUT;d.trigger=UI_INPUT_KEY_DOWN;d.key=i?'O':'D';d.modifiers=UI_INPUT_MODIFIER_CONTROL;d.capability_id=id;d.resource=s->capabilities.resources[1].ref;d.purpose=UI_CAP_UI_INVOKE;d.observe=p2_notice;d.user_data=p2_find(&s->capabilities,id);status=ui_capability_input_bind(s->context.host,&d,&binding);if(status)return status;}
 return sample_project(s);
}
