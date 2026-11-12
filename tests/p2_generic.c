#include "ui_internal.h"
#include "../examples/generic_components/app.c"
#include "p2_example_logic.h"
static void checkbox(sample_t *s,int qualified)
{ui_web_view_t *view=ui_components_view(s->table);ui_rect_t rect={0};ui_input_event_t e={0};CHECK(ui_web_view_get_element_rect(view,"check-9007199254741000-checked",&rect)==UI_STATUS_OK);e.size=sizeof(e);e.pointer_button=1;e.x=rect.x+rect.width/2;e.y=rect.y+rect.height/2;e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(view,&e,qualified);e.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(view,&e,qualified);}
static void tree_expand(sample_t *s)
{ui_web_view_t *view=ui_components_view(s->tree);ui_rect_t rect={0};ui_input_event_t e={0};ui_element_presentation_t p={0};CHECK(ui_web_view_get_element_rect(view,"tree-label-1",&rect)==UI_STATUS_OK);e.size=sizeof(e);e.pointer_button=1;e.x=rect.x+5;e.y=rect.y+rect.height/2;e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(view,&e,1);e.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(view,&e,1);p.size=sizeof(p);CHECK(ui_component_get_presentation(s->tree,"tree-label-1001",&p)==UI_STATUS_OK&&p.visible);}
static void context_action(sample_t *s)
{ui_web_view_t *view=ui_components_view(s->tree);ui_rect_t rect={0};ui_input_event_t e={0};CHECK(ui_web_view_get_element_rect(view,"tree-label-1001",&rect)==UI_STATUS_OK);e.size=sizeof(e);e.pointer_button=2;e.x=rect.x+8;e.y=rect.y+rect.height/2;e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(view,&e,1);e.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(view,&e,1);CHECK(s->context.host->menu_popup!=NULL);(void)ui_host_close_menu(s->context.host);}
/* Exercise the application's real bindings/collectors after private fixture
 * admission. This deliberately does not classify a Windows hardware event. */
