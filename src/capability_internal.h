#ifndef UI_CAPABILITY_INTERNAL_H
#define UI_CAPABILITY_INTERNAL_H
#include "ui_framework/capability.h"
#include "yyjson.h"
typedef struct ui_cap_json_doc { yyjson_doc *doc; void *pool; } ui_cap_json_doc_t;
ui_status_t ui_cap_parse(const char *, size_t, size_t, ui_cap_json_doc_t *);
void ui_cap_doc_free(ui_cap_json_doc_t *);
ui_status_t ui_cap_schema_canonical(const char *, char **);
ui_status_t ui_cap_jcs_value(yyjson_val *, char **, size_t *);
ui_status_t ui_cap_hash_bytes(const char *, size_t, char[65]);
char *ui_cap_quote(const char *);
int ui_cap_text(const char *, size_t, int);
int ui_capabilities_busy(const ui_host_t *);
void ui_capabilities_destroy(ui_host_t *);
void ui_capabilities_set_instance(ui_host_t *, uint64_t);
#endif
