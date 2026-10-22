/* Restricted document support in Light. No JSContext, script evaluation or
 * message path is shared with the trusted application document loader. */
#include "ui_framework/light_document.h"
#include "lexbor/url/url.h"
#define LD_MAX_FRAGMENTS 8192
#define LD_MAX_RESOURCES 40
typedef struct ld_meta {
    lxb_dom_node_t *dom;
    wchar_t *wide;
    int length, offset, block, default_block, link, form, literal, image_state, image_width, image_height, line_clamped;
    long long list_next;int list_step;
    char *url, *name;
} ld_meta_t;
typedef struct ld_fragment { int node, start, count; ui_rect_t rect; } ld_fragment_t;
typedef struct ld_resource {
    uint64_t id;
    int node, kind, issued, complete, sheet_order;
    char *url;
} ld_resource_t;
struct ld_document {
    ui_document_event_fn callback;
    void *user;
    ui_web_view_t *common;
    ui_document_state_t state;
    ld_meta_t *meta;
    ld_fragment_t *fragments;
    int fragment_count, fragment_capacity, text_length, text_capacity;
    int sheet_order,current_sheet;
    uint64_t text_hash;
    wchar_t *text;
    int anchor, caret, selecting, failed, loading, dispatching;
    int no_content,has_base;
    ld_resource_t resources[LD_MAX_RESOURCES];
    int resource_count;
    char source[4096];
    char document_url[4096];
    LARGE_INTEGER frequency;
};
typedef struct ld_flow { int left, right, x, y, height, maximum; HDC dc; } ld_flow_t;
static int ld_font_size(lw_view_t *view,int size)
{if(size<1||size>128){view->restricted->failed=1;return 16;}return size;}
typedef struct ld_url_buffer { char data[4096]; size_t length; } ld_url_buffer_t;
static lxb_status_t ld_url_write(const lxb_char_t *data,size_t size,void *user)
{
    ld_url_buffer_t *b=(ld_url_buffer_t *)user;
    if(size>=sizeof(b->data)-b->length)return LXB_STATUS_ERROR;
    memcpy(b->data+b->length,data,size);b->length+=size;b->data[b->length]=0;return LXB_STATUS_OK;
}
static char *ld_resolve(const char *base,const char *input)
{
    lxb_url_parser_t *p=lxb_url_parser_create();lxb_url_t *b,*u;ld_url_buffer_t out={{0},0};char *result=NULL;
    if(!p)return NULL;
    if(lxb_url_parser_init(p,NULL)!=LXB_STATUS_OK){lxb_url_parser_destroy(p,true);return NULL;}
    b=lxb_url_parse(p,NULL,(const lxb_char_t *)base,strlen(base));lxb_url_parser_clean(p);
    u=b?lxb_url_parse(p,b,(const lxb_char_t *)input,strlen(input)):NULL;
    /* Lexbor 2.5 numeric relative-host metadata omission; do not alter dependency. */
    if(u&&b&&u->host.type==LXB_URL_HOST_TYPE__UNDEF&&(b->host.type==LXB_URL_HOST_TYPE_IPV4||b->host.type==LXB_URL_HOST_TYPE_IPV6))u->host.type=b->host.type;
    if(u&&(u->scheme.type==LXB_URL_SCHEMEL_TYPE_HTTP||u->scheme.type==LXB_URL_SCHEMEL_TYPE_HTTPS)&&
       !u->username.length&&!u->password.length&&lxb_url_serialize(u,ld_url_write,&out,false)==LXB_STATUS_OK)result=_strdup(out.data);
    lxb_url_parser_memory_destroy(p);lxb_url_parser_destroy(p,true);return result;
}
static int ld_tag(lw_node_t *n,const char *tag){return !_stricmp(n->tag,tag);}
static int ld_supported(lw_node_t *n)
{
    static const char *tags[]={"p","div","section","article","main","header","footer","nav","aside","address","blockquote","ul","ol","li","pre","code","form","input","textarea","button","table","thead","tbody","tfoot","tr","td","th","caption","br","hr","h1","h2","h3","h4","h5","h6","span","strong","b","em","i","a","img","label","noscript","html","body"};
    for(size_t i=0;i<sizeof(tags)/sizeof(tags[0]);++i)if(ld_tag(n,tags[i]))return 1;return 0;
}
static char *ld_attribute(lxb_dom_node_t *dom,const char *name)
{
    size_t length=0;const lxb_char_t *value;
    if(!dom||dom->type!=LXB_DOM_NODE_TYPE_ELEMENT)return NULL;
    value=lxb_dom_element_get_attribute(lxb_dom_interface_element(dom),(const lxb_char_t *)name,strlen(name),&length);
    if(!value)return NULL;
    if(length>=LW_ATTR_CAP)return NULL;
    {char *s=(char *)malloc(length+1);if(s){memcpy(s,value,length);s[length]=0;}return s;}
}
static void ld_reset(lw_view_t *v)
{
    ld_document_t *d=v->restricted;int i;
    if(!d)return;
    if(d->meta)for(i=0;i<v->node_count;++i){free(d->meta[i].wide);free(d->meta[i].url);free(d->meta[i].name);}
    for(i=0;i<d->resource_count;++i)free(d->resources[i].url);
    free(d->meta);free(d->fragments);free(d->text);
    {ui_document_event_fn callback=d->callback;void *user=d->user;ui_web_view_t *common=d->common;
     memset(d,0,sizeof(*d));d->callback=callback;d->user=user;d->common=common;d->state.zoom_percent=100;QueryPerformanceFrequency(&d->frequency);}
}
static ui_status_t ld_append_text(ld_document_t *d,ld_meta_t *m,const wchar_t *text)
{
    int count=(int)wcslen(text),needed=d->text_length+count+2;wchar_t *p;
    if((size_t)needed>LW_HTML_LIMIT)return UI_STATUS_LIMIT_EXCEEDED;
    if(needed>d->text_capacity){int capacity=d->text_capacity?d->text_capacity*2:128;if(capacity<needed)capacity=needed;if(capacity>(int)LW_HTML_LIMIT)capacity=(int)LW_HTML_LIMIT;
        p=(wchar_t *)realloc(d->text,(size_t)capacity*sizeof(*p));if(!p)return UI_STATUS_OUT_OF_MEMORY;d->text=p;d->text_capacity=capacity;}
    m->offset=d->text_length;memcpy(d->text+d->text_length,text,(size_t)count*sizeof(wchar_t));d->text_length+=count;d->text[d->text_length]=0;return UI_STATUS_OK;
}
static int ld_add(lw_view_t *v,int parent,lxb_dom_node_t *dom,const char *tag)
{
    int i=v->node_count;lw_node_t *n;ld_meta_t *m;
    if(lw_reserve_nodes(v,i+1)!=UI_STATUS_OK){v->restricted->failed=1;return -1;}
    ++v->node_count;n=&v->nodes[i];memset(n,0,sizeof(*n));
    n->used=n->attached=n->visible=1;n->uid=++v->next_uid;n->kind=1;n->parent=parent;n->first_child=n->next_sibling=-1;
    n->object=JS_UNDEFINED;for(int k=0;k<LW_EVENTS;++k)n->listeners[k]=JS_UNDEFINED;
    strcpy_s(n->tag,sizeof(n->tag),tag);m=&v->restricted->meta[i];m->dom=dom;m->link=-1;m->form=-1;
    if(parent>=0){int last=v->nodes[parent].first_child;if(last<0)v->nodes[parent].first_child=i;
        else{while(v->nodes[last].next_sibling>=0)last=v->nodes[last].next_sibling;v->nodes[last].next_sibling=i;}
m->link=v->restricted->meta[parent].link;m->form=v->restricted->meta[parent].form;}
    return i;
}
static ui_status_t ld_text(lw_view_t *v,int parent,lxb_dom_node_t *dom,const char *text,int literal)
{
    int i=ld_add(v,parent,dom,"span");lw_node_t *n;ld_meta_t *m;wchar_t *w;ui_status_t status;
    if(i<0)return UI_STATUS_LIMIT_EXCEEDED;n=&v->nodes[i];m=&v->restricted->meta[i];
    if(strlen(text)>=sizeof(n->text))return UI_STATUS_LIMIT_EXCEEDED;
    strcpy(n->text,text);
    m->literal=literal;
    w=lw_wide(n->text);if(!w)return UI_STATUS_VALIDATION_FAILED;m->wide=w;m->length=(int)wcslen(w);
    status=ld_append_text(v->restricted,m,w);return status;
}
static ui_status_t ld_resource_add(lw_view_t *v,int node,int kind,const char *url)
{
    ld_document_t *d=v->restricted;ld_resource_t *r;char *resolved=ld_resolve(d->source,url);
    if(!resolved){++d->state.restrictions;++d->state.failed_resources;if(kind==UI_DOCUMENT_IMAGE)d->meta[node].image_state=2;return UI_STATUS_OK;}
    if(d->resource_count>=LD_MAX_RESOURCES){free(resolved);return UI_STATUS_LIMIT_EXCEEDED;}
    r=&d->resources[d->resource_count++];r->id=(uint64_t)d->resource_count;r->node=node;r->kind=kind;r->url=resolved;r->sheet_order=kind==UI_DOCUMENT_STYLESHEET?++d->sheet_order:0;
    ++d->state.pending_resources;return UI_STATUS_OK;
}
static void ld_css_restriction(lw_view_t *v){++v->restricted->state.restrictions;}
static int ld_style_order(lw_view_t *v){return v->restricted->current_sheet;}
static void ld_style_defaults(lw_view_t *v,int i)
{
    lw_node_t *n=&v->nodes[i];lw_style_t *st=&n->style;
    if(i==0)st->font_family=1;
    if(v->restricted->meta[i].dom&&v->restricted->meta[i].dom->type==LXB_DOM_NODE_TYPE_ELEMENT&&lxb_dom_element_has_attribute(lxb_dom_interface_element(v->restricted->meta[i].dom),(const lxb_char_t *)"hidden",6))st->hidden=1;
    if(ld_tag(n,"pre")){st->white_space=2;st->nowrap=1;}
    if(ld_tag(n,"code")||ld_tag(n,"pre"))st->font_family=3;
    if(ld_tag(n,"strong")||ld_tag(n,"b")||ld_tag(n,"th"))st->font_weight=FW_BOLD;
    if(ld_tag(n,"em")||ld_tag(n,"i"))st->font_italic=1;
    if(n->tag[0]=='H'||n->tag[0]=='h')if(n->tag[1]>='1'&&n->tag[1]<='6'&&!n->tag[2]){st->font_size=32-(n->tag[1]-'1')*3;st->font_weight=FW_BOLD;}
    if(ld_tag(n,"a")&&v->restricted->meta[i].url)st->color=0x0067b8;
}
static ui_status_t ld_declarations(lw_view_t *v,char *out,const char *input)
{
    char copy[LW_ATTR_CAP],*at;size_t used=0;out[0]=0;
    if(strlen(input)>=sizeof(copy))return UI_STATUS_LIMIT_EXCEEDED;strcpy(copy,input);at=copy;
    while(at&&*at){char *end=strchr(at,';'),*key,*colon,*value,one[LW_ATTR_CAP],test[LW_ATTR_CAP];lw_style_t style;int allowed;
        if(end)*end++=0;key=lw_trim(at);at=end;if(!*key)continue;colon=strchr(key,':');
        if(!colon){++v->restricted->state.restrictions;continue;}*colon=0;value=lw_trim(colon+1);key=lw_trim(key);
        allowed=!strcmp(key,"color")||!strcmp(key,"background")||!strcmp(key,"background-color")||!strncmp(key,"font-",5)||!strcmp(key,"white-space")||!strcmp(key,"line-height")||!strcmp(key,"display")||!strncmp(key,"margin",6)||!strncmp(key,"padding",7)||!strncmp(key,"border",6)||!strcmp(key,"width")||!strcmp(key,"height");
        if(!strcmp(key,"visibility")&&(!strcmp(value,"hidden")||!strcmp(value,"collapse"))){key="display";value="none";allowed=1;++v->restricted->state.restrictions;}
        if(!strcmp(key,"display")&&strcmp(value,"none")&&strcmp(value,"block")&&strcmp(value,"inline")&&strcmp(value,"inline-block"))allowed=0;
        snprintf(one,sizeof(one),"%s:%s;",key,value);strcpy(test,one);
        if(!allowed||lw_parse_style(&style,test)!=UI_STATUS_OK){++v->restricted->state.restrictions;continue;}
        if(used+strlen(one)>=LW_ATTR_CAP)return UI_STATUS_LIMIT_EXCEEDED;strcpy(out+used,one);used+=strlen(one);
    }return UI_STATUS_OK;
}
static long long ld_integer(lw_view_t *v,lxb_dom_node_t *dom,const char *name,long long fallback)
{
    char *text=ld_attribute(dom,name),*end;long long value;
    if(!text)return fallback;value=_strtoi64(text,&end,10);
    if(end==text){++v->restricted->state.restrictions;value=fallback;}
    else if(value<INT_MIN||value>INT_MAX){v->restricted->failed=1;value=fallback;}
    free(text);return value;
}
static ui_status_t ld_css(lw_view_t *v,const char *css)
{
    int before=v->rule_count;ui_status_t s;
    if(strlen(css)>65536)return UI_STATUS_LIMIT_EXCEEDED;
    if(!v->restricted->current_sheet)v->restricted->current_sheet=++v->restricted->sheet_order;
    s=lw_stylesheet(v,css);v->restricted->current_sheet=0;
    if(s!=UI_STATUS_OK){v->rule_count=before;++v->restricted->state.restrictions;
        return s==UI_STATUS_OUT_OF_MEMORY||s==UI_STATUS_LIMIT_EXCEEDED?s:UI_STATUS_OK;}
    /* Light document flow intentionally ignores CSS positioning and dimensions. */
    if(strstr(css,"position")||strstr(css,"flex")||strstr(css,"overflow"))++v->restricted->state.restrictions;
    return UI_STATUS_OK;
}
static ui_status_t ld_collect(lw_view_t *v,lxb_dom_node_t *parent,int pi,int depth)
{
    lxb_dom_node_t *dom;ld_document_t *d=v->restricted;
    if(depth>64)return UI_STATUS_LIMIT_EXCEEDED;
    for(dom=lxb_dom_node_first_child(parent);dom;dom=lxb_dom_node_next(dom)){
        size_t length;const lxb_char_t *tag;int i;lw_node_t *n;ld_meta_t *m;ui_status_t s=UI_STATUS_OK;
        if(dom->type==LXB_DOM_NODE_TYPE_TEXT){lxb_char_t *text=lxb_dom_node_text_content(dom,&length);
            if(!text)return UI_STATUS_OUT_OF_MEMORY;s=ld_text(v,pi,dom,(char *)text,0);lxb_dom_document_destroy_text(dom->owner_document,text);
            if(s!=UI_STATUS_OK)return s;continue;}
        if(dom->type!=LXB_DOM_NODE_TYPE_ELEMENT)continue;
        tag=lxb_dom_node_name(dom,&length);if(length>=24)return UI_STATUS_UNSUPPORTED;
        {char t[24];memcpy(t,tag,length);t[length]=0;
            if(!_stricmp(t,"script")||!_stricmp(t,"style")||!_stricmp(t,"link")||!_stricmp(t,"meta")){
                if(!_stricmp(t,"style")){lxb_char_t *text=lxb_dom_node_text_content(dom,&length);if(!text)return UI_STATUS_OUT_OF_MEMORY;s=ld_css(v,(const char *)text);lxb_dom_document_destroy_text(dom->owner_document,text);if(s!=UI_STATUS_OK)return s;}
                else if(!_stricmp(t,"link")){char *rel=ld_attribute(dom,"rel"),*href=ld_attribute(dom,"href");if(rel&&href&&!_stricmp(rel,"stylesheet"))s=ld_resource_add(v,0,UI_DOCUMENT_STYLESHEET,href);else ++d->state.restrictions;free(rel);free(href);if(s!=UI_STATUS_OK)return s;}
                else if(!_stricmp(t,"script"))++d->state.restrictions;
                else{char *equiv=ld_attribute(dom,"http-equiv");if(equiv)++d->state.restrictions;free(equiv);}
                continue;}
            i=ld_add(v,pi,dom,t);}
        if(i<0)return UI_STATUS_LIMIT_EXCEEDED;n=&v->nodes[i];m=&d->meta[i];
        if(!ld_supported(n))++d->state.restrictions;
        if(lw_attr(lxb_dom_interface_element(dom),"id",n->id,sizeof(n->id))!=UI_STATUS_OK||
           lw_attr(lxb_dom_interface_element(dom),"class",n->classes,sizeof(n->classes))!=UI_STATUS_OK)return UI_STATUS_LIMIT_EXCEEDED;
        {lxb_dom_attr_t *a;for(a=lxb_dom_element_first_attribute(lxb_dom_interface_element(dom));a;a=lxb_dom_element_next_attribute(a)){
            const lxb_char_t *name=lxb_dom_attr_qualified_name(a,&length);size_t value_length;
            (void)lxb_dom_attr_value(a,&value_length);if(value_length>=LW_ATTR_CAP)return UI_STATUS_LIMIT_EXCEEDED;
            if(length>=2&&!_strnicmp((const char *)name,"on",2))++d->state.restrictions;}}
        m->block=ld_tag(n,"p")||ld_tag(n,"div")||ld_tag(n,"section")||ld_tag(n,"article")||ld_tag(n,"main")||ld_tag(n,"header")||ld_tag(n,"footer")||ld_tag(n,"blockquote")||ld_tag(n,"ul")||ld_tag(n,"ol")||ld_tag(n,"li")||ld_tag(n,"pre")||ld_tag(n,"form")||ld_tag(n,"table")||ld_tag(n,"tr")||ld_tag(n,"hr")||ld_tag(n,"caption")||(toupper((unsigned char)n->tag[0])=='H'&&n->tag[1]>='1'&&n->tag[1]<='6'&&!n->tag[2]);
        {char *style=ld_attribute(dom,"style");if(style){s=ld_declarations(v,n->inline_style,style);free(style);if(s!=UI_STATUS_OK)return s;}}
        if(ld_tag(n,"a")){char *href=ld_attribute(dom,"href");if(href){m->url=ld_resolve(d->source,href);free(href);
            if(m->url){m->link=i;}else{++d->state.restrictions;s=ld_text(v,i,NULL,"[blocked link] ",1);}}}
        n=&v->nodes[i];
        if(ld_tag(n,"form")){char *action=ld_attribute(dom,"action"),*method=ld_attribute(dom,"method"),*enctype=ld_attribute(dom,"enctype");
            m->form=i;m->url=ld_resolve(d->source,action&&*action?action:d->document_url);
            if((method&&_stricmp(method,"get"))||(enctype&&_stricmp(enctype,"application/x-www-form-urlencoded"))||!m->url){n->disabled=1;++d->state.restrictions;s=ld_text(v,i,NULL,"[form refused: only GET is supported] ",1);}
            free(action);free(method);free(enctype);}
        n=&v->nodes[i];
        if(ld_tag(n,"input")||ld_tag(n,"textarea")){char *type=ld_attribute(dom,"type");m->name=ld_attribute(dom,"name");
            n->disabled=lxb_dom_element_has_attribute(lxb_dom_interface_element(dom),(const lxb_char_t *)"disabled",8);
            n->readonly=lxb_dom_element_has_attribute(lxb_dom_interface_element(dom),(const lxb_char_t *)"readonly",8);
            if(!type||!_stricmp(type,"text")||!_stricmp(type,"search")||!_stricmp(type,"hidden")){n->kind=3;
                if(type&&!_stricmp(type,"hidden"))n->visible=0;
                if(lw_attr(lxb_dom_interface_element(dom),"value",n->value,sizeof(n->value))!=UI_STATUS_OK)s=UI_STATUS_LIMIT_EXCEEDED;
            }else if(!_stricmp(type,"submit")){n->kind=2;if(lw_attr(lxb_dom_interface_element(dom),"value",n->value,sizeof(n->value))!=UI_STATUS_OK)s=UI_STATUS_LIMIT_EXCEEDED;if(lw_attr(lxb_dom_interface_element(dom),"value",n->text,sizeof(n->text))!=UI_STATUS_OK)s=UI_STATUS_LIMIT_EXCEEDED;if(!*n->text)strcpy(n->text,"Submit");}
            else{n->disabled=1;if(m->form>=0)v->nodes[m->form].disabled=1;++d->state.restrictions;s=ld_text(v,i,NULL,"[form refused: unsupported input]",1);}
            if(ld_tag(n,"textarea")){++d->state.restrictions;n->kind=1;n->disabled=1;if(m->form>=0)v->nodes[m->form].disabled=1;s=ld_text(v,i,NULL,"[textarea form control refused]",1);}free(type);}
        n=&v->nodes[i];
        if(ld_tag(n,"button")){char *type=ld_attribute(dom,"type");n->kind=2;n->disabled=lxb_dom_element_has_attribute(lxb_dom_interface_element(dom),(const lxb_char_t *)"disabled",8);m->name=ld_attribute(dom,"name");
            if(lw_attr(lxb_dom_interface_element(dom),"value",n->value,sizeof(n->value))!=UI_STATUS_OK)s=UI_STATUS_LIMIT_EXCEEDED;
            if(m->form<0){n->disabled=1;++d->state.restrictions;}
            if(type&&_stricmp(type,"submit")){n->disabled=1;++d->state.restrictions;}free(type);
            {lxb_char_t *text=lxb_dom_node_text_content(dom,&length);if(!text)return UI_STATUS_OUT_OF_MEMORY;s=lw_copy(n->text,sizeof(n->text),text,length);lxb_dom_document_destroy_text(dom->owner_document,text);}}
        if((ld_tag(n,"input")||ld_tag(n,"button"))&&m->form>=0){char *override=ld_attribute(dom,"formaction"),*method=ld_attribute(dom,"formmethod"),*owner=ld_attribute(dom,"form");
            if(override||method||owner){v->nodes[m->form].disabled=1;++d->state.restrictions;s=ld_text(v,m->form,NULL,"[form refused: submit overrides/association]",1);}free(override);free(method);free(owner);}
        n=&v->nodes[i];
        if(ld_tag(n,"img")){m->image_width=(int)ld_integer(v,dom,"width",0);m->image_height=(int)ld_integer(v,dom,"height",0);if(m->image_width<0||m->image_height<0){++d->state.restrictions;m->image_width=m->image_height=0;}if(m->image_width>LW_MAX_DIMENSION||m->image_height>LW_MAX_DIMENSION)d->failed=1;char *src=ld_attribute(dom,"src"),*alt=ld_attribute(dom,"alt");n->kind=5;
            if(alt)strcpy_s(n->text,sizeof(n->text),alt);else strcpy(n->text,"image");
            if(src){m->image_state=1;s=ld_resource_add(v,i,UI_DOCUMENT_IMAGE,src);}else ++d->state.restrictions;
            free(src);free(alt);}
        if(ld_tag(n,"ol")){char *kind=ld_attribute(dom,"type");if(kind&&strcmp(kind,"1"))++d->state.restrictions;free(kind);int count=0;lxb_dom_node_t *child;int reversed=lxb_dom_element_has_attribute(lxb_dom_interface_element(dom),(const lxb_char_t *)"reversed",8);m->list_step=reversed?-1:1;
            for(child=lxb_dom_node_first_child(dom);child;child=lxb_dom_node_next(child))if(child->type==LXB_DOM_NODE_TYPE_ELEMENT){size_t len;const lxb_char_t *tagname=lxb_dom_node_name(child,&len);if(lw_name((const char *)tagname,len,"li"))++count;}
            m->list_next=ld_integer(v,dom,"start",reversed?count:1);}
        if(ld_tag(n,"li")){char marker[48]="• ";if(pi>=0&&ld_tag(&v->nodes[pi],"ol")){ld_meta_t *list=&d->meta[pi];long long number=ld_integer(v,dom,"value",list->list_next);list->list_next=number+list->list_step;snprintf(marker,sizeof(marker),"%lld. ",number);}s=ld_text(v,i,NULL,marker,1);}
        n=&v->nodes[i];
        if(ld_tag(n,"iframe")||ld_tag(n,"object")||ld_tag(n,"embed")||ld_tag(n,"canvas")||ld_tag(n,"svg")||ld_tag(n,"audio")||ld_tag(n,"video")||ld_tag(n,"select")){
            char label[80];if(ld_tag(n,"select")&&m->form>=0)v->nodes[m->form].disabled=1;++d->state.restrictions;snprintf(label,sizeof(label),"[unsupported content: %s]",n->tag);s=ld_text(v,i,NULL,label,1);m->block=1;
        }else if(n->kind!=3&&n->kind!=5&&!ld_tag(n,"button"))s=ld_collect(v,dom,i,depth+1);
        if(s!=UI_STATUS_OK)return s;
        n=&v->nodes[i];
        m->default_block=m->block;
        if(m->block){ld_meta_t dummy={0};s=ld_append_text(d,&dummy,L"\n");if(s!=UI_STATUS_OK)return s;}
        if(ld_tag(n,"br")){ld_meta_t dummy={0};s=ld_append_text(d,&dummy,L"\n");if(s!=UI_STATUS_OK)return s;}

    }
    return UI_STATUS_OK;
}
static ui_status_t ld_head(lw_view_t *v,lxb_dom_node_t *parent,int depth)
{
    lxb_dom_node_t *c;ld_document_t *d=v->restricted;
    if(depth>64)return UI_STATUS_LIMIT_EXCEEDED;
    for(c=lxb_dom_node_first_child(parent);c;c=lxb_dom_node_next(c))if(c->type==LXB_DOM_NODE_TYPE_ELEMENT){
        size_t length;const lxb_char_t *name=lxb_dom_node_name(c,&length);ui_status_t s=UI_STATUS_OK;
        if(lw_name((const char *)name,length,"title")||lw_name((const char *)name,length,"style")){
            lxb_char_t *text=lxb_dom_node_text_content(c,&length);if(!text)return UI_STATUS_OUT_OF_MEMORY;
            if(lw_name((const char *)name,strlen("title"),"title"))s=lw_copy(d->state.title_utf8,sizeof(d->state.title_utf8),text,length);
            else s=ld_css(v,(const char *)text);lxb_dom_document_destroy_text(c->owner_document,text);
        }else if(lw_name((const char *)name,length,"link")){char *rel=ld_attribute(c,"rel"),*href=ld_attribute(c,"href");
            if(rel&&href&&!_stricmp(rel,"stylesheet"))s=ld_resource_add(v,0,UI_DOCUMENT_STYLESHEET,href);else ++d->state.restrictions;free(rel);free(href);
        }else if(lw_name((const char *)name,length,"meta")){char *refresh=ld_attribute(c,"http-equiv");if(refresh)++d->state.restrictions;free(refresh);}
        else if(lw_name((const char *)name,length,"base")){char *href=ld_attribute(c,"href"),*resolved=href?ld_resolve(d->source,href):NULL;
            if(!d->has_base&&resolved){strcpy(d->source,resolved);d->has_base=1;}else ++d->state.restrictions;free(href);free(resolved);}
        else if(lw_name((const char *)name,length,"script"))++d->state.restrictions;
        else s=ld_head(v,c,depth+1);
        if(s!=UI_STATUS_OK)return s;
    }return UI_STATUS_OK;
}
static ui_status_t ld_validate_tree(lxb_dom_node_t *parent,int depth,int *count)
{
    lxb_dom_node_t *child;if(depth>64)return UI_STATUS_LIMIT_EXCEEDED;
    for(child=lxb_dom_node_first_child(parent);child;child=lxb_dom_node_next(child)){
        if(++*count>LW_MAX_NODES)return UI_STATUS_LIMIT_EXCEEDED;
        if(ld_validate_tree(child,depth+1,count)!=UI_STATUS_OK)return UI_STATUS_LIMIT_EXCEEDED;
    }return UI_STATUS_OK;
}
static void ld_text_tree(lw_view_t *v,int i)
{
    ld_document_t *d=v->restricted;ld_meta_t *m=&d->meta[i];lw_node_t *n=&v->nodes[i];ld_meta_t dummy={0};int c;
    m->offset=-1;if(!n->visible)return;
    if(m->block&&d->text_length&&d->text[d->text_length-1]!=L'\n')if(ld_append_text(d,&dummy,L"\n")!=UI_STATUS_OK)d->failed=1;
    if((m->wide&&n->kind!=5)||(n->kind==5&&!n->image)){
        char label[LW_TEXT_CAP];const char *raw=n->text;wchar_t *wide;int mode=n->style.white_space;
        if(n->kind==5){snprintf(label,sizeof(label),"[%s] %.4000s",m->image_state==1?"image pending":m->image_state==2?"image failed":"image unavailable",n->text);raw=label;}
        wide=lw_wide(raw);if(!wide){d->failed=1;return;}
        if(!m->literal&&mode!=2&&mode!=3){wchar_t *a=wide,*b=wide;int space=d->text_length&&iswspace(d->text[d->text_length-1]);
            while(*a){if(*a==L' '||*a==L'\t'||*a==L'\r'||*a==L'\n'||*a==L'\f'){if(mode==4&&*a==L'\n'){*b++=L'\n';space=1;}else if(!space){*b++=L' ';space=1;}}else{*b++=*a;space=0;}++a;}*b=0;}
        free(m->wide);m->wide=wide;m->length=(int)wcslen(wide);if(ld_append_text(d,m,wide)!=UI_STATUS_OK)d->failed=1;
    }
    if(n->kind==5&&n->image){free(m->wide);m->wide=NULL;m->length=0;}
    for(c=n->first_child;c>=0;c=v->nodes[c].next_sibling)ld_text_tree(v,c);
    if(ld_tag(n,"td")||ld_tag(n,"th")){if(ld_append_text(d,&dummy,L"\t")!=UI_STATUS_OK)d->failed=1;}
    if(m->block||ld_tag(n,"br"))if(ld_append_text(d,&dummy,L"\n")!=UI_STATUS_OK)d->failed=1;
}
static void ld_line(ld_flow_t *f)
{if(f->x>f->maximum)f->maximum=f->x;f->y+=f->height?f->height:22;f->x=f->left;f->height=0;}
static void ld_fragment(lw_view_t *v,int i,int start,int count,int x,int y,int width,int height)
{
    ld_document_t *d=v->restricted;ld_fragment_t *p;
    if(d->fragment_count==LD_MAX_FRAGMENTS){d->failed=1;return;}
    if(d->fragment_count==d->fragment_capacity){int capacity=d->fragment_capacity?d->fragment_capacity*2:64;
        p=(ld_fragment_t *)realloc(d->fragments,(size_t)capacity*sizeof(*p));if(!p){d->failed=1;return;}d->fragments=p;d->fragment_capacity=capacity;}
    p=&d->fragments[d->fragment_count++];p->node=i;p->start=start;p->count=count;p->rect=(ui_rect_t){x,y,width,height};
}
static int ld_measure(lw_view_t *v,int i,HDC dc)
{
    ld_meta_t *m=&v->restricted->meta[i];lw_node_t *n=&v->nodes[i];SIZE size={0};int child,width=0;
    if(!n->visible)return 0;
    if(m->wide){HGDIOBJ old=SelectObject(dc,n->font);GetTextExtentPoint32W(dc,m->wide,m->length,&size);SelectObject(dc,old);return MulDiv(size.cx,96,(int)v->dpi);}
    for(child=n->first_child;child>=0;child=v->nodes[child].next_sibling){int w=ld_measure(v,child,dc);if(v->restricted->meta[child].block){if(w>width)width=w;}else if(width<LW_MAX_DIMENSION)width+=w;}
    return width;
}
static void ld_flow_node(lw_view_t *,int,ld_flow_t *);
typedef struct ld_cell {int node,row,col,rows,cols,height,first,last;} ld_cell_t;
static void ld_table_rows(lw_view_t *v,int node,int *rows,int *count)
{
    int c;for(c=v->nodes[node].first_child;c>=0;c=v->nodes[c].next_sibling){
        if(!v->nodes[c].visible||ld_tag(&v->nodes[c],"table"))continue;
        if(ld_tag(&v->nodes[c],"tr"))rows[(*count)++]=c;else if(ld_tag(&v->nodes[c],"thead")||ld_tag(&v->nodes[c],"tbody")||ld_tag(&v->nodes[c],"tfoot"))ld_table_rows(v,c,rows,count);
    }
}
static void ld_shift(lw_view_t *v,int i,int dy)
{v->nodes[i].rect.y+=dy;for(int c=v->nodes[i].first_child;c>=0;c=v->nodes[c].next_sibling)ld_shift(v,c,dy);}
static void ld_table(lw_view_t *v,int node,ld_flow_t *f)
{
    ld_document_t *d=v->restricted;int count=0,used=0,cols=0,widths[64]={0},occupied[64]={0};int *rows=NULL,*heights=NULL,*ys=NULL;ld_cell_t *cells=NULL;int top=f->y;
    rows=calloc((size_t)v->node_count,sizeof(int));heights=calloc((size_t)v->node_count,sizeof(int));ys=calloc((size_t)v->node_count+1,sizeof(int));cells=calloc((size_t)v->node_count,sizeof(*cells));
    if(!rows||!heights||!ys||!cells){d->failed=1;goto done;}
    for(int c=v->nodes[node].first_child;c>=0;c=v->nodes[c].next_sibling)if(ld_tag(&v->nodes[c],"caption"))ld_flow_node(v,c,f);
    ld_table_rows(v,node,rows,&count);if(!count)goto done;
    for(int r=0;r<count;++r){int col=0,group_end=r+1;heights[r]=24;
        while(group_end<count&&v->nodes[rows[group_end]].parent==v->nodes[rows[r]].parent)++group_end;
        for(int c=v->nodes[rows[r]].first_child;c>=0;c=v->nodes[c].next_sibling)if(v->nodes[c].visible&&(ld_tag(&v->nodes[c],"td")||ld_tag(&v->nodes[c],"th"))){
            long long span=ld_integer(v,d->meta[c].dom,"colspan",1),down=ld_integer(v,d->meta[c].dom,"rowspan",1);int available;
            if(span<1)span=1;if(down<0)down=1;if(down==0||down>group_end-r)down=group_end-r;
            if(span>64){d->failed=1;goto done;}
            for(;;){if(col+(int)span>64){d->failed=1;goto done;}available=1;for(int k=col;k<col+(int)span;++k)if(occupied[k]>r){available=0;col=k+1;break;}if(available)break;}
            cells[used++]=(ld_cell_t){c,r,col,(int)down,(int)span,0,0,0};for(int k=col;k<col+(int)span;++k)occupied[k]=r+(int)down;col+=(int)span;if(col>cols)cols=col;
        }
    }
    for(int c=0;c<cols;++c)widths[c]=100;
    for(int j=0;j<used;++j){ld_cell_t *cell=&cells[j];int wanted=ld_measure(v,cell->node,f->dc)+16,have=0;
        if(v->nodes[cell->node].style.white_space!=1&&v->nodes[cell->node].style.white_space!=2&&wanted>480*cell->cols)wanted=480*cell->cols;
        for(int k=0;k<cell->cols;++k)have+=widths[cell->col+k];if(wanted>have){int add=(wanted-have+cell->cols-1)/cell->cols;for(int k=0;k<cell->cols;++k)widths[cell->col+k]+=add;}}
    for(int j=0;j<used;++j){ld_cell_t *cell=&cells[j];int x=f->left,w=0;for(int k=0;k<cell->col;++k)x+=widths[k];for(int k=0;k<cell->cols;++k)w+=widths[cell->col+k];
        ld_flow_t inner={x+6,x+w-6,x+6,6,0,0,f->dc};cell->first=d->fragment_count;ld_flow_node(v,cell->node,&inner);if(inner.height)ld_line(&inner);cell->last=d->fragment_count;cell->height=inner.y+6;if(cell->height<24)cell->height=24;
        if(inner.maximum>f->maximum)f->maximum=inner.maximum;if(cell->rows==1&&cell->height>heights[cell->row])heights[cell->row]=cell->height;}
    for(int j=0;j<used;++j){ld_cell_t *cell=&cells[j];int h=0;for(int k=0;k<cell->rows;++k)h+=heights[cell->row+k];if(h<cell->height)heights[cell->row+cell->rows-1]+=cell->height-h;}
    ys[0]=f->y;for(int r=0;r<count;++r)ys[r+1]=ys[r]+heights[r];
    for(int j=0;j<used;++j){ld_cell_t *cell=&cells[j];int x=f->left,w=0;for(int k=0;k<cell->col;++k)x+=widths[k];for(int k=0;k<cell->cols;++k)w+=widths[cell->col+k];
        ld_shift(v,cell->node,ys[cell->row]);for(int k=cell->first;k<cell->last;++k)d->fragments[k].rect.y+=ys[cell->row];v->nodes[cell->node].rect=(ui_rect_t){x,ys[cell->row],w,ys[cell->row+cell->rows]-ys[cell->row]};}
    {int w=0;for(int k=0;k<cols;++k)w+=widths[k];for(int r=0;r<count;++r)v->nodes[rows[r]].rect=(ui_rect_t){f->left,ys[r],w,heights[r]};if(f->left+w>f->maximum)f->maximum=f->left+w;v->nodes[node].rect=(ui_rect_t){f->left,top,w,ys[count]-top};}
    f->y=ys[count];f->x=f->left;
done:free(rows);free(heights);free(ys);free(cells);
}
static void ld_flow_node(lw_view_t *v,int i,ld_flow_t *f)
{
    lw_node_t *n=&v->nodes[i];ld_meta_t *m=&v->restricted->meta[i];int c,start_y,start_x,first_fragment=v->restricted->fragment_count,lineheight=n->style.line_height?MulDiv(n->style.line_height,(int)v->restricted->state.zoom_percent,100):n->style.font_size+7,old_left=f->left,old_right=f->right;
    if(!n->visible)return;
    {TEXTMETRICW metrics;HGDIOBJ old=SelectObject(f->dc,n->font);if(GetTextMetricsW(f->dc,&metrics)){int minimum=MulDiv(metrics.tmHeight,96,(int)v->dpi);if(lineheight<minimum){lineheight=minimum;if(!m->line_clamped){m->line_clamped=1;++v->restricted->state.restrictions;}}}SelectObject(f->dc,old);}
    if(m->block){if(f->height)ld_line(f);f->y+=(n->style.margin_set&1)?n->style.margin[0]:6;f->left+=n->style.margin[3];f->right-=n->style.margin[1];f->x=f->left;}
    start_y=f->y;start_x=f->x;
    if(m->block){f->y+=n->style.padding[0];f->left+=n->style.padding[3];f->right-=n->style.padding[1];f->x=f->left;if(f->right<=f->left)f->right=f->left+1;}
    if(ld_tag(n,"br")){ld_line(f);n->rect=(ui_rect_t){f->x,f->y,1,lineheight};return;}
    if(ld_tag(n,"table")){ld_table(v,i,f);}
    else if(m->wide){
        int at=0,nowrap=n->style.white_space==1||n->style.white_space==2;HGDIOBJ old=SelectObject(f->dc,n->font);
        while(at<m->length){int available=f->right-f->x,fit=0,end=m->length-at,segment;SIZE size={0};
            if(m->wide[at]==L'\n'){ld_line(f);++at;continue;}
            if(m->wide[at]==L'\t'){SIZE tab={0};GetTextExtentPoint32W(f->dc,L"    ",4,&tab);int step=MulDiv(tab.cx,96,(int)v->dpi);if(step<1)step=4;f->x=f->left+((f->x-f->left)/step+1)*step;++at;continue;}
            for(segment=0;segment<end&&m->wide[at+segment]!=L'\n'&&m->wide[at+segment]!=L'\t';++segment){}
            if(!nowrap){if(n->style.white_space!=3&&f->x==f->left&&m->wide[at]==L' '){++at;continue;}
                GetTextExtentExPointW(f->dc,m->wide+at,segment,lw_pixel(available>0?available:1,v->dpi),&fit,NULL,&size);
                if(fit<1&&f->height){ld_line(f);continue;}if(fit<1)fit=1;
                if(fit<segment&&fit>1){int split=fit;while(split>0&&m->wide[at+split-1]!=L' '&&m->wide[at+split-1]<0x2e80)--split;
                    if(split>0)fit=split;}
                if(fit<segment){SCRIPT_STRING_ANALYSIS shaped=lw_shape(f->dc,m->wide+at,segment);const SCRIPT_LOGATTR *attrs=shaped?ScriptString_pLogAttr(shaped):NULL;while(fit<segment&&((attrs&&!attrs[fit].fCharStop)||(!attrs&&m->wide[at+fit]>=0xdc00&&m->wide[at+fit]<=0xdfff)))++fit;if(shaped)ScriptStringFree(&shaped);}
            }else fit=segment;
            GetTextExtentPoint32W(f->dc,m->wide+at,fit,&size);
            {int width=(size.cx*96+(int)v->dpi-1)/(int)v->dpi;
             ld_fragment(v,i,at,fit,f->x,f->y,width,lineheight);f->x+=width;if(lineheight>f->height)f->height=lineheight;}
            at+=fit;if(!nowrap&&at<m->length)ld_line(f);
        }SelectObject(f->dc,old);
    }else if(n->kind==3||n->kind==2||n->kind==5){
        int w=220,h=32;if(n->kind==2){SIZE size={0};wchar_t *wide=lw_wide(n->text);HGDIOBJ old=SelectObject(f->dc,n->font);if(wide){GetTextExtentPoint32W(f->dc,wide,(int)wcslen(wide),&size);free(wide);}SelectObject(f->dc,old);w=MulDiv(size.cx,96,(int)v->dpi)+24;}
        if(n->kind==5){ui_image_info_t info={0};info.size=sizeof(info);w=180;h=40;
            if(n->image&&ui_image_get_info(v->host,n->image,&info)==UI_STATUS_OK){int iw=n->style.width>=0?(n->style.width_percent?MulDiv(f->right-f->left,n->style.width,100):n->style.width):m->image_width,ih=n->style.height>=0?(n->style.height_percent?MulDiv(v->height,n->style.height,100):n->style.height):m->image_height;
                if(iw<=0&&ih<=0){iw=(int)info.width;ih=(int)info.height;}else if(iw<=0)iw=MulDiv(ih,(int)info.width,(int)info.height);else if(ih<=0)ih=MulDiv(iw,(int)info.height,(int)info.width);
                w=MulDiv(iw,(int)v->restricted->state.zoom_percent,100);h=MulDiv(ih,(int)v->restricted->state.zoom_percent,100);
                if(w>f->right-f->left){h=MulDiv(h,f->right-f->left,w);w=f->right-f->left;}}}
        if(w>f->right-f->left&&n->kind!=5)w=f->right-f->left;
        if(f->x+w>f->right&&f->height)ld_line(f);
        n->rect=(ui_rect_t){f->x,f->y,w,h};n->viewport=n->rect;f->x+=w+6;if(h>f->height)f->height=h;
    }else{
        int indent=ld_tag(n,"ul")||ld_tag(n,"ol")||ld_tag(n,"blockquote")?20:0;
        f->left+=indent;f->x+=indent;
        for(c=n->first_child;c>=0;c=v->nodes[c].next_sibling)ld_flow_node(v,c,f);
        f->left-=indent;
    }
    if(m->block){if(f->height)ld_line(f);f->y+=n->style.padding[2];}
    if(n->kind==1&&!ld_tag(n,"table"))n->rect=(ui_rect_t){start_x,start_y,f->right-start_x,f->y-start_y+(f->height?f->height:lineheight)};
    if((n->kind==1||m->wide)&&!m->block){int initialized=0;ui_rect_t bounds={0};
        if(m->wide){for(int j=first_fragment;j<v->restricted->fragment_count;++j){ui_rect_t r=v->restricted->fragments[j].rect;if(!initialized){bounds=r;initialized=1;}else{int right=bounds.x+bounds.width,bottom=bounds.y+bounds.height;if(r.x<bounds.x)bounds.x=r.x;if(r.y<bounds.y)bounds.y=r.y;if(r.x+r.width>right)right=r.x+r.width;if(r.y+r.height>bottom)bottom=r.y+r.height;bounds.width=right-bounds.x;bounds.height=bottom-bounds.y;}}}
        else for(c=n->first_child;c>=0;c=v->nodes[c].next_sibling){ui_rect_t r=v->nodes[c].rect;if(!initialized){bounds=r;initialized=1;}else{int right=bounds.x+bounds.width,bottom=bounds.y+bounds.height;if(r.x<bounds.x)bounds.x=r.x;if(r.y<bounds.y)bounds.y=r.y;if(r.x+r.width>right)right=r.x+r.width;if(r.y+r.height>bottom)bottom=r.y+r.height;bounds.width=right-bounds.x;bounds.height=bottom-bounds.y;}}
        if(initialized)n->rect=bounds;}
    if(m->block){f->y+=(n->style.margin_set&4)?n->style.margin[2]:6;f->left=old_left;f->right=old_right;f->x=f->left;}
    if((ld_tag(n,"td")||ld_tag(n,"th"))&&!n->style.border)n->style.border=1;
}
static void ld_layout(lw_view_t *v)
{
    ld_document_t *d=v->restricted;LARGE_INTEGER a,b;HDC dc;ld_flow_t flow;int i,ox,oy;ui_rect_t clip;
    if(!d||v->root<0)return;QueryPerformanceCounter(&a);
    lw_compute_style(v,0);
    for(i=0;i<v->node_count;++i){lw_node_t *n=&v->nodes[i];ld_meta_t *m=&d->meta[i];int size=MulDiv(n->style.font_size,(int)d->state.zoom_percent,100);
        if(n->font)DeleteObject(n->font);n->style.font_size=size;
        n->font=CreateFontW(-lw_pixel(size,v->dpi),0,0,0,n->style.font_weight,n->style.font_italic,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,n->style.font_family==3?L"Consolas":n->style.font_family==4?L"Times New Roman":n->style.font_family==0?L"Segoe UI":L"Microsoft YaHei UI");
        if(ld_tag(n,"input")){char *type=ld_attribute(m->dom,"type");if(type&&!_stricmp(type,"hidden"))n->visible=0;free(type);}
        m->block=n->style.display_kind==1?1:n->style.display_kind==2?0:m->default_block;n->bar_x=n->bar_y=0;
    }
    d->text_length=0;ld_text_tree(v,0);{uint64_t hash=UINT64_C(1469598103934665603);for(int at=0;at<d->text_length;++at){hash^=(uint16_t)d->text[at];hash*=UINT64_C(1099511628211);}if(d->text_hash&&hash!=d->text_hash)d->anchor=d->caret=0;d->text_hash=hash;}if(d->anchor>d->text_length)d->anchor=d->text_length;if(d->caret>d->text_length)d->caret=d->text_length;
    d->fragment_count=0;dc=GetDC(v->hwnd);flow=(ld_flow_t){12+v->nodes[0].style.padding[3]+v->nodes[0].style.margin[3],v->width>36?v->width-24-v->nodes[0].style.padding[1]-v->nodes[0].style.margin[1]:12,12+v->nodes[0].style.padding[3]+v->nodes[0].style.margin[3],12+v->nodes[0].style.padding[0]+v->nodes[0].style.margin[0],0,0,dc};if(flow.right<=flow.left)flow.right=flow.left+1;
    for(i=v->nodes[0].first_child;i>=0;i=v->nodes[i].next_sibling)ld_flow_node(v,i,&flow);
    if(flow.height)ld_line(&flow);ReleaseDC(v->hwnd,dc);if(flow.y>LW_MAX_DIMENSION||flow.maximum>LW_MAX_DIMENSION)d->failed=1;
    {lw_node_t *root=&v->nodes[0];root->content_height=flow.y+12;root->content_width=flow.maximum+12;
     root->rect=(ui_rect_t){0,0,v->width,v->height};root->clip=root->rect;root->viewport=(ui_rect_t){0,0,v->width>12?v->width-12:0,v->height>12?v->height-12:0};
     root->bar_y=root->content_height>root->viewport.height;root->bar_x=root->content_width>root->viewport.width;
     if(root->content_height<root->viewport.height)root->content_height=root->viewport.height;if(root->content_width<root->viewport.width)root->content_width=root->viewport.width;
     if(root->scroll_x>root->content_width-root->viewport.width)root->scroll_x=root->content_width-root->viewport.width;
     if(root->scroll_y>root->content_height-root->viewport.height)root->scroll_y=root->content_height-root->viewport.height;
     if(root->scroll_x<0)root->scroll_x=0;if(root->scroll_y<0)root->scroll_y=0;ox=root->scroll_x;oy=root->scroll_y;clip=root->viewport;
     d->state.scroll_x=ox;d->state.scroll_y=oy;d->state.content_width=root->content_width;d->state.content_height=root->content_height;}
    for(i=1;i<v->node_count;++i){lw_node_t *n=&v->nodes[i];n->rect.x-=ox;n->rect.y-=oy;n->viewport=n->rect;n->clip=lw_intersect(n->rect,clip);
        n->style.outline=v->focused==i&&v->keyboard_focus?1:0;n->style.outline_color=0x0067b8;n->style.outline_offset=-1;}
    for(i=0;i<d->fragment_count;++i){d->fragments[i].rect.x-=ox;d->fragments[i].rect.y-=oy;}
    d->no_content=d->fragment_count==0;
    if(d->no_content)for(i=1;i<v->node_count;++i)if(v->nodes[i].visible&&v->nodes[i].kind>=2){d->no_content=0;break;}
    QueryPerformanceCounter(&b);d->state.layout_ms=(double)(b.QuadPart-a.QuadPart)*1000.0/d->frequency.QuadPart;
    d->state.node_count=(uint32_t)v->node_count;v->dirty=0;
}
static int ld_paint_text(lw_view_t *v,int node,HDC dc)
{
    ld_document_t *d=v->restricted;ld_meta_t *m=&d->meta[node];lw_node_t *n=&v->nodes[node];int i;
    if(!m->wide)return 0;
    SelectObject(dc,n->font);SetTextColor(dc,lw_rgb(n->style.color));
    for(i=0;i<d->fragment_count;++i){ld_fragment_t *f=&d->fragments[i];SCRIPT_STRING_ANALYSIS a;int first,last,lo=d->anchor<d->caret?d->anchor:d->caret,hi=d->anchor>d->caret?d->anchor:d->caret;
        if(f->node!=node)continue;a=lw_shape(dc,m->wide+f->start,f->count);if(!a)continue;
        first=lo-m->offset-f->start;last=hi-m->offset-f->start;if(first<0)first=0;if(last<0)last=0;if(first>f->count)first=f->count;if(last>f->count)last=f->count;
        ScriptStringOut(a,lw_pixel(f->rect.x,v->dpi),lw_pixel(f->rect.y,v->dpi),0,NULL,first,last,FALSE);ScriptStringFree(&a);
        if(m->link>=0){int y=lw_pixel(f->rect.y+f->rect.height-4,v->dpi);HPEN pen=CreatePen(PS_SOLID,1,lw_rgb(n->style.color));HGDIOBJ old=SelectObject(dc,pen);MoveToEx(dc,lw_pixel(f->rect.x,v->dpi),y,NULL);LineTo(dc,lw_pixel(f->rect.x+f->rect.width,v->dpi),y);SelectObject(dc,old);DeleteObject(pen);}
    }return 1;
}
static int ld_position(lw_view_t *v,int x,int y,int *link)
{
    ld_document_t *d=v->restricted;int i,best=-1,dist=INT_MAX,result=0;*link=-1;
    for(i=0;i<d->fragment_count;++i){ld_fragment_t *f=&d->fragments[i];int dy=y<f->rect.y?f->rect.y-y:y>f->rect.y+f->rect.height?y-f->rect.y-f->rect.height:0;
        int dx=x<f->rect.x?f->rect.x-x:x>f->rect.x+f->rect.width?x-f->rect.x-f->rect.width:0;int metric=dy*1000+dx;
        if(metric<dist){dist=metric;best=i;if(!metric)break;}}
    if(best>=0){ld_fragment_t *f=&d->fragments[best];ld_meta_t *m=&d->meta[f->node];lw_node_t *n=&v->nodes[f->node];HDC dc=GetDC(v->hwnd);HGDIOBJ old=SelectObject(dc,n->font);SCRIPT_STRING_ANALYSIS a=lw_shape(dc,m->wide+f->start,f->count);int cp=0,trailing=0;
        if(a){ScriptStringXtoCP(a,lw_pixel(x-f->rect.x,v->dpi),&cp,&trailing);ScriptStringFree(&a);}SelectObject(dc,old);ReleaseDC(v->hwnd,dc);
        cp+=trailing;if(cp<0)cp=0;if(cp>f->count)cp=f->count;result=m->offset+f->start+cp;
        if(dist==0)*link=m->link;}
    return result;
}
static void ld_emit(lw_view_t *v,ui_document_event_t *e)
{
    ld_document_t *d=v->restricted;if(!d->callback)return;e->size=sizeof(*e);e->navigation_id=d->state.navigation_id;
    ++d->dispatching;++v->dispatch_depth;ui_dispatch_enter(v->host);d->callback(d->common,e,d->user);ui_dispatch_leave(v->host);--v->dispatch_depth;--d->dispatching;
}
static void ld_activate(lw_view_t *v,int i)
{
    ld_document_t *d=v->restricted;ld_meta_t *m=&d->meta[i];ui_document_event_t e={0};ui_document_field_t fields[64];size_t count=0;int form=m->form,submitter=i;
    if(m->url&&m->link==i){e.kind=UI_DOCUMENT_NAVIGATE;e.url_utf8=m->url;e.method="GET";ld_emit(v,&e);return;}
    if(form>=0&&!lw_disabled(v,form)&&d->meta[form].url){
        if(lw_input(&v->nodes[i])){submitter=-1;for(int j=0;j<v->node_count;++j)if(d->meta[j].form==form&&v->nodes[j].kind==2&&!lw_disabled(v,j)){submitter=j;break;}}
        for(int j=0;j<v->node_count;++j)if(d->meta[j].form==form&&d->meta[j].name&&!lw_disabled(v,j)&&
            (v->nodes[j].kind==3||j==submitter)){
            if(count==64){++d->state.restrictions;return;}fields[count].name_utf8=d->meta[j].name;fields[count++].value_utf8=v->nodes[j].value;}
        e.kind=UI_DOCUMENT_SUBMIT;e.url_utf8=d->meta[form].url;e.method="GET";e.fields=fields;e.field_count=count;ld_emit(v,&e);
    }
}
static void ld_reveal(lw_view_t *v,int i)
{
    lw_node_t *root=&v->nodes[0],*n=&v->nodes[i];
    if(n->rect.y<0)root->scroll_y+=n->rect.y;else if(n->rect.y+n->rect.height>root->viewport.height)root->scroll_y+=n->rect.y+n->rect.height-root->viewport.height;
    if(n->rect.x<0)root->scroll_x+=n->rect.x;else if(n->rect.x+n->rect.width>root->viewport.width)root->scroll_x+=n->rect.x+n->rect.width-root->viewport.width;
    lw_layout(v);
}
static ui_status_t ld_input(lw_view_t *v,const ui_input_event_t *e)
{
    ld_document_t *d=v->restricted;int hit=lw_hit(v,e->x,e->y),link=-1;
    if(e->kind==UI_INPUT_CANCEL){d->selecting=0;v->dragging=0;lw_scroll_cancel(v,0);return UI_STATUS_OK;}
    if(e->kind==UI_INPUT_POINTER_DOWN){v->keyboard_focus=0;
        if(lw_scroll_input(v,e))return UI_STATUS_OK;
        if(hit>=0&&(lw_input(&v->nodes[hit])||v->nodes[hit].kind==2)&&!lw_disabled(v,hit)){v->focused=hit;v->pressed=hit;lw_text_pointer(v,hit,e->x,e->y,0);}
        else{v->focused=-1;d->caret=ld_position(v,e->x,e->y,&link);if(!(e->modifiers&UI_INPUT_MODIFIER_SHIFT))d->anchor=d->caret;d->selecting=1;v->pressed=link;if(v->hwnd)SetCapture(v->hwnd);}
        if(v->hwnd)SetFocus(v->hwnd);lw_layout(v);return UI_STATUS_OK;}
    if(lw_scroll_input(v,e))return UI_STATUS_OK;
    if(e->kind==UI_INPUT_POINTER_MOVE){if(d->selecting){d->caret=ld_position(v,e->x,e->y,&link);if(e->y<0)v->nodes[0].scroll_y-=24;else if(e->y>v->height)v->nodes[0].scroll_y+=24;lw_layout(v);}
        else if(v->dragging)lw_text_pointer(v,v->focused,e->x,e->y,1);return UI_STATUS_OK;}
    if(e->kind==UI_INPUT_POINTER_UP){int pressed=v->pressed;v->pressed=-1;
        if(d->selecting){d->caret=ld_position(v,e->x,e->y,&link);d->selecting=0;if(d->anchor==d->caret&&link==pressed&&link>=0)ld_activate(v,link);}
        else if(hit==pressed&&hit>=0&&v->nodes[hit].kind==2&&!lw_disabled(v,hit))ld_activate(v,hit);
        v->dragging=0;if(v->hwnd&&GetCapture()==v->hwnd)ReleaseCapture();lw_layout(v);return UI_STATUS_OK;}
    if(e->kind==UI_INPUT_WHEEL){if(e->modifiers&UI_INPUT_MODIFIER_SHIFT)v->nodes[0].scroll_x-=e->wheel_delta*48/120;else v->nodes[0].scroll_y-=e->wheel_delta*48/120;lw_layout(v);return UI_STATUS_OK;}
    if(e->kind==UI_INPUT_TEXT)return lw_text_insert(v,e->text_utf8);
    if(e->kind==UI_INPUT_KEY_DOWN){int key=(int)e->key_code;v->keyboard_focus=1;
        if(e->modifiers&UI_INPUT_MODIFIER_CONTROL){if(key=='C'){if(v->focused>=0&&lw_input(&v->nodes[v->focused])){lw_clipboard(v,&v->nodes[v->focused],0,0);return UI_STATUS_OK;}return ui_light_document_copy(d->common);}
            if(key=='A'&&v->focused<0){d->anchor=0;d->caret=d->text_length;lw_layout(v);return UI_STATUS_OK;}}
        if(key==VK_TAB){int i=v->focused,delta=e->modifiers&UI_INPUT_MODIFIER_SHIFT?-1:1;if(i<0&&delta<0)i=0;
            for(int k=0;k<v->node_count;++k){int next=i+delta;
                if(v->focused>=0&&(next<0||next>=v->node_count)){ui_document_event_t event={0};event.kind=UI_DOCUMENT_FOCUS_OUT;event.resource_id=delta<0?1u:0u;v->focused=-1;ld_emit(v,&event);lw_layout(v);return UI_STATUS_OK;}
                i=(next+v->node_count)%v->node_count;if(v->nodes[i].visible&&!lw_disabled(v,i)&&(d->meta[i].link==i||v->nodes[i].kind==2||lw_input(&v->nodes[i]))){v->focused=i;ld_reveal(v,i);return UI_STATUS_OK;}}}
        if(key==VK_RETURN&&v->focused>=0){ld_activate(v,v->focused);return UI_STATUS_OK;}
        if(lw_text_key(v,e))return UI_STATUS_OK;
        if(key==VK_SPACE&&v->focused>=0&&d->meta[v->focused].link==v->focused){ld_activate(v,v->focused);return UI_STATUS_OK;}
        {lw_node_t *r=&v->nodes[0];switch(key){case VK_HOME:if(e->modifiers&UI_INPUT_MODIFIER_CONTROL)r->scroll_y=0;r->scroll_x=0;break;
            case VK_END:if(e->modifiers&UI_INPUT_MODIFIER_CONTROL)r->scroll_y=r->content_height;r->scroll_x=r->content_width;break;
            case VK_UP:r->scroll_y-=24;break;case VK_DOWN:r->scroll_y+=24;break;case VK_PRIOR:r->scroll_y-=r->viewport.height;break;case VK_NEXT:case VK_SPACE:r->scroll_y+=r->viewport.height;break;
            case VK_LEFT:r->scroll_x-=24;break;case VK_RIGHT:r->scroll_x+=24;break;default:(void)ui_host_dispatch_shortcut(v->host,e->key_code,e->modifiers,v->focused>=0&&lw_input(&v->nodes[v->focused]));return UI_STATUS_UNSUPPORTED;}
            lw_layout(v);return UI_STATUS_OK;}
    }return UI_STATUS_UNSUPPORTED;
}
static ui_status_t ld_flush(lw_view_t *v)
{
    ld_document_t *d=v->restricted;int i;if(!d)return UI_STATUS_NOT_FOUND;
    if(!d->callback){if(v->dirty)lw_layout(v);return d->failed?UI_STATUS_LIMIT_EXCEEDED:UI_STATUS_OK;}
    for(i=0;i<d->resource_count;++i){ld_resource_t *r=&d->resources[i];if(!r->issued&&!r->complete){ui_document_event_t e={0};r->issued=1;e.kind=(ui_document_event_kind_t)r->kind;e.resource_id=r->id;e.url_utf8=r->url;ld_emit(v,&e);}}
    if(v->dirty)lw_layout(v);return d->failed?UI_STATUS_LIMIT_EXCEEDED:UI_STATUS_OK;
}
static lw_view_t *ld_view(ui_web_view_t *view)
{
    if(!view||view->backend->ops.create_view!=lw_create_view||!ui_language_thread_ok(view->host))return NULL;
    {lw_view_t *v=(lw_view_t *)view->user_data;return v->backend->restricted?v:NULL;}
}
ui_web_backend_t *ui_light_document_backend_create(const ui_light_web_config_t *config)
{
    ui_web_backend_t *b=ui_light_web_backend_create(config);if(b)((lw_backend_t *)b->user_data)->restricted=1;return b;
}
ui_status_t ui_light_document_set_event_callback(ui_web_view_t *view,ui_document_event_fn callback,void *user)
{
    lw_view_t *v=ld_view(view);if(!v)return UI_STATUS_INVALID_ARGUMENT;if(v->restricted->dispatching)return UI_STATUS_CANCELLED;
    v->restricted->callback=callback;v->restricted->user=user;v->restricted->common=view;return UI_STATUS_OK;
}
ui_status_t ui_light_document_load(ui_web_view_t *view,const ui_document_load_t *load)
{
    lw_view_t *v=ld_view(view);ld_document_t *d;ui_status_t status;lxb_html_parser_t *parser;wchar_t *valid;lxb_dom_node_t *body;LARGE_INTEGER a,b;
    if(!v||!load||load->size<sizeof(*load)||!load->navigation_id||!load->source_url||!load->html_utf8)return UI_STATUS_INVALID_ARGUMENT;
    d=v->restricted;if(v->dispatch_depth||d->dispatching||v->host->dispatch_blocked)return UI_STATUS_CANCELLED;
    if(strlen(load->html_utf8)>LW_HTML_LIMIT||strlen(load->source_url)>=sizeof(d->source))return UI_STATUS_LIMIT_EXCEEDED;
    valid=lw_wide(load->html_utf8);if(!valid)return UI_STATUS_VALIDATION_FAILED;free(valid);
    {char *url=ld_resolve(load->source_url,load->source_url);if(!url)return UI_STATUS_INVALID_ARGUMENT;free(url);}
    (void)ui_light_document_cancel_resources(view);lw_scroll_cancel(v,0);lw_free_document(v);d=v->restricted;d->common=view;d->state.navigation_id=load->navigation_id;d->state.size=sizeof(d->state);strcpy(d->source,load->source_url);strcpy(d->document_url,load->source_url);
    QueryPerformanceCounter(&a);d->meta=(ld_meta_t *)calloc(LW_MAX_NODES,sizeof(ld_meta_t));if(!d->meta)return UI_STATUS_OUT_OF_MEMORY;
    parser=lxb_html_parser_create();if(!parser)return UI_STATUS_OUT_OF_MEMORY;
    if(lxb_html_parser_init(parser)!=LXB_STATUS_OK){lxb_html_parser_destroy(parser);return UI_STATUS_OUT_OF_MEMORY;}
    v->document=lxb_html_parse(parser,(const lxb_char_t *)load->html_utf8,strlen(load->html_utf8));lxb_html_parser_destroy(parser);if(!v->document)return UI_STATUS_VALIDATION_FAILED;
    {int count=0;if(ld_validate_tree(lxb_dom_interface_node(v->document),0,&count)!=UI_STATUS_OK){lw_free_document(v);return UI_STATUS_LIMIT_EXCEEDED;}}
    body=lxb_dom_interface_node(lxb_html_document_body_element(v->document));v->root=ld_add(v,-1,body,"body");if(v->root<0)return UI_STATUS_OUT_OF_MEMORY;
    if(lw_attr(lxb_dom_interface_element(body),"id",v->nodes[0].id,sizeof(v->nodes[0].id))!=UI_STATUS_OK||
       lw_attr(lxb_dom_interface_element(body),"class",v->nodes[0].classes,sizeof(v->nodes[0].classes))!=UI_STATUS_OK||
       lw_attr(lxb_dom_interface_element(body),"style",v->nodes[0].inline_style,sizeof(v->nodes[0].inline_style))!=UI_STATUS_OK){lw_free_document(v);return UI_STATUS_LIMIT_EXCEEDED;}
    if(*v->nodes[0].inline_style){char filtered[LW_ATTR_CAP];status=ld_declarations(v,filtered,v->nodes[0].inline_style);if(status!=UI_STATUS_OK){lw_free_document(v);return status;}strcpy(v->nodes[0].inline_style,filtered);}
    status=ld_head(v,lxb_dom_interface_node(lxb_html_document_head_element(v->document)),0);
    if(status==UI_STATUS_OK)status=ld_collect(v,body,0,0);
    if(status==UI_STATUS_OK){for(int form=0;form<v->node_count;++form)if(ld_tag(&v->nodes[form],"form")){int controls=0,submit=0;for(int j=0;j<v->node_count;++j)if(d->meta[j].form==form&&d->meta[j].name&&!lw_disabled(v,j)){if(v->nodes[j].kind==3)++controls;else if(v->nodes[j].kind==2)submit=1;}if(controls+submit>64){status=UI_STATUS_LIMIT_EXCEEDED;break;}}}
    QueryPerformanceCounter(&b);d->state.parse_ms=(double)(b.QuadPart-a.QuadPart)*1000.0/d->frequency.QuadPart;
    if(status!=UI_STATUS_OK){lw_free_document(v);return status;}lw_layout(v);if(d->failed||d->no_content){status=d->failed?UI_STATUS_LIMIT_EXCEEDED:UI_STATUS_UNSUPPORTED;lw_free_document(v);return status;}return UI_STATUS_OK;
}
ui_status_t ui_light_document_get_state(ui_web_view_t *view,ui_document_state_t *state)
{
    lw_view_t *v=ld_view(view);if(!v||!state||state->size<sizeof(*state))return UI_STATUS_INVALID_ARGUMENT;
    *state=v->restricted->state;state->size=sizeof(*state);return UI_STATUS_OK;
}
ui_status_t ui_light_document_resource(ui_web_view_t *view,uint64_t navigation,uint64_t resource,ui_image_id_t image,const char *css)
{
    lw_view_t *v=ld_view(view);ld_document_t *d;int i;ui_status_t s=UI_STATUS_OK;
    if(!v)return UI_STATUS_INVALID_ARGUMENT;d=v->restricted;if(navigation!=d->state.navigation_id)return UI_STATUS_NOT_FOUND;
    if(d->dispatching||v->host->dispatch_blocked)return UI_STATUS_CANCELLED;
    for(i=0;i<d->resource_count;++i){ld_resource_t *r=&d->resources[i];if(r->id!=resource||r->complete)continue;
        if(r->kind==UI_DOCUMENT_IMAGE){v->nodes[r->node].image=image;d->meta[r->node].image_state=image?0:2;}else if(css){d->current_sheet=r->sheet_order;s=ld_css(v,css);}
        if((r->kind==UI_DOCUMENT_IMAGE&&!image)||(r->kind==UI_DOCUMENT_STYLESHEET&&!css)||s!=UI_STATUS_OK)++d->state.failed_resources;
        r->complete=1;--d->state.pending_resources;lw_layout(v);return d->failed?UI_STATUS_LIMIT_EXCEEDED:d->no_content?UI_STATUS_UNSUPPORTED:s;}
    return UI_STATUS_NOT_FOUND;
}
ui_status_t ui_light_document_cancel_resources(ui_web_view_t *view)
{
    lw_view_t *v=ld_view(view);ld_document_t *d;if(!v)return UI_STATUS_INVALID_ARGUMENT;d=v->restricted;if(d->dispatching)return UI_STATUS_CANCELLED;
    for(int i=0;i<d->resource_count;++i){ld_resource_t *r=&d->resources[i];if(!r->complete){r->complete=1;--d->state.pending_resources;++d->state.failed_resources;
        if(r->kind==UI_DOCUMENT_IMAGE)d->meta[r->node].image_state=2;
        if(r->issued){ui_document_event_t event={0};event.kind=UI_DOCUMENT_CANCEL_RESOURCE;event.resource_id=r->id;event.url_utf8=r->url;ld_emit(v,&event);}}}lw_layout(v);return UI_STATUS_OK;
}
ui_status_t ui_light_document_scroll(ui_web_view_t *view,int x,int y)
{lw_view_t *v=ld_view(view);if(!v||v->root<0)return UI_STATUS_INVALID_ARGUMENT;v->nodes[0].scroll_x=x;v->nodes[0].scroll_y=y;lw_layout(v);return UI_STATUS_OK;}
ui_status_t ui_light_document_fragment(ui_web_view_t *view,const char *fragment)
{
    lw_view_t *v=ld_view(view);if(!v||!fragment)return UI_STATUS_INVALID_ARGUMENT;
    for(int i=0;i<v->node_count;++i)if(v->nodes[i].visible&&!strcmp(v->nodes[i].id,fragment)){v->nodes[0].scroll_y+=v->nodes[i].rect.y;v->nodes[0].scroll_x+=v->nodes[i].rect.x-12;lw_layout(v);return UI_STATUS_OK;}return UI_STATUS_NOT_FOUND;
}
ui_status_t ui_light_document_zoom(ui_web_view_t *view,uint32_t percent)
{lw_view_t *v=ld_view(view);if(!v||percent<50||percent>200)return UI_STATUS_INVALID_ARGUMENT;v->restricted->state.zoom_percent=percent;lw_layout(v);return v->restricted->failed?UI_STATUS_LIMIT_EXCEEDED:UI_STATUS_OK;}
ui_status_t ui_light_document_find(ui_web_view_t *view,const char *query,int reverse)
{
    lw_view_t *v=ld_view(view);ld_document_t *d;wchar_t *q;int length,start,i;
    if(!v||!query)return UI_STATUS_INVALID_ARGUMENT;d=v->restricted;q=lw_wide(query);if(!q)return UI_STATUS_VALIDATION_FAILED;
    length=(int)wcslen(q);if(!length){d->anchor=d->caret=0;free(q);lw_layout(v);return UI_STATUS_OK;}
    start=reverse?d->anchor-1:d->caret;
    for(i=0;i<d->text_length;++i){int pos=(start+(reverse?-i:i)+d->text_length)%d->text_length;
        if(pos+length>d->text_length)continue;
        if(CompareStringOrdinal(d->text+pos,length,q,length,TRUE)==CSTR_EQUAL){d->anchor=pos;d->caret=pos+length;v->focused=-1;free(q);
            for(int j=0;j<d->fragment_count;++j){ld_fragment_t *f=&d->fragments[j];int offset=d->meta[f->node].offset+f->start;
                if(pos>=offset&&pos<offset+f->count){v->nodes[0].scroll_y+=f->rect.y;v->nodes[0].scroll_x+=f->rect.x-12;break;}}
            lw_layout(v);return UI_STATUS_OK;}}
    free(q);return UI_STATUS_NOT_FOUND;
}
ui_status_t ui_light_document_copy(ui_web_view_t *view)
{
    lw_view_t *v=ld_view(view);ld_document_t *d;int lo,hi;HGLOBAL data;wchar_t *target;
    if(!v)return UI_STATUS_INVALID_ARGUMENT;d=v->restricted;lo=d->anchor<d->caret?d->anchor:d->caret;hi=d->anchor>d->caret?d->anchor:d->caret;
    if(hi<=lo)return UI_STATUS_NOT_FOUND;if(!OpenClipboard(v->hwnd))return UI_STATUS_PLATFORM_ERROR;
    data=GlobalAlloc(GMEM_MOVEABLE,(size_t)(hi-lo+1)*sizeof(wchar_t));target=data?(wchar_t *)GlobalLock(data):NULL;
    if(!target){if(data)GlobalFree(data);CloseClipboard();return UI_STATUS_OUT_OF_MEMORY;}
    memcpy(target,d->text+lo,(size_t)(hi-lo)*sizeof(wchar_t));target[hi-lo]=0;GlobalUnlock(data);EmptyClipboard();
    if(!SetClipboardData(CF_UNICODETEXT,data)){GlobalFree(data);CloseClipboard();return UI_STATUS_PLATFORM_ERROR;}CloseClipboard();return UI_STATUS_OK;
}
