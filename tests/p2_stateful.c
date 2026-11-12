/* Real application business implementation + private admission logic, NOT
 * physical input. DLL/package loading is independently checked in p2_apps. */
#include "ui_internal.h"
#include "../examples/stateful_components/app.c"
static unsigned checks,failures;
static unsigned legacy_sort_replies;
static void legacy_result(const ui_result_t *result,void *data)
{(void)data;if(!strcmp(result->command_id,"test.sort"))++legacy_sort_replies;}
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static const char *parameters;
static uint64_t last_call;static ui_status_t last_status;
static ui_status_t collect_test(const ui_cap_ui_context_t *c,char *out,size_t capacity,size_t *length,void *data)
{(void)c;(void)data;*length=strlen(parameters);if(*length>=capacity)return UI_STATUS_LIMIT_EXCEEDED;memcpy(out,parameters,*length+1);return UI_STATUS_OK;}
static void observe_test(const ui_cap_ui_notice_t *n,void *data)
{last_status=n->status;if(n->call_id)last_call=n->call_id;p2_notice(n,data);}
static ui_cap_result_v2_t invoke_test(state_app_t *s,const char *id,const char *params)
{
 ui_cap_ui_binding_desc_t d={0};ui_cap_result_v2_t result={0};ui_menu_item_desc_t menu={0};uint64_t binding;char item[128];p2_cap_t *cap=p2_find(&s->capabilities,id);
 snprintf(item,sizeof(item),"test.%s",id);menu.size=sizeof(menu);menu.id=item;menu.title=id;menu.command_id=id;menu.menu_path="Test";
 {ui_status_t st=ui_host_register_menu_item(s->scene->context.host,&menu);CHECK(st==UI_STATUS_OK||st==UI_STATUS_ALREADY_EXISTS);}
 d.size=sizeof(d);d.version=1;d.capability_id=id;d.menu_item_id=item;d.resource=s->capabilities.resources[cap->spec->resource].ref;d.purpose=UI_CAP_UI_INVOKE;d.collect=collect_test;d.observe=observe_test;d.user_data=cap;
 CHECK(ui_capability_ui_bind(s->scene->context.host,&d,&binding)==UI_STATUS_OK);parameters=params;last_call=0;last_status=UI_STATUS_OK;
 ui_capabilities_human_begin(s->scene->context.host,s);(void)ui_capabilities_ui_action(s->scene->context.host,item,s);ui_capabilities_human_end(s->scene->context.host);
 {DWORD start=GetTickCount();while(s->io&&GetTickCount()-start<5000){state_tick(s);SwitchToThread();}CHECK(!s->io);}
 result.size=sizeof(result);result.version=2;if(last_call){CHECK(ui_capability_get_result_v2(s->scene->context.host,last_call,&result)==UI_STATUS_OK);CHECK(ui_capability_forget_v2(s->scene->context.host,last_call)==(result.outcome==UI_CAP2_UNKNOWN?UI_STATUS_PENDING:UI_STATUS_OK));}else result.outcome=UI_CAP2_REJECTED;
 CHECK(ui_capability_ui_begin_unbind(s->scene->context.host,binding)==UI_STATUS_OK);CHECK(ui_capability_ui_finish_unbind(s->scene->context.host,binding)==UI_STATUS_OK);return result;
}
static void draft(ui_component_t *c,const char *field,const char *value)
{ui_components_form_draft(c,field,value);}
static void select_row(state_app_t *s,uint64_t row,uint32_t modifiers)
{ui_cap_input_context_t context={0};char node[80],params[16384];size_t length;ui_cap_result_v2_t r;snprintf(node,sizeof(node),"cell-%llu-name",(unsigned long long)row);context.size=sizeof(context);context.version=1;context.control_id=node;context.event.size=sizeof(context.event);context.event.kind=UI_INPUT_POINTER_UP;context.event.pointer_button=1;context.event.modifiers=modifiers;CHECK(state_collect(&context,params,sizeof(params),&length,p2_find(&s->capabilities,"test.select"))==UI_STATUS_OK);r=invoke_test(s,"test.select",params);CHECK(r.outcome==UI_CAP2_OK);}
static ui_status_t form_action(state_app_t *s,ui_component_t *component,const char *action)
{ui_status_t status;ui_capabilities_human_begin(s->scene->context.host,ui_components_view(component)->user_data);status=ui_components_form_action(component,action);ui_capabilities_human_end(s->scene->context.host);return status;}
static void actual_forms(state_app_t *s)
{
 ui_component_t *a=s->scene->form,*b=s->scene->dialog;ui_cap_form_state_t state={0};ui_cell_t cell={0};ui_cap_result_v2_t result={0};uint64_t revision=s->revision;
 CHECK(form_action(s,a,"conflict-reload")==UI_STATUS_OK);CHECK(form_action(s,b,"conflict-reload")==UI_STATUS_OK);
 draft(a,"name","actual form collected name");draft(b,"name","actual dialog private name");CHECK(s->revision==revision);
 CHECK(form_action(s,a,"submit")==UI_STATUS_OK);CHECK(!strcmp(s->name,"actual form collected name")&&s->revision==revision+1);
 result.size=sizeof(result);result.version=2;CHECK(ui_capability_get_result_v2(s->scene->context.host,s->capabilities.resources[0].last_call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_OK&&result.output_json&&strstr(result.output_json,"actual form collected name"));p2_poll(&s->scene->context,&s->capabilities);
 state.size=sizeof(state);state.version=1;cell.size=sizeof(cell);CHECK(ui_capability_form_get_state(b,&state)==UI_STATUS_OK&&state.conflict_fields==1);CHECK(ui_component_get_field(b,"name",&cell)==UI_STATUS_OK&&!strcmp(cell.text,"actual dialog private name"));
 result=invoke_test(s,"test.modal","{}");CHECK(result.outcome==UI_CAP2_OK&&s->scene->context.host->modal_component==b);revision=s->revision;
 CHECK(form_action(s,b,"submit")==UI_STATUS_VALIDATION_FAILED&&s->revision==revision&&!strcmp(s->name,"actual form collected name"));
 CHECK(form_action(s,b,"conflict-keep")==UI_STATUS_OK);CHECK(form_action(s,b,"submit")==UI_STATUS_OK);CHECK(!strcmp(s->name,"actual dialog private name")&&s->revision==revision+1&&s->scene->context.host->modal_component==NULL);p2_poll(&s->scene->context,&s->capabilities);
 CHECK(form_action(s,a,"conflict-reload")==UI_STATUS_OK);CHECK(ui_component_get_field(a,"name",&cell)==UI_STATUS_OK&&!strcmp(cell.text,"actual dialog private name"));
 printf("SIM stateful actual form/dialog collectors: shared name=%s revision=%llu\n",s->name,(unsigned long long)s->revision);
}

int wmain(int argc,wchar_t **argv)
{
 ui_host_config_t config={0};ui_app_context_t context={0};ui_native_shell_config_t shell={0};ui_assistant_config_t assistant={0};state_app_t *s=NULL;void *app=NULL;ui_cap_result_v2_t result;ui_cell_t cell={0};ui_cap_form_state_t form={0};char before[128],after[128];
 if(argc!=2)return 2;SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_HOME",argv[1]);SetEnvironmentVariableW(L"UI_STATE_EXAMPLE_PROFILE",L"A");config.size=sizeof(config);config.api_version=10;config.result_callback=legacy_result;context.size=sizeof(context);context.host=ui_host_create(&config);CHECK(context.host!=NULL);if(!context.host)return 1;
 {ui_status_t start=state_create(&context,&app,&assistant);fprintf(stderr,"stateful create status=%d\n",start);CHECK(start==UI_STATUS_OK);if(start){if(app)state_destroy(app);ui_host_destroy(context.host);return 1;}}s=app;if(!s||!s->scene){ui_host_destroy(context.host);return 1;}shell.size=sizeof(shell);shell.host=context.host;context.shell=ui_native_shell_create_offscreen(&shell);CHECK(context.shell!=NULL);s->scene->context=context;s->capabilities.context=context;
 CHECK(ui_component_mount_offscreen(s->scene->table,960,400,96)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(s->scene->form,320,400,96)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(s->scene->dialog,320,240,96)==UI_STATUS_OK);CHECK(state_bindings(s)==UI_STATUS_OK);
 CHECK(s->revision==1&&!strcmp(s->name,"Untitled"));result=invoke_test(s,"test.edit","{\"id\":\"9007199254741000\",\"column\":\"name\",\"text\":\"Actual business cell\"}");CHECK(result.outcome==UI_CAP2_OK);CHECK(s->revision==2&&!strcmp(state_cell_text(s,ROW_BASE,"name"),"Actual business cell"));CHECK(ui_component_query(s->scene->table,0,0,128,0,1)==UI_STATUS_OK);
 {ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_component_get_presentation(s->scene->table,"cell-9007199254741000-name",&p)==UI_STATUS_OK&&!strcmp(p.text_utf8,"Actual business cell"));}
 (void)state_revision(s,0,before);result=invoke_test(s,"test.edit","{\"id\":\"9007199254741000\",\"column\":\"value\",\"text\":\"not a number\"}");CHECK(result.outcome==UI_CAP2_FAILED);(void)state_revision(s,0,after);CHECK(!strcmp(before,after)&&!strcmp(state_cell_text(s,ROW_BASE,"value"),"42"));
 draft(s->scene->form,"name","form private draft");draft(s->scene->dialog,"name","dialog private draft");result=invoke_test(s,"test.submit","{\"fields\":{\"name\":\"Shared business name\",\"mode\":\"high\",\"color\":\"#112233FF\"}}");CHECK(result.outcome==UI_CAP2_OK&&!strcmp(s->name,"Shared business name")&&!strcmp(s->mode,"high"));
 cell.size=sizeof(cell);CHECK(ui_component_get_field(s->scene->form,"name",&cell)==UI_STATUS_OK&&!strcmp(cell.text,"form private draft"));CHECK(ui_component_get_field(s->scene->dialog,"name",&cell)==UI_STATUS_OK&&!strcmp(cell.text,"dialog private draft"));form.size=sizeof(form);form.version=1;CHECK(ui_capability_form_get_state(s->scene->dialog,&form)==UI_STATUS_OK&&form.conflict_fields==1);
 CHECK(ui_components_form_action(s->scene->dialog,"submit")==UI_STATUS_PERMISSION_DENIED);ui_capabilities_human_begin(context.host,ui_components_view(s->scene->dialog)->user_data);CHECK(ui_components_form_action(s->scene->dialog,"conflict-reload")==UI_STATUS_OK);ui_capabilities_human_end(context.host);CHECK(ui_component_get_field(s->scene->dialog,"name",&cell)==UI_STATUS_OK&&!strcmp(cell.text,"Shared business name"));
 result=invoke_test(s,"test.modal.close","{\"fields\":{\"name\":\"Dialog commits same property\"}}");CHECK(result.outcome==UI_CAP2_OK&&!strcmp(s->name,"Dialog commits same property"));CHECK(!strcmp(s->mode,"high"));CHECK(ui_capability_form_get_state(s->scene->form,&form)==UI_STATUS_OK&&form.conflict_fields!=0);
 {uint64_t request=ui_host_invoke(context.host,"test.edit","{\"id\":\"9007199254741000\",\"column\":\"name\",\"text\":\"bypass\"}","human");CHECK(request!=0);CHECK(strcmp(state_cell_text(s,ROW_BASE,"name"),"bypass"));}
 result=invoke_test(s,"test.edit","{\"id\":\"9007199254841000\",\"column\":\"name\",\"text\":\"outside\"}");CHECK(result.outcome==UI_CAP2_FAILED);
 {uint64_t revision=s->revision;result=invoke_test(s,"test.edit","{\"id\":\"9007199254741000\",\"column\":\"name\",\"text\":\"truncated\\u0000suffix\"}");CHECK(result.outcome==UI_CAP2_FAILED&&s->revision==revision&&!strcmp(state_cell_text(s,ROW_BASE,"name"),"Actual business cell"));}
 {char json[6500],text[6001];uint64_t revision=s->revision;for(unsigned i=0;i<2000;++i)memcpy(text+i*3,"中",3);text[6000]=0;snprintf(json,sizeof(json),"{\"id\":\"9007199254741000\",\"column\":\"name\",\"text\":\"%s\"}",text);result=invoke_test(s,"test.edit",json);CHECK(result.outcome==UI_CAP2_FAILED&&s->revision==revision);result=invoke_test(s,"test.submit","{\"fields\":{\"name\":\"must not partially apply\",\"color\":\"bad color\"}}");CHECK(result.outcome==UI_CAP2_FAILED&&s->revision==revision&&!strcmp(s->name,"Dialog commits same property"));}
 CHECK(ui_component_set_column_width(s->scene->table,"name",280)==UI_STATUS_OK);result=invoke_test(s,"state.save","{}");CHECK(result.outcome==UI_CAP2_OK);CHECK(ui_component_set_column_width(s->scene->table,"name",360)==UI_STATUS_OK);result=invoke_test(s,"state.load","{}");CHECK(result.outcome==UI_CAP2_OK);{int width=0;CHECK(ui_component_get_column_width(s->scene->table,"name",&width)==UI_STATUS_OK&&width==280);}
 result=invoke_test(s,"language.zh","{}");CHECK(result.outcome==UI_CAP2_OK);{ui_language_info_t language={0};language.size=sizeof(language);CHECK(ui_host_get_language(context.host,&language)==UI_STATUS_OK&&!strcmp(language.language,"zh-CN"));}
 select_row(s,ROW_BASE+2,0);select_row(s,ROW_BASE+4,UI_INPUT_MODIFIER_CONTROL);select_row(s,ROW_BASE+4,UI_INPUT_MODIFIER_CONTROL);CHECK(s->selected_count==1&&s->selected[0]==ROW_BASE+2&&s->selection_anchor==ROW_BASE+4);select_row(s,ROW_BASE+6,UI_INPUT_MODIFIER_SHIFT);CHECK(s->selected_count==3&&s->selected[0]==ROW_BASE+4&&s->selected[2]==ROW_BASE+6);select_row(s,ROW_BASE+8,UI_INPUT_MODIFIER_CONTROL|UI_INPUT_MODIFIER_SHIFT);CHECK(s->selected_count==5&&s->selection_anchor==ROW_BASE+4);
 {uint64_t revision=s->revision;ui_component_state_t state={0};result=invoke_test(s,"test.sort","{\"column\":\"name\",\"direction\":-1}");CHECK(result.outcome==UI_CAP2_OK&&s->revision==revision&&legacy_sort_replies==0);state.size=sizeof(state);CHECK(ui_component_get_state(s->scene->table,&state)==UI_STATUS_OK&&state.sort_direction==-1);result=invoke_test(s,"test.select","{\"ids\":[\"9007199254840999\"]}");CHECK(result.outcome==UI_CAP2_OK&&s->selected_count==1&&s->selected[0]==ROW_BASE+ROW_TOTAL-1&&s->revision==revision);}
 actual_forms(s);
 {wchar_t temporary[4096];int created;CHECK(_snwprintf_s(temporary,4096,_TRUNCATE,L"%s.tmp",s->path)>=0);created=CreateDirectoryW(temporary,NULL)!=0;CHECK(created);
   if(created){uint64_t uncertain;p2_cap_t *cap=p2_find(&s->capabilities,"state.save");result=invoke_test(s,"state.save","{}");uncertain=last_call;CHECK(result.outcome==UI_CAP2_UNKNOWN&&result.effect==UI_CAP2_EFFECT_UNKNOWN&&!s->io);CHECK(RemoveDirectoryW(temporary));p2_poll(&context,&s->capabilities);CHECK(cap->verify_binding!=0);
     CHECK(ui_capability_cancel_v2(context.host,uncertain)==UI_STATUS_OK);CHECK(ui_capability_forget_v2(context.host,uncertain)==UI_STATUS_PENDING);result=invoke_test(s,"state.save","{}");CHECK(result.outcome==UI_CAP2_REJECTED&&!s->io&&s->capabilities.resources[2].last_call==uncertain);
     ui_capabilities_human_begin(context.host,s);CHECK(ui_capabilities_ui_action(context.host,cap->verify_item,s)==UI_STATUS_PENDING);ui_capabilities_human_end(context.host);CHECK(ui_capability_forget_v2(context.host,uncertain)==UI_STATUS_PENDING);}}
 s->scene->context.workspace=NULL;state_unmount(s);ui_native_shell_destroy(context.shell);ui_host_destroy(context.host);state_destroy(s);
 printf("P2 stateful actual memory business (not physical): checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
