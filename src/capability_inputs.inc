/* C2C-1: metadata adapters into the existing UI admission/executor. */
#ifdef _WIN32
#include <commctrl.h>
#endif
struct input_binding {
    ui_cap_input_binding_desc_t desc;ui_binding_t *binding;ui_host_t *host;void *owner;
    ui_web_view_t *view;ui_input_event_t event;ui_input_chain_t chain;
    char *control;const char *value;size_t bytes;
    ui_cap_component_selector_t selector;struct editor_store *editors;char node[256];
};
static int selector_match(input_binding_t *,const char *,const ui_input_event_t *);
static ui_status_t input_expected_revision(input_binding_t *,char [128]);
static void editor_observe(input_binding_t *,const ui_cap_ui_notice_t *);
static void editor_collect_snapshot(input_binding_t *);
static void editor_cancel(input_binding_t *,const char *);
int ui_inputs_origin_eligible(int ok,int origin,int device)
{return ok&&origin==1&&(device==1||device==2||device==4||device==8||device==16);}
int ui_inputs_current_source(void)
{
#ifdef _WIN32
    INPUT_MESSAGE_SOURCE source={0};int ok=GetCurrentInputMessageSource(&source);
    return ui_inputs_origin_eligible(ok,(int)source.originId,(int)source.deviceType);
#else
    return 0;
#endif
}
int ui_inputs_chain(ui_input_chain_t *c,const ui_input_event_t *e,uint64_t identity,int eligible)
{
    int accept=0;if(!eligible||e->kind==UI_INPUT_CANCEL){memset(c,0,sizeof(*c));return 0;}
    if(e->kind==UI_INPUT_POINTER_DOWN){c->pressed=identity;c->button=e->pointer_button;c->modifiers=e->modifiers;}
    else if(e->kind==UI_INPUT_POINTER_UP){accept=identity&&c->pressed==identity&&c->button==e->pointer_button&&c->modifiers==e->modifiers;memset(c,0,sizeof(*c));}
    else if(e->kind==UI_INPUT_KEY_DOWN)accept=1;
    return accept;
}
static int input_available(ui_host_t *host,input_binding_t *i)
{
    if(i->desc.kind==UI_CAP_INPUT_SHORTCUT)return !host->modal_component;
    if(i->desc.kind==UI_CAP_INPUT_TOOLBAR){ui_toolbar_item_entry_t *t;ui_toolbar_entry_t *bar;
      for(t=host->toolbar_items;t&&strcmp(t->id,i->control);t=t->next){}if(!t||strcmp(t->command_id,i->desc.capability_id))return 0;
      for(bar=host->toolbars;bar&&strcmp(bar->id,t->toolbar_id);bar=bar->next){}
      return bar&&bar->visible&&t->state.enabled&&t->state.visible&&!t->state.busy&&!host->modal_component;}
    if(!i->owner)return 0;
    if(i->desc.kind==UI_CAP_INPUT_COMPONENT)return ui_components_input_allowed(host,i->view->user_data)&&(!i->selector||ui_components_input_node_allowed(i->desc.component,i->node));
    if(i->desc.kind==UI_CAP_INPUT_LIGHT_CONTROL)return !host->modal_component;
    if(i->desc.kind==UI_CAP_INPUT_SURFACE){ui_surface_t *s=i->owner;return s->visible&&!host->modal_component;}
#ifdef _WIN32
    if(i->desc.kind==UI_CAP_INPUT_NATIVE_BUTTON)return IsWindow((HWND)i->owner)&&IsWindowVisible((HWND)i->owner)&&IsWindowEnabled((HWND)i->owner)&&!host->modal_component;
#endif
    return 0;
}
static ui_status_t input_collect(const ui_cap_ui_context_t *ctx,char *json,size_t capacity,size_t *length,void *data)
{
    input_binding_t *i=data;ui_cap_input_context_t c={0};c.size=sizeof(c);c.version=1;c.invocation=*ctx;c.event=i->event;c.control_id=i->selector?i->node:i->control;c.value_utf8=i->value;
    editor_collect_snapshot(i);
    if(i->desc.collect)return i->desc.collect(&c,json,capacity,length,i->desc.user_data);
    if(capacity<3)return UI_STATUS_LIMIT_EXCEEDED;memcpy(json,"{}",3);*length=2;return UI_STATUS_OK;
}
static void input_observe(const ui_cap_ui_notice_t *n,void *data)
{input_binding_t *i=data;editor_observe(i,n);if(i->desc.observe)i->desc.observe(n,i->desc.user_data);}
static int input_match(input_binding_t *i,ui_cap_input_kind_t kind,void *owner,const char *node,const ui_input_event_t *e)
{
    if(i->desc.kind!=kind||(kind!=UI_CAP_INPUT_SHORTCUT&&kind!=UI_CAP_INPUT_TOOLBAR&&i->owner!=owner))return 0;
    if(i->selector)return selector_match(i,node?node:"",e);
    if(kind!=UI_CAP_INPUT_SHORTCUT&&strcmp(i->control,node?node:""))return 0;
    return e->kind==i->desc.trigger&&e->modifiers==i->desc.modifiers&&
      (e->kind==UI_INPUT_KEY_DOWN?e->key_code==i->desc.key:e->pointer_button==i->desc.pointer_button);
}
int ui_inputs_has(ui_host_t *host,ui_cap_input_kind_t kind,void *owner,const char *node,const ui_input_event_t *e)
{
    ui_binding_t *b;registry_t *r=host?host->capabilities:NULL;if(!r||!e)return 0;
    for(b=r->bindings;b;b=b->next)if(b->input&&input_match(b->input,kind,owner,node,e))return 1;return 0;
}
int ui_inputs_route(ui_host_t *host,ui_cap_input_kind_t kind,void *owner,const char *node,const ui_input_event_t *e,void *origin_owner)
{
    ui_binding_t *b;registry_t *r=host?host->capabilities:NULL;if(!r||!e)return 0;
    for(b=r->bindings;b;b=b->next)if(b->input&&input_match(b->input,kind,owner,node,e)){
      if(r->work->used||r->ui_notifying)return 1;
      if(b->input->selector)snprintf(b->input->node,sizeof(b->input->node),"%s",node?node:"");
      b->input->event=*e;b->input->event.text_utf8=NULL;
      (void)ui_capabilities_ui_action(host,b->desc.menu_item_id,origin_owner);return 1;}return 0;
}
int ui_inputs_light_has(ui_host_t *host,void *backend,const char *node,const ui_input_event_t *event)
{
    ui_binding_t *b;ui_web_view_t *v;registry_t *r=host?host->capabilities:NULL;if(!r)return 0;
    for(v=host->web_views;v&&v->user_data!=backend;v=v->host_next){}if(!v)return 0;
    for(b=r->bindings;b;b=b->next)if(b->input&&b->input->view==v&&input_match(b->input,b->input->desc.kind,b->input->owner,node,event))return 1;
    return 0;
}
int ui_inputs_light(ui_host_t *host,void *backend,const char *node,const ui_input_event_t *event,const char *value)
{
    ui_binding_t *b;ui_web_view_t *v;registry_t *r=host?host->capabilities:NULL;if(!r)return 0;
    for(v=host->web_views;v&&v->user_data!=backend;v=v->host_next){}if(!v)return 0;
    if(r->work->used||r->ui_notifying)return 1;
    if(value&&strlen(value)>4096)return 1;
    if(value)memcpy(r->work->input_value,value,strlen(value)+1);
    for(b=r->bindings;b;b=b->next)if(b->input&&b->input->view==v&&input_match(b->input,b->input->desc.kind,b->input->owner,node,event)){
      int follow=b->input->selector==UI_CAP_ROW_SELECT;b->input->value=value?r->work->input_value:NULL;
      if(b->input->editors&&event->kind==UI_INPUT_KEY_DOWN&&event->key_code==27){uint64_t sequence;
        if(ui_capabilities_control_consume(host,backend,&sequence)!=UI_STATUS_OK)return 1;
        editor_cancel(b->input,node);return 2;}
      (void)ui_inputs_route(host,b->input->desc.kind,b->input->owner,node,event,backend);return follow?2:1;}
    return 0;
}
int ui_inputs_owner_busy(ui_host_t *host,void *owner)
{
    ui_binding_t *b;registry_t *r=host?host->capabilities:NULL;if(!r||!host->capability_ui_callbacks)return 0;
    {ui_web_view_t *v;for(v=host->web_views;v;v=v->host_next)if(v==owner&&r->human_owner==v->user_data)return 1;}
    for(b=r->bindings;b;b=b->next){if(!b->retiring&&b->component&&ui_components_view(b->component)==owner)return 1;
      if(b->input&&(b->input->owner==owner||b->input->view==owner))return 1;}return 0;
}
void ui_inputs_owner_destroyed(ui_host_t *host,void *owner)
{
    ui_binding_t *b;registry_t *r=host?host->capabilities:NULL;if(!r)return;
    for(b=r->bindings;b;b=b->next)if(b->input&&(b->input->owner==owner||b->input->view==owner)){
      b->input->owner=NULL;b->input->view=NULL;if(!b->retiring){b->retiring=1;++r->generation;}}
}
#ifdef _WIN32
static uint32_t input_modifiers_now(void)
{return (GetKeyState(VK_CONTROL)&0x8000?1u:0u)|(GetKeyState(VK_SHIFT)&0x8000?2u:0u)|(GetKeyState(VK_MENU)&0x8000?4u:0u);}
static LRESULT CALLBACK input_button_proc(HWND hwnd,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    input_binding_t *i=(input_binding_t *)data;ui_binding_t *b=i->binding;ui_host_t *host=i->host;
    ui_input_event_t e={0};int semantic=0,eligible=0;LRESULT result;(void)id;
    if(!host)return DefSubclassProc(hwnd,message,wp,lp);
    ++b->leases;++host->capability_ui_callbacks;++host->assistant_callbacks;ui_dispatch_enter(host);
    e.size=sizeof(e);e.modifiers=input_modifiers_now();
    if(message==WM_LBUTTONDOWN||message==WM_LBUTTONUP||message==WM_KEYDOWN)ui_capabilities_human_invalidate(host,hwnd);
    if(message==WM_LBUTTONDOWN||message==WM_LBUTTONUP){e.kind=message==WM_LBUTTONDOWN?UI_INPUT_POINTER_DOWN:UI_INPUT_POINTER_UP;e.pointer_button=1;e.x=MulDiv((short)LOWORD(lp),96,(int)ui_platform_get_dpi(hwnd));e.y=MulDiv((short)HIWORD(lp),96,(int)ui_platform_get_dpi(hwnd));
      {RECT rect;int x=(short)LOWORD(lp),y=(short)HIWORD(lp);GetClientRect(hwnd,&rect);
       eligible=ui_inputs_chain(&i->chain,&e,x>=0&&y>=0&&x<rect.right&&y<rect.bottom?1u:0u,ui_inputs_current_source());}semantic=e.kind==UI_INPUT_POINTER_UP;}
    else if(message==WM_KEYDOWN&&(wp==VK_SPACE||wp==VK_RETURN)){e.kind=UI_INPUT_KEY_DOWN;e.key_code=(uint32_t)wp;semantic=1;eligible=!(lp&(1L<<30))&&ui_inputs_current_source();}
    else if(message==WM_KILLFOCUS||message==WM_CAPTURECHANGED||message==WM_CANCELMODE){memset(&i->chain,0,sizeof(i->chain));ui_capabilities_human_invalidate(host,hwnd);}
    if(message==BM_CLICK)result=0;
    else if(semantic){
      if(eligible)ui_capabilities_human_begin(host,hwnd);
      (void)ui_inputs_route(host,UI_CAP_INPUT_NATIVE_BUTTON,hwnd,"",&e,hwnd);
      if(eligible)ui_capabilities_human_end(host);SendMessageW(hwnd,BM_SETSTATE,FALSE,0);if(GetCapture()==hwnd)ReleaseCapture();result=0;
    }else result=DefSubclassProc(hwnd,message,wp,lp);
    if(message==WM_NCDESTROY){ui_inputs_owner_destroyed(host,hwnd);}
    --b->leases;--host->capability_ui_callbacks;--host->assistant_callbacks;ui_dispatch_leave(host);return result;
}
#endif
static void input_cleanup(void *data)
{
    input_binding_t *i=data;
#ifdef _WIN32
    if(i->binding&&i->desc.kind==UI_CAP_INPUT_NATIVE_BUTTON&&i->owner&&IsWindow((HWND)i->owner)){
      RemoveWindowSubclass((HWND)i->owner,input_button_proc,(UINT_PTR)i->binding->id);}
#endif
    free(i->editors);free(i->control);free(i);
}
ui_status_t ui_capability_input_bind(ui_host_t *host,const ui_cap_input_binding_desc_t *d,uint64_t *id)
{
    registry_t *r;ui_binding_t *b;input_binding_t *i;ui_cap_ui_binding_desc_t action={0};ui_web_view_t *view=NULL;void *owner=NULL;
    char endpoint[80];ui_status_t s=registry(host,&r);size_t bytes;const char *node;
    if(id)*id=0;if(s)return s;
    if(!d||!id||d->size<sizeof(*d)||d->version!=1||d->kind<UI_CAP_INPUT_SHORTCUT||d->kind>UI_CAP_INPUT_NATIVE_BUTTON||
      d->modifiers>7||(d->trigger!=UI_INPUT_POINTER_UP&&d->trigger!=UI_INPUT_KEY_DOWN)||
      (d->trigger==UI_INPUT_KEY_DOWN?(!d->key||d->key>255||d->pointer_button):(!d->pointer_button||d->pointer_button>3||d->key)))return UI_STATUS_INVALID_ARGUMENT;
    if((d->kind==UI_CAP_INPUT_NATIVE_BUTTON||d->kind==UI_CAP_INPUT_TOOLBAR)&&
      (d->trigger==UI_INPUT_POINTER_UP?d->pointer_button!=1:(d->key!=13&&d->key!=32)))return UI_STATUS_UNSUPPORTED;
    if(d->purpose==UI_CAP_UI_RECONCILE&&d->collect)return UI_STATUS_INVALID_ARGUMENT;
    node=d->control_id?d->control_id:"";if(!ui_cap_text(node,255,1))return UI_STATUS_INVALID_ARGUMENT;
    if(d->kind==UI_CAP_INPUT_SHORTCUT){if(d->view||d->component||d->surface||d->native_button||*node||d->trigger!=UI_INPUT_KEY_DOWN)return UI_STATUS_INVALID_ARGUMENT;}
    else if(d->kind==UI_CAP_INPUT_TOOLBAR){ui_toolbar_item_entry_t *t;if(d->view||d->component||d->surface||d->native_button||!*node)return UI_STATUS_INVALID_ARGUMENT;
      for(t=host->toolbar_items;t&&strcmp(t->id,node);t=t->next){}if(!t||!d->capability_id||strcmp(t->command_id,d->capability_id))return UI_STATUS_NOT_FOUND;}
    else if(d->kind==UI_CAP_INPUT_LIGHT_CONTROL||d->kind==UI_CAP_INPUT_COMPONENT){
      if(d->surface||d->native_button||(d->kind==UI_CAP_INPUT_COMPONENT?(d->view||!d->component):(!d->view||d->component))||!*node)return UI_STATUS_INVALID_ARGUMENT;
      if(d->kind==UI_CAP_INPUT_COMPONENT&&ui_components_form_bound(d->component))return UI_STATUS_ALREADY_EXISTS;
      view=d->kind==UI_CAP_INPUT_COMPONENT?ui_components_view(d->component):d->view;
      {ui_web_view_t *v;for(v=host->web_views;v&&v!=view;v=v->host_next){}if(!v)return UI_STATUS_NOT_FOUND;}
      if(!view->backend->trusted_light)return UI_STATUS_UNSUPPORTED;owner=d->kind==UI_CAP_INPUT_COMPONENT?(void *)d->component:(void *)view;
    }else if(d->kind==UI_CAP_INPUT_SURFACE){ui_surface_t *v;
      if(d->view||d->component||d->native_button||!d->surface||*node)return UI_STATUS_INVALID_ARGUMENT;
      for(v=host->surfaces;v&&v!=d->surface;v=v->next){}if(!v)return UI_STATUS_NOT_FOUND;owner=v;
    }else{
      if(d->view||d->component||d->surface||!d->native_button||*node)return UI_STATUS_INVALID_ARGUMENT;
#ifdef _WIN32
      {wchar_t name[32];DWORD process=0;HWND hwnd=d->native_button;
       if(GetWindowThreadProcessId(hwnd,&process)!=GetCurrentThreadId()||process!=GetCurrentProcessId()||!GetClassNameW(hwnd,name,32)||wcscmp(name,L"Button")||
         !host->native_parent||!IsChild((HWND)host->native_parent,hwnd))return UI_STATUS_UNSUPPORTED;owner=hwnd;}
#else
      return UI_STATUS_UNSUPPORTED;
#endif
    }
    for(b=r->bindings;b;b=b->next)if(b->input&&b->input->desc.kind==d->kind&&b->input->owner==owner&&!strcmp(b->input->control,node)&&
      b->input->desc.trigger==d->trigger&&b->input->desc.key==d->key&&b->input->desc.modifiers==d->modifiers&&b->input->desc.pointer_button==d->pointer_button)return UI_STATUS_ALREADY_EXISTS;
    if(d->kind==UI_CAP_INPUT_COMPONENT){ui_input_event_t event={0};event.kind=d->trigger;event.key_code=d->key;event.pointer_button=d->pointer_button;event.modifiers=d->modifiers;
      for(b=r->bindings;b;b=b->next)if(b->input&&b->input->owner==owner&&b->input->selector&&selector_match(b->input,node,&event))return UI_STATUS_ALREADY_EXISTS;}
    bytes=sizeof(*i)+strlen(node)+1;if(bytes>UI_CAPABILITY_CATALOG_BYTES-r->catalog_bytes)return UI_STATUS_LIMIT_EXCEEDED;
    i=calloc(1,sizeof(*i));if(!i)return UI_STATUS_OUT_OF_MEMORY;i->desc=*d;i->host=host;i->owner=owner;i->view=view;i->control=ui_strdup(node);i->bytes=bytes;
    if(!i->control){free(i);return UI_STATUS_OUT_OF_MEMORY;}
    snprintf(endpoint,sizeof(endpoint),"@input:%llu",(unsigned long long)(r->binding_sequence+1));
    action.size=sizeof(action);action.version=1;action.capability_id=d->capability_id;action.resource=d->resource;action.menu_item_id=endpoint;
    action.purpose=d->purpose;action.reconcile_call_id=d->reconcile_call_id;action.collect=d->purpose==UI_CAP_UI_RECONCILE?NULL:input_collect;action.observe=input_observe;action.user_data=i;
    r->catalog_bytes+=bytes;s=bind_ui_common(host,&action,id,NULL,input_cleanup,1);
    if(s){r->catalog_bytes-=bytes;input_cleanup(i);return s;}
    b=find_binding(r,*id);b->input=i;i->binding=b;b->bytes+=bytes;i->desc.capability_id=b->desc.capability_id;i->desc.control_id=i->control;
    if(d->kind==UI_CAP_INPUT_COMPONENT)b->component=d->component;
#ifdef _WIN32
    if(d->kind==UI_CAP_INPUT_NATIVE_BUTTON&&(!SetWindowSubclass((HWND)owner,input_button_proc,(UINT_PTR)b->id,(DWORD_PTR)i))){
      (void)ui_capability_ui_begin_unbind(host,*id);(void)ui_capability_ui_finish_unbind(host,*id);*id=0;return UI_STATUS_PLATFORM_ERROR;}
#endif
    return UI_STATUS_OK;
}

