#ifndef UI_AI_MODEL_INTERNAL_H
#define UI_AI_MODEL_INTERNAL_H
#include "ui_framework/ai_model_view.h"
#include "ai_responses.h"
#include <winhttp.h>
#include <wincred.h>
#include <commctrl.h>
typedef struct am_binding {struct am_binding *next;ui_ai_service_t *service;ui_web_view_t *view;HWND window;uint64_t id;int used,action,pressed;uint32_t modifiers;char control[128],run[65],hash[65];ui_ai_model_display_check_fn display_check;void *display_data;uint64_t display_generation;} am_binding_t;
typedef struct am_credential_status {
 char stage[24],error[64],entry[32];int dialog,opened,submitted,written,config_saved,readable;
 unsigned observed;DWORD prompt_code,write_code,config_code,read_code;uint64_t revision;
} am_credential_status_t;
struct am_config {
 char path[4096],model[128],ref[128],binding[65],operation[65],error[96];
 char pending_model[128];uint64_t revision,pending_revision,sequence;
 int ready,job,busy,host_status,blocked;uint64_t ui_generation;wchar_t secret[513];am_binding_t *bindings;
 am_credential_status_t credential;
#ifdef UI_AI_FIXTURE
 char *fixture[16];unsigned fixture_count,requests;int fixture_hold,fixture_http,fixture_pause;unsigned fixture_ticks;DWORD fixture_error;
 HANDLE fixture_cb_thread,fixture_cb_gate,fixture_cb_enter;int fixture_cb_slot;
 HANDLE fixture_meter_enter,fixture_meter_release;int fixture_meter_fault;
 HANDLE fixture_scan_gate,fixture_scan_enter;char fixture_scan_id[65];int fixture_scan_armed,fixture_scan_slot,fixture_scan_same;unsigned fixture_scan_done;
 wchar_t fixture_key[513];char fixture_body[AM_REQUEST+1];
 int (*fixture_reply)(void *,unsigned,const char *,char *,size_t);void *fixture_reply_data;
 int fixture_batch_mode;
 int fixture_credentials,fixture_prompt_result,fixture_prompt_reenter,fixture_credential_hold,fixture_dialog_checks;
 DWORD fixture_write_error,fixture_config_error,fixture_read_error;wchar_t fixture_credential[513];
 char fixture_write_ref[128],fixture_read_ref[128];
#endif
};
typedef struct am_net {
 ui_ai_service_t *service;HINTERNET session,connection,request;
 int callback_set,callbacks,closed,closing,flags,error,started,finished,sent,response_complete;
 DWORD received,http,transport_error;char block[16384];uint64_t begin,progress;
} am_net_t;
struct am_run {
 as_run_t *assistant;
 char model[128],ref[128],binding[65],request_id[65],error[96],catalog[65],remote_ids[64][65];unsigned remote_count;
 uint64_t revision,until;int confirmed,wait_saved,confirmation_saved,validated,recovered,charged,admitted,request_saved;
 int phase;unsigned ordinal,index,validation_kind,validation_ticket;size_t emitted,output_total;
 uint64_t metered_input;unsigned metered_ordinal;char metered_hash[65];
 char *items,*body,*batch,*preview;am_stream_t *stream;am_net_t net;
 void *arena;yyjson_alc allocator;
};
enum {AM_WAIT=0,AM_PREPARE,AM_NETWORK,AM_VALIDATE,AM_TOOLS,AM_FINAL};
#define AM_SCRATCH (2u*1024u*1024u)
static void am_close(ui_ai_service_t *,am_run_t *);
#endif
