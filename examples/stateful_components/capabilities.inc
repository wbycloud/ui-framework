/* Application memory model. Source rows and both property projections read it. */
struct state_form {state_app_t *app;p2_cap_t *cap;int dialog;uint64_t binding;};
#define ST_FIELDS "{\"type\":\"object\",\"properties\":{\"fields\":{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\",\"maxLength\":4096},\"mode\":{\"type\":\"string\",\"enum\":[\"low\",\"medium\",\"high\"]},\"color\":{\"type\":\"string\",\"maxLength\":15}},\"additionalProperties\":false}},\"required\":[\"fields\"],\"additionalProperties\":false}"
#define ST_DIALOG "{\"type\":\"object\",\"properties\":{\"fields\":{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\",\"maxLength\":4096}},\"required\":[\"name\"],\"additionalProperties\":false}},\"required\":[\"fields\"],\"additionalProperties\":false}"
#define ST_CELL "{\"type\":\"object\",\"properties\":{\"id\":{\"title\":\"对象ID\",\"type\":\"string\",\"maxLength\":20},\"column\":{\"title\":\"列\",\"type\":\"string\",\"maxLength\":11},\"text\":{\"title\":\"新文本\",\"type\":\"string\",\"maxLength\":4096}},\"required\":[\"id\",\"column\",\"text\"],\"additionalProperties\":false}"
#define ST_CELL_READ "{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"string\",\"maxLength\":20},\"column\":{\"type\":\"string\",\"maxLength\":11}},\"required\":[\"id\",\"column\"],\"additionalProperties\":false}"
#define ST_SORT "{\"type\":\"object\",\"properties\":{\"column\":{\"type\":\"string\",\"maxLength\":11},\"direction\":{\"type\":\"integer\",\"enum\":[-1,0,1]}},\"required\":[\"column\",\"direction\"],\"additionalProperties\":false}"
#define ST_SELECT "{\"type\":\"object\",\"properties\":{\"ids\":{\"type\":\"array\",\"items\":{\"type\":\"string\",\"maxLength\":20},\"maxItems\":512},\"anchor\":{\"type\":\"string\",\"maxLength\":20}},\"required\":[\"ids\"],\"additionalProperties\":false}"
#define ST_TITLE "{\"type\":\"object\",\"properties\":{\"title\":{\"type\":\"string\",\"maxLength\":4096}},\"required\":[\"title\"],\"additionalProperties\":false}"
static const char st_document_output[]="{\"type\":\"object\",\"properties\":{\"revision\":{\"type\":\"string\"},\"name\":{\"type\":\"string\"},\"mode\":{\"type\":\"string\"},\"color\":{\"type\":\"string\"},\"title\":{\"type\":\"string\"},\"sort_column\":{\"type\":\"string\"},\"language\":{\"type\":\"string\"},\"id\":{\"type\":\"string\"},\"column\":{\"type\":\"string\"},\"text\":{\"type\":\"string\"},\"row_count\":{\"type\":\"integer\"},\"column_count\":{\"type\":\"integer\"},\"changed_cells\":{\"type\":\"integer\"},\"theme\":{\"type\":\"integer\"},\"sort_direction\":{\"type\":\"integer\"},\"image_id\":{\"type\":\"integer\"},\"last_io_status\":{\"type\":\"integer\"},\"io_pending\":{\"type\":\"boolean\"},\"selected_ids\":{\"type\":\"array\",\"items\":{\"type\":\"string\"},\"maxItems\":512}},\"required\":[\"revision\",\"name\",\"mode\",\"color\",\"row_count\",\"column_count\",\"changed_cells\"],\"additionalProperties\":false}";
static const char st_view_output[]="{\"type\":\"object\",\"properties\":{\"revision\":{\"type\":\"string\"},\"name\":{\"type\":\"string\"},\"mode\":{\"type\":\"string\"},\"color\":{\"type\":\"string\"},\"title\":{\"type\":\"string\"},\"sort_column\":{\"type\":\"string\"},\"language\":{\"type\":\"string\"},\"id\":{\"type\":\"string\"},\"column\":{\"type\":\"string\"},\"text\":{\"type\":\"string\"},\"row_count\":{\"type\":\"integer\"},\"column_count\":{\"type\":\"integer\"},\"changed_cells\":{\"type\":\"integer\"},\"theme\":{\"type\":\"integer\"},\"sort_direction\":{\"type\":\"integer\"},\"image_id\":{\"type\":\"integer\"},\"last_io_status\":{\"type\":\"integer\"},\"io_pending\":{\"type\":\"boolean\"},\"selected_ids\":{\"type\":\"array\",\"items\":{\"type\":\"string\"},\"maxItems\":512},\"dialog_open\":{\"type\":\"boolean\"},\"selection_anchor\":{\"type\":\"string\"}},\"required\":[\"revision\",\"theme\",\"title\",\"sort_column\",\"sort_direction\",\"image_id\",\"selected_ids\",\"dialog_open\",\"selection_anchor\"],\"additionalProperties\":false}";
static const char st_preferences_output[]="{\"type\":\"object\",\"properties\":{\"revision\":{\"type\":\"string\"},\"name\":{\"type\":\"string\"},\"mode\":{\"type\":\"string\"},\"color\":{\"type\":\"string\"},\"title\":{\"type\":\"string\"},\"sort_column\":{\"type\":\"string\"},\"language\":{\"type\":\"string\"},\"id\":{\"type\":\"string\"},\"column\":{\"type\":\"string\"},\"text\":{\"type\":\"string\"},\"row_count\":{\"type\":\"integer\"},\"column_count\":{\"type\":\"integer\"},\"changed_cells\":{\"type\":\"integer\"},\"theme\":{\"type\":\"integer\"},\"sort_direction\":{\"type\":\"integer\"},\"image_id\":{\"type\":\"integer\"},\"last_io_status\":{\"type\":\"integer\"},\"io_pending\":{\"type\":\"boolean\"},\"selected_ids\":{\"type\":\"array\",\"items\":{\"type\":\"string\"},\"maxItems\":512}},\"required\":[\"revision\",\"language\",\"last_io_status\",\"io_pending\"],\"additionalProperties\":false}";
static const p2_spec_t st_specs[]={
 {"state.document.read","Read memory document","read",p2_empty_schema,st_document_output,"document",0},
 {"state.cell.read","Read actual cell","read",ST_CELL_READ,st_document_output,"document",0},
 {"test.edit","Edit cell","write",ST_CELL,st_document_output,"document",0},
 {"test.submit","Submit properties","write",ST_FIELDS,st_document_output,"document",0},
 {"test.modal.close","Submit dialog name","write",ST_DIALOG,st_document_output,"document",0},
 {"state.view.read","Read actual interface state","read",p2_empty_schema,st_view_output,"view",1},
 {"test.select","Select rows","view",ST_SELECT,st_view_output,"view",1},
 {"test.sort","Sort source","view",ST_SORT,st_view_output,"view",1},
 {"test.render","Refresh GL frame","view",p2_empty_schema,st_view_output,"view",1},
 {"test.reset","Reset layout","view",p2_empty_schema,st_view_output,"view",1},
 {"test.modal","Open properties","view",p2_empty_schema,st_view_output,"view",1},
 {"test.image.release","Release image","view",p2_empty_schema,st_view_output,"diagnostic",1},
 {"test.late","Reject late image","view",p2_empty_schema,st_view_output,"diagnostic",1},
 {"state.reset-column","Reset Name width","view",p2_empty_schema,st_view_output,"view",1},
 {"state.reset","Reset widths and layout","view",p2_empty_schema,st_view_output,"view",1},
 {"state.title","Use explicit draft as tab title","view",ST_TITLE,st_view_output,"view",1},
 {"state.theme-system","Follow system theme","view",p2_empty_schema,st_view_output,"view",1},
 {"state.load","Reload UST1 preferences","view",p2_empty_schema,st_view_output,"preferences",1},
 {"state.preferences.read","Read preference state","read",p2_empty_schema,st_preferences_output,"preferences",2},
 {"state.save","Save UST1 preferences","write",p2_empty_schema,st_preferences_output,"preferences",2},
 {"language.zh","Simplified Chinese","write",p2_empty_schema,st_preferences_output,"preferences",2},
 {"language.en","English","write",p2_empty_schema,st_preferences_output,"preferences",2}
};
static int state_column(state_app_t *s,const char *id)
{if(!strcmp(id,"name"))return 0;if(!strcmp(id,"value"))return 1;for(int i=2;i<64;++i)if(!strcmp(id,s->scene->column_ids[i]))return i;return -1;}
static state_cell_t *state_cell(state_app_t *s,uint64_t id,const char *column)
{for(size_t i=0;i<s->cell_count;++i)if(s->cells[i].id==id&&!strcmp(s->cells[i].column,column))return &s->cells[i];return NULL;}
static const char *state_cell_text(state_app_t *s,uint64_t id,const char *column)
{state_cell_t *c=state_cell(s,id,column);return c?c->text:!strcmp(column,"name")?"editable":!strcmp(column,"value")?"42":"wide cell";}
static void state_rows(fixture_t *scene,ui_component_t *component,ui_row_t *rows,size_t count)
{state_app_t *s=scene->application_data;if(component!=scene->table)return;for(size_t i=0;i<count;++i){ui_cell_t *cells=(ui_cell_t *)rows[i].cells;rows[i].content_version=s->revision;
 for(size_t col=0;col<rows[i].cell_count;++col)cells[col].text=state_cell_text(s,rows[i].id,cells[col].column_id);}}
