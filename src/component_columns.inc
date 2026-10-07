/* API8 user column preferences; no storage or business data owned here. */
static size_t column_index(const ui_component_t *c,const char *id)
{size_t i;for(i=0;i<c->desc.column_count&&strcmp(c->columns[i].id,id);++i){}return i;}
static ui_status_t columns_changed(ui_component_t *c)
{ui_status_t status=UI_STATUS_OK;if(c->view&&c->desc.source)status=query_viewport(c,c->first,c->first_column,0);render_component(c);return status;}
static void cancel_column_resize(ui_component_t *c,int requery)
{if(!c->column_drag_active)return;c->column_drag_active=0;c->columns[c->column_drag_index].width=c->column_drag_origin;if(requery)(void)columns_changed(c);}
ui_status_t ui_component_get_column_width(const ui_component_t *c,const char *id,int *width)
{size_t i;if(!c||!id||!width)return UI_STATUS_INVALID_ARGUMENT;if(c->desc.kind!=UI_COMPONENT_TABLE)return UI_STATUS_UNSUPPORTED;
 i=column_index(c,id);if(i==c->desc.column_count)return UI_STATUS_NOT_FOUND;*width=c->columns[i].width;return UI_STATUS_OK;}
ui_status_t ui_component_set_column_width(ui_component_t *c,const char *id,int width)
{size_t i;if(!c||!id||width<24||width>4096)return UI_STATUS_INVALID_ARGUMENT;if(c->desc.kind!=UI_COMPONENT_TABLE)return UI_STATUS_UNSUPPORTED;
 if(c->host->dispatch_blocked||c->host->modal_component)return UI_STATUS_CANCELLED;i=column_index(c,id);if(i==c->desc.column_count)return UI_STATUS_NOT_FOUND;
 if(c->columns[i].width==width)return UI_STATUS_OK;c->columns[i].width=width;return columns_changed(c);}
ui_status_t ui_component_reset_columns(ui_component_t *c,const char *id)
{size_t first=0,end,i;if(!c)return UI_STATUS_INVALID_ARGUMENT;if(c->desc.kind!=UI_COMPONENT_TABLE)return UI_STATUS_UNSUPPORTED;
 if(c->host->dispatch_blocked||c->host->modal_component)return UI_STATUS_CANCELLED;end=c->desc.column_count;
 if(id){first=column_index(c,id);if(first==end)return UI_STATUS_NOT_FOUND;end=first+1;}
 for(i=first;i<end;++i)c->columns[i].width=c->initial_widths[i];return columns_changed(c);}
#ifdef _WIN32
static int column_text_width(HDC dc,const char *text)
{SIZE size={0};int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,NULL,0);wchar_t *wide;if(n<=0)return 0;
 wide=(wchar_t *)malloc((size_t)n*sizeof(*wide));if(!wide)return 0;MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text,-1,wide,n);GetTextExtentPoint32W(dc,wide,n-1,&size);free(wide);return size.cx;}
