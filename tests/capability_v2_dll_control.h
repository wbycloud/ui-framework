#ifndef C2_DLL_CONTROL_H
#define C2_DLL_CONTROL_H
#include "ui_framework/capability_v2.h"
typedef struct c2_dll_api {
    ui_status_t (*resource)(ui_host_t *,const ui_cap_resource_desc_t *);
    ui_status_t (*capability)(ui_host_t *,const ui_capability_desc_v2_t *);
    ui_status_t (*complete)(ui_host_t *,uint64_t,const ui_cap_completion_v2_t *);
    ui_status_t (*forget)(ui_host_t *,uint64_t);
    ui_status_t (*begin)(ui_host_t *,const char *);
    ui_status_t (*finish)(ui_host_t *,const char *);
    ui_status_t (*quiesce)(ui_host_t *);
    void (*destroy)(ui_host_t *);
} c2_dll_api_t;
typedef void *(*c2_dll_create_fn)(ui_host_t *,const c2_dll_api_t *,int);
typedef ui_status_t (*c2_dll_pump_fn)(void *);
typedef void (*c2_dll_signal_fn)(void *,int);
typedef int (*c2_dll_state_fn)(void *,int);
typedef void (*c2_dll_destroy_fn)(void *);
#endif
