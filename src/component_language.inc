static void update_dialog_caption(ui_component_t *c)
{
#ifdef _WIN32
 if(c->dialog_window){const char *text=*c->desc.title?c->desc.title:ui_language_text(c->host,"Application dialog");int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,NULL,0);wchar_t *wide=count?malloc((size_t)count*sizeof(*wide)):NULL;if(wide){MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,wide,count);SetWindowTextW((HWND)c->dialog_window,wide);free(wide);}}
#else
 (void)c;
#endif
}
static ui_status_t replace_caption(ui_component_t *c,const char **field,const char *text)
{
 char *copy;size_t old_bytes,new_bytes;if(!c||!text||strlen(text)>4095||!ui_language_thread_ok(c->host))return UI_STATUS_INVALID_ARGUMENT;
 if(c->host->dispatch_blocked)return UI_STATUS_CANCELLED;old_bytes=strlen(*field)+1;new_bytes=strlen(text)+1;
 if(c->description_bytes-old_bytes+new_bytes>COMPONENT_MAX_BYTES)return UI_STATUS_LIMIT_EXCEEDED;
 copy=ui_strdup(text);if(!copy)return UI_STATUS_OUT_OF_MEMORY;free((void *)*field);*field=copy;c->description_bytes=c->description_bytes-old_bytes+new_bytes;
 if(field==&c->desc.title)update_dialog_caption(c);(void)ui_host_hide_tooltip(c->host);render_component(c);return ui_host_emit_event(c->host,"ui.components.changed","{}");
}
ui_status_t ui_component_set_title(ui_component_t *c,const char *text)
{return c?replace_caption(c,&c->desc.title,text):UI_STATUS_INVALID_ARGUMENT;}
ui_status_t ui_component_set_column_title(ui_component_t *c,const char *id,const char *title)
{if(!c||!id||!*id)return UI_STATUS_INVALID_ARGUMENT;for(size_t i=0;i<c->desc.column_count;++i)if(!strcmp(c->columns[i].id,id))return replace_caption(c,&c->columns[i].title,title);return UI_STATUS_NOT_FOUND;}
ui_status_t ui_component_set_field_text(ui_component_t *c,const ui_field_text_t *text)
{
 size_t index,added=0,removed=0;char *copies[4]={0},**labels=NULL;ui_field_desc_t *f;
 if(!c||!text||text->size<sizeof(*text)||!text->id||!ui_language_thread_ok(c->host))return UI_STATUS_INVALID_ARGUMENT;
 if(c->host->dispatch_blocked)return UI_STATUS_CANCELLED;
 for(index=0;index<c->desc.field_count&&strcmp(c->fields[index].id,text->id);++index){}if(index==c->desc.field_count)return UI_STATUS_NOT_FOUND;f=&c->fields[index];
 if(text->option_labels?text->option_count!=f->option_count:text->option_count!=0)return UI_STATUS_INVALID_ARGUMENT;
 const char *values[4]={text->title,text->unit,text->group,text->help};const char **fields[4]={&f->title,&f->unit,&f->group,&f->help};
 for(size_t i=0;i<4;++i)if(values[i]){if(strlen(values[i])>4095)return UI_STATUS_INVALID_ARGUMENT;added+=strlen(values[i])+1;removed+=strlen(*fields[i])+1;}
 if(text->option_labels){for(size_t i=0;i<text->option_count;++i){if(!text->option_labels[i]||strlen(text->option_labels[i])>4095)return UI_STATUS_INVALID_ARGUMENT;added+=strlen(text->option_labels[i])+1;if(c->option_labels[index])removed+=strlen(c->option_labels[index][i])+1;}}
 if(c->description_bytes-removed+added>COMPONENT_MAX_BYTES)return UI_STATUS_LIMIT_EXCEEDED;
 for(size_t i=0;i<4;++i)if(values[i]&&!(copies[i]=ui_strdup(values[i])))goto failed;
 if(text->option_labels&&text->option_count){labels=calloc(text->option_count,sizeof(*labels));if(!labels)goto failed;for(size_t i=0;i<text->option_count;++i)if(!(labels[i]=ui_strdup(text->option_labels[i])))goto failed;}
 for(size_t i=0;i<4;++i)if(values[i]){free((void *)*fields[i]);*fields[i]=copies[i];}
 if(text->option_labels){if(c->option_labels[index]){for(size_t i=0;i<f->option_count;++i)free(c->option_labels[index][i]);free(c->option_labels[index]);}c->option_labels[index]=labels;}
 c->description_bytes=c->description_bytes-removed+added;(void)ui_host_hide_tooltip(c->host);render_component(c);return UI_STATUS_OK;
failed:for(size_t i=0;i<4;++i)free(copies[i]);if(labels){for(size_t i=0;i<text->option_count;++i)free(labels[i]);free(labels);}return UI_STATUS_OUT_OF_MEMORY;
}
static void component_language_json(ui_component_t *c,ui_json_t *json)
{ui_language_json(c->host,json);}
static void validation_error(ui_component_t *c,size_t index,const char *key)
{(void)ui_component_set_error(c,c->fields[index].id,ui_language_text(c->host,key));c->validation_errors[index]=key;}
void ui_components_language_changed(ui_host_t *host)
{for(ui_component_t *c=host->components;c;c=c->next){update_dialog_caption(c);for(size_t i=0;i<c->desc.field_count;++i)if(c->validation_errors[i])validation_error(c,i,c->validation_errors[i]);(void)ui_host_hide_tooltip(c->host);render_component(c);}}
