#ifndef P2_EXAMPLE_LOGIC_H
#define P2_EXAMPLE_LOGIC_H
/* This private fixture tests admission/business contracts, never OS/physical
 * source classification. No production export signs human authority. */
static unsigned checks,failures,logic_entry;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static const char *logic_parameters;static uint64_t logic_call;static ui_status_t logic_status;
static char logic_result[UI_CAPABILITY_OUTPUT_BYTES+1];
static ui_status_t logic_collect(const ui_cap_ui_context_t *c,char *out,size_t capacity,size_t *length,void *data)
{(void)c;(void)data;*length=strlen(logic_parameters);if(*length>=capacity)return UI_STATUS_LIMIT_EXCEEDED;memcpy(out,logic_parameters,*length+1);return UI_STATUS_OK;}
static void logic_observe(const ui_cap_ui_notice_t *n,void *data)
{(void)data;logic_status=n->status;if(n->call_id)logic_call=n->call_id;}
static ui_cap_outcome_v2_t logic_invoke(p2_app_t *app,const char *capability,const char *params)
{
 p2_cap_t *cap=p2_find(app,capability);ui_menu_item_desc_t menu={0};ui_cap_ui_binding_desc_t desc={0};ui_cap_result_v2_t result={0};char item[64];uint64_t binding=0;
 CHECK(cap!=NULL);if(!cap)return UI_CAP2_REJECTED;snprintf(item,sizeof(item),"fixture.%u",++logic_entry);menu.size=sizeof(menu);menu.id=item;menu.title=capability;menu.menu_path="Fixture";menu.command_id=capability;CHECK(ui_host_register_menu_item(app->context.host,&menu)==UI_STATUS_OK);
 desc.size=sizeof(desc);desc.version=1;desc.menu_item_id=item;desc.capability_id=capability;desc.resource=app->resources[cap->spec->resource].ref;desc.purpose=UI_CAP_UI_INVOKE;desc.collect=logic_collect;desc.observe=logic_observe;
 CHECK(ui_capability_ui_bind(app->context.host,&desc,&binding)==UI_STATUS_OK);logic_parameters=params;logic_call=0;logic_result[0]=0;logic_status=UI_STATUS_OK;
 ui_capabilities_human_begin(app->context.host,app);(void)ui_capabilities_ui_action(app->context.host,item,app);ui_capabilities_human_end(app->context.host);
 result.size=sizeof(result);result.version=2;result.outcome=UI_CAP2_REJECTED;
 if(logic_call){CHECK(ui_capability_get_result_v2(app->context.host,logic_call,&result)==UI_STATUS_OK);if(result.output_json)strcpy_s(logic_result,sizeof(logic_result),result.output_json);CHECK(ui_capability_forget_v2(app->context.host,logic_call)==UI_STATUS_OK);}
 CHECK(ui_capability_ui_begin_unbind(app->context.host,binding)==UI_STATUS_OK);CHECK(ui_capability_ui_finish_unbind(app->context.host,binding)==UI_STATUS_OK);return result.outcome;
}
#endif
