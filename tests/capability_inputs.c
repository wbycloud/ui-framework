#include "ui_internal.h"
#include "ui_framework/capability_entry.h"
#ifdef UI_CAP_INPUT_LIGHT
#include "ui_framework/light_web.h"
#include "ui_framework/native.h"
#include <windows.h>
#include <commctrl.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
typedef struct model {ui_host_t *host;unsigned value,rev,runs;uint64_t call;int async,invalid,modal;ui_status_t status;char code[64],captured_value[128];ui_input_event_t captured;ui_web_view_t *destroy_view;ui_web_backend_t *destroy_backend;} model_t;
static ui_status_t revision(char *out,size_t capacity,void *data){model_t *m=data;snprintf(out,capacity,"r%u",m->rev);return UI_STATUS_OK;}
static void complete(model_t *m)
{ui_cap_completion_v2_t c={0};c.size=sizeof(c);c.version=2;c.outcome=UI_CAP2_OK;c.effect=UI_CAP2_EFFECT_APPLIED;c.output_json=m->invalid?"{\"bad\":1}":"{}";++m->value;++m->rev;CHECK(ui_capability_complete_v2(m->host,m->call,&c)==UI_STATUS_OK);}
static ui_status_t business_check(const ui_cap_target_v2_t *t,const char *p,void *data)
{model_t *m=data;(void)t;(void)p;if(m->modal)m->host->modal_component=(ui_component_t *)m;return UI_STATUS_OK;}
static void execute(uint64_t id,const ui_cap_target_v2_t *t,const char *p,void *data)
{model_t *m=data;(void)p;CHECK(id==m->call);CHECK(t->resource.instance_id!=0);++m->runs;if(!m->async)complete(m);}
static ui_status_t reconcile(uint64_t id,const ui_cap_target_v2_t *t,const char *p,ui_cap_reconciliation_v2_t *proof,char *json,size_t capacity,size_t *length,void *data)
{model_t *m=data;(void)id;(void)t;(void)p;proof->resolution=UI_CAP2_RESOLVED_APPLIED;snprintf(proof->observed_revision,128,"r%u",m->rev);*length=(size_t)snprintf(json,capacity,"{}");return UI_STATUS_OK;}
static ui_status_t collect(const ui_cap_input_context_t *c,char *out,size_t capacity,size_t *length,void *data)
{model_t *m=data;CHECK(c->size==sizeof(*c)&&c->version==1&&c->invocation.target.version==2);CHECK(c->event.text_utf8==NULL);m->captured=c->event;
#ifdef UI_CAP_INPUT_LIGHT
 if(m->destroy_view){ui_web_view_destroy(m->destroy_view);ui_light_web_backend_destroy(m->destroy_backend);CHECK(m->host->web_views==m->destroy_view);}
#endif
 if(c->value_utf8)strcpy_s(m->captured_value,sizeof(m->captured_value),c->value_utf8);*length=(size_t)snprintf(out,capacity,"{}");return UI_STATUS_OK;}