static ui_status_t state_revision(void *data,unsigned resource,char out[128])
{state_app_t *s=data;if(resource==0)snprintf(out,128,"doc-%llu",(unsigned long long)s->revision);
 else if(resource==1){unsigned char *bytes=NULL;size_t length=0;ui_status_t status=snapshot(s,&bytes,&length);if(status)return status;ui_component_state_t modal={0};ui_theme_info_t theme={0};modal.size=sizeof(modal);(void)ui_component_get_state(s->scene->dialog,&modal);theme.size=sizeof(theme);(void)ui_host_get_theme(s->scene->context.host,&theme);snprintf(out,128,"view-%llu-%08x-%d-%d",(unsigned long long)s->view_revision,checksum(bytes,length),(int)theme.mode,modal.modal);free(bytes);}
 else snprintf(out,128,"prefs-%llu",(unsigned long long)s->settings_revision);return UI_STATUS_OK;}
static ui_status_t state_describe(void *data,unsigned resource,yyjson_mut_doc *doc,yyjson_mut_val *root)
{
 state_app_t *s=data;char revision[128];ui_status_t status=state_revision(s,resource,revision);if(status)return status;yyjson_mut_obj_add_strcpy(doc,root,"revision",revision);
 if(resource==0){yyjson_mut_obj_add_strcpy(doc,root,"name",s->name);yyjson_mut_obj_add_strcpy(doc,root,"mode",s->mode);yyjson_mut_obj_add_strcpy(doc,root,"color",s->color);
   yyjson_mut_obj_add_uint(doc,root,"row_count",ROW_TOTAL);yyjson_mut_obj_add_uint(doc,root,"column_count",64);yyjson_mut_obj_add_uint(doc,root,"changed_cells",s->cell_count);}
 else if(resource==1){ui_component_state_t modal={0};ui_theme_info_t theme={0};modal.size=sizeof(modal);(void)ui_component_get_state(s->scene->dialog,&modal);yyjson_mut_obj_add_bool(doc,root,"dialog_open",modal.modal!=0);char title[4097]={0};size_t title_bytes=0;yyjson_mut_val *ids=yyjson_mut_arr(doc);theme.size=sizeof(theme);
   (void)ui_host_get_theme(s->scene->context.host,&theme);(void)ui_workspace_get_instance_title(s->scene->context.workspace,s->scene->context.instance_id,title,sizeof(title),&title_bytes);
   yyjson_mut_obj_add_sint(doc,root,"theme",theme.mode);yyjson_mut_obj_add_strcpy(doc,root,"title",title);
   yyjson_mut_obj_add_strcpy(doc,root,"sort_column",s->sort);yyjson_mut_obj_add_sint(doc,root,"sort_direction",s->direction);yyjson_mut_obj_add_uint(doc,root,"image_id",s->scene->image);
   for(size_t i=0;i<s->selected_count;++i){char text[24];snprintf(text,sizeof(text),"%llu",(unsigned long long)s->selected[i]);yyjson_mut_arr_add_strcpy(doc,ids,text);}yyjson_mut_obj_add_val(doc,root,"selected_ids",ids);{char anchor[24];snprintf(anchor,sizeof(anchor),"%llu",(unsigned long long)s->selection_anchor);yyjson_mut_obj_add_strcpy(doc,root,"selection_anchor",anchor);}}
 else{ui_language_info_t info={0};info.size=sizeof(info);(void)ui_host_get_language(s->scene->context.host,&info);yyjson_mut_obj_add_strcpy(doc,root,"language",info.language);yyjson_mut_obj_add_sint(doc,root,"last_io_status",s->last_io_status);yyjson_mut_obj_add_bool(doc,root,"io_pending",s->io!=NULL);}
 return UI_STATUS_OK;
}
static ui_status_t state_form_read(char revision[128],char *out,size_t capacity,size_t *length,void *data)
{state_form_t *f=data;state_app_t *s=f->app;yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *r=d?yyjson_mut_obj(d):NULL;ui_status_t status;if(!r){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,r);
 yyjson_mut_obj_add_strcpy(d,r,"name",s->name);if(!f->dialog){yyjson_mut_obj_add_strcpy(d,r,"mode",s->mode);yyjson_mut_obj_add_strcpy(d,r,"color",s->color);}status=state_revision(s,0,revision);if(!status)status=p2_json_copy(d,out,capacity,length);yyjson_mut_doc_free(d);return status;}
