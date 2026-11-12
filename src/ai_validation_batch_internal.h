#ifndef UI_AI_VALIDATION_BATCH_INTERNAL_H
#define UI_AI_VALIDATION_BATCH_INTERNAL_H
#include "ui_framework/ai_validation_batch.h"
#define AB_REFERENCE "ui-framework/xuanshu/deepseek/d15f7883aeeac194fe15029c233b44bdeb1f20dcd3d2f680373ed5e70803ab38"
#define AB_START 3
#define AB_EXPORT_BYTES (512u*1024u)
typedef struct ab_batch {
 ui_cap_target_t target;
 char id[37],hash[65],catalog[65],profile[65],preview[8192],error[96];
 uint64_t revision;
 int configure_requested,check_requested,need_key,prepared,accepting,accepted,saved;
 int submitting,confirming,busy,discovered,read_proven,ending,quiesced;
 int cancel_requested,cancel_inflight,cancel_missed,export_done,export_ok;
} ab_batch_t;
static int ab_restrict_submit(ui_ai_service_t *,am_run_t *);
static void ab_credential_notice(ui_ai_service_t *);
static int ab_worker(ui_ai_service_t *,int);
static void ab_ui(ui_ai_service_t *);
static ui_status_t ab_accept(ui_ai_service_t *,const char *);
static int ab_result(ui_ai_service_t *,ai_run_t *,ai_tool_t *);
static int ab_tool_allowed(ui_ai_service_t *,ai_run_t *,yyjson_val *);
#endif
