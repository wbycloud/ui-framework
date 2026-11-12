#include "ui_internal.h"
#include "../examples/web_counter/app.c"
#include "p2_example_logic.h"
int main(void)
{
 ui_host_config_t config={0};ui_app_context_t context={0};ui_assistant_config_t assistant={0};counter_t *s=NULL;void *app=NULL;ui_light_web_config_t light={0};ui_cap_input_binding_desc_t binding={0};uint64_t id;ui_rect_t rect={0};ui_input_event_t e={0};
 config.size=sizeof(config);config.api_version=10;context.size=sizeof(context);context.host=ui_host_create(&config);CHECK(context.host!=NULL);if(!context.host)return 1;CHECK(create(&context,&app,&assistant)==UI_STATUS_OK);s=app;if(!s)return 1;
 light.size=sizeof(light);s->backend=ui_light_web_backend_create(&light);CHECK(s->backend!=NULL);s->view=ui_web_view_create(context.host,s->backend);CHECK(s->view!=NULL);CHECK(ui_web_view_load_html(s->view,page)==UI_STATUS_OK);CHECK(ui_web_view_set_rect(s->view,&(ui_rect_t){0,0,600,300},96)==UI_STATUS_OK);
 binding.size=sizeof(binding);binding.version=1;binding.kind=UI_CAP_INPUT_LIGHT_CONTROL;binding.view=s->view;binding.control_id="add";binding.trigger=UI_INPUT_POINTER_UP;binding.pointer_button=1;binding.capability_id="counter.increment";binding.resource=s->capabilities.resources[0].ref;binding.purpose=UI_CAP_UI_INVOKE;binding.observe=p2_notice;binding.user_data=p2_find(&s->capabilities,"counter.increment");CHECK(ui_capability_input_bind(context.host,&binding,&id)==UI_STATUS_OK);
 CHECK(ui_web_view_get_element_rect(s->view,"add",&rect)==UI_STATUS_OK);e.size=sizeof(e);e.pointer_button=1;e.x=rect.x+rect.width/2;e.y=rect.y+rect.height/2;e.kind=UI_INPUT_POINTER_DOWN;(void)ui_web_view_dispatch_input(s->view,&e);e.kind=UI_INPUT_POINTER_UP;(void)ui_web_view_dispatch_input(s->view,&e);CHECK(s->count==0&&s->revision==1);
 e.kind=UI_INPUT_POINTER_DOWN;(void)ui_inputs_fixture_light(s->view,&e,1);e.kind=UI_INPUT_POINTER_UP;(void)ui_inputs_fixture_light(s->view,&e,1);CHECK(s->count==1&&s->revision==2);{ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s->view,"count",&p)==UI_STATUS_OK&&p.text_utf8&&!strcmp(p.text_utf8,"1"));}p2_poll(&context,&s->capabilities);
 CHECK(logic_invoke(&s->capabilities,"counter.increment","{}")==UI_CAP2_OK);CHECK(s->count==2&&strstr(logic_result,"\"count\":2"));(void)ui_host_invoke(context.host,"counter.increment","{}","human");CHECK(s->count==2);
 CHECK(logic_invoke(&s->capabilities,"counter.increment","{\"source\":\"human\"}")==UI_CAP2_REJECTED&&s->count==2);s->count=UINT_MAX;CHECK(logic_invoke(&s->capabilities,"counter.increment","{}")==UI_CAP2_FAILED&&s->count==UINT_MAX);
 CHECK(ui_capability_ui_begin_unbind(context.host,id)==UI_STATUS_OK);CHECK(ui_capability_ui_finish_unbind(context.host,id)==UI_STATUS_OK);unmount(s);ui_host_destroy(context.host);destroy(s);printf("P2 counter actual business/Light binding (not physical): checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
