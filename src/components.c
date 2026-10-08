#include "ui_internal.h"
#include "json_ui.h"
#include "language_json.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
#include "ui_framework/light_web.h"
#endif
#if defined(UI_FRAMEWORK_ENABLE_LIGHT_WEB) || defined(UI_FRAMEWORK_HAS_WEBVIEW2)
#include "component_page.h"
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#define COMPONENT_MAX_BATCH 512u
#define COMPONENT_MAX_BYTES (8u * 1024u * 1024u)
typedef struct component_page {
    uint64_t parent, first, total, request, parent_index, ancestor;
    size_t bytes; uint64_t used;
    ui_component_batch_t *batch;
    int expanded, pending;
    struct component_page *next;
} component_page_t;
typedef struct component_thumb {
    uint64_t id, version, request, image, used;
    uint32_t dpi;
    int failed;
    struct component_thumb *next;
} component_thumb_t;
typedef struct component_preview {
    uint64_t image;
    int used;
    struct component_preview *next;
} component_preview_t;
struct ui_component {
    ui_host_t *host;
    ui_component_desc_t desc;
    ui_column_desc_t *columns;
    int initial_widths[64];
    size_t column_drag_index;
    int column_drag_active,column_drag_origin;
    const ui_rect_t *tooltip_anchor; /* Borrowed only during show_tooltip. */
    uint64_t tooltip_row_id;
    ui_field_desc_t *fields;
    char **option_labels[64];
    const char *validation_errors[64];
    size_t description_bytes;
    ui_cell_t *values, *drafts;
    uint64_t generation, next_request, first, selected, use_sequence;
    uint64_t *selection, selection_request, selection_anchor, selection_anchor_id;
    size_t selection_count, selection_requested;
    int selection_extend, sort_direction;
    char sort_column[33];
    size_t first_column, column_count, visible_count, rendered_nodes, row_nodes_created;
    size_t render_column;
    int buffered;
    component_page_t *pages;
    component_thumb_t *thumbs;
    component_preview_t *previews;
    ui_content_slot_t *slot;
    ui_web_backend_t *backend;
    int backend_owned;
    ui_web_view_t *view;
    int visible, dirty, rendering, again, width, height, modal, focused;
    ui_dialog_layout_t dialog_layout;
    int *field_heights;
    ui_status_t presentation_status;
    ui_status_t resource_status;
    uint32_t dpi;
    void *dialog_window, *previous_focus;
    struct ui_component *next;
};
static void render_component(ui_component_t *c);
static uint64_t tree_total(ui_component_t *c);
static void invoke_component(ui_component_t *c,const char *command,const char *params);
static void cancel_column_resize(ui_component_t *c,int requery);
#if defined(UI_FRAMEWORK_ENABLE_LIGHT_WEB) || defined(UI_FRAMEWORK_HAS_WEBVIEW2)
static ui_web_backend_t *component_backend(ui_component_t *c,void *parent)
{
    c->backend_owned=!c->desc.web_backend;if(c->desc.web_backend)return c->desc.web_backend;
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    {ui_light_web_config_t config={0};config.size=sizeof(config);config.parent_hwnd=parent;return ui_light_web_backend_create(&config);}
#else
    (void)parent;return NULL;
#endif
}
#endif
static void release_backend(ui_component_t *c)
{
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    if(c->backend&&c->backend_owned)ui_light_web_backend_destroy(c->backend);
#endif
    c->backend=NULL;
}