static int input_surface_valid(void *data)
{ui_surface_t *s=data;return s->visible&&s->host->app_active&&!s->host->dispatch_blocked&&!s->host->modal_component;}
int ui_inputs_surface(ui_surface_t *surface,const ui_input_event_t *event,int eligible)
{
    ui_host_t *host=surface->host;int qualified;int handled=0;ui_capabilities_human_invalidate(host,surface);qualified=ui_inputs_chain(&surface->input_chain,event,1,eligible);
    if(event->kind==UI_INPUT_KEY_DOWN&&ui_inputs_has(host,UI_CAP_INPUT_SHORTCUT,NULL,"",event)){
      if(qualified){ui_capabilities_human_begin(host,surface);ui_capabilities_human_guard(host,input_surface_valid,surface);}
      handled=ui_inputs_route(host,UI_CAP_INPUT_SHORTCUT,NULL,"",event,surface);
    }else if(ui_inputs_has(host,UI_CAP_INPUT_SURFACE,surface,"",event)){
      if(qualified){ui_capabilities_human_begin(host,surface);ui_capabilities_human_guard(host,input_surface_valid,surface);}
      handled=ui_inputs_route(host,UI_CAP_INPUT_SURFACE,surface,"",event,surface);
    }
    if(qualified&&handled)ui_capabilities_human_end(host);return handled;
}