static void observe(const ui_cap_ui_notice_t *n,void *data)
{model_t *m=data;if(n->stage==UI_CAP_UI_ADMITTED)m->call=n->call_id;m->status=n->status;strcpy_s(m->code,sizeof(m->code),n->code);}
static void register_model(model_t *m)
{
 ui_cap_resource_desc_t r={0};ui_capability_desc_v2_t d={0};CHECK(ui_capability_enable_read_entries(m->host)==UI_STATUS_OK);
 r.size=sizeof(r);r.version=1;r.id="document";r.kind=UI_CAP_DOCUMENT;r.get_revision=revision;r.user_data=m;CHECK(ui_capability_register_resource(m->host,&r)==UI_STATUS_OK);
 d.size=sizeof(d);d.version=2;d.capability_version=1;d.id="change";d.name_utf8="Change";d.description_utf8="";d.category="fixture";d.target_kind=UI_CAP_DOCUMENT;d.effect="write";
 d.input_schema_json=d.output_schema_json=d.reconcile_schema_json="{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";
 d.execute=execute;d.check=business_check;d.reconcile=reconcile;d.user_data=m;CHECK(ui_capability_register_v2(m->host,&d)==UI_STATUS_OK);
}
static void init(model_t *m){ui_host_config_t c={0};c.size=sizeof(c);c.api_version=10;m->host=ui_host_create(&c);CHECK(m->host!=NULL);register_model(m);}
static ui_cap_input_binding_desc_t descriptor(model_t *m,ui_cap_input_kind_t kind)
{ui_cap_input_binding_desc_t d={0};d.size=sizeof(d);d.version=1;d.kind=kind;d.capability_id="change";d.resource.size=sizeof(d.resource);d.resource.version=2;
 CHECK(ui_capability_resource_ref_v2(m->host,"document",&d.resource)==UI_STATUS_OK);d.purpose=UI_CAP_UI_INVOKE;d.collect=collect;d.observe=observe;d.user_data=m;
 d.trigger=kind==UI_CAP_INPUT_SHORTCUT?UI_INPUT_KEY_DOWN:UI_INPUT_POINTER_UP;if(kind==UI_CAP_INPUT_SHORTCUT){d.key='K';d.modifiers=UI_INPUT_MODIFIER_CONTROL;}else d.pointer_button=1;return d;}
