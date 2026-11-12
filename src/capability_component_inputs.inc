/* CI-1, a selector on the existing input binding, not a second executor. */
typedef struct editor_record {char node[256],base[128];uint64_t generation;int conflict;} editor_record_t;
struct editor_store {editor_record_t records[64];char pending_node[256],text[4097];uint64_t pending_call,submitted;int reading;};
static int selector_node(ui_cap_component_selector_t selector,const char *node)
{
    const char *p=node,*end;uint64_t id=0;const char *prefix=selector==UI_CAP_CELL_EDIT?"edit-":selector==UI_CAP_CELL_TOGGLE?"check-":selector==UI_CAP_ROW_RENAME?"rename-":
      selector==UI_CAP_COLUMN_SORT?"sort-":selector==UI_CAP_ROW_CONTEXT?"row-":"cell-";
    if(selector==UI_CAP_ROW_SELECT||selector==UI_CAP_ROW_CONTEXT){if(!strncmp(node,"tree-label-",11))prefix="tree-label-";else if(!strncmp(node,"cell-",5))prefix="cell-";else if(!strncmp(node,"row-",4))prefix="row-";}
    if(strncmp(node,prefix,strlen(prefix)))return 0;p+=strlen(prefix);if(selector==UI_CAP_COLUMN_SORT)return *p&&strlen(p)<=32;
    if(*p<'1'||*p>'9')return 0;end=p;while(*end>='0'&&*end<='9'){unsigned digit=(unsigned)(*end-'0');if(id>(UINT64_MAX-digit)/10u)return 0;id=id*10u+digit;++end;}
    if(selector==UI_CAP_CELL_EDIT||selector==UI_CAP_CELL_TOGGLE||!strcmp(prefix,"cell-"))return *end=='-'&&end[1]&&strlen(end+1)<=32;
    return *end==0;
}
static int selector_match(input_binding_t *i,const char *node,const ui_input_event_t *e)
{
    if(!selector_node(i->selector,node))return 0;
    if(i->selector==UI_CAP_CELL_EDIT||i->selector==UI_CAP_ROW_RENAME)return e->kind==UI_INPUT_KEY_DOWN&&(e->key_code==13||e->key_code==27);
    if(i->selector==UI_CAP_ROW_SELECT&&!strncmp(node,"cell-",5)&&e->kind==UI_INPUT_KEY_DOWN&&(e->key_code==9||(e->key_code>=35&&e->key_code<=40)))return 1;
    if(i->selector==UI_CAP_ROW_CONTEXT)return e->kind==UI_INPUT_POINTER_UP&&e->pointer_button==2;
    return (e->kind==UI_INPUT_POINTER_UP&&e->pointer_button==1)||(e->kind==UI_INPUT_KEY_DOWN&&(e->key_code==32||(e->key_code==13&&strncmp(node,"cell-",5))));
}
static editor_record_t *editor_find(struct editor_store *s,const char *node)
{size_t at;if(!s)return NULL;for(at=0;at<64;++at)if(!strcmp(s->records[at].node,node))return &s->records[at];return NULL;}
static void editor_cancel(input_binding_t *i,const char *node)
{editor_record_t *d=editor_find(i->editors,node);if(d)memset(d,0,sizeof(*d));}
static ui_status_t input_expected_revision(input_binding_t *i,char out[128])
{
    editor_record_t *d;if(!i||!i->editors)return UI_STATUS_OK;
    d=editor_find(i->editors,i->node);if(!d)return UI_STATUS_VALIDATION_FAILED;
    strcpy(out,d->base);return UI_STATUS_OK;
}
void ui_inputs_editor_event(ui_host_t *host,ui_component_t *component,const char *action,const char *node)
{
    registry_t *r=host?host->capabilities:NULL;ui_binding_t *b;if(!r||!node||strlen(node)>255)return;
    for(b=r->bindings;b;b=b->next)if(b->input&&b->input->owner==component&&b->input->editors&&selector_node(b->input->selector,node)){
      struct editor_store *s=b->input->editors;editor_record_t *d=editor_find(s,node);size_t at;resource_t *res;char rev[128];ui_status_t status;
      if(b->retiring)return;if(!strcmp(action,"editor-change")){if(d&&d->generation!=UINT64_MAX)++d->generation;return;}
      if(strcmp(action,"editor-open")||d||s->reading)return;
      for(at=0;at<64&&s->records[at].node[0];++at){}if(at==64){observe_v2(host,r,b,0,UI_CAP_UI_REJECTED,UI_STATUS_LIMIT_EXCEEDED,"budget_exceeded",NULL);return;}
      res=find_resource(r,b->desc.resource.resource_id);if(!ref_matches(r,res,&b->desc.resource)||res->retiring)return;
      ++b->leases;++host->capability_ui_callbacks;++host->assistant_callbacks;ui_dispatch_enter(host);s->reading=1;
      status=revision(host,r,res,rev);if(!status&&!b->retiring&&!res->retiring&&!is_closing(r)){d=&s->records[at];strcpy(d->node,node);strcpy(d->base,rev);d->generation=1;d->conflict=0;}
      s->reading=0;--b->leases;--host->capability_ui_callbacks;--host->assistant_callbacks;ui_dispatch_leave(host);return;
    }
}
static void editor_collect_snapshot(input_binding_t *i)
{
    struct editor_store *s=i->editors;editor_record_t *d;if(!s)return;d=editor_find(s,i->node);if(!d)return;
    strcpy(s->pending_node,i->node);s->submitted=d->generation;snprintf(s->text,sizeof(s->text),"%s",i->value?i->value:"");
}
static void editor_observe(input_binding_t *i,const ui_cap_ui_notice_t *n)
{
    struct editor_store *s=i->editors;editor_record_t *d;ui_cap_result_v2_t result={0};if(!s)return;
    if(n->stage==UI_CAP_UI_ADMITTED){s->pending_call=n->call_id;return;}
    if(n->stage!=UI_CAP_UI_FINISHED||n->call_id!=s->pending_call)return;
    d=editor_find(s,s->pending_node);if(!d)return;result.size=sizeof(result);result.version=2;
    if(ui_capability_get_result_v2(i->host,n->call_id,&result)!=UI_STATUS_OK)return;
    if(result.outcome==UI_CAP2_STALE)d->conflict=1;
    if(result.outcome==UI_CAP2_OK&&d->generation==s->submitted&&ui_components_editor_accept(i->desc.component,d->node,s->text))memset(d,0,sizeof(*d));
    s->pending_call=0;
}
ui_status_t ui_capability_component_input_bind(ui_host_t *host,const ui_cap_component_input_desc_t *d,uint64_t *id)
{
    registry_t *r;ui_status_t status=registry(host,&r);ui_cap_input_binding_desc_t plain={0};ui_binding_t *b;struct editor_store *store=NULL;size_t bytes=0;char node[40];
    if(id)*id=0;if(status)return status;if(!d||!id||d->size<sizeof(*d)||d->version!=1||!d->component||d->selector<UI_CAP_CELL_EDIT||d->selector>UI_CAP_CELL_TOGGLE||!d->collect)return UI_STATUS_INVALID_ARGUMENT;
    for(b=r->bindings;b;b=b->next)if(b->input&&b->input->owner==d->component&&
      (b->input->selector==d->selector||(!b->input->selector&&selector_node(d->selector,b->input->control))))return UI_STATUS_ALREADY_EXISTS;
    if(d->selector<=UI_CAP_ROW_RENAME){bytes=sizeof(*store);if(bytes>UI_CAPABILITY_CATALOG_BYTES-r->catalog_bytes)return UI_STATUS_LIMIT_EXCEEDED;store=calloc(1,bytes);if(!store)return UI_STATUS_OUT_OF_MEMORY;r->catalog_bytes+=bytes;}
    snprintf(node,sizeof(node),"@selector:%u",(unsigned)d->selector);plain.size=sizeof(plain);plain.version=1;plain.kind=UI_CAP_INPUT_COMPONENT;plain.component=d->component;plain.control_id=node;
    plain.capability_id=d->capability_id;plain.resource=d->resource;plain.purpose=UI_CAP_UI_INVOKE;plain.collect=d->collect;plain.observe=d->observe;plain.user_data=d->user_data;
    plain.trigger=UI_INPUT_KEY_DOWN;plain.key=13;status=ui_capability_input_bind(host,&plain,id);
    if(status){r->catalog_bytes-=bytes;free(store);return status;}b=find_binding(r,*id);b->bytes+=bytes;b->input->selector=d->selector;b->input->editors=store;return UI_STATUS_OK;
}
ui_status_t ui_capability_component_editor_state(ui_host_t *host,uint64_t id,const char *node,ui_cap_editor_state_t *out)
{
    registry_t *r;ui_status_t status=registry(host,&r);ui_binding_t *b;editor_record_t *d;resource_t *res;char rev[128];struct editor_store *s;
    if(status)return status;if(!ui_cap_text(node,255,0)||!out||out->size<sizeof(*out)||out->version!=1)return UI_STATUS_INVALID_ARGUMENT;
    b=find_binding(r,id);if(!b||!b->input||!b->input->editors)return UI_STATUS_NOT_FOUND;s=b->input->editors;d=editor_find(s,node);if(!d)return UI_STATUS_NOT_FOUND;
    if(s->reading)return UI_STATUS_PENDING;res=find_resource(r,b->desc.resource.resource_id);if(!res)return UI_STATUS_NOT_FOUND;
    ++b->leases;++host->capability_ui_callbacks;++host->assistant_callbacks;ui_dispatch_enter(host);s->reading=1;status=revision(host,r,res,rev);s->reading=0;
    if(!status){out->generation=d->generation;out->conflict=(uint32_t)(d->conflict||strcmp(d->base,rev)!=0);strcpy(out->base_revision,d->base);}
    --b->leases;--host->capability_ui_callbacks;--host->assistant_callbacks;ui_dispatch_leave(host);return status;
}
int ui_inputs_component_action_bound(ui_host_t *host,ui_component_t *component,const char *action)
{
    registry_t *r=host?host->capabilities:NULL;ui_binding_t *b;int role=!strcmp(action,"edit")?UI_CAP_CELL_EDIT:!strcmp(action,"rename")?UI_CAP_ROW_RENAME:
      !strcmp(action,"select")?UI_CAP_ROW_SELECT:!strcmp(action,"sort")?UI_CAP_COLUMN_SORT:!strcmp(action,"context")?UI_CAP_ROW_CONTEXT:0;
    if(!r||!role)return 0;for(b=r->bindings;b;b=b->next)if(b->input&&b->input->owner==component&&(b->input->selector==role||(role==UI_CAP_CELL_EDIT&&b->input->selector==UI_CAP_CELL_TOGGLE)))return 1;return 0;
}