#endif
ui_status_t ui_component_fit_column(ui_component_t *c,const char *id)
{
 size_t index,i,j;component_page_t *p;int width=24;
 if(!c||!id)return UI_STATUS_INVALID_ARGUMENT;if(c->desc.kind!=UI_COMPONENT_TABLE)return UI_STATUS_UNSUPPORTED;
 if(c->host->dispatch_blocked||c->host->modal_component)return UI_STATUS_CANCELLED;index=column_index(c,id);if(index==c->desc.column_count)return UI_STATUS_NOT_FOUND;
#ifdef _WIN32
 HDC dc=CreateCompatibleDC(NULL);HFONT body,header;HGDIOBJ old;if(!dc)return UI_STATUS_PLATFORM_ERROR;
 body=CreateFontW(-13,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Segoe UI");header=CreateFontW(-12,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Segoe UI");
 if(!body||!header){if(body)DeleteObject(body);if(header)DeleteObject(header);DeleteDC(dc);return UI_STATUS_PLATFORM_ERROR;}
 old=SelectObject(dc,header);width=column_text_width(dc,c->columns[index].title)+32;SelectObject(dc,body);
 for(p=c->pages;p;p=p->next)if(p->batch)for(i=0;i<p->batch->row_count;++i){const ui_row_t *r=&p->batch->rows[i];for(j=0;j<r->cell_count;++j){const ui_cell_t *v=&r->cells[j];int need;
  if(strcmp(v->column_id,id))continue;need=v->kind==UI_VALUE_BOOLEAN?32:v->kind==UI_VALUE_IMAGE?32:v->kind==UI_VALUE_STYLE?48:column_text_width(dc,v->text)+18;if(need>width)width=need;}}
 SelectObject(dc,old);DeleteObject(body);DeleteObject(header);DeleteDC(dc);
#else
 (void)p;(void)i;(void)j;return UI_STATUS_UNSUPPORTED;
#endif
 return ui_component_set_column_width(c,id,width<24?24:width>4096?4096:width);
}
static void column_write(unsigned char *p,uint32_t v)
{p[0]=(unsigned char)v;p[1]=(unsigned char)(v>>8);p[2]=(unsigned char)(v>>16);p[3]=(unsigned char)(v>>24);}
static uint32_t column_read(const unsigned char *p)
{return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static int column_saved_id(const unsigned char *p,size_t capacity)
{size_t n=0,i;while(n<capacity&&p[n])++n;if(!n||n==capacity)return 0;for(i=n;i<capacity;++i)if(p[i])return 0;
#ifdef _WIN32
 if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,(const char *)p,(int)n,NULL,0))return 0;
#endif
 return 1;}
ui_status_t ui_component_save_columns(const ui_component_t *c,void *data,size_t capacity,size_t *bytes)
{
 size_t i,count=0,at=80;unsigned char *p=(unsigned char *)data;if(!c||!bytes)return UI_STATUS_INVALID_ARGUMENT;if(c->desc.kind!=UI_COMPONENT_TABLE)return UI_STATUS_UNSUPPORTED;
 for(i=0;i<c->desc.column_count;++i)if(c->columns[i].width!=c->initial_widths[i])++count;
 *bytes=80+count*40;if(!data)return UI_STATUS_OK;if(capacity<*bytes)return UI_STATUS_LIMIT_EXCEEDED;memset(p,0,*bytes);memcpy(p,"UCW1",4);column_write(p+4,1);column_write(p+8,(uint32_t)count);strcpy((char *)p+16,c->desc.id);
 for(i=0;i<c->desc.column_count;++i)if(c->columns[i].width!=c->initial_widths[i]){strcpy((char *)p+at,c->columns[i].id);column_write(p+at+36,(uint32_t)c->columns[i].width);at+=40;}return UI_STATUS_OK;
}
ui_status_t ui_component_restore_columns(ui_component_t *c,const void *data,size_t bytes)
{
 const unsigned char *p=(const unsigned char *)data;size_t i,j,index,count;int widths[64];
 if(!c||!p||bytes<80||bytes>2640)return UI_STATUS_INVALID_ARGUMENT;if(c->desc.kind!=UI_COMPONENT_TABLE)return UI_STATUS_UNSUPPORTED;
 if(c->host->dispatch_blocked||c->host->modal_component)return UI_STATUS_CANCELLED;
 if(memcmp(p,"UCW1",4))return UI_STATUS_INVALID_ARGUMENT;if(column_read(p+4)!=1)return UI_STATUS_UNSUPPORTED;count=column_read(p+8);
 if(count>64||bytes!=80+count*40||column_read(p+12)||!column_saved_id(p+16,64)||strcmp((const char *)p+16,c->desc.id))return UI_STATUS_INVALID_ARGUMENT;
 memcpy(widths,c->initial_widths,sizeof(widths));
 for(i=0;i<count;++i){const unsigned char *record=p+80+i*40;uint32_t width=column_read(record+36);
  if(!column_saved_id(record,33)||record[33]||record[34]||record[35]||width<24||width>4096)return UI_STATUS_INVALID_ARGUMENT;
  for(j=0;j<i;++j)if(!strcmp((const char *)record,(const char *)p+80+j*40))return UI_STATUS_INVALID_ARGUMENT;
  index=column_index(c,(const char *)record);if(index<c->desc.column_count)widths[index]=(int)width;
 }
 for(i=0;i<c->desc.column_count;++i)c->columns[i].width=widths[i];return columns_changed(c);
}