static void forget(model_t *m){if(m->call){CHECK(ui_capability_forget_v2(m->host,m->call)==UI_STATUS_OK);m->call=0;}}
static void unbind(model_t *m,uint64_t id){CHECK(ui_capability_ui_begin_unbind(m->host,id)==UI_STATUS_OK);CHECK(ui_capability_ui_finish_unbind(m->host,id)==UI_STATUS_OK);}
static void origin_case(void)
{
 ui_input_chain_t c={0};ui_input_event_t e={0};int origin,device;e.size=sizeof(e);e.pointer_button=1;
 for(origin=0;origin<8;++origin)for(device=0;device<32;++device){CHECK(ui_inputs_origin_eligible(1,origin,device)==(origin==1&&(device==1||device==2||device==4||device==8||device==16)));CHECK(!ui_inputs_origin_eligible(0,origin,device));}
 e.kind=UI_INPUT_POINTER_DOWN;CHECK(!ui_inputs_chain(&c,&e,7,0));e.kind=UI_INPUT_POINTER_UP;CHECK(!ui_inputs_chain(&c,&e,7,1));
 e.kind=UI_INPUT_POINTER_DOWN;CHECK(!ui_inputs_chain(&c,&e,7,1));e.kind=UI_INPUT_POINTER_UP;CHECK(!ui_inputs_chain(&c,&e,7,0));
 e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_chain(&c,&e,7,1);e.kind=UI_INPUT_POINTER_UP;CHECK(!ui_inputs_chain(&c,&e,8,1));
 e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_chain(&c,&e,7,1);e.kind=UI_INPUT_POINTER_UP;e.pointer_button=2;CHECK(!ui_inputs_chain(&c,&e,7,1));
 e.pointer_button=1;e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_chain(&c,&e,7,1);e.kind=UI_INPUT_POINTER_UP;e.modifiers=1;CHECK(!ui_inputs_chain(&c,&e,7,1));
 e.modifiers=0;e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_chain(&c,&e,7,1);e.kind=UI_INPUT_CANCEL;(void)ui_inputs_chain(&c,&e,7,1);e.kind=UI_INPUT_POINTER_UP;CHECK(!ui_inputs_chain(&c,&e,7,1));
 e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_chain(&c,&e,7,1);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_inputs_chain(&c,&e,7,1));CHECK(!ui_inputs_chain(&c,&e,7,1));
}
static void shortcut_surface_case(void)
{
 model_t m={0};ui_cap_input_binding_desc_t d;ui_input_event_t e={0};ui_surface_desc_t sd={0};ui_surface_t *surface;uint64_t id,second;ui_cap_result_v2_t result={0};init(&m);
 d=descriptor(&m,UI_CAP_INPUT_SHORTCUT);CHECK(ui_capability_input_bind(m.host,&d,&id)==UI_STATUS_OK);CHECK(ui_capability_input_bind(m.host,&d,&second)==UI_STATUS_ALREADY_EXISTS&&second==0);
 e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code='K';e.modifiers=1;CHECK(ui_host_dispatch_shortcut(m.host,'K',1,0)==0&&m.runs==0);
 CHECK(ui_inputs_route(m.host,UI_CAP_INPUT_SHORTCUT,NULL,"",&e,&m)&&m.runs==0&&m.status==UI_STATUS_PERMISSION_DENIED);
 ui_capabilities_human_begin(m.host,&m);CHECK(ui_inputs_route(m.host,UI_CAP_INPUT_SHORTCUT,NULL,"",&e,&m));CHECK(m.runs==1&&m.value==1);
 CHECK(ui_inputs_route(m.host,UI_CAP_INPUT_SHORTCUT,NULL,"",&e,&m)&&m.runs==1);ui_capabilities_human_end(m.host);forget(&m);unbind(&m,id);
 sd.size=sizeof(sd);sd.id="canvas";sd.kind=UI_SURFACE_NATIVE;sd.visible=1;sd.rect.width=sd.rect.height=120;
#ifdef UI_CAP_INPUT_LIGHT
 m.host->native_parent=CreateWindowExW(0,L"STATIC",L"C2 canvas",WS_OVERLAPPED,-16000,-16000,160,160,NULL,NULL,GetModuleHandleW(NULL),NULL);
 surface=ui_surface_create(m.host,&sd);
#else
 /* Headless is a registry/chain fixture; real HWND surface is tested in Light. */
 surface=calloc(1,sizeof(*surface));if(surface){surface->host=m.host;surface->visible=1;surface->kind=UI_SURFACE_NATIVE;surface->id=ui_strdup("canvas");m.host->surfaces=surface;}
#endif
CHECK(surface!=NULL);if(!surface){ui_host_destroy(m.host);return;}
 d=descriptor(&m,UI_CAP_INPUT_SURFACE);d.surface=surface;CHECK(ui_capability_input_bind(m.host,&d,&id)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_DOWN;e.key_code=0;e.modifiers=0;e.pointer_button=1;e.x=14;e.y=29;
 CHECK(ui_surface_dispatch_input(surface,&e)!=UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_surface_dispatch_input(surface,&e)==UI_STATUS_PERMISSION_DENIED&&m.runs==1);
 e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_surface(surface,&e,1);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_inputs_surface(surface,&e,1));CHECK(m.runs==2&&m.value==2&&m.captured.x==14&&m.captured.y==29);forget(&m);
 m.modal=1;e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_surface(surface,&e,1);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_inputs_surface(surface,&e,1));CHECK(m.runs==2);m.host->modal_component=NULL;m.modal=0;forget(&m);
 m.async=1;e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_surface(surface,&e,1);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_inputs_surface(surface,&e,1)&&m.runs==3);
 CHECK(ui_capability_cancel_v2(m.host,m.call)==UI_STATUS_OK);result.size=sizeof(result);result.version=2;CHECK(ui_capability_get_result_v2(m.host,m.call,&result)==UI_STATUS_OK&&result.state==UI_CAP_RUNNING);
 unbind(&m,id);complete(&m);CHECK(ui_capability_get_result_v2(m.host,m.call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_OK);forget(&m);ui_surface_destroy(surface);
#ifdef UI_CAP_INPUT_LIGHT
 DestroyWindow((HWND)m.host->native_parent);
#endif
 ui_host_destroy(m.host);
}
static void unknown_input_case(void)
{
 model_t m={0};ui_cap_input_binding_desc_t d;ui_input_event_t e={0};uint64_t invoke,review;ui_cap_result_v2_t result={0};init(&m);m.invalid=1;
 d=descriptor(&m,UI_CAP_INPUT_SHORTCUT);CHECK(ui_capability_input_bind(m.host,&d,&invoke)==UI_STATUS_OK);
 e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code='K';e.modifiers=1;ui_capabilities_human_begin(m.host,&m);CHECK(ui_inputs_route(m.host,UI_CAP_INPUT_SHORTCUT,NULL,"",&e,&m));ui_capabilities_human_end(m.host);
 result.size=sizeof(result);result.version=2;CHECK(ui_capability_get_result_v2(m.host,m.call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_UNKNOWN);
 CHECK(ui_capability_forget_v2(m.host,m.call)==UI_STATUS_PENDING);d.purpose=UI_CAP_UI_RECONCILE;d.reconcile_call_id=m.call;d.key='R';
 CHECK(ui_capability_input_bind(m.host,&d,&review)==UI_STATUS_INVALID_ARGUMENT);d.collect=NULL;CHECK(ui_capability_input_bind(m.host,&d,&review)==UI_STATUS_OK);
 e.key_code='R';ui_capabilities_human_begin(m.host,&m);CHECK(ui_inputs_route(m.host,UI_CAP_INPUT_SHORTCUT,NULL,"",&e,&m));ui_capabilities_human_end(m.host);
 CHECK(ui_capability_get_result_v2(m.host,m.call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_UNKNOWN&&result.resolution==UI_CAP2_RESOLVED_APPLIED);CHECK(m.runs==1&&m.value==1);forget(&m);
 unbind(&m,invoke);unbind(&m,review);ui_host_destroy(m.host);
}
#ifdef UI_CAP_INPUT_LIGHT
static int native_commands,source_ok,source_origin,source_device;
static LRESULT CALLBACK test_parent_proc(HWND h,UINT msg,WPARAM wp,LPARAM lp)
{
 if(msg==WM_COMMAND){++native_commands;return 0;}
 if(msg==WM_APP+1){INPUT_MESSAGE_SOURCE s={0};source_ok=GetCurrentInputMessageSource(&s);source_origin=s.originId;source_device=s.deviceType;return 0;}
 return DefWindowProcW(h,msg,wp,lp);
}
static void native_controls_case(void)
{
 model_t m={0};WNDCLASSW c={0};HWND parent,button;ui_cap_input_binding_desc_t d;uint64_t id;ui_input_event_t e={0};int before;
 ui_toolbar_desc_t bar={0};ui_toolbar_item_desc_t tool={0};ui_native_shell_config_t shell_config={0};ui_native_shell_t *shell;
 init(&m);c.lpfnWndProc=test_parent_proc;c.hInstance=GetModuleHandleW(NULL);c.lpszClassName=L"C2CapabilityInputsTest";RegisterClassW(&c);
 parent=CreateWindowExW(WS_EX_TOOLWINDOW,c.lpszClassName,L"C2",WS_POPUP|WS_VISIBLE,-16000,-16000,240,160,NULL,NULL,c.hInstance,NULL);CHECK(parent!=NULL);m.host->native_parent=parent;
 button=CreateWindowExW(0,L"BUTTON",L"Action",WS_CHILD|WS_VISIBLE,0,70,90,25,parent,(HMENU)1,c.hInstance,NULL);CHECK(button!=NULL);
 d=descriptor(&m,UI_CAP_INPUT_NATIVE_BUTTON);d.native_button=button;CHECK(ui_capability_input_bind(m.host,&d,&id)==UI_STATUS_OK);
 before=native_commands;SendMessageW(button,BM_CLICK,0,0);SendMessageW(button,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(3,3));SendMessageW(button,WM_LBUTTONUP,0,MAKELPARAM(3,3));
 CHECK(m.runs==0&&native_commands==before);SendMessageW(parent,WM_APP+1,0,0);CHECK(!ui_inputs_origin_eligible(source_ok,source_origin,source_device));
 printf("Program message source: ok=%d origin=%d device=%d (not physical input)\n",source_ok,source_origin,source_device);
 e.size=sizeof(e);e.kind=UI_INPUT_POINTER_UP;e.pointer_button=1;ui_capabilities_human_begin(m.host,button);
 CHECK(ui_inputs_route(m.host,UI_CAP_INPUT_NATIVE_BUTTON,button,"",&e,button));ui_capabilities_human_end(m.host);CHECK(m.runs==1&&m.value==1);forget(&m);
 unbind(&m,id);SendMessageW(button,BM_CLICK,0,0);CHECK(native_commands==before+1);
 bar.size=sizeof(bar);bar.id="bar";bar.title="Bar";bar.visible=1;CHECK(ui_host_register_toolbar(m.host,&bar)==UI_STATUS_OK);
 tool.size=sizeof(tool);tool.id="tool";tool.toolbar_id="bar";tool.title="Tool";tool.command_id="change";CHECK(ui_host_register_toolbar_item(m.host,&tool)==UI_STATUS_OK);
 d=descriptor(&m,UI_CAP_INPUT_TOOLBAR);d.control_id="tool";CHECK(ui_capability_input_bind(m.host,&d,&id)==UI_STATUS_OK);
 shell_config.size=sizeof(shell_config);shell_config.host=m.host;shell_config.native_parent=parent;shell=ui_native_shell_create(&shell_config);CHECK(shell!=NULL);
 CHECK(ui_inputs_route(m.host,UI_CAP_INPUT_TOOLBAR,NULL,"tool",&e,&m)&&m.runs==1);
 ui_capabilities_human_begin(m.host,&m);CHECK(ui_inputs_route(m.host,UI_CAP_INPUT_TOOLBAR,NULL,"tool",&e,&m));ui_capabilities_human_end(m.host);CHECK(m.runs==2&&m.value==2);forget(&m);unbind(&m,id);
 if(shell)ui_native_shell_destroy(shell);DestroyWindow(parent);ui_host_destroy(m.host);
}
static void row_source(ui_component_t *component,const ui_component_query_t *q,void *data)
{
 ui_component_batch_t batch={0};ui_row_t row={0};ui_cell_t cell={0};(void)data;if(q->kind!=UI_QUERY_ROWS)return;
 row.size=sizeof(row);row.id=7;row.title="Stable row";cell.size=sizeof(cell);cell.column_id="name";cell.kind=UI_VALUE_TEXT;cell.text="value";row.cells=&cell;row.cell_count=1;
 batch.size=sizeof(batch);batch.component_generation=q->component_generation;batch.request_id=q->request_id;batch.parent_id=q->parent_id;batch.first=q->first;batch.total_count=1;
 if(q->first==0){batch.rows=&row;batch.row_count=1;}CHECK(ui_component_submit(component,&batch)==UI_STATUS_OK);
}
static void component_inputs_case(void)
{
 unsigned kind;for(kind=1;kind<=3;++kind){model_t m={0};ui_component_t *component=NULL;ui_component_desc_t desc={0};ui_column_desc_t column={0};ui_cap_input_binding_desc_t binding;
 ui_web_view_t *view;ui_rect_t rect={0};uint64_t id;ui_input_event_t event={0};const char *control=kind==2?"cell-7-name":"tree-label-7";init(&m);
 column.size=sizeof(column);column.id="name";column.title="Name";column.kind=UI_VALUE_TEXT;column.width=180;
 desc.size=sizeof(desc);desc.id="rows";desc.kind=(ui_component_kind_t)kind;desc.columns=&column;desc.column_count=1;desc.source=row_source;
 CHECK(ui_component_register(m.host,&desc,&component)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(component,320,230,96)==UI_STATUS_OK);view=ui_components_view(component);CHECK(view!=NULL);
 binding=descriptor(&m,UI_CAP_INPUT_COMPONENT);binding.component=component;binding.control_id=control;CHECK(ui_capability_input_bind(m.host,&binding,&id)==UI_STATUS_OK);
 CHECK(ui_web_view_get_element_rect(view,control,&rect)==UI_STATUS_OK&&rect.width>0);event.size=sizeof(event);event.pointer_button=1;event.x=rect.x+4;event.y=rect.y+4;
 event.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(view,&event,1);event.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(view,&event,1);CHECK(m.runs==1&&m.value==1);forget(&m);
 CHECK(ui_component_unregister(component)==UI_STATUS_OK);CHECK(ui_capability_ui_finish_unbind(m.host,id)==UI_STATUS_OK);ui_host_destroy(m.host);
 }
}
static void light_case(void)
{
 model_t m={0};ui_light_web_config_t c={0};ui_web_backend_t *backend;ui_web_view_t *view;ui_cap_input_binding_desc_t d;ui_input_event_t e={0};uint64_t id;ui_rect_t p={0};init(&m);
 c.size=sizeof(c);backend=ui_light_web_backend_create(&c);CHECK(backend!=NULL);view=ui_web_view_create(m.host,backend);CHECK(view!=NULL);
 CHECK(ui_web_view_resize(view,180,140,96)==UI_STATUS_OK);CHECK(ui_web_view_load_html(view,"<html><body><button id='submit'>Do</button></body></html>")==UI_STATUS_OK);
 d=descriptor(&m,UI_CAP_INPUT_LIGHT_CONTROL);d.view=view;d.control_id="submit";CHECK(ui_capability_input_bind(m.host,&d,&id)==UI_STATUS_OK);
 e.size=sizeof(e);e.kind=UI_INPUT_POINTER_UP;e.pointer_button=1;CHECK(ui_inputs_light(m.host,view->user_data,"submit",&e,NULL)&&m.runs==0);
 ui_capabilities_human_begin(m.host,view->user_data);CHECK(ui_inputs_light(m.host,view->user_data,"submit",&e,NULL)&&m.runs==1);ui_capabilities_human_end(m.host);forget(&m);
 CHECK(ui_web_view_get_element_rect(view,"submit",&p)==UI_STATUS_OK);e.x=p.x+2;e.y=p.y+2;e.kind=UI_INPUT_POINTER_DOWN;
 (void)ui_web_view_dispatch_input(view,&e);e.kind=UI_INPUT_POINTER_UP;(void)ui_web_view_dispatch_input(view,&e);CHECK(m.runs==1);

 e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(view,&e,0);e.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(view,&e,1);CHECK(m.runs==1);
 e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(view,&e,1);e.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(view,&e,0);CHECK(m.runs==1);
 m.destroy_view=view;m.destroy_backend=backend;
 e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(view,&e,1);e.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(view,&e,1);m.destroy_view=NULL;CHECK(m.runs==2&&m.value==2);forget(&m);
 e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(view,&e,1);
 CHECK(ui_web_view_load_html(view,"<html><body><button id='submit'>Reopened DOM</button><input id='edit' value='unsubmitted'></body></html>")==UI_STATUS_OK);
 e.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(view,&e,1);CHECK(m.runs==2);
 {ui_cap_input_binding_desc_t key=descriptor(&m,UI_CAP_INPUT_LIGHT_CONTROL);uint64_t key_id;
  key.view=view;key.control_id="edit";key.trigger=UI_INPUT_KEY_DOWN;key.key=13;key.pointer_button=0;
  CHECK(ui_capability_input_bind(m.host,&key,&key_id)==UI_STATUS_OK);CHECK(ui_web_view_get_element_rect(view,"edit",&p)==UI_STATUS_OK);
  e.kind=UI_INPUT_POINTER_DOWN;e.x=p.x+2;e.y=p.y+2;(void)ui_web_view_dispatch_input(view,&e);
  e.kind=UI_INPUT_KEY_DOWN;e.key_code=13;e.pointer_button=0;(void)ui_inputs_fixture_light(view,&e,1);
  CHECK(m.runs==3&&!strcmp(m.captured_value,"unsubmitted"));forget(&m);unbind(&m,key_id);}
 ui_web_view_destroy(view);CHECK(ui_capability_ui_finish_unbind(m.host,id)==UI_STATUS_OK);ui_light_web_backend_destroy(backend);ui_host_destroy(m.host);
}
#endif
#ifndef UI_CAP_INPUT_WORKSPACE
int main(void){origin_case();shortcut_surface_case();unknown_input_case();
#ifdef UI_CAP_INPUT_LIGHT
light_case();native_controls_case();component_inputs_case();
#endif
printf("C2-C inputs checks=%u failures=%u\n",checks,failures);return failures?1:0;}
#endif
