#ifndef UI_CAPABILITY_EDIT_INTERNAL_H
#define UI_CAPABILITY_EDIT_INTERNAL_H
#include "ui_framework/capability_edit_extensions.h"
/* Private Service-only admission. These symbols are not DLL exports. */
ui_status_t ui_cap_edit_prepare(ui_host_t *,const char *,const ui_cap_target_v2_t *,const char *,const char *,const char *,uint64_t *);
ui_status_t ui_cap_edit_invoke(ui_host_t *,uint64_t,ui_cap_edit_stage_t,int (*)(void *),void *);
ui_status_t ui_cap_edit_result(ui_host_t *,uint64_t,ui_cap_edit_completion_t *,char [128]);
ui_status_t ui_cap_edit_finish(ui_host_t *,uint64_t,ui_cap_outcome_v2_t,ui_cap_effect_v2_t,const char *);
ui_status_t ui_cap_edit_release(ui_host_t *,uint64_t,int);
ui_status_t ui_cap_edit_undo_receipt(ui_host_t *,uint64_t,const char *);
int ui_cap_edit_cancelled(ui_host_t *,uint64_t);
#endif