ui_status_t ui_inputs_origin_host(ui_host_t *host,int add)
{
 registry_t *r;ui_status_t s=registry(host,&r);size_t bytes=64u;if(s)return s;
 if(add){if(bytes>UI_CAPABILITY_CATALOG_BYTES-r->catalog_bytes)return UI_STATUS_LIMIT_EXCEEDED;r->catalog_bytes+=bytes;}
 else r->catalog_bytes-=bytes;return UI_STATUS_OK;
}
typedef struct forward_guard {registry_t *source;uint64_t sequence;int (*valid)(void *);void *valid_data;int (*extra)(void *);void *extra_data;} forward_guard_t;
static int forward_valid(void *data)
{forward_guard_t *g=data;return g->source->human_sequence==g->sequence&&!is_closing(g->source)&&(!g->valid||g->valid(g->valid_data))&&(!g->extra||g->extra(g->extra_data));}
int ui_inputs_forward(ui_web_view_t *view,ui_host_t *dest,ui_cap_input_kind_t kind,const char *node,const ui_input_event_t *event,int (*extra)(void *),void *data)
{
 registry_t *source=view->host->capabilities;forward_guard_t g;int routed;
 if(!source||!source->human_active||source->human_owner!=view->user_data||!ui_inputs_has(dest,kind,NULL,node,event))return 0;
 g.source=source;g.sequence=source->human_sequence;g.valid=source->human_valid;g.valid_data=source->human_valid_data;g.extra=extra;g.extra_data=data;
 if(!forward_valid(&g))return 0;source->human_active=0;
 ++view->host->capability_ui_callbacks;++view->host->assistant_callbacks;ui_dispatch_enter(view->host);
 ui_capabilities_human_begin(dest,view->user_data);ui_capabilities_human_guard(dest,forward_valid,&g);
 routed=ui_inputs_route(dest,kind,NULL,node,event,view->user_data);ui_capabilities_human_end(dest);
 --view->host->capability_ui_callbacks;--view->host->assistant_callbacks;ui_dispatch_leave(view->host);return routed;
}

int ui_inputs_component_bound(ui_host_t *host,ui_component_t *c)
{registry_t *r=host?host->capabilities:NULL;ui_binding_t *b;if(!r)return 0;
 for(b=r->bindings;b;b=b->next)if(b->input&&b->input->desc.kind==UI_CAP_INPUT_COMPONENT&&b->input->owner==c)return 1;return 0;}

/* Service host actions consume the same private qualified origin, not a page
 * message. This function is private and never a DLL export. */
int ui_inputs_consume_action(ui_host_t *host,void *owner)
{registry_t *r=host?host->capabilities:NULL;int valid;if(!r||!r->human_active||r->human_owner!=owner)return 0;valid=!is_closing(r)&&(!r->human_valid||r->human_valid(r->human_valid_data));r->human_active=0;return valid;}