static void alias_case(sample_t *s,const char *item,unsigned operation,int toolbar,unsigned serial)
{
 p2_cap_t *cap=p2_find(&s->capabilities,commands[operation]);p2_resource_t *resource=&s->capabilities.resources[cap->spec->resource];
 uint64_t previous=resource->last_call,document=s->revision;unsigned image=s->version;char text[80],fallback[256];ui_cap_result_v2_t result={0};ui_status_t status;
 (void)ui_component_close_dialog(s->dialog);(void)ui_host_close_menu(s->context.host);
 ui_capabilities_human_begin(s->context.host,ui_components_view(s->form)->user_data);CHECK(ui_components_form_action(s->form,"conflict-reload")==UI_STATUS_OK);ui_capabilities_human_end(s->context.host);
 snprintf(text,sizeof(text),"actual alias %u",serial);ui_components_form_draft(s->form,"name",text);
 {uint64_t row=9007199254741001ULL,tree=1;CHECK(ui_component_set_selection(s->table,&row,1)==UI_STATUS_OK);CHECK(ui_component_set_selection(s->tree,&tree,1)==UI_STATUS_OK);}
 ui_capabilities_human_begin(s->context.host,s);
 if(toolbar){ui_input_event_t event={0};event.size=sizeof(event);event.kind=UI_INPUT_POINTER_UP;event.pointer_button=1;CHECK(ui_inputs_route(s->context.host,UI_CAP_INPUT_TOOLBAR,NULL,item,&event,s));status=UI_STATUS_OK;}
 else status=ui_capabilities_ui_action(s->context.host,item,s);
 ui_capabilities_human_end(s->context.host);CHECK(status==UI_STATUS_OK);
 CHECK(resource->last_call!=previous);result.size=sizeof(result);result.version=2;CHECK(ui_capability_get_result_v2(s->context.host,resource->last_call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_OK&&result.output_json);if(!result.output_json)return;
 if(operation==0)CHECK(s->context.host->modal_component==s->dialog&&strstr(result.output_json,"\"dialog_open\":true"));
 else if(operation==1)CHECK(s->version==image+1&&strstr(result.output_json,"\"image_version\":"));
 else if(operation==2)CHECK(!strcmp(s->name,text)&&s->revision==document+1&&strstr(result.output_json,text));
 else if(operation==3)CHECK(!strcmp(sample_value(s,9007199254741001ULL,"name",fallback),text)&&s->revision==document+1&&strstr(result.output_json,text));
 else if(operation==4)CHECK(strstr(result.output_json,"\"table_selected\":\"9007199254741001\""));
 else if(operation==5)CHECK(!strcmp(sample_value(s,1,"",fallback),text)&&s->revision==document+1&&strstr(result.output_json,text));
 else if(operation==6)CHECK(s->context.host->menu_popup!=NULL);
 else CHECK(s->context.host->modal_component==NULL&&strstr(result.output_json,"\"dialog_open\":false"));
 if(operation!=2&&operation!=3&&operation!=5)CHECK(s->revision==document);
 printf("SIM binding %s => %s actual outcome=%d revision=%llu\n",item,cap->spec->id,(int)result.outcome,(unsigned long long)s->revision);
 (void)ui_component_close_dialog(s->dialog);(void)ui_host_close_menu(s->context.host);p2_poll(&s->context,&s->capabilities);
}
static void aliases(sample_t *s)
{
 unsigned serial=1;for(unsigned i=0;i<8;++i)alias_case(s,commands[i],i,0,serial++);
 alias_case(s,"node.dialog",0,0,serial++);alias_case(s,"node.refresh",1,0,serial++);
 for(unsigned i=0;i<2;++i)alias_case(s,commands[i],i,1,serial++);
#ifdef UI_SAMPLE_API4
 for(unsigned i=0;i<7;++i){char item[40];snprintf(item,sizeof(item),"feature.menu.%u",i);alias_case(s,item,i,0,serial++);}
 for(unsigned i=0;i<13;++i){char item[40];snprintf(item,sizeof(item),"feature.tool.%02u",i);alias_case(s,item,i%8,1,serial++);}
#endif
 for(unsigned i=0;i<
#ifdef UI_SAMPLE_API4
 2
#else
 1
#endif
 ;++i){uint64_t revision=s->revision;unsigned image=s->version;ui_input_event_t event={0};event.size=sizeof(event);event.kind=UI_INPUT_KEY_DOWN;event.key_code=i?'O':'D';event.modifiers=UI_INPUT_MODIFIER_CONTROL;
   ui_capabilities_human_begin(s->context.host,s);CHECK(ui_inputs_route(s->context.host,UI_CAP_INPUT_SHORTCUT,NULL,"",&event,s));ui_capabilities_human_end(s->context.host);
   CHECK(s->revision==revision);if(i)CHECK(s->version==image+1);else CHECK(s->context.host->modal_component==s->dialog);(void)ui_component_close_dialog(s->dialog);p2_poll(&s->context,&s->capabilities);}
}
int main(void)
{
 ui_host_config_t config={0};ui_app_context_t context={0};ui_assistant_config_t assistant={0};sample_t *s;void *app=NULL;char fallback[256];uint64_t revision;ui_cap_form_state_t state={0};ui_cell_t cell={0};
 config.size=sizeof(config);config.api_version=10;context.size=sizeof(context);context.host=ui_host_create(&config);CHECK(context.host!=NULL);if(!context.host)return 1;
 {ui_status_t st=create(&context,&app,&assistant);CHECK(st==UI_STATUS_OK);if(st){ui_host_destroy(context.host);if(app)destroy(app);return 1;}}s=app;CHECK(!strcmp(s->name,"Untitled"));
 CHECK(ui_component_mount_offscreen(s->table,900,400,96)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(s->tree,300,400,96)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(s->form,320,480,96)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(s->dialog,320,480,96)==UI_STATUS_OK);CHECK(sample_bindings(s)==UI_STATUS_OK);
 checkbox(s,0);CHECK(s->revision==1&&!strcmp(sample_value(s,9007199254741000ULL,"checked",fallback),"false"));checkbox(s,1);CHECK(s->revision==2&&!strcmp(sample_value(s,9007199254741000ULL,"checked",fallback),"true"));
 {ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_component_get_presentation(s->table,"check-9007199254741000-checked",&p)==UI_STATUS_OK&&p.text_utf8&&strstr(p.text_utf8,"☑"));}p2_poll(&context,&s->capabilities);
 tree_expand(s);p2_poll(&context,&s->capabilities);context_action(s);p2_poll(&context,&s->capabilities);
 CHECK(logic_invoke(&s->capabilities,"demo.edit","{\"id\":\"9007199254741001\",\"column\":\"name\",\"text\":\"real table model\"}")==UI_CAP2_OK);CHECK(!strcmp(sample_value(s,9007199254741001ULL,"name",fallback),"real table model"));CHECK(strstr(logic_result,"real table model")!=NULL);
 CHECK(logic_invoke(&s->capabilities,"demo.rename","{\"id\":\"1\",\"text\":\"renamed root\"}")==UI_CAP2_OK);CHECK(!strcmp(sample_value(s,1,"",fallback),"renamed root"));{ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_component_get_presentation(s->tree,"tree-label-1",&p)==UI_STATUS_OK&&strstr(p.text_utf8,"renamed root"));}
 ui_components_form_draft(s->form,"name","private form draft");ui_components_form_draft(s->dialog,"name","private dialog draft");CHECK(logic_invoke(&s->capabilities,"demo.submit","{\"fields\":{\"name\":\"shared actual name\",\"size\":2.5,\"enabled\":false,\"mode\":\"手动\",\"notes\":\"actual notes\"}}")==UI_CAP2_OK);
 CHECK(!strcmp(s->name,"shared actual name")&&s->size==2.5&&!s->enabled&&!strcmp(s->notes,"actual notes"));state.size=sizeof(state);state.version=1;CHECK(ui_capability_form_get_state(s->dialog,&state)==UI_STATUS_OK&&state.conflict_fields==1);cell.size=sizeof(cell);CHECK(ui_component_get_field(s->dialog,"name",&cell)==UI_STATUS_OK&&!strcmp(cell.text,"private dialog draft"));
 {uint64_t before=s->revision;ui_capabilities_human_begin(context.host,s);CHECK(ui_capabilities_ui_action(context.host,"demo.submit",s)==UI_STATUS_VALIDATION_FAILED);ui_capabilities_human_end(context.host);CHECK(s->revision==before&&!strcmp(s->name,"shared actual name"));}
 revision=s->revision;CHECK(logic_invoke(&s->capabilities,"demo.submit","{\"fields\":{\"size\":0}}")==UI_CAP2_FAILED);CHECK(s->size==2.5&&s->revision==revision);CHECK(logic_invoke(&s->capabilities,"demo.edit","{\"id\":\"999\",\"column\":\"name\",\"text\":\"wrong resource object\"}")==UI_CAP2_FAILED&&s->revision==revision);
 {char json[1024],text[601];for(unsigned i=0;i<200;++i)memcpy(text+i*3,"中",3);text[600]=0;snprintf(json,sizeof(json),"{\"id\":\"9007199254741001\",\"column\":\"name\",\"text\":\"%s\"}",text);CHECK(logic_invoke(&s->capabilities,"demo.edit",json)==UI_CAP2_FAILED);CHECK(s->revision==revision&&!strcmp(sample_value(s,9007199254741001ULL,"name",fallback),"real table model"));}
 CHECK(logic_invoke(&s->capabilities,"demo.edit","{\"id\":\"9007199254741001\",\"column\":\"name\",\"text\":\"truncated\\u0000suffix\"}")==UI_CAP2_FAILED);CHECK(s->revision==revision&&!strcmp(sample_value(s,9007199254741001ULL,"name",fallback),"real table model"));
 CHECK(logic_invoke(&s->capabilities,"demo.select","{\"component\":\"table\",\"id\":\"9007199254741001\"}")==UI_CAP2_OK);CHECK(strstr(logic_result,"9007199254741001")!=NULL&&s->revision==revision);
 {unsigned version=s->version;CHECK(logic_invoke(&s->capabilities,"demo.refresh","{}")==UI_CAP2_OK);CHECK(s->version==version+1&&s->revision==revision&&strstr(logic_result,"\"image_version\":2,"));}
 {ui_input_event_t e={0};ui_component_state_t selected={0};ui_web_view_t *view=ui_components_view(s->table);e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code=40;ui_capabilities_human_begin(context.host,view->user_data);CHECK(ui_inputs_route(context.host,UI_CAP_INPUT_COMPONENT,s->table,"cell-9007199254741001-name",&e,view->user_data));ui_capabilities_human_end(context.host);selected.size=sizeof(selected);CHECK(ui_component_get_state(s->table,&selected)==UI_STATUS_OK&&selected.selected_id==9007199254741002ULL);p2_poll(&context,&s->capabilities);}
#ifdef UI_SAMPLE_API4
 CHECK(logic_invoke(&s->capabilities,"demo.close_refuse","{}")==UI_CAP2_OK);CHECK(close_app(s,UI_APP_CLOSE_TAB)==UI_APP_CLOSE_REFUSE);CHECK(logic_invoke(&s->capabilities,"demo.close_allow","{}")==UI_CAP2_OK&&!s->refuse_close);
#endif
 (void)ui_host_invoke(context.host,"demo.edit","{\"id\":\"9007199254741001\",\"column\":\"name\",\"text\":\"bypass\"}","human");CHECK(!strcmp(sample_value(s,9007199254741001ULL,"name",fallback),"real table model"));
 aliases(s);
 unmount(s);ui_host_destroy(context.host);destroy(s);printf("P2 shared example %s actual business/checkbox (not physical): checks=%u failures=%u\n",
#ifdef UI_SAMPLE_API4
 "framework_features",
#else
 "generic_components",
#endif
 checks,failures);return failures?1:0;
}