static char *copy_string(const char *text, size_t *bytes)
{
    size_t length = text ? strlen(text) : 0;
    if (length > 4095 || *bytes > COMPONENT_MAX_BYTES - length - 1) return NULL;
    *bytes += length + 1; return ui_strdup(text);
}
static void free_cell(ui_cell_t *v)
{ free((void *)v->column_id);free((void *)v->text);free((void *)v->error);memset(v,0,sizeof(*v)); }
static int copy_cell(ui_cell_t *to,const ui_cell_t *from,size_t *bytes)
{
    if (!from || from->size < sizeof(*from) || from->kind < UI_VALUE_TEXT || from->kind > UI_VALUE_GROUP) return 0;
    *to=*from;to->column_id=to->text=to->error=NULL;
    to->column_id=copy_string(from->column_id,bytes);to->text=copy_string(from->text,bytes);to->error=copy_string(from->error,bytes);
    if(!to->column_id||!to->text||!to->error){free_cell(to);return 0;}return 1;
}
void ui_component_batch_free(ui_component_batch_t *batch)
{
    size_t i,j;if(!batch)return;
    for(i=0;i<batch->row_count;++i){ui_row_t *row=(ui_row_t *)&batch->rows[i];
        for(j=0;j<row->cell_count;++j)free_cell((ui_cell_t *)&row->cells[j]);
        free((void *)row->cells);free((void *)row->title);}
    free((void *)batch->rows);free(batch);
}
ui_component_batch_t *ui_component_batch_copy(const ui_component_batch_t *source,size_t *bytes)
{
    ui_component_batch_t *batch;ui_row_t *rows;size_t i,j;
    if(!source||source->size<sizeof(*source)||source->row_count>COMPONENT_MAX_BATCH||
        (source->row_count&&(!source->rows||source->first>source->total_count||source->row_count>source->total_count-source->first)))return NULL;
    *bytes=sizeof(*batch)+source->row_count*sizeof(*rows);
    batch=(ui_component_batch_t *)calloc(1,sizeof(*batch));if(!batch)return NULL;
    *batch=*source;batch->rows=NULL;batch->row_count=0;
    rows=(ui_row_t *)calloc(source->row_count?source->row_count:1,sizeof(*rows));
    if(!rows){free(batch);return NULL;}batch->rows=rows;
    for(i=0;i<source->row_count;++i){const ui_row_t *input=&source->rows[i];ui_cell_t *cells;
        if(input->size<sizeof(*input)||!input->id||input->id==source->parent_id||input->cell_count>64||(input->cell_count&&!input->cells))goto failed;
        for(j=0;j<i;++j)if(rows[j].id==input->id)goto failed;
        rows[i]=*input;rows[i].cells=NULL;rows[i].cell_count=0;rows[i].title=NULL;batch->row_count=i+1;
        rows[i].title=copy_string(input->title,bytes);if(!rows[i].title)goto failed;
        *bytes+=input->cell_count*sizeof(*cells);if(*bytes>COMPONENT_MAX_BYTES)goto failed;
        cells=(ui_cell_t *)calloc(input->cell_count?input->cell_count:1,sizeof(*cells));if(!cells)goto failed;
        rows[i].cells=cells;
        for(j=0;j<input->cell_count;++j){if(!copy_cell(&cells[j],&input->cells[j],bytes))goto failed;rows[i].cell_count=j+1;}
    }
    return batch;
failed:ui_component_batch_free(batch);return NULL;
}
static component_page_t *page(ui_component_t *c,uint64_t parent,int create)
{
    component_page_t *p;for(p=c->pages;p;p=p->next)if(p->parent==parent)return p;
    if(!create)return NULL;p=(component_page_t *)calloc(1,sizeof(*p));if(!p)return NULL;
    p->parent=parent;p->next=c->pages;c->pages=p;return p;
}
static ui_row_t *cached_row(ui_component_t *c,uint64_t id)
{
    component_page_t *p;size_t i;
    for(p=c->pages;p;p=p->next)if(p->batch)for(i=0;i<p->batch->row_count;++i)
        if(p->batch->rows[i].id==id)return (ui_row_t *)&p->batch->rows[i];
    return NULL;
}
static size_t row_bytes(const ui_row_t *r)
{
    size_t i,n=sizeof(*r)+strlen(r->title)+1+r->cell_count*sizeof(ui_cell_t);
    for(i=0;i<r->cell_count;++i)n+=strlen(r->cells[i].column_id)+strlen(r->cells[i].text)+strlen(r->cells[i].error)+3;
    return n;
}
static size_t cache_bytes(const ui_component_t *c)
{component_page_t *p;size_t n=c->selection_count*sizeof(uint64_t)+(c->desc.column_count?sizeof(c->initial_widths):0);for(p=c->pages;p;p=p->next)if(p->batch)n+=p->bytes;return n;}
static void invalidate_item(ui_component_t *c,uint64_t id,int depth)
{
    component_page_t *p;component_thumb_t **t;
    if(depth>32)return;
    ui_menus_component_invalidated(c->host,c,id);
    {size_t i;for(i=0;i<c->selection_count;++i)if(c->selection[i]==id){memmove(c->selection+i,c->selection+i+1,(c->selection_count-i-1)*sizeof(uint64_t));--c->selection_count;break;}c->selection_request=0;}
    for(p=c->pages;p;p=p->next)if(p->ancestor==id&&p->parent&&p->parent!=id)invalidate_item(c,p->parent,depth+1);
    p=page(c,id,0);if(p){ui_component_batch_free(p->batch);p->batch=NULL;p->bytes=0;p->total=0;p->expanded=p->pending=0;p->request=++c->next_request;}
    for(t=&c->thumbs;*t&&(*t)->id!=id;t=&(*t)->next){}
    if(*t){component_thumb_t *old=*t;*t=old->next;if(old->image)(void)ui_image_release(c->host,old->image);ui_images_release_metadata(c->host,sizeof(*old));free(old);}
}
static void free_pages(ui_component_t *c)
{
    while(c->pages){component_page_t *p=c->pages;c->pages=p->next;ui_component_batch_free(p->batch);free(p);}
}
static void free_thumbs(ui_component_t *c)
{
    while(c->thumbs){component_thumb_t *t=c->thumbs;c->thumbs=t->next;
        if(t->image)(void)ui_image_release(c->host,t->image);ui_images_release_metadata(c->host,sizeof(*t));free(t);}
}
static void release_previews(ui_component_t *c,int all)
{
    component_preview_t **link=&c->previews;
    while(*link){component_preview_t *p=*link;
        if(all||!p->used){*link=p->next;(void)ui_image_release(c->host,p->image);ui_images_release_metadata(c->host,sizeof(*p));free(p);}
        else link=&p->next;}
}
/* Only framework-generated previews are owned here; application IDs are borrowed. */
static ui_status_t cell_preview(ui_component_t *c,ui_cell_t *cell)
{
    component_preview_t *p;ui_image_info_t info={0};ui_status_t status;info.size=sizeof(info);
    if(cell->image_id&&ui_image_get_info(c->host,cell->image_id,&info)!=UI_STATUS_OK)cell->image_id=0;
    if(!cell->image_id){
        p=NULL;if(!cell->style.image_id){
            p=(component_preview_t *)calloc(1,sizeof(*p));if(!p)return UI_STATUS_OUT_OF_MEMORY;
            status=ui_images_reserve_metadata(c->host,sizeof(*p));if(status!=UI_STATUS_OK){free(p);return status;}}
        status=ui_image_create_preview(c->host,&cell->style,40,24,&cell->image_id);
        if(status!=UI_STATUS_OK){if(p){ui_images_release_metadata(c->host,sizeof(*p));free(p);}return status;}
        if(p){p->image=cell->image_id;p->next=c->previews;c->previews=p;(void)ui_image_set_evictable(c->host,p->image);}}
    for(p=c->previews;p;p=p->next)if(p->image==cell->image_id){p->used=1;break;}
    return UI_STATUS_OK;
}
const char *ui_host_status_text(const ui_host_t *host)
{const ui_component_t *c;for(c=host->components;c;c=c->next)if(c->desc.kind==UI_COMPONENT_STATUS&&c->visible)return c->desc.title;return "";}
#include "component_language.inc"
ui_status_t ui_component_set_text(ui_component_t *c,const char *text)
{return ui_component_set_title(c,text?text:"");}
static void free_description(ui_component_t *c)
{
    free((void *)c->desc.sort_command);free(c->selection);
    size_t i,j;free((void *)c->desc.id);free((void *)c->desc.title);
    for(i=0;i<c->desc.column_count;++i){free((void *)c->columns[i].id);free((void *)c->columns[i].title);}
    for(i=0;i<c->desc.field_count;++i){ui_field_desc_t *f=&c->fields[i];
        free((void *)f->id);free((void *)f->title);free((void *)f->unit);free((void *)f->group);free((void *)f->help);
        for(j=0;j<f->option_count;++j)free((void *)f->options[j]);free((void *)f->options);
        if(c->option_labels[i]){for(j=0;j<f->option_count;++j)free(c->option_labels[i][j]);free(c->option_labels[i]);}
        free_cell(&c->values[i]);free_cell(&c->drafts[i]);}
    free(c->columns);free(c->fields);free(c->values);free(c->drafts);free(c->field_heights);
    free((void *)c->desc.commands.select);free((void *)c->desc.commands.edit);
    free((void *)c->desc.commands.rename);free((void *)c->desc.commands.context_menu);
    free((void *)c->desc.commands.submit);free((void *)c->desc.commands.cancel);
}
ui_component_t *ui_component_find(ui_host_t *host,const char *id)
{ui_component_t *c;if(!host||!id)return NULL;for(c=host->components;c;c=c->next)if(!strcmp(c->desc.id,id))return c;return NULL;}
ui_status_t ui_component_register(ui_host_t *host,const ui_component_desc_t *d,ui_component_t **out)
{
    ui_component_t *c;size_t i,j,bytes=0;ui_status_t status=UI_STATUS_OUT_OF_MEMORY;
    if(!host||!d||d->size<offsetof(ui_component_desc_t,web_backend)||!out||!d->id||!*d->id||strlen(d->id)>63||d->kind<UI_COMPONENT_TREE||d->kind>UI_COMPONENT_DIALOG||
        d->column_count>64||d->field_count>64||(d->column_count&&!d->columns)||(d->field_count&&!d->fields)||
        (d->row_height&&d->row_height<24)||
        (d->size>=offsetof(ui_component_desc_t,selection_flags)+sizeof(d->selection_flags)&&(d->selection_flags&~UI_SELECTION_MULTIPLE)))return UI_STATUS_INVALID_ARGUMENT;
    *out=NULL;if(ui_component_find(host,d->id))return UI_STATUS_ALREADY_EXISTS;
    c=(ui_component_t *)calloc(1,sizeof(*c));if(!c)return UI_STATUS_OUT_OF_MEMORY;
    c->host=host;memcpy(&c->desc,d,offsetof(ui_component_desc_t,web_backend));
    if(d->size>=offsetof(ui_component_desc_t,web_backend)+sizeof(d->web_backend))c->desc.web_backend=d->web_backend;
    c->desc.column_count=c->desc.field_count=0;
    memset(&c->desc.commands,0,sizeof(c->desc.commands));
    c->desc.id=copy_string(d->id,&bytes);c->desc.title=copy_string(d->title,&bytes);
    if(!c->desc.id||!c->desc.title)goto failed;
#define COPY_BIND(name) c->desc.commands.name=copy_string(d->commands.name,&bytes);if(!c->desc.commands.name)goto failed
    COPY_BIND(select);COPY_BIND(edit);COPY_BIND(rename);COPY_BIND(context_menu);COPY_BIND(submit);COPY_BIND(cancel);
#undef COPY_BIND
    if(d->size>=offsetof(ui_component_desc_t,sort_command)+sizeof(d->sort_command)){c->desc.sort_command=copy_string(d->sort_command,&bytes);if(!c->desc.sort_command)goto failed;}
    if(d->size>=offsetof(ui_component_desc_t,selection_flags)+sizeof(d->selection_flags))c->desc.selection_flags=d->selection_flags;
    c->columns=(ui_column_desc_t *)calloc(d->column_count?d->column_count:1,sizeof(*c->columns));
    c->fields=(ui_field_desc_t *)calloc(d->field_count?d->field_count:1,sizeof(*c->fields));
    c->values=(ui_cell_t *)calloc(d->field_count?d->field_count:1,sizeof(*c->values));
    c->drafts=(ui_cell_t *)calloc(d->field_count?d->field_count:1,sizeof(*c->drafts));
    c->field_heights=(int *)calloc(d->field_count?d->field_count:1,sizeof(int));
    if(!c->columns||!c->fields||!c->values||!c->drafts||!c->field_heights)goto failed;
    for(i=0;i<d->column_count;++i){const ui_column_desc_t *f=&d->columns[i];
        if(f->size<sizeof(*f)||!f->id||!*f->id||strlen(f->id)>32||f->width<0||f->kind<UI_VALUE_TEXT||f->kind>UI_VALUE_GROUP){status=UI_STATUS_INVALID_ARGUMENT;goto failed;}
        c->columns[i]=*f;c->columns[i].id=c->columns[i].title=NULL;c->desc.column_count=i+1;
        c->columns[i].id=copy_string(f->id,&bytes);c->columns[i].title=copy_string(f->title,&bytes);
        if(!c->columns[i].id||!c->columns[i].title)goto failed;
        for(j=0;j<i;++j)if(!strcmp(c->columns[j].id,f->id)){status=UI_STATUS_ALREADY_EXISTS;goto failed;}
        if(!c->columns[i].width)c->columns[i].width=120;
        c->initial_widths[i]=c->columns[i].width;
    }
    for(i=0;i<d->field_count;++i){const ui_field_desc_t *f=&d->fields[i];ui_cell_t v;
        if(f->size<offsetof(ui_field_desc_t,help)||!f->id||!*f->id||strlen(f->id)>32||f->option_count>128||(f->option_count&&!f->options)||f->kind<UI_VALUE_TEXT||f->kind>UI_VALUE_GROUP){
            status=UI_STATUS_INVALID_ARGUMENT;goto failed;}
        memcpy(&c->fields[i],f,offsetof(ui_field_desc_t,help));c->fields[i].id=c->fields[i].title=c->fields[i].unit=c->fields[i].group=NULL;
        c->fields[i].options=NULL;c->fields[i].option_count=0;c->desc.field_count=i+1;
        c->fields[i].id=copy_string(f->id,&bytes);c->fields[i].title=copy_string(f->title,&bytes);
        c->fields[i].unit=copy_string(f->unit,&bytes);c->fields[i].group=copy_string(f->group,&bytes);
        c->fields[i].help=copy_string(f->size>=offsetof(ui_field_desc_t,help)+sizeof(f->help)?f->help:NULL,&bytes);
        if(!c->fields[i].id||!c->fields[i].title||!c->fields[i].unit||!c->fields[i].group||!c->fields[i].help)goto failed;
        for(j=0;j<i;++j)if(!strcmp(c->fields[j].id,f->id)){status=UI_STATUS_ALREADY_EXISTS;goto failed;}
        if(f->option_count){char **options=(char **)calloc(f->option_count,sizeof(*options));if(!options)goto failed;
            c->fields[i].options=(const char *const *)options;
            for(j=0;j<f->option_count;++j){options[j]=copy_string(f->options[j],&bytes);if(!options[j])goto failed;c->fields[i].option_count=j+1;}}
        memset(&v,0,sizeof(v));v.size=sizeof(v);v.column_id=f->id;v.kind=f->kind;v.flags=f->flags;v.style.size=sizeof(v.style);
        if(!copy_cell(&c->values[i],&v,&bytes)||!copy_cell(&c->drafts[i],&v,&bytes))goto failed;
    }
    c->desc.columns=c->columns;c->desc.fields=c->fields;c->generation=++host->component_generation;
    c->visible=1;c->visible_count=20;c->column_count=d->column_count;c->desc.row_height=d->row_height?d->row_height:32;
    c->description_bytes=bytes;c->next=host->components;host->components=c;*out=c;return UI_STATUS_OK;
failed:free_description(c);free(c);return status;
}
ui_status_t ui_component_get_state(const ui_component_t *c,ui_component_state_t *s)
{
    component_page_t *p;
    if(!c||!s||s->size<offsetof(ui_component_state_t,selected_count))return UI_STATUS_INVALID_ARGUMENT;
    p=page((ui_component_t *)c,0,0);s->generation=c->generation;s->first=c->first;s->selected_id=c->selected;
    s->total_count=c->desc.kind==UI_COMPONENT_TREE?tree_total((ui_component_t *)c):(p?p->total:0);s->row_count=p&&p->batch?p->batch->row_count:0;
    s->first_column=c->first_column;s->column_count=c->column_count;s->rendered_nodes=c->rendered_nodes;
    s->dirty=c->dirty;s->visible=c->visible;s->modal=c->modal;s->focused=c->host->focused_component==c;
    s->cached_rows=s->cached_bytes=0;s->presentation_status=c->presentation_status!=UI_STATUS_OK?c->presentation_status:c->resource_status;s->row_nodes_created=c->row_nodes_created;
    for(p=c->pages;p;p=p->next)if(p->batch){s->cached_rows+=p->batch->row_count;s->cached_bytes+=p->bytes;}
    s->cached_bytes+=c->selection_count*sizeof(uint64_t);
    if(s->size>=offsetof(ui_component_state_t,selected_count)+sizeof(s->selected_count))s->selected_count=c->selection_count;
    if(s->size>=offsetof(ui_component_state_t,sort_column)+sizeof(s->sort_column))s->sort_column=c->sort_column;
    if(s->size>=offsetof(ui_component_state_t,sort_direction)+sizeof(s->sort_direction))s->sort_direction=c->sort_direction;
    return UI_STATUS_OK;
}
static ui_status_t request_rows(ui_component_t *c,uint64_t parent,uint64_t first,size_t count,size_t first_column,size_t column_count)
{
    component_page_t *p;ui_component_query_t q;
    if(!c||!c->desc.source||!count||count>COMPONENT_MAX_BATCH||first_column>c->desc.column_count||
        column_count>c->desc.column_count-first_column)return UI_STATUS_INVALID_ARGUMENT;
    if(c->host->dispatch_blocked)return UI_STATUS_CANCELLED;
    p=page(c,parent,1);if(!p)return UI_STATUS_OUT_OF_MEMORY;
    p->first=first;p->request=++c->next_request;p->pending=1;p->used=++c->use_sequence;
    memset(&q,0,sizeof(q));q.size=sizeof(q);q.kind=UI_QUERY_ROWS;q.parent_id=parent;q.first=first;q.count=count;
    q.first_column=first_column;q.column_count=column_count;q.component_generation=c->generation;q.request_id=p->request;q.dpi=c->view?c->view->dpi:c->host->dpi;
    q.sort_column=c->sort_column;q.sort_direction=c->sort_direction;
    ui_dispatch_enter(c->host);c->desc.source(c,&q,c->desc.user_data);ui_dispatch_leave(c->host);return UI_STATUS_OK;
}
ui_status_t ui_component_query(ui_component_t *c,uint64_t parent,uint64_t first,size_t count,size_t first_column,size_t column_count)
{
    if(!c)return UI_STATUS_INVALID_ARGUMENT;
    if(!parent&&c->desc.kind!=UI_COMPONENT_TREE)c->first=first;
    c->buffered=0;c->first_column=c->render_column=first_column;c->column_count=column_count;
    return request_rows(c,parent,first,count,first_column,column_count);
}
static size_t viewport_row_budget(size_t columns)
{return (900-columns*5)/(columns*2+6);}
static ui_status_t query_viewport(ui_component_t *c,uint64_t first,size_t column,int viewport_height)
{
    size_t columns=0;int64_t used=0;int height=viewport_height>0?viewport_height:c->height>64?c->height-64:0;ui_rect_t rows;
    c->buffered=1;c->first=first;c->first_column=column;
    c->render_column=column?column-1:0;
    while(column+columns<c->desc.column_count&&used<c->width){used+=c->columns[column+columns].width;++columns;}
    if(column+columns<c->desc.column_count)++columns;
    if(column)++columns;c->column_count=columns;
    if(viewport_height<=0&&c->view&&ui_web_view_get_element_rect(c->view,"rows",&rows)==UI_STATUS_OK&&rows.height>0)height=rows.height;
    c->visible_count=(size_t)((height+c->desc.row_height-1)/c->desc.row_height)+2+(size_t)(first<2?first:2);
    if(c->visible_count<4)c->visible_count=4;
    {size_t budget=viewport_row_budget(columns);if(c->visible_count>budget)c->visible_count=budget;}
    if(c->visible_count>128)c->visible_count=128;
    return request_rows(c,0,c->desc.kind==UI_COMPONENT_TREE?0:(first>2?first-2:0),c->visible_count,c->render_column,columns);
}
#include "component_columns.inc"
ui_status_t ui_component_submit(ui_component_t *c,const ui_component_batch_t *batch)
{
    component_page_t *p;ui_component_batch_t *copy;size_t bytes,i;
    if(!c||!batch||batch->size<sizeof(*batch))return UI_STATUS_INVALID_ARGUMENT;
    p=page(c,batch->parent_id,0);
    if(c->host->dispatch_blocked||batch->component_generation!=c->generation||!p||batch->request_id!=p->request||batch->first!=p->first)return UI_STATUS_CANCELLED;
    copy=ui_component_batch_copy(batch,&bytes);if(!copy)return UI_STATUS_INVALID_ARGUMENT;
    for(i=0;i<copy->row_count;++i)if(copy->rows[i].parent_id!=batch->parent_id){ui_component_batch_free(copy);return UI_STATUS_INVALID_ARGUMENT;}
    if(bytes>2u*1024u*1024u-c->selection_count*sizeof(uint64_t)){ui_component_batch_free(copy);return UI_STATUS_LIMIT_EXCEEDED;}
    ui_component_batch_free(p->batch);p->batch=copy;p->pending=0;p->total=batch->total_count;p->bytes=bytes;p->used=++c->use_sequence;
    {component_page_t *at;size_t total=c->selection_count*sizeof(uint64_t);for(at=c->pages;at;at=at->next)if(at->batch)total+=at->bytes;
     while(total>2u*1024u*1024u){component_page_t *old=NULL;for(at=c->pages;at;at=at->next)if(at!=p&&at->batch&&(!old||at->used<old->used))old=at;
        if(!old)break;total-=old->bytes;ui_component_batch_free(old->batch);old->batch=NULL;old->bytes=0;}}
    render_component(c);return UI_STATUS_OK;
}
ui_status_t ui_component_update_rows(ui_component_t *c,const ui_row_t *rows,size_t count)
{
    size_t i,bytes,total;ui_component_batch_t input,*copy;
    if(!c||(count&&!rows))return UI_STATUS_INVALID_ARGUMENT;
    if(c->rendering)return UI_STATUS_CANCELLED;
    memset(&input,0,sizeof(input));input.size=sizeof(input);input.rows=rows;input.row_count=count;input.total_count=count;
    copy=ui_component_batch_copy(&input,&bytes);if(!copy)return UI_STATUS_INVALID_ARGUMENT;
    total=cache_bytes(c);
    for(i=0;i<count;++i){ui_row_t *target=cached_row(c,copy->rows[i].id);
        if(target&&copy->rows[i].content_version>=target->content_version)total=total-row_bytes(target)+row_bytes(&copy->rows[i]);}
    if(total>2u*1024u*1024u){ui_component_batch_free(copy);return UI_STATUS_LIMIT_EXCEEDED;}
    for(i=0;i<count;++i){ui_row_t *target=cached_row(c,copy->rows[i].id);
        if(target&&copy->rows[i].content_version>=target->content_version){size_t j;ui_row_t *replacement=(ui_row_t *)&copy->rows[i];
            for(j=0;j<target->cell_count;++j)free_cell((ui_cell_t *)&target->cells[j]);free((void *)target->cells);free((void *)target->title);
            *target=*replacement;memset(replacement,0,sizeof(*replacement));}}
    {component_page_t *p;for(p=c->pages;p;p=p->next)if(p->batch){size_t j;p->bytes=sizeof(*p->batch);for(j=0;j<p->batch->row_count;++j)p->bytes+=row_bytes(&p->batch->rows[j]);}}
    ui_component_batch_free(copy);render_component(c);return UI_STATUS_OK;
}
ui_status_t ui_component_remove_rows(ui_component_t *c,const uint64_t *ids,size_t count)
{
    component_page_t *p;size_t i,j,k;
    if(!c||(count&&!ids))return UI_STATUS_INVALID_ARGUMENT;
    if(c->rendering)return UI_STATUS_CANCELLED;
    c->selection_request=0;c->selection_anchor_id=0;
    for(i=0;i<count;++i){size_t at;if(c->selection_anchor_id==ids[i])c->selection_anchor_id=0;for(at=0;at<c->selection_count;++at)if(c->selection[at]==ids[i]){memmove(c->selection+at,c->selection+at+1,(c->selection_count-at-1)*sizeof(uint64_t));--c->selection_count;break;}if(c->selected==ids[i])c->selected=c->selection_count?c->selection[c->selection_count-1]:0;}
    for(p=c->pages;p;p=p->next)if(p->batch){ui_row_t *rows=(ui_row_t *)p->batch->rows;
        for(i=0;i<count;++i)for(j=0;j<p->batch->row_count;++j)if(rows[j].id==ids[i]){
            component_page_t *child;uint64_t index=p->batch->first+j;invalidate_item(c,ids[i],0);
            for(child=c->pages;child;child=child->next)if(child->ancestor==p->parent&&child->parent_index>index)--child->parent_index;
            for(k=0;k<rows[j].cell_count;++k)free_cell((ui_cell_t *)&rows[j].cells[k]);free((void *)rows[j].cells);free((void *)rows[j].title);
            memmove(&rows[j],&rows[j+1],(p->batch->row_count-j-1)*sizeof(*rows));--p->batch->row_count;
            if(p->total)--p->total;p->batch->total_count=p->total;if(c->selected==ids[i])c->selected=0;break;}}
    render_component(c);return UI_STATUS_OK;
}
ui_status_t ui_component_insert_rows(ui_component_t *c,uint64_t parent,uint64_t index,const ui_row_t *rows,size_t count)
{
    component_page_t *p;ui_component_batch_t input={0},*copy;size_t bytes,at,keep,i;ui_row_t *merged;
    if(!c||!count||!rows||count>COMPONENT_MAX_BATCH)return UI_STATUS_INVALID_ARGUMENT;
    if(c->rendering)return UI_STATUS_CANCELLED;
    p=page(c,parent,0);if(!p||index>p->total||p->total>UINT64_MAX-count)return UI_STATUS_INVALID_ARGUMENT;
    c->selection_request=0;c->selection_anchor_id=0;
    input.size=sizeof(input);input.total_count=count;input.rows=rows;input.row_count=count;
    copy=ui_component_batch_copy(&input,&bytes);if(!copy)return UI_STATUS_INVALID_ARGUMENT;
    if(cache_bytes(c)+bytes>2u*1024u*1024u){ui_component_batch_free(copy);return UI_STATUS_LIMIT_EXCEEDED;}
    for(i=0;i<count;++i)if(cached_row(c,rows[i].id)){ui_component_batch_free(copy);return UI_STATUS_ALREADY_EXISTS;}
    p->total+=count;
    if(p->batch&&index>=p->batch->first&&index<=p->batch->first+p->batch->row_count){
        at=(size_t)(index-p->batch->first);keep=p->batch->row_count+count;if(keep>COMPONENT_MAX_BATCH){ui_component_batch_free(copy);return ui_component_query(c,parent,p->first,c->visible_count,c->first_column,c->column_count);}
        merged=(ui_row_t *)realloc((void *)p->batch->rows,keep*sizeof(*merged));if(!merged){p->total-=count;ui_component_batch_free(copy);return UI_STATUS_OUT_OF_MEMORY;}
        memmove(merged+at+count,merged+at,(p->batch->row_count-at)*sizeof(*merged));memcpy(merged+at,copy->rows,count*sizeof(*merged));
        p->batch->rows=merged;p->batch->row_count=keep;p->batch->total_count=p->total;free((void *)copy->rows);free(copy);
        p->bytes+=bytes-sizeof(input);
    }else{ui_component_batch_free(copy);if(index<p->first){p->first+=count;if(p->batch)p->batch->first+=count;}}
    {component_page_t *child;for(child=c->pages;child;child=child->next)
        if(child->parent&&child->ancestor==parent&&child->parent_index>=index)child->parent_index+=count;}
    render_component(c);return UI_STATUS_OK;
}
ui_status_t ui_component_select(ui_component_t *c,uint64_t id)
{return ui_component_set_selection(c,id?&id:NULL,id?1:0);}
ui_status_t ui_component_expand(ui_component_t *c,uint64_t id,int expanded)
{
    component_page_t *p;ui_row_t *r;ui_status_t status=UI_STATUS_OK;
    if(!c||c->desc.kind!=UI_COMPONENT_TREE)return UI_STATUS_INVALID_ARGUMENT;
    r=cached_row(c,id);p=page(c,id,0);if((!r||!r->has_children)&&!(p&&p->expanded&&!expanded))return UI_STATUS_NOT_FOUND;
    p=page(c,id,1);if(!p)return UI_STATUS_OUT_OF_MEMORY;p->expanded=expanded!=0;
    {component_page_t *at;size_t i;for(at=c->pages;at;at=at->next)if(at->batch)for(i=0;i<at->batch->row_count;++i)if(at->batch->rows[i].id==id){p->ancestor=at->parent;p->parent_index=at->batch->first+i;}}
    if(expanded)status=ui_component_query(c,id,0,c->visible_count,0,c->desc.column_count);
    render_component(c);return status;
}
ui_status_t ui_component_set_source(ui_component_t *c,ui_component_source_fn source,void *user)
{
    if(!c)return UI_STATUS_INVALID_ARGUMENT;if(c->rendering)return UI_STATUS_CANCELLED;cancel_column_resize(c,0);ui_menus_component_invalidated(c->host,c,0);free_pages(c);free_thumbs(c);
    c->generation=++c->host->component_generation;c->next_request=0;c->first=c->selected=0;c->resource_status=UI_STATUS_OK;c->desc.source=source;c->desc.user_data=user;
    free(c->selection);c->selection=NULL;c->selection_count=0;c->selection_request=0;c->selection_anchor=c->selection_anchor_id=0;c->sort_column[0]=0;c->sort_direction=0;
    render_component(c);return UI_STATUS_OK;
}
ui_status_t ui_component_set_field(ui_component_t *c,const char *id,const ui_cell_t *value)
{
    size_t i,bytes=0;ui_cell_t copy,draft;
    if(!c||!id||!value)return UI_STATUS_INVALID_ARGUMENT;
    for(i=0;i<c->desc.field_count;++i)if(!strcmp(c->fields[i].id,id)){
        if(value->kind!=c->fields[i].kind)return UI_STATUS_INVALID_ARGUMENT;
        memset(&copy,0,sizeof(copy));memset(&draft,0,sizeof(draft));
        if(!copy_cell(&copy,value,&bytes))return UI_STATUS_INVALID_ARGUMENT;
        if(!copy_cell(&draft,value,&bytes)){free_cell(&copy);return UI_STATUS_OUT_OF_MEMORY;}
        copy.flags|=c->fields[i].flags&(UI_VALUE_REQUIRED|UI_VALUE_MULTILINE);draft.flags=copy.flags;
        free_cell(&c->values[i]);c->values[i]=copy;
        if(!(c->drafts[i].flags&UI_VALUE_MODIFIED)){free_cell(&c->drafts[i]);c->drafts[i]=draft;c->validation_errors[i]=NULL;}
        else{c->drafts[i].flags=(copy.flags&~UI_VALUE_MODIFIED)|UI_VALUE_MODIFIED;free_cell(&draft);}
        (void)ui_host_hide_tooltip(c->host);render_component(c);return UI_STATUS_OK;}
    return UI_STATUS_NOT_FOUND;
}
ui_status_t ui_component_set_error(ui_component_t *c,const char *id,const char *error)
{
    size_t i,bytes=0;char *copy;if(!c||!id)return UI_STATUS_INVALID_ARGUMENT;
    for(i=0;i<c->desc.field_count;++i)if(!strcmp(c->fields[i].id,id)){
        copy=copy_string(error,&bytes);if(!copy)return UI_STATUS_INVALID_ARGUMENT;
        free((void *)c->drafts[i].error);c->drafts[i].error=copy;c->validation_errors[i]=NULL;render_component(c);return UI_STATUS_OK;}
    return UI_STATUS_NOT_FOUND;
}
ui_status_t ui_component_accept_fields(ui_component_t *c)
{
    size_t i,bytes=0;if(!c)return UI_STATUS_INVALID_ARGUMENT;
    for(i=0;i<c->desc.field_count;++i){ui_cell_t value={0};
        if(!copy_cell(&value,&c->drafts[i],&bytes))return UI_STATUS_OUT_OF_MEMORY;
        value.flags&=~UI_VALUE_MODIFIED;c->drafts[i].flags&=~UI_VALUE_MODIFIED;
        free_cell(&c->values[i]);c->values[i]=value;}
    c->dirty=0;render_component(c);return UI_STATUS_OK;
}
static component_thumb_t *thumb(ui_component_t *c,uint64_t id,int create)
{
    component_thumb_t *t,**link,**old=NULL;size_t count=0;
    for(link=&c->thumbs;*link;link=&(*link)->next){t=*link;if(t->id==id){t->used=++c->use_sequence;return t;}
        ++count;if(!old||t->used<(*old)->used)old=link;}
    if(!create)return NULL;
    if(count>=512&&old){t=*old;*old=t->next;if(t->image)(void)ui_image_release(c->host,t->image);ui_images_release_metadata(c->host,sizeof(*t));free(t);}
    if(ui_images_reserve_metadata(c->host,sizeof(*t))!=UI_STATUS_OK){c->resource_status=UI_STATUS_LIMIT_EXCEEDED;return NULL;}
    t=(component_thumb_t *)calloc(1,sizeof(*t));if(!t){ui_images_release_metadata(c->host,sizeof(*t));c->resource_status=UI_STATUS_OUT_OF_MEMORY;return NULL;}
    t->id=id;t->used=++c->use_sequence;t->next=c->thumbs;c->thumbs=t;return t;
}
ui_status_t ui_component_thumbnail(ui_component_t *c,const ui_thumbnail_result_t *r)
{
    component_thumb_t *t;ui_row_t *row;ui_status_t status;
    if(!c||!r||r->size<sizeof(*r))return UI_STATUS_INVALID_ARGUMENT;
    row=cached_row(c,r->item_id);t=thumb(c,r->item_id,0);
    if(c->host->dispatch_blocked||!row||!t||r->component_generation!=c->generation||r->request_id!=t->request||
        r->content_version!=row->content_version||r->content_version!=t->version||r->dpi!=t->dpi)return UI_STATUS_CANCELLED;
    if(r->failed){t->failed=1;render_component(c);return UI_STATUS_OK;}
    if(t->image){status=ui_image_update(c->host,t->image,&r->rgba);if(status==UI_STATUS_NOT_FOUND)t->image=0;}
    else status=UI_STATUS_NOT_FOUND;
    if(!t->image)status=ui_image_create(c->host,&r->rgba,&t->image);
    c->resource_status=status;if(status==UI_STATUS_OK){(void)ui_image_set_evictable(c->host,t->image);t->failed=0;render_component(c);}
    else{ui_json_t error={0};t->failed=1;uj_add(&error,"{\"component\":");uj_string(&error,c->desc.id);uj_fmt(&error,",\"status\":%d,\"request\":\"%llu\"}",status,(unsigned long long)r->request_id);
        if(!error.failed)(void)ui_host_emit_event(c->host,"ui.components.resource-error",error.data);free(error.data);render_component(c);}return status;
}
static uint64_t row_image(ui_component_t *c,const ui_row_t *r)
{
    component_thumb_t *t;ui_component_query_t q;ui_image_info_t info;
    if(r->image_id)return r->image_id;if(!c->desc.source)return 0;
    if(c->host->dispatch_blocked){t=thumb(c,r->id,0);return t?t->image:0;}
    t=thumb(c,r->id,1);if(!t)return 0;memset(&info,0,sizeof(info));info.size=sizeof(info);
    if(t->image&&ui_image_get_info(c->host,t->image,&info)!=UI_STATUS_OK){t->image=0;t->request=0;}
    if(t->version!=r->content_version||t->dpi!=(c->view?c->view->dpi:c->host->dpi)){if(t->image)(void)ui_image_release(c->host,t->image);t->image=0;t->request=0;t->failed=0;}
    if(!t->request){t->request=++c->next_request;t->version=r->content_version;t->dpi=c->view?c->view->dpi:c->host->dpi;
        memset(&q,0,sizeof(q));q.size=sizeof(q);q.kind=UI_QUERY_THUMBNAIL;q.component_generation=c->generation;
        q.request_id=t->request;q.item_id=r->id;q.content_version=r->content_version;q.dpi=t->dpi;
        q.pixel_width=q.pixel_height=(uint32_t)(((uint64_t)c->desc.row_height*t->dpi+95)/96);
        c->desc.source(c,&q,c->desc.user_data);}
    return t->image;
}