static ui_status_t state_project(state_app_t *s)
{if(s->capabilities.staging)return UI_STATUS_OK;for(unsigned i=0;i<2;++i){char revision[128],text[32768];size_t length;ui_cap_form_projection_t p={0};ui_status_t status=state_form_read(revision,text,sizeof(text),&length,&s->forms[i]);if(status)return status;p.size=sizeof(p);p.version=1;p.revision=revision;p.fields_json=text;p.length=length;
 status=ui_capability_form_project(i?s->scene->dialog:s->scene->form,&p);if(status)return status;}return UI_STATUS_OK;}
static void state_form_notice(const ui_cap_ui_notice_t *n,void *data)
{state_form_t *f=data;p2_notice(n,f->cap);/* Do not close a dialog whose newer draft was not accepted. */
 if(f->dialog&&n->stage==UI_CAP_UI_FINISHED){ui_cap_form_state_t state={0};state.size=sizeof(state);state.version=1;if(ui_capability_form_get_state(f->app->scene->dialog,&state)==UI_STATUS_OK&&!state.dirty_fields&&!state.conflict_fields)(void)ui_component_close_dialog(f->app->scene->dialog);}}
static DWORD WINAPI state_io_worker(void *data)
{
 state_app_t *s=data;HANDLE file;DWORD bytes=0;LARGE_INTEGER size;const wchar_t *path=s->io_kind==3?s->language_path:s->path;s->io_status=UI_STATUS_PLATFORM_ERROR;s->io_changed=0;
 if(s->io_kind==1||s->io_kind==3){wchar_t temporary[4096];if(_snwprintf_s(temporary,4096,_TRUNCATE,L"%s.tmp",path)<0)return 0;
   file=CreateFileW(temporary,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);if(file==INVALID_HANDLE_VALUE)return 0;
   if(WriteFile(file,s->io_data,(DWORD)s->io_bytes,&bytes,NULL)&&bytes==s->io_bytes&&FlushFileBuffers(file))s->io_status=UI_STATUS_OK;CloseHandle(file);
   if(s->io_status==UI_STATUS_OK){if(MoveFileExW(temporary,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))s->io_changed=1;else s->io_status=UI_STATUS_PLATFORM_ERROR;}
 }else{file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);if(file==INVALID_HANDLE_VALUE)s->io_status=GetLastError()==ERROR_FILE_NOT_FOUND?UI_STATUS_NOT_FOUND:UI_STATUS_PLATFORM_ERROR;
   else{if(!GetFileSizeEx(file,&size)||size.QuadPart<STATE_HEADER||size.QuadPart>STATE_LIMIT)s->io_status=UI_STATUS_INVALID_ARGUMENT;
     else{s->io_bytes=(size_t)size.QuadPart;s->io_data=malloc(s->io_bytes);if(!s->io_data)s->io_status=UI_STATUS_OUT_OF_MEMORY;else s->io_status=ReadFile(file,s->io_data,(DWORD)s->io_bytes,&bytes,NULL)&&bytes==s->io_bytes?UI_STATUS_OK:UI_STATUS_PLATFORM_ERROR;}CloseHandle(file);}
   if(s->io_kind==4){unsigned char language[16]={0};s->language_load=UI_STATUS_NOT_FOUND;strcpy_s(s->io_language,sizeof(s->io_language),"en-US");file=CreateFileW(s->language_path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
     if(file!=INVALID_HANDLE_VALUE){s->language_load=UI_STATUS_INVALID_ARGUMENT;if(GetFileSizeEx(file,&size)&&size.QuadPart==16&&ReadFile(file,language,16,&bytes,NULL)&&bytes==16&&!memcmp(language,"UIL1",4)){
       if(read32(language+4)!=1)s->language_load=UI_STATUS_UNSUPPORTED;else if((language[8]==1||language[8]==2)&&language[9]==s->profile&&!language[10]&&!language[11]&&!read32(language+12)){strcpy_s(s->io_language,sizeof(s->io_language),language[8]==1?"zh-CN":"en-US");s->language_load=UI_STATUS_OK;}}CloseHandle(file);}else if(GetLastError()!=ERROR_FILE_NOT_FOUND)s->language_load=UI_STATUS_PLATFORM_ERROR;}
 }return 0;
}
static ui_status_t state_io_start(state_app_t *s,int kind)
{if(s->io)return UI_STATUS_LIMIT_EXCEEDED;s->io_kind=kind;s->io_changed=0;s->io=CreateThread(NULL,0,state_io_worker,s,0,NULL);if(!s->io){free(s->io_data);s->io_data=NULL;s->io_bytes=0;return UI_STATUS_PLATFORM_ERROR;}return UI_STATUS_PENDING;}
static int state_quiet(void *data,unsigned resource){(void)resource;return !((state_app_t *)data)->io;}
static void state_tick(void *data)
{
 state_app_t *s=data;if(s->io&&WaitForSingleObject(s->io,0)==WAIT_OBJECT_0){int kind=s->io_kind;ui_status_t status=s->io_status;CloseHandle(s->io);s->io=NULL;
   if((kind==2||kind==4)&&status==UI_STATUS_OK&&!s->capabilities.closing){char before[128],after[128];(void)state_revision(s,1,before);status=apply_snapshot(s,s->io_data,s->io_bytes);(void)state_revision(s,1,after);s->io_changed=strcmp(before,after)!=0;}
   if(kind==4&&!s->capabilities.closing){(void)ui_host_set_language(s->scene->context.host,s->io_language);refresh_language(s);++s->settings_revision;}
   free(s->io_data);s->io_data=NULL;s->io_bytes=0;s->last_io_status=status;
   if(kind!=4){unsigned resource=kind==2?1u:2u;yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *r=d?yyjson_mut_obj(d):NULL;if(d)yyjson_mut_doc_set_root(d,r);
     if(resource==2&&s->io_changed)++s->settings_revision;
     /* A failed language save retained its runtime effect; filesystem failure
        is not proof that no change occurred. Leave it UNKNOWN, never retry. */
     s->capabilities.resources[resource].pending.effect_override=status==UI_STATUS_OK?(s->io_changed?UI_CAP2_EFFECT_APPLIED:UI_CAP2_EFFECT_NONE):(kind==1||kind==3)?UI_CAP2_EFFECT_UNKNOWN:s->io_changed?UI_CAP2_EFFECT_APPLIED:UI_CAP2_EFFECT_NONE;
     if(r&&status==UI_STATUS_OK)status=state_describe(s,resource,d,r);p2_finish(&s->capabilities,resource,status,d);yyjson_mut_doc_free(d);}
   report(s,kind==1?"saved":"loaded",status);}
 if(s->capabilities.closing&&!s->close_posted&&!s->io&&(!s->scene->worker||WaitForSingleObject(s->scene->worker,0)==WAIT_OBJECT_0)){
   if(ui_workspace_post_close_complete(s->scene->context.workspace,s->scene->context.instance_id,UI_APP_CLOSE_ALLOW)==UI_STATUS_OK)s->close_posted=1;}
}
static ui_status_t state_run(void *data,const char *id,yyjson_val *params,yyjson_mut_doc *out,yyjson_mut_val *root,uint64_t call)
{
 state_app_t *s=data;ui_status_t status=UI_STATUS_OK;unsigned resource=0;p2_cap_t *cap=p2_find(&s->capabilities,id);(void)call;if(!cap)return UI_STATUS_NOT_FOUND;resource=cap->spec->resource;
 if(s->io&&strcmp(cap->spec->effect,"read"))return UI_STATUS_LIMIT_EXCEEDED;
 if(!strcmp(id,"state.cell.read")||!strcmp(id,"test.edit")){uint64_t row;const char *column=p2_text(params,"column");if(!p2_id(p2_text(params,"id"),&row)||row<ROW_BASE||row>=ROW_BASE+ROW_TOTAL||state_column(s,column)<0)return UI_STATUS_VALIDATION_FAILED;
   if(!strcmp(id,"test.edit")){const char *text=p2_text(params,"text");if(strlen(text)>4096)return UI_STATUS_LIMIT_EXCEEDED;state_cell_t *cell=state_cell(s,row,column);if(!strcmp(column,"value")){char *end;double number=strtod(text,&end);if(!*text||*end||!isfinite(number))return UI_STATUS_VALIDATION_FAILED;}
     if(strcmp(state_cell_text(s,row,column),text)){if(!cell){if(s->cell_count==64)return UI_STATUS_LIMIT_EXCEEDED;cell=&s->cells[s->cell_count++];cell->id=row;strcpy_s(cell->column,sizeof(cell->column),column);}strcpy_s(cell->text,sizeof(cell->text),text);++s->revision;
       status=s->capabilities.staging?UI_STATUS_OK:ui_component_query(s->scene->table,0,0,128,0,1);if(!status)status=state_project(s);}}
   yyjson_mut_obj_add_strcpy(out,root,"id",p2_text(params,"id"));yyjson_mut_obj_add_strcpy(out,root,"column",column);yyjson_mut_obj_add_strcpy(out,root,"text",state_cell_text(s,row,column));
 }else if(!strcmp(id,"test.submit")||!strcmp(id,"test.modal.close")){yyjson_val *fields=yyjson_obj_get(params,"fields"),*value;int changed=0;
   value=yyjson_obj_get(fields,"name");if(value&&strlen(yyjson_get_str(value))>4096)return UI_STATUS_LIMIT_EXCEEDED;
   value=yyjson_obj_get(fields,"color");if(value){const char *color=yyjson_get_str(value);size_t length=strlen(color);if((length!=7&&length!=9)||color[0]!='#')return UI_STATUS_VALIDATION_FAILED;for(size_t n=1;n<length;++n)if(!((color[n]>='0'&&color[n]<='9')||(color[n]>='a'&&color[n]<='f')||(color[n]>='A'&&color[n]<='F')))return UI_STATUS_VALIDATION_FAILED;}

   value=yyjson_obj_get(fields,"name");if(value&&strcmp(s->name,yyjson_get_str(value))){strcpy_s(s->name,sizeof(s->name),yyjson_get_str(value));changed=1;}
   value=yyjson_obj_get(fields,"mode");if(value&&strcmp(s->mode,yyjson_get_str(value))){strcpy_s(s->mode,sizeof(s->mode),yyjson_get_str(value));changed=1;}
   value=yyjson_obj_get(fields,"color");if(value&&strcmp(s->color,yyjson_get_str(value))){strcpy_s(s->color,sizeof(s->color),yyjson_get_str(value));changed=1;}if(changed)++s->revision;status=state_project(s);
 }else if(!strcmp(id,"test.select")){yyjson_val *ids=yyjson_obj_get(params,"ids"),*anchor_value=yyjson_obj_get(params,"anchor");uint64_t selected[512],anchor=0;size_t count=yyjson_arr_size(ids),at;yyjson_val *value;yyjson_arr_foreach(ids,at,count,value){if(!p2_id(yyjson_get_str(value),&selected[at])||selected[at]<ROW_BASE||selected[at]>=ROW_BASE+ROW_TOTAL)return UI_STATUS_VALIDATION_FAILED;}
   if(anchor_value){if(!p2_id(yyjson_get_str(anchor_value),&anchor)||anchor<ROW_BASE||anchor>=ROW_BASE+ROW_TOTAL)return UI_STATUS_VALIDATION_FAILED;}else if(count)anchor=selected[count-1];
   status=ui_component_set_selection(s->scene->table,selected,count);if(!status){if(s->selection_anchor!=anchor||s->selected_count!=count||memcmp(s->selected,selected,count*sizeof(*selected)))++s->view_revision;s->selection_anchor=anchor;memcpy(s->selected,selected,count*sizeof(*selected));s->selected_count=count;}
 }else if(!strcmp(id,"test.sort")){const char *column=p2_text(params,"column");int direction=(int)yyjson_get_sint(yyjson_obj_get(params,"direction"));if(state_column(s,column)<0)return UI_STATUS_VALIDATION_FAILED;status=ui_component_set_sort(s->scene->table,column,direction);if(!status){if(strcmp(s->sort,column)||s->direction!=direction){++s->view_revision;s->selection_anchor=0;}strcpy_s(s->sort,sizeof(s->sort),column);s->direction=direction;}
 }else if(!strcmp(id,"state.save")){status=snapshot(s,&s->io_data,&s->io_bytes);if(!status)return state_io_start(s,1);
 }else if(!strcmp(id,"state.load"))return state_io_start(s,2);
 else if(!strcmp(id,"language.zh")||!strcmp(id,"language.en")){const char *language=!strcmp(id,"language.zh")?"zh-CN":"en-US";status=ui_host_set_language(s->scene->context.host,language);if(status)return status;++s->settings_revision;s->io_data=calloc(1,16);if(!s->io_data)return UI_STATUS_OUT_OF_MEMORY;s->io_bytes=16;memcpy(s->io_data,"UIL1",4);write32(s->io_data+4,1);s->io_data[8]=!strcmp(language,"zh-CN")?1:2;s->io_data[9]=s->profile;return state_io_start(s,3);
 }else if(!strcmp(id,"test.render")){status=render(s->scene);}
 else if(!strcmp(id,"test.modal")){status=state_project(s);if(!status)status=ui_component_show_dialog(s->scene->dialog);}
 else if(!strcmp(id,"test.reset"))status=ui_shell_reset_layout(ui_host_get_shell(s->scene->context.host));
 else if(!strcmp(id,"state.reset-column"))status=ui_component_reset_columns(s->scene->table,"name");
 else if(!strcmp(id,"state.reset")){status=ui_component_reset_columns(s->scene->table,NULL);if(!status)status=ui_shell_reset_layout(ui_host_get_shell(s->scene->context.host));}
 else if(!strcmp(id,"state.title")){status=ui_workspace_set_instance_title(s->scene->context.workspace,s->scene->context.instance_id,p2_text(params,"title"));if(!status)++s->view_revision;}
 else if(!strcmp(id,"state.theme-system"))status=ui_host_set_theme(s->scene->context.host,UI_THEME_SYSTEM);
 else if(!strcmp(id,"test.image.release")){if(s->scene->image){status=ui_image_release(s->scene->context.host,s->scene->image);if(!status){s->scene->image=0;++s->view_revision;}}}
 else if(!strcmp(id,"test.late")){ui_thumbnail_result_t r={0};r.size=sizeof(r);r.component_generation=s->scene->late.component_generation;r.request_id=s->scene->late.request_id;r.item_id=s->scene->late.item_id;r.content_version=s->scene->late.content_version;r.dpi=s->scene->late.dpi;r.failed=1;status=ui_component_set_source(s->scene->tree,source,s->scene);if(!status){status=ui_component_thumbnail(s->scene->tree,&r);status=status==UI_STATUS_CANCELLED?UI_STATUS_OK:UI_STATUS_VALIDATION_FAILED;}}
 if(status)return status;return state_describe(s,resource,out,root);
}
static ui_status_t state_title_params(const ui_cap_ui_context_t *context,char *out,size_t capacity,size_t *length,void *data)
{p2_cap_t *c=data;state_app_t *s=c->app->model;ui_cell_t cell={0};yyjson_mut_doc *d;yyjson_mut_val *r;ui_status_t status;(void)context;cell.size=sizeof(cell);status=ui_component_get_field(s->scene->form,"name",&cell);if(status)return status;d=yyjson_mut_doc_new(NULL);r=d?yyjson_mut_obj(d):NULL;if(!r){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,r);yyjson_mut_obj_add_strcpy(d,r,"title",cell.text?cell.text:"");status=p2_json_copy(d,out,capacity,length);yyjson_mut_doc_free(d);return status;}
static ui_status_t state_collect(const ui_cap_input_context_t *context,char *out,size_t capacity,size_t *length,void *data)
{
 p2_cap_t *c=data;state_app_t *s=c->app->model;const char *node=context->control_id;yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *r=d?yyjson_mut_obj(d):NULL;ui_status_t status=UI_STATUS_OK;
 if(!r){yyjson_mut_doc_free(d);return UI_STATUS_OUT_OF_MEMORY;}yyjson_mut_doc_set_root(d,r);
 if(!strcmp(c->spec->id,"test.edit")){const char *start=node+5,*end=strchr(start,'-');char id[24];if(!end||end-start>20){status=UI_STATUS_INVALID_ARGUMENT;goto done;}memcpy(id,start,(size_t)(end-start));id[end-start]=0;yyjson_mut_obj_add_strcpy(d,r,"id",id);yyjson_mut_obj_add_strcpy(d,r,"column",end+1);yyjson_mut_obj_add_strcpy(d,r,"text",context->value_utf8?context->value_utf8:"");}
 else if(!strcmp(c->spec->id,"test.sort")){yyjson_mut_obj_add_strcpy(d,r,"column",node+5);yyjson_mut_obj_add_sint(d,r,"direction",!strcmp(s->sort,node+5)&&s->direction==1?-1:1);}
 else{const char *start=strchr(node,'-'),*end;char id[24];uint64_t clicked,selected[512],anchor;size_t count=0;yyjson_mut_val *ids=yyjson_mut_arr(d);if(!start){status=UI_STATUS_INVALID_ARGUMENT;goto done;}++start;end=strchr(start,'-');if(!end)end=start+strlen(start);if(end-start>20){status=UI_STATUS_INVALID_ARGUMENT;goto done;}memcpy(id,start,(size_t)(end-start));id[end-start]=0;if(!p2_id(id,&clicked)){status=UI_STATUS_INVALID_ARGUMENT;goto done;}
   if(context->event.kind==UI_INPUT_KEY_DOWN){int delta=context->event.key_code==38?-1:context->event.key_code==40?1:0;if(s->direction==-1)delta=-delta;if(delta<0&&clicked>ROW_BASE)--clicked;if(delta>0&&clicked<ROW_BASE+ROW_TOTAL-1)++clicked;}
   anchor=s->selection_anchor;
   if((context->event.modifiers&UI_INPUT_MODIFIER_SHIFT)&&s->selected_count&&anchor){uint64_t first=anchor<clicked?anchor:clicked,last=anchor>clicked?anchor:clicked;if(last-first>=512){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}
     if(context->event.modifiers&UI_INPUT_MODIFIER_CONTROL){count=s->selected_count;memcpy(selected,s->selected,count*sizeof(*selected));}
     for(uint64_t offset=0;offset<=last-first;++offset){uint64_t value=s->direction==-1?last-offset:first+offset;size_t at;for(at=0;at<count&&selected[at]!=value;++at){}if(at==count){if(count==512){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}selected[count++]=value;}}
   }else if(context->event.modifiers&UI_INPUT_MODIFIER_CONTROL){int found=0;anchor=clicked;for(size_t n=0;n<s->selected_count;++n){if(s->selected[n]==clicked)found=1;else selected[count++]=s->selected[n];}if(!found){if(count==512){status=UI_STATUS_LIMIT_EXCEEDED;goto done;}selected[count++]=clicked;}}
   else{anchor=clicked;selected[count++]=clicked;}
   snprintf(id,sizeof(id),"%llu",(unsigned long long)anchor);yyjson_mut_obj_add_strcpy(d,r,"anchor",id);

   for(size_t n=0;n<count;++n){snprintf(id,sizeof(id),"%llu",(unsigned long long)selected[n]);yyjson_mut_arr_add_strcpy(d,ids,id);}yyjson_mut_obj_add_val(d,r,"ids",ids);}

 status=p2_json_copy(d,out,capacity,length);done:yyjson_mut_doc_free(d);return status;
}
typedef struct state_business {
 uint64_t revision;char name[4097],mode[16],color[16];state_cell_t cells[64];size_t cell_count;
} state_business_t;
static ui_status_t state_capture(void *data,void **out,size_t *bytes)
{state_app_t *s=data;state_business_t *b=calloc(1,sizeof(*b));if(!b)return UI_STATUS_OUT_OF_MEMORY;b->revision=s->revision;memcpy(b->name,s->name,sizeof(b->name));memcpy(b->mode,s->mode,sizeof(b->mode));memcpy(b->color,s->color,sizeof(b->color));memcpy(b->cells,s->cells,sizeof(b->cells));b->cell_count=s->cell_count;*out=b;*bytes=sizeof(*b);return UI_STATUS_OK;}
static ui_status_t state_restore(void *data,const void *copy,size_t bytes,int publish)
{state_app_t *s=data;const state_business_t *b=copy;uint64_t revision=s->revision;if(!b||bytes!=sizeof(*b)||b->cell_count>64)return UI_STATUS_INVALID_ARGUMENT;if(publish&&revision==UINT64_MAX)return UI_STATUS_LIMIT_EXCEEDED;
 memcpy(s->name,b->name,sizeof(s->name));memcpy(s->mode,b->mode,sizeof(s->mode));memcpy(s->color,b->color,sizeof(s->color));memcpy(s->cells,b->cells,sizeof(s->cells));s->cell_count=b->cell_count;s->revision=publish?revision+1:b->revision;return UI_STATUS_OK;}
