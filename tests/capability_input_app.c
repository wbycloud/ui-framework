#include "ui_framework/application.h"
#include <stdlib.h>
static ui_status_t UI_APP_CALL create(const ui_app_context_t *context,void **state,ui_assistant_config_t *config)
{(void)context;(void)config;*state=calloc(1,1);return *state?UI_STATUS_OK:UI_STATUS_OUT_OF_MEMORY;}
static ui_status_t UI_APP_CALL mount(void *state,const ui_app_context_t *context){(void)state;(void)context;return UI_STATUS_OK;}
static ui_app_close_decision_t UI_APP_CALL close_app(void *state,ui_app_close_reason_t reason){(void)state;(void)reason;return UI_APP_CLOSE_ALLOW;}
static void UI_APP_CALL unmount(void *state){(void)state;}
static void UI_APP_CALL destroy(void *state){free(state);}
UI_APP_EXPORT const ui_app_descriptor_t *UI_APP_CALL ui_app_query_v1(void)
{static const ui_app_descriptor_t d={sizeof(d),UI_APPLICATION_ABI_VERSION,UI_FRAMEWORK_API_VERSION,create,mount,NULL,close_app,unmount,destroy,NULL};return &d;}