/* The presentation layer below uses the existing Web view and JSON bridge.
 * Data remains C-owned; only visible rows are serialized. */
ui_status_t ui_component_show_menu(ui_component_t *c,uint64_t id,const char *path)
{
    ui_menu_popup_desc_t popup={0};
    if(!c||!path||!*path||c->desc.kind>UI_COMPONENT_LIST)return UI_STATUS_INVALID_ARGUMENT;
    popup.size=sizeof(popup);popup.path=path;popup.target_id=id;popup.anchor.size=sizeof(popup.anchor);
    popup.anchor.kind=UI_MENU_ANCHOR_COMPONENT_ROW;popup.anchor.component=c;popup.anchor.row_id=id;
    return ui_host_show_menu(c->host,&popup);
}
static void json_cell(ui_json_t *json,const ui_cell_t *cell)
{
    uj_add(json,"{\"column\":");uj_string(json,cell->column_id);uj_add(json,",\"text\":");uj_string(json,cell->text);
    uj_add(json,",\"error\":");uj_string(json,cell->error);uj_fmt(json,",\"kind\":%d,\"flags\":%u,\"color\":%u,\"image\":\"%llu\"}",
        cell->kind,cell->flags,cell->color_rgba,(unsigned long long)cell->image_id);
}
static void json_row(ui_component_t *c,ui_json_t *json,const ui_row_t *r,int depth,size_t position)
{
    size_t j,k,emitted=0;int need_image=c->desc.kind!=UI_COMPONENT_TABLE;component_page_t *child=page(c,r->id,0);uint64_t image;
    if(!need_image)for(j=0;j<r->cell_count;++j)if(r->cells[j].kind==UI_VALUE_IMAGE){
        for(k=c->render_column;k<c->desc.column_count&&k<c->render_column+c->column_count;++k)if(!strcmp(c->columns[k].id,r->cells[j].column_id)){need_image=1;break;}}
    image=need_image?row_image(c,r):r->image_id;
    component_thumb_t *image_state=thumb(c,r->id,0);
    uj_fmt(json,"{\"id\":\"%llu\",\"version\":\"%llu\",\"image\":\"%llu\",\"position\":%zu,\"depth\":%d,\"children\":%s,\"expanded\":%s,\"title\":",
        (unsigned long long)r->id,(unsigned long long)r->content_version,(unsigned long long)image,position,depth,
        r->has_children?"true":"false",child&&child->expanded?"true":"false");uj_string(json,r->title);
    uj_fmt(json,",\"index\":\"%llu\"",(unsigned long long)(c->first-(c->buffered?(c->first<2?c->first:2):0)+position));
    uj_add(json,",\"imageState\":");uj_string(json,image?"ready":image_state&&image_state->failed?"failed":image_state&&image_state->request?"loading":"empty");uj_add(json,",\"cells\":[");
    for(j=0;j<r->cell_count;++j){ui_cell_t cell=r->cells[j];
        if(c->desc.column_count){for(k=c->render_column;k<c->desc.column_count&&k<c->render_column+c->column_count;++k)if(!strcmp(c->columns[k].id,cell.column_id))break;
            if(k>=c->desc.column_count||k>=c->render_column+c->column_count)continue;}
        if(emitted++)uj_add(json,",");
        if(cell.kind==UI_VALUE_IMAGE&&!cell.image_id)cell.image_id=image;
        if(cell.kind==UI_VALUE_STYLE){ui_status_t status=cell_preview(c,&cell);
            if(status!=UI_STATUS_OK)c->resource_status=status;else ((ui_cell_t *)&r->cells[j])->image_id=cell.image_id;}
        json_cell(json,&cell);}
    uj_add(json,"]}");
}
static uint64_t tree_extra(ui_component_t *c,component_page_t *p,int depth)
{
    component_page_t *child;uint64_t total=p&&p->expanded?p->total:0;if(!total||depth>32)return total;
    for(child=c->pages;child;child=child->next)if(child!=p&&child->ancestor==p->parent&&child->parent_index<p->total&&child->expanded){uint64_t extra=tree_extra(c,child,depth+1);if(UINT64_MAX-total<extra)return UINT64_MAX;total+=extra;}
    return total;
}
static uint64_t tree_total(ui_component_t *c)
{
    component_page_t *root=page(c,0,0),*child;uint64_t total=root?root->total:0;
    for(child=c->pages;child;child=child->next)if(child->parent&&child->ancestor==0&&child->parent_index<(root?root->total:0)&&child->expanded){uint64_t extra=tree_extra(c,child,0);if(UINT64_MAX-total<extra)return UINT64_MAX;total+=extra;}return total;
}
static component_page_t *expanded_at(ui_component_t *c,uint64_t parent,uint64_t index)
{component_page_t *p;for(p=c->pages;p;p=p->next)if(p->parent&&p->ancestor==parent&&p->parent_index==index&&p->expanded)return p;return NULL;}
/* Seek by metadata, never materialize the whole expanded tree. */
static int tree_seek(ui_component_t *c,component_page_t *p,uint64_t offset,int depth,component_page_t **target,uint64_t *index,int *level)
{
    uint64_t cursor=0;component_page_t *next,*at;if(!p||depth>32)return 0;
    while(cursor<p->total){uint64_t boundary=p->total,plain;
        next=NULL;for(at=c->pages;at;at=at->next)if(at->parent&&at->ancestor==p->parent&&at->expanded&&at->parent_index>=cursor&&at->parent_index<boundary){boundary=at->parent_index;next=at;}
        plain=boundary-cursor;if(offset<plain){*target=p;*index=cursor+offset;*level=depth;return 1;}offset-=plain;cursor=boundary;
        if(!next)break;
        if(!offset){*target=p;*index=cursor;*level=depth;return 1;}--offset;++cursor;
        plain=tree_extra(c,next,depth);if(offset<plain)return tree_seek(c,next,offset,depth+1,target,index,level);offset-=plain;
    }
    return 0;
}
static void render_rows(ui_component_t *c,ui_json_t *json,component_page_t *p,int depth,size_t *position)
{
    size_t i;(void)depth;if(!p)return;
    if(c->desc.kind!=UI_COMPONENT_TREE){if(!p->batch)return;for(i=0;i<p->batch->row_count&&*position<c->visible_count;++i){if(*position)uj_add(json,",");json_row(c,json,&p->batch->rows[i],0,(*position)++);}return;}
    for(i=0;i<c->visible_count;++i){component_page_t *target=NULL;uint64_t index;int level;
        if(!tree_seek(c,p,(c->buffered?(c->first>2?c->first-2:0):c->first)+i,0,&target,&index,&level))break;
        if(!target->batch||index<target->batch->first||index>=target->batch->first+target->batch->row_count){
            if(target->first!=index||!target->pending)(void)request_rows(c,target->parent,index,c->visible_count,0,c->desc.column_count);
            if(!target->batch||index<target->batch->first||index>=target->batch->first+target->batch->row_count)break;
        }
        if(*position)uj_add(json,",");json_row(c,json,&target->batch->rows[(size_t)(index-target->batch->first)],level,(*position)++);
    }
}
static void render_component(ui_component_t *c)
{
    ui_json_t json={0};size_t i,j,position=0;component_page_t *root;component_preview_t *preview;
    if(!c->view)return;if(c->rendering){c->again=1;return;}c->rendering=1;ui_dispatch_enter(c->host);
    for(preview=c->previews;preview;preview=preview->next)preview->used=0;
    root=page(c,0,0);
    if(root&&c->first){uint64_t total=c->desc.kind==UI_COMPONENT_TREE?tree_total(c):root->total;
        if(c->first>=total){(void)query_viewport(c,total?total-1:0,c->first_column,0);root=page(c,0,0);}}
    uj_fmt(&json,"{\"kind\":%d,\"rowHeight\":%d,\"width\":%d,\"height\":%d,\"first\":\"%llu\",\"total\":\"%llu\",\"selected\":\"%llu\",\"title\":",
        c->desc.kind,c->desc.row_height,c->width,c->height,(unsigned long long)c->first,(unsigned long long)(c->desc.kind==UI_COMPONENT_TREE?tree_total(c):(root?root->total:0)),(unsigned long long)c->selected);
    uj_string(&json,c->desc.title);component_language_json(c,&json);uj_add(&json,",\"selection\":[");for(i=0;i<c->selection_count;++i){if(i)uj_add(&json,",");uj_fmt(&json,"\"%llu\"",(unsigned long long)c->selection[i]);}
    uj_add(&json,"],\"sortColumn\":");uj_string(&json,c->sort_column);uj_fmt(&json,",\"sortDirection\":%d,\"sortable\":%s,\"experience\":%s,\"totalColumns\":%zu,\"generation\":\"%llu\",\"columns\":[",c->sort_direction,c->desc.sort_command&&*c->desc.sort_command?"true":"false",c->desc.selection_flags||c->desc.sort_command&&*c->desc.sort_command?"true":"false",c->desc.column_count,(unsigned long long)c->generation);
    for(i=c->render_column;i<c->desc.column_count&&i<c->render_column+c->column_count;++i){if(i>c->render_column)uj_add(&json,",");
        uj_add(&json,"{\"id\":");uj_string(&json,c->columns[i].id);uj_add(&json,",\"title\":");uj_string(&json,c->columns[i].title);
        uj_fmt(&json,",\"width\":%d,\"kind\":%d,\"buffer\":%s}",c->columns[i].width,c->columns[i].kind,c->buffered&&i<c->first_column?"true":"false");}
    uj_add(&json,"],\"columnWidths\":[");for(i=0;i<c->desc.column_count;++i){if(i)uj_add(&json,",");uj_fmt(&json,"%d",c->columns[i].width);}
    uj_fmt(&json,"],\"firstColumn\":%zu,\"visibleCount\":%zu,\"bufferStart\":%zu,\"buffered\":%s,\"rows\":[",c->first_column,c->visible_count,c->buffered?(size_t)(c->first<2?c->first:2):0,c->buffered?"true":"false");render_rows(c,&json,root,0,&position);uj_add(&json,"],\"fields\":[");
    for(i=0;i<c->desc.field_count;++i){const ui_field_desc_t *f=&c->fields[i];ui_image_info_t image={0};image.size=sizeof(image);if(i)uj_add(&json,",");
        uj_add(&json,"{\"id\":");uj_string(&json,f->id);uj_add(&json,",\"title\":");uj_string(&json,f->title);
        uj_add(&json,",\"unit\":");uj_string(&json,f->unit);uj_add(&json,",\"group\":");uj_string(&json,f->group);
        uj_add(&json,",\"help\":");uj_string(&json,f->help);
        uj_fmt(&json,",\"height\":%d,\"kind\":%d,\"flags\":%u,\"value\":",c->field_heights[i]?c->field_heights[i]:98,f->kind,c->drafts[i].flags);
        if(f->kind==UI_VALUE_STYLE){ui_status_t status=cell_preview(c,&c->drafts[i]);if(status!=UI_STATUS_OK)c->resource_status=status;}
        json_cell(&json,&c->drafts[i]);
        if(c->drafts[i].image_id)(void)ui_image_get_info(c->host,c->drafts[i].image_id,&image);
        uj_fmt(&json,",\"imageWidth\":%u,\"imageHeight\":%u,\"options\":[",image.width,image.height);
        for(j=0;j<f->option_count;++j){if(j)uj_add(&json,",");uj_string(&json,f->options[j]);}uj_add(&json,"],\"optionLabels\":[");
        for(j=0;j<f->option_count;++j){if(j)uj_add(&json,",");uj_string(&json,c->option_labels[i]?c->option_labels[i][j]:f->options[j]);}uj_add(&json,"]}");}
    {int actions=c->desc.kind==UI_COMPONENT_DIALOG||c->desc.commands.submit[0]||c->desc.commands.cancel[0];
     for(i=0;i<c->desc.field_count;++i)if(!(c->drafts[i].flags&(UI_VALUE_READONLY|UI_VALUE_DISABLED))&&c->fields[i].kind!=UI_VALUE_GROUP)actions=1;
     uj_fmt(&json,"],\"formActions\":%s",actions?"true":"false");}
    uj_fmt(&json,",\"dark\":%s,\"loading\":%s,\"modal\":%s,\"visible\":%s,\"inputAllowed\":%s}",c->host->menu_dark?"true":"false",root&&root->pending?"true":"false",c->modal?"true":"false",c->visible?"true":"false",c->host->app_active&&!c->host->dispatch_blocked&&(!c->host->modal_component||c->host->modal_component==c)?"true":"false");
    c->presentation_status=json.failed?UI_STATUS_LIMIT_EXCEEDED:ui_web_view_post_json(c->view,json.data);
    free(json.data);release_previews(c,0);
    c->rendering=0;ui_dispatch_leave(c->host);if(c->again){c->again=0;render_component(c);}
}
void ui_components_set_theme(ui_host_t *host,int dark)
{
    ui_component_t *c;if(host->menu_dark==dark)return;host->menu_dark=dark;
#ifdef _WIN32
    ui_shell_sync_visual(host);
#endif
    for(c=host->components;c;c=c->next)render_component(c);
}
static void invoke_component(ui_component_t *c,const char *command,const char *params)
{
    ui_command_state_t state={0};state.size=sizeof(state);
    if(command&&*command&&ui_host_get_command_state(c->host,command,&state)==UI_STATUS_OK&&state.enabled&&!state.busy)
        (void)ui_host_invoke(c->host,command,params,"component");
}
#include "component_experience.inc"
static void component_message(ui_web_view_t *view,const char *json,void *user)
{
    ui_component_t *c=(ui_component_t *)user;char action[64],field[128],text[4096];uint64_t id;size_t i;
    (void)view;if(!uj_get(json,"action",action,sizeof(action)))return;
    if(c->host->modal_component&&c->host->modal_component!=c)return;
    if(!strcmp(action,"tip")){ui_row_t *row=cached_row(c,uj_u64(json,"id"));ui_menu_anchor_t anchor={0};const char *tip=NULL;
        if(uj_u64(json,"generation")!=c->generation||uj_u64(json,"languageGeneration")!=c->host->language.generation)return;
        if(row){tip=row->title;anchor.kind=UI_MENU_ANCHOR_COMPONENT_ROW;anchor.component=c;anchor.row_id=row->id;
            if(uj_get(json,"column",field,sizeof(field)))for(i=0;i<row->cell_count;++i)if(!strcmp(row->cells[i].column_id,field)){tip=row->cells[i].text;break;}}
        else if(uj_get(json,"column",field,sizeof(field))){i=column_index(c,field);if(i<c->desc.column_count)tip=c->columns[i].title;}
        else if(uj_get(json,"field",field,sizeof(field)))for(i=0;i<c->desc.field_count;++i)if(!strcmp(c->fields[i].id,field)){int value=uj_get(json,"value",text,sizeof(text))?atoi(text):0;tip=value==1?c->drafts[i].text:value==2?c->drafts[i].error:value==3?c->fields[i].help:c->fields[i].title;uint64_t option=uj_u64(json,"option");if(uj_get(json,"option",text,sizeof(text))&&option<c->fields[i].option_count)tip=c->option_labels[i]?c->option_labels[i][option]:c->fields[i].options[option];break;}
        if(tip&&*tip){uint64_t x=uj_u64(json,"x"),y=uj_u64(json,"y"),w=uj_u64(json,"width"),h=uj_u64(json,"height");
            if(x>=(uint64_t)c->width||y>=(uint64_t)c->height||!w||!h||w>(uint64_t)c->width-x||h>(uint64_t)c->height-y)return;
            anchor.size=sizeof(anchor);anchor.rect=(ui_rect_t){(int)x,(int)y,(int)w,(int)h};
            if(row){c->tooltip_anchor=&anchor.rect;c->tooltip_row_id=row->id;}else{anchor.kind=UI_MENU_ANCHOR_CONTENT_SLOT;anchor.slot=c->slot;if(!c->slot){anchor.kind=UI_MENU_ANCHOR_HOST;
#ifdef _WIN32
                HWND native=(HWND)ui_web_view_native_handle(c->view);if(native&&c->host->native_parent){POINT at={MulDiv(anchor.rect.x,(int)c->dpi,96),MulDiv(anchor.rect.y,(int)c->dpi,96)};
                    ClientToScreen(native,&at);ScreenToClient((HWND)c->host->native_parent,&at);anchor.rect.x=MulDiv(at.x,96,(int)c->host->dpi);anchor.rect.y=MulDiv(at.y,96,(int)c->host->dpi);
                    anchor.rect.width=MulDiv(anchor.rect.width,(int)c->dpi,(int)c->host->dpi);anchor.rect.height=MulDiv(anchor.rect.height,(int)c->dpi,(int)c->host->dpi);}
#endif
            }}
            (void)ui_host_show_tooltip(c->host,&anchor,tip);c->tooltip_anchor=NULL;}return;}
    if(!strcmp(action,"tip-hide")){ui_menus_hide_tooltip(c->host);return;}
    if(!strcmp(action,"focus")){c->focused=uj_get(json,"focused",field,sizeof(field))&&*field;
        if(c->focused)c->host->focused_component=c;else if(c->host->focused_component==c)c->host->focused_component=NULL;return;}
    if(!strcmp(action,"metrics")){ui_rect_t rows;size_t wanted,budget;
        int has_height=uj_get(json,"rows_height",field,sizeof(field));memset(&rows,0,sizeof(rows));if(has_height)rows.height=atoi(field);
        c->rendered_nodes=(size_t)uj_u64(json,"nodes");c->row_nodes_created=(size_t)uj_u64(json,"mounts");c->focused=uj_get(json,"focused",field,sizeof(field))&&*field;
#ifdef _WIN32
        if(c->view&&GetFocus()==(HWND)ui_web_view_native_handle(c->view))c->host->focused_component=c;
#endif
        if(c->buffered&&c->desc.source&&c->desc.kind<=UI_COMPONENT_LIST&&(has_height||ui_web_view_get_element_rect(c->view,"rows",&rows)==UI_STATUS_OK)&&rows.height>0){
            wanted=(size_t)((rows.height+c->desc.row_height-1)/c->desc.row_height)+2+(size_t)(c->first<2?c->first:2);budget=viewport_row_budget(c->column_count);
            if(wanted<4)wanted=4;if(wanted>128)wanted=128;if(wanted>budget)wanted=budget;
            if(wanted!=c->visible_count)(void)query_viewport(c,c->first,c->first_column,rows.height);}return;}
    if((!strcmp(action,"scroll")||!strcmp(action,"columns"))&&uj_get(json,"generation",field,sizeof(field))&&uj_u64(json,"generation")!=c->generation)return;
    if(!strcmp(action,"column-begin")||!strcmp(action,"column-end")||!strcmp(action,"column-cancel")){
        if(uj_u64(json,"generation")!=c->generation)return;
        if(!strcmp(action,"column-cancel")){cancel_column_resize(c,1);return;}
        if(!strcmp(action,"column-end")){c->column_drag_active=0;return;}
        if(c->desc.kind==UI_COMPONENT_TABLE&&c->host->app_active&&!c->host->dispatch_blocked&&uj_get(json,"column",field,sizeof(field))){size_t column=column_index(c,field);
            if(column<c->desc.column_count){cancel_column_resize(c,0);c->column_drag_active=1;c->column_drag_index=column;c->column_drag_origin=c->columns[column].width;}}return;}
    if(!strcmp(action,"column-width")||!strcmp(action,"column-fit")||!strcmp(action,"column-reset")){
        if(!c->host->app_active||c->host->dispatch_blocked||uj_u64(json,"generation")!=c->generation)return;
        if(uj_get(json,"column",field,sizeof(field))){if(!strcmp(action,"column-fit"))(void)ui_component_fit_column(c,field);
            else if(!strcmp(action,"column-reset"))(void)ui_component_reset_columns(c,*field?field:NULL);
            else{uint64_t width=uj_u64(json,"width");if(width<=4096)(void)ui_component_set_column_width(c,field,(int)width);}}return;}
    if(!strcmp(action,"scroll")){uint64_t first=uj_u64(json,"first");component_page_t *p=page(c,0,0);
        if(c->desc.kind==UI_COMPONENT_TREE){uint64_t total=tree_total(c);first=first<total?first:total?total-1:0;(void)query_viewport(c,first,c->first_column,0);render_component(c);return;}
        if(p&&first>=p->total)first=p->total?p->total-1:0;
        (void)query_viewport(c,first,c->first_column,0);return;}
    if(!strcmp(action,"columns")){size_t first=(size_t)uj_u64(json,"first");if(first<c->desc.column_count)
        (void)query_viewport(c,c->first,first,0);return;}
    id=uj_u64(json,"id");
    if(!strcmp(action,"expand")){component_page_t *p=page(c,id,0);(void)ui_component_expand(c,id,!p||!p->expanded);return;}
    if(!strcmp(action,"select")){user_selection(c,json,id);return;}
    if(!strcmp(action,"sort")&&uj_get(json,"column",field,sizeof(field))){int direction=atoi(uj_get(json,"direction",text,sizeof(text))?text:"0");(void)ui_component_set_sort(c,field,direction);return;}
    if(!strcmp(action,"edit")){ui_row_t *row=cached_row(c,id);if(row&&uj_get(json,"column",field,sizeof(field)))for(i=0;i<row->cell_count;++i)if(!strcmp(row->cells[i].column_id,field)&&!(row->cells[i].flags&(UI_VALUE_READONLY|UI_VALUE_DISABLED))){invoke_component(c,c->desc.commands.edit,json);break;}return;}
    if(!strcmp(action,"rename")){invoke_component(c,c->desc.commands.rename,json);return;}
    if(!strcmp(action,"context")){invoke_component(c,c->desc.commands.context_menu,json);return;}
    if(!strcmp(action,"draft")&&uj_get(json,"field",field,sizeof(field))&&uj_get(json,"text",text,sizeof(text))){
        for(i=0;i<c->desc.field_count;++i)if(!strcmp(c->fields[i].id,field)&&!(c->drafts[i].flags&(UI_VALUE_READONLY|UI_VALUE_DISABLED))){
            char *copy=ui_strdup(text);if(copy){uint32_t rgba;free((void *)c->drafts[i].text);c->drafts[i].text=copy;c->drafts[i].flags=(c->drafts[i].flags&~UI_VALUE_MIXED)|UI_VALUE_MODIFIED;c->dirty=1;
                if(c->fields[i].kind==UI_VALUE_COLOR&&parse_color(text,&rgba))c->drafts[i].color_rgba=rgba;}break;}return;}
    if(!strcmp(action,"cancel")){for(i=0;i<c->desc.field_count;++i){size_t bytes=0;ui_cell_t copy={0};if(copy_cell(&copy,&c->values[i],&bytes)){free_cell(&c->drafts[i]);c->drafts[i]=copy;}}
        c->dirty=0;invoke_component(c,c->desc.commands.cancel,json);if(c->modal)(void)ui_component_close_dialog(c);else render_component(c);return;}
    if(!strcmp(action,"submit")){ui_json_t params={0};int valid=1;uj_add(&params,"{\"fields\":{");
        for(i=0;i<c->desc.field_count;++i){const char *value=c->drafts[i].text;char *end;
            (void)ui_component_set_error(c,c->fields[i].id,"");
            if((c->drafts[i].flags&UI_VALUE_REQUIRED)&&!*value){validation_error(c,i,"必填字段");valid=0;}
            if(c->fields[i].kind==UI_VALUE_NUMBER&&*value){double number=strtod(value,&end);if(end==value||*end||!isfinite(number)){validation_error(c,i,"请输入有效数字");valid=0;}}
            if(c->fields[i].kind==UI_VALUE_ENUM&&*value){size_t option;for(option=0;option<c->fields[i].option_count&&strcmp(c->fields[i].options[option],value);++option){}if(option==c->fields[i].option_count){validation_error(c,i,"请选择有效选项");valid=0;}}
            if(c->fields[i].kind==UI_VALUE_COLOR&&*value){uint32_t rgba;if(!parse_color(value,&rgba)){validation_error(c,i,"颜色格式为 #RRGGBB 或 #RRGGBBAA");valid=0;}}
            if(i)uj_add(&params,",");uj_string(&params,c->fields[i].id);uj_add(&params,":");uj_string(&params,value);}
        uj_add(&params,"}}");if(valid&&!params.failed)invoke_component(c,c->desc.commands.submit,params.data);free(params.data);render_component(c);}
}
ui_status_t ui_component_mount(ui_component_t *c,ui_content_slot_t *slot)
{
#if defined(UI_FRAMEWORK_ENABLE_LIGHT_WEB) || defined(UI_FRAMEWORK_HAS_WEBVIEW2)
    ui_status_t status;ui_rect_t rect;uint64_t caps=0;
    if(!c||!slot||c->view||!ui_content_slot_belongs_to(slot,c->host))return UI_STATUS_INVALID_ARGUMENT;
    c->backend=component_backend(c,ui_content_slot_native_handle(slot));if(!c->backend)return UI_STATUS_UNSUPPORTED;
    c->view=ui_web_view_create(c->host,c->backend);if(!c->view){release_backend(c);return UI_STATUS_OUT_OF_MEMORY;}
    if(c->desc.web_backend&&(ui_web_view_get_capabilities(c->view,&caps)!=UI_STATUS_OK||!(caps&UI_WEB_CAP_COMPONENTS)||!ui_web_view_native_handle(c->view))){ui_web_view_destroy(c->view);c->view=NULL;release_backend(c);return UI_STATUS_UNSUPPORTED;}
#ifdef _WIN32
    if(c->desc.web_backend)(void)SetParent((HWND)ui_web_view_native_handle(c->view),(HWND)ui_content_slot_native_handle(slot));
#endif
    status=ui_web_view_set_message_callback(c->view,component_message,c);
    if(status==UI_STATUS_OK)status=ui_web_view_load_html(c->view,ui_component_page);
    if(status==UI_STATUS_OK)status=ui_content_slot_attach_web_view(slot,c->view);
    if(status!=UI_STATUS_OK){ui_web_view_destroy(c->view);c->view=NULL;release_backend(c);return status;}
    c->slot=slot;c->width=-1;c->height=-1;(void)rect;
    ui_components_layout(c->host);render_component(c);return UI_STATUS_OK;
#else
    (void)c;(void)slot;return UI_STATUS_UNSUPPORTED;
#endif
}
ui_status_t ui_component_set_visible(ui_component_t *c,int visible)
{if(!c)return UI_STATUS_INVALID_ARGUMENT;if(!visible)cancel_column_resize(c,1);if(!visible)(void)ui_host_hide_tooltip(c->host);c->visible=visible!=0;render_component(c);return ui_host_emit_event(c->host,"ui.components.changed","{}");}
void ui_components_layout(ui_host_t *host)
{
    ui_component_t *c;for(c=host->components;c;c=c->next)if(c->slot){ui_rect_t rect;
        if(ui_content_slot_get_rect(c->slot,&rect)==UI_STATUS_OK&&(rect.width!=c->width||rect.height!=c->height||c->dpi!=c->view->dpi)){
            cancel_column_resize(c,0);c->width=rect.width;c->height=rect.height;c->dpi=c->view->dpi;
            if(c->desc.source)(void)query_viewport(c,c->first,c->first_column,0);render_component(c);}}
}
static int dialog_initial_height(const ui_component_t *c)
{
    size_t i;int height=24+28+16+24;
    for(i=0;i<c->desc.field_count;++i){const ui_field_desc_t *f=&c->fields[i];int h=(f->flags&UI_VALUE_MULTILINE)?(c->field_heights[i]?c->field_heights[i]:98):28;
        height+=24+h+8;if(*f->help)height+=36;if(*f->unit)height+=20;if(c->drafts[i].error&&*c->drafts[i].error)height+=24;}
    return height>32767?32767:height;
}
ui_status_t ui_component_get_dialog_layout(const ui_component_t *c,ui_dialog_layout_t *out)
{
    if(!c||c->desc.kind!=UI_COMPONENT_DIALOG||!out||out->size<sizeof(*out))return UI_STATUS_INVALID_ARGUMENT;
    *out=c->dialog_layout;out->size=sizeof(*out);if(!out->preferred_width)out->preferred_width=440;
    if(!out->preferred_height)out->preferred_height=dialog_initial_height(c);if(!out->min_width)out->min_width=240;if(!out->min_height)out->min_height=180;
    if(out->preferred_width<out->min_width)out->preferred_width=out->min_width;if(out->preferred_height<out->min_height)out->preferred_height=out->min_height;
    if(out->max_width&&out->preferred_width>out->max_width)out->preferred_width=out->max_width;if(out->max_height&&out->preferred_height>out->max_height)out->preferred_height=out->max_height;return UI_STATUS_OK;
}
#if defined(_WIN32) && (defined(UI_FRAMEWORK_ENABLE_LIGHT_WEB) || defined(UI_FRAMEWORK_HAS_WEBVIEW2))
static RECT dialog_outer(ui_component_t *c,int width,int height)
{RECT rect={0,0,MulDiv(width,(int)c->dpi,96),MulDiv(height,(int)c->dpi,96)};AdjustWindowRectExForDpi(&rect,WS_POPUP|WS_CAPTION|WS_THICKFRAME,FALSE,WS_EX_TOOLWINDOW,ui_platform_get_dpi(c->dialog_window?c->dialog_window:c->host->native_parent));return rect;}
static void dialog_position(ui_component_t *c,HWND window)
{
    ui_dialog_layout_t layout={0};MONITORINFO monitor={0};RECT owner,outer;int x,y,w,h;layout.size=sizeof(layout);(void)ui_component_get_dialog_layout(c,&layout);outer=dialog_outer(c,layout.preferred_width,layout.preferred_height);GetWindowRect((HWND)c->host->native_parent,&owner);x=owner.left+40;y=owner.top+40;w=outer.right-outer.left;h=outer.bottom-outer.top;monitor.cbSize=sizeof(monitor);
    if(GetMonitorInfoW(MonitorFromWindow((HWND)c->host->native_parent,MONITOR_DEFAULTTONEAREST),&monitor)){if(w>monitor.rcWork.right-monitor.rcWork.left)w=monitor.rcWork.right-monitor.rcWork.left;if(h>monitor.rcWork.bottom-monitor.rcWork.top)h=monitor.rcWork.bottom-monitor.rcWork.top;if(x+w>monitor.rcWork.right)x=monitor.rcWork.right-w;if(y+h>monitor.rcWork.bottom)y=monitor.rcWork.bottom-h;if(x<monitor.rcWork.left)x=monitor.rcWork.left;if(y<monitor.rcWork.top)y=monitor.rcWork.top;}
    SetWindowPos(window,NULL,x,y,w,h,SWP_NOZORDER|SWP_NOACTIVATE);
}
#endif
ui_status_t ui_component_set_dialog_layout(ui_component_t *c,const ui_dialog_layout_t *layout)
{
    size_t i;int minw,minh;if(!c||c->desc.kind!=UI_COMPONENT_DIALOG||!layout||layout->size<sizeof(*layout))return UI_STATUS_INVALID_ARGUMENT;
    {int values[]={layout->preferred_width,layout->preferred_height,layout->min_width,layout->min_height,layout->max_width,layout->max_height};for(i=0;i<6;++i)if(values[i]<0||values[i]>32767)return UI_STATUS_INVALID_ARGUMENT;}minw=layout->min_width?layout->min_width:240;minh=layout->min_height?layout->min_height:180;
    if(minw<160||minh<120)return UI_STATUS_INVALID_ARGUMENT;
    if((layout->max_width&&layout->max_width<minw)||(layout->max_height&&layout->max_height<minh))return UI_STATUS_INVALID_ARGUMENT;c->dialog_layout=*layout;
#if defined(_WIN32) && (defined(UI_FRAMEWORK_ENABLE_LIGHT_WEB) || defined(UI_FRAMEWORK_HAS_WEBVIEW2))
    if(c->dialog_window)dialog_position(c,(HWND)c->dialog_window);
#endif
    if(c->view&&!c->dialog_window){ui_dialog_layout_t resolved={0};ui_status_t status;resolved.size=sizeof(resolved);(void)ui_component_get_dialog_layout(c,&resolved);
        status=ui_web_view_resize(c->view,resolved.preferred_width,resolved.preferred_height,c->dpi);if(status!=UI_STATUS_OK)return status;c->width=resolved.preferred_width;c->height=resolved.preferred_height;render_component(c);}
    return UI_STATUS_OK;
}
ui_status_t ui_component_set_field_layout(ui_component_t *c,const ui_field_layout_t *layout)
{
    size_t i;if(!c||!layout||layout->size<sizeof(*layout)||!layout->id||layout->visible_rows<0||layout->visible_rows>100||layout->height<0||layout->height>32767||(layout->height&&layout->height<42)||(c->desc.kind!=UI_COMPONENT_FORM&&c->desc.kind!=UI_COMPONENT_DIALOG))return UI_STATUS_INVALID_ARGUMENT;
    for(i=0;i<c->desc.field_count;++i)if(!strcmp(c->fields[i].id,layout->id)){if(c->fields[i].kind!=UI_VALUE_TEXT||!(c->fields[i].flags&UI_VALUE_MULTILINE))return UI_STATUS_INVALID_ARGUMENT;
        c->field_heights[i]=layout->height?layout->height:layout->visible_rows?(layout->visible_rows==1?42:layout->visible_rows*19+22):0;(void)ui_host_hide_tooltip(c->host);render_component(c);return UI_STATUS_OK;}
    return UI_STATUS_NOT_FOUND;
}
#if defined(_WIN32) && (defined(UI_FRAMEWORK_ENABLE_LIGHT_WEB) || defined(UI_FRAMEWORK_HAS_WEBVIEW2))
static LRESULT CALLBACK dialog_proc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp)
{
    ui_component_t *c=(ui_component_t *)GetWindowLongPtrW(hwnd,GWLP_USERDATA);
    if(message==WM_NCCREATE){c=(ui_component_t *)((CREATESTRUCTW *)lp)->lpCreateParams;SetWindowLongPtrW(hwnd,GWLP_USERDATA,(LONG_PTR)c);return TRUE;}
    if(!c)return DefWindowProcW(hwnd,message,wp,lp);
    if(message==WM_CLOSE){ui_dispatch_enter(c->host);invoke_component(c,c->desc.commands.cancel,"{}");(void)ui_component_close_dialog(c);ui_dispatch_leave(c->host);return 0;}
    if(message==WM_GETMINMAXINFO){MINMAXINFO *limits=(MINMAXINFO *)lp;
        ui_dialog_layout_t layout={0};RECT min,max;MONITORINFO monitor={0};layout.size=sizeof(layout);(void)ui_component_get_dialog_layout(c,&layout);min=dialog_outer(c,layout.min_width,layout.min_height);max=dialog_outer(c,layout.max_width?layout.max_width:32767,layout.max_height?layout.max_height:32767);monitor.cbSize=sizeof(monitor);
        if(GetMonitorInfoW(MonitorFromWindow(hwnd,MONITOR_DEFAULTTONEAREST),&monitor)){if(max.right-max.left>monitor.rcWork.right-monitor.rcWork.left)max.right=max.left+monitor.rcWork.right-monitor.rcWork.left;if(max.bottom-max.top>monitor.rcWork.bottom-monitor.rcWork.top)max.bottom=max.top+monitor.rcWork.bottom-monitor.rcWork.top;}
        limits->ptMaxTrackSize.x=max.right-max.left;limits->ptMaxTrackSize.y=max.bottom-max.top;limits->ptMinTrackSize.x=min.right-min.left<limits->ptMaxTrackSize.x?min.right-min.left:limits->ptMaxTrackSize.x;limits->ptMinTrackSize.y=min.bottom-min.top<limits->ptMaxTrackSize.y?min.bottom-min.top:limits->ptMaxTrackSize.y;return 0;}
    if(message==WM_SIZE&&c->view){RECT rect;uint32_t dpi=c->dpi?c->dpi:ui_platform_get_dpi(hwnd);c->dpi=dpi;GetClientRect(hwnd,&rect);c->width=MulDiv(rect.right,96,(int)dpi);c->height=MulDiv(rect.bottom,96,(int)dpi);
        (void)ui_web_view_resize(c->view,c->width,c->height,dpi);render_component(c);return 0;}
    if(message==WM_DPICHANGED){RECT *r=(RECT *)lp;c->dpi=LOWORD(wp);SetWindowPos(hwnd,NULL,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);return 0;}
    return DefWindowProcW(hwnd,message,wp,lp);
}
static ui_status_t create_dialog(ui_component_t *c)
{
    WNDCLASSW wc={0};ui_status_t status;RECT owner;HWND parent=(HWND)c->host->native_parent;
    c->dpi=c->host->dpi;
    wc.lpfnWndProc=dialog_proc;wc.hInstance=GetModuleHandleW(L"ui_framework.dll");wc.lpszClassName=L"UIFrameworkWebDialog3";wc.hCursor=LoadCursorW(NULL,MAKEINTRESOURCEW(32512));
    if(!RegisterClassW(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return UI_STATUS_PLATFORM_ERROR;
    GetWindowRect(parent,&owner);c->dialog_window=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"Application dialog",
        WS_POPUP|WS_CAPTION|WS_THICKFRAME,owner.left+40,owner.top+40,MulDiv(440,(int)c->host->dpi,96),MulDiv(380,(int)c->host->dpi,96),GetAncestor(parent,GA_ROOT),NULL,wc.hInstance,c);
    if(!c->dialog_window)return UI_STATUS_PLATFORM_ERROR;
    dialog_position(c,(HWND)c->dialog_window);
    update_dialog_caption(c);
    c->backend=component_backend(c,c->dialog_window);
    c->view=c->backend?ui_web_view_create(c->host,c->backend):NULL;
    if(!c->view){status=UI_STATUS_OUT_OF_MEMORY;goto fail;}
    if(c->desc.web_backend){HWND child=(HWND)ui_web_view_native_handle(c->view);if(!child){status=UI_STATUS_UNSUPPORTED;goto fail;}(void)SetParent(child,(HWND)c->dialog_window);}
    status=ui_web_view_set_message_callback(c->view,component_message,c);
    if(status==UI_STATUS_OK)status=ui_web_view_load_html(c->view,ui_component_page);
    if(status!=UI_STATUS_OK)goto fail;
    SendMessageW((HWND)c->dialog_window,WM_SIZE,0,0);return UI_STATUS_OK;
fail:
    if(c->view)ui_web_view_destroy(c->view);c->view=NULL;
    release_backend(c);
    DestroyWindow((HWND)c->dialog_window);c->dialog_window=NULL;return status;
}
#endif
ui_status_t ui_component_show_dialog(ui_component_t *c)
{
    if(!c||c->desc.kind!=UI_COMPONENT_DIALOG)return UI_STATUS_INVALID_ARGUMENT;
    if(c->host->modal_component&&c->host->modal_component!=c)return UI_STATUS_ALREADY_EXISTS;
    (void)ui_host_hide_tooltip(c->host);(void)ui_host_close_menu(c->host);c->host->menu_pressed_key=0;
#if defined(_WIN32) && !defined(UI_FRAMEWORK_ENABLE_LIGHT_WEB) && !defined(UI_FRAMEWORK_HAS_WEBVIEW2)
    return UI_STATUS_UNSUPPORTED;
#else
#if defined(_WIN32) && (defined(UI_FRAMEWORK_ENABLE_LIGHT_WEB) || defined(UI_FRAMEWORK_HAS_WEBVIEW2))
    if(c->host->run_mode==UI_RUN_OFFSCREEN||(!c->host->native_parent&&c->view)){
        if(!c->view){ui_dialog_layout_t layout={0};ui_status_t status;layout.size=sizeof(layout);(void)ui_component_get_dialog_layout(c,&layout);status=ui_component_mount_offscreen(c,layout.preferred_width,layout.preferred_height,c->host->dpi);if(status!=UI_STATUS_OK)return status;}
    }else{
    if(!c->dialog_window){ui_status_t status=create_dialog(c);if(status!=UI_STATUS_OK)return status;}
    c->previous_focus=GetFocus();EnableWindow((HWND)c->host->native_parent,FALSE);
    ShowWindow((HWND)c->dialog_window,c->host->app_active?SW_SHOW:SW_HIDE);
    if(c->host->app_active)SetFocus((HWND)ui_web_view_native_handle(c->view));
    }
#endif
    {ui_component_t *other;for(other=c->host->components;other;other=other->next)cancel_column_resize(other,1);}
    c->modal=1;c->visible=1;c->host->modal_component=c;{ui_component_t *other;for(other=c->host->components;other;other=other->next){if(other!=c&&other->view){ui_input_event_t cancel={0};cancel.size=sizeof(cancel);cancel.kind=UI_INPUT_CANCEL;(void)ui_web_view_dispatch_input(other->view,&cancel);}render_component(other);}}return UI_STATUS_OK;
#endif
}
ui_status_t ui_component_get_field(const ui_component_t *c,const char *id,ui_cell_t *value)
{
    size_t i;if(!c||!id||!value||value->size<sizeof(*value))return UI_STATUS_INVALID_ARGUMENT;
    for(i=0;i<c->desc.field_count;++i)if(!strcmp(c->fields[i].id,id)){*value=c->drafts[i];return UI_STATUS_OK;}
    return UI_STATUS_NOT_FOUND;
}
ui_status_t ui_component_close_dialog(ui_component_t *c)
{
    if(!c)return UI_STATUS_INVALID_ARGUMENT;(void)ui_host_hide_tooltip(c->host);c->modal=0;c->visible=0;
    if(c->host->modal_component==c)c->host->modal_component=NULL;
#ifdef _WIN32
    if(c->dialog_window){ShowWindow((HWND)c->dialog_window,SW_HIDE);EnableWindow((HWND)c->host->native_parent,TRUE);
        if(c->host->app_active&&IsWindow((HWND)c->previous_focus)&&IsWindowVisible((HWND)c->previous_focus)&&IsWindowEnabled((HWND)c->previous_focus)){ui_component_t *previous;
            c->host->focused_component=NULL;for(previous=c->host->components;previous;previous=previous->next)
                if(previous->view&&ui_web_view_native_handle(previous->view)==c->previous_focus)c->host->focused_component=previous;
            SetFocus((HWND)c->previous_focus);}}
#endif
    render_component(c);return UI_STATUS_OK;
}
void ui_components_active(ui_host_t *host,int active)
{
    ui_component_t *c;host->app_active=active!=0;if(!active){host->menu_pressed_key=0;(void)ui_host_close_menu(host);}
    for(c=host->components;c;c=c->next){
        if(!active)cancel_column_resize(c,1);render_component(c);
#ifdef _WIN32
        if(c->dialog_window)ShowWindow((HWND)c->dialog_window,active&&c->modal?SW_SHOWNOACTIVATE:SW_HIDE);
        if(active&&c->view&&c->visible&&(host->focused_component==c||host->modal_component==c))SetFocus((HWND)ui_web_view_native_handle(c->view));
#endif
    }
}
static void destroy_component(ui_component_t *c)
{
    (void)ui_host_hide_tooltip(c->host);
    ui_menus_component_invalidated(c->host,c,0);
    if(c->host->modal_component==c)(void)ui_component_close_dialog(c);
    if(c->host->focused_component==c)c->host->focused_component=NULL;
    if(c->view)ui_web_view_destroy(c->view);
#ifdef _WIN32
    if(c->dialog_window)DestroyWindow((HWND)c->dialog_window);
#endif
    release_backend(c);
    free_pages(c);free_thumbs(c);release_previews(c,1);free_description(c);free(c);
}
ui_status_t ui_component_unregister(ui_component_t *c)
{
    ui_component_t **link;if(!c)return UI_STATUS_INVALID_ARGUMENT;
    if(c->host->dispatch_depth)return UI_STATUS_CANCELLED;
    for(link=&c->host->components;*link&&*link!=c;link=&(*link)->next){}
    if(!*link)return UI_STATUS_NOT_FOUND;*link=c->next;destroy_component(c);return UI_STATUS_OK;
}
void ui_components_destroy(ui_host_t *host)
{while(host->components){ui_component_t *c=host->components;host->components=c->next;destroy_component(c);}}

ui_status_t ui_component_mount_offscreen(ui_component_t *c,int width,int height,uint32_t dpi)
{
#ifdef UI_FRAMEWORK_ENABLE_LIGHT_WEB
    ui_light_web_config_t config={0};ui_status_t status;
    if(!c||c->view||width<=0||height<=0||dpi<48||dpi>768)return UI_STATUS_INVALID_ARGUMENT;
    if(c->desc.web_backend)return UI_STATUS_UNSUPPORTED;
    config.size=sizeof(config);c->backend_owned=1;c->backend=ui_light_web_backend_create(&config);if(!c->backend)return UI_STATUS_OUT_OF_MEMORY;
    c->view=ui_web_view_create(c->host,c->backend);if(!c->view){ui_light_web_backend_destroy(c->backend);c->backend=NULL;return UI_STATUS_OUT_OF_MEMORY;}
    status=ui_web_view_set_message_callback(c->view,component_message,c);
    if(status==UI_STATUS_OK)status=ui_web_view_load_html(c->view,ui_component_page);
    if(status==UI_STATUS_OK)status=ui_web_view_resize(c->view,width,height,dpi);
    if(status!=UI_STATUS_OK){ui_web_view_destroy(c->view);c->view=NULL;ui_light_web_backend_destroy(c->backend);c->backend=NULL;return status;}
    c->width=width;c->height=height;c->dpi=dpi;if(c->desc.source)(void)query_viewport(c,c->first,c->first_column,0);render_component(c);return UI_STATUS_OK;
#else
    (void)c;(void)width;(void)height;(void)dpi;return UI_STATUS_UNSUPPORTED;
#endif
}
int ui_components_input_allowed(const ui_host_t *host,const void *view_data)
{
    ui_component_t *c;
    for(c=host->components;c;c=c->next)if(c->view&&c->view->user_data==view_data)
        return !host->dispatch_blocked&&host->app_active&&c->visible&&(!host->modal_component||host->modal_component==c);
    return 1;
}
ui_status_t ui_component_dispatch_input(ui_component_t *c,const ui_input_event_t *event)
{
    if(!c||!event)return UI_STATUS_INVALID_ARGUMENT;if(!c->view)return UI_STATUS_NOT_FOUND;
    if(event&&event->kind!=UI_INPUT_CANCEL&&!ui_components_input_allowed(c->host,c->view->user_data))return UI_STATUS_CANCELLED;
    return ui_web_view_dispatch_input(c->view,event);
}
ui_status_t ui_component_get_presentation(ui_component_t *c,const char *id,ui_element_presentation_t *out)
{return !c?UI_STATUS_INVALID_ARGUMENT:!c->view?UI_STATUS_NOT_FOUND:ui_web_view_get_presentation(c->view,id,out);}
ui_status_t ui_component_capture_rgba(ui_component_t *c,int width,int height,uint32_t dpi,ui_pixel_buffer_t *out)
{if(!c||!c->view||width<=0||height<=0||dpi<48||dpi>768||!out||out->size<sizeof(*out))return UI_STATUS_INVALID_ARGUMENT;
 if(c->width!=width||c->height!=height||c->dpi!=dpi){cancel_column_resize(c,0);c->width=width;c->height=height;c->dpi=dpi;render_component(c);}return ui_web_view_capture_rgba(c->view,width,height,dpi,out);}
ui_status_t ui_component_flush(ui_component_t *c,uint32_t budget)
{return !c?UI_STATUS_INVALID_ARGUMENT:!c->view?UI_STATUS_NOT_FOUND:ui_web_view_flush(c->view,budget);}

int ui_component_menu_target_valid(ui_component_t *c,uint64_t generation,uint64_t id)
{return c&&c->generation==generation&&c->visible&&(!id||cached_row(c,id)!=NULL);}
ui_status_t ui_component_menu_anchor(ui_component_t *c,ui_host_t *host,uint64_t id,ui_rect_t *rect,void **native,uint64_t *generation)
{
    ui_element_presentation_t state={0};char element[80];ui_status_t status;
    if(!c||c->host!=host||!c->view||!cached_row(c,id))return UI_STATUS_NOT_FOUND;
    if(c->tooltip_anchor&&c->tooltip_row_id==id){*rect=*c->tooltip_anchor;*native=ui_web_view_native_handle(c->view);*generation=c->generation;return UI_STATUS_OK;}
    snprintf(element,sizeof(element),"row-%llu",(unsigned long long)id);state.size=sizeof(state);
    status=ui_web_view_get_presentation(c->view,element,&state);if(status!=UI_STATUS_OK)return status;
    if(!state.visible)return UI_STATUS_NOT_FOUND;*rect=state.clip;*native=ui_web_view_native_handle(c->view);*generation=c->generation;return UI_STATUS_OK;
}