static ui_status_t state_publish(void *data)
{state_app_t *s=data;ui_status_t status=state_project(s);if(!status)status=ui_component_query(s->scene->table,0,0,128,0,1);return status;}
static ui_status_t state_capabilities(state_app_t *s,const ui_app_context_t *context)
{const char *resources[]={"state.document","state.view","state.preferences"};ui_cap_resource_kind_t kinds[]={UI_CAP_DOCUMENT,UI_CAP_INSTANCE_VIEW,UI_CAP_USER_SETTINGS};s->revision=s->settings_revision=s->view_revision=1;strcpy_s(s->name,sizeof(s->name),"Untitled");strcpy_s(s->mode,sizeof(s->mode),"low");strcpy_s(s->color,sizeof(s->color),"#000000FF");s->capabilities.capture_business=state_capture;s->capabilities.restore_business=state_restore;s->capabilities.project_business=state_publish;s->capabilities.run=state_run;s->capabilities.revision=state_revision;s->capabilities.describe=state_describe;s->capabilities.quiet=state_quiet;s->capabilities.tick=state_tick;ui_status_t status=p2_register(&s->capabilities,context,s,st_specs,sizeof(st_specs)/sizeof(st_specs[0]),resources,kinds,3);
 if(status)return status;
 ui_ai_agent_profile_t profile={0};profile.size=sizeof(profile);profile.version=1;profile.id="maintenance.editor";
 profile.instructions_utf8="维护组件文档中的明确单元格。对象ID、列和新文本必须明确且符合Schema。读取真实业务值及revision。编辑通过原可撤销事务，保持提交、回读、撤销回执与保存状态，参数缺失时先提问。不可访问用户设置或其他应用。";
 profile.allowed_capability_ids_json="[\"state.document.read\",\"state.cell.read\",\"test.edit\"]";
 return ui_ai_agent_profile_register(context->host,&profile);}
static ui_status_t state_bindings(state_app_t *s)
{
 ui_status_t status;s->forms=calloc(2,sizeof(*s->forms));if(!s->forms)return UI_STATUS_OUT_OF_MEMORY;
 for(unsigned i=0;i<2;++i){ui_cap_form_binding_desc_t d={0};state_form_t *f=&s->forms[i];f->app=s;f->dialog=(int)i;f->cap=p2_find(&s->capabilities,i?"test.modal.close":"test.submit");d.size=sizeof(d);d.version=1;d.component_id=i?"dialog":"form";d.capability_id=f->cap->spec->id;d.resource=s->capabilities.resources[0].ref;d.read_model=state_form_read;d.observe=state_form_notice;d.user_data=f;status=ui_capability_form_bind(s->scene->context.host,&d,&f->binding);if(status)return status;}
 for(unsigned i=0;i<3;++i){const char *ids[]={"test.edit","test.select","test.sort"};ui_cap_component_selector_t selectors[]={UI_CAP_CELL_EDIT,UI_CAP_ROW_SELECT,UI_CAP_COLUMN_SORT};ui_cap_component_input_desc_t d={0};uint64_t binding;p2_cap_t *c=p2_find(&s->capabilities,ids[i]);d.size=sizeof(d);d.version=1;d.component=s->scene->table;d.selector=selectors[i];d.capability_id=c->spec->id;d.resource=s->capabilities.resources[c->spec->resource].ref;d.collect=state_collect;d.observe=p2_notice;d.user_data=c;status=ui_capability_component_input_bind(s->scene->context.host,&d,&binding);if(status)return status;}
 {const char *ids[]={"test.render","test.reset","test.image.release","test.late"};for(unsigned i=0;i<4;++i){ui_menu_item_desc_t item={0};item.size=sizeof(item);item.id=item.command_id=ids[i];item.title=p2_find(&s->capabilities,ids[i])->spec->title;item.menu_path="Diagnostics";status=ui_host_register_menu_item(s->scene->context.host,&item);if(status)return status;status=p2_bind_menu(&s->capabilities,ids[i],ids[i],NULL);if(status)return status;}}
 return state_project(s);
}
