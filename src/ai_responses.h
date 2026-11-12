#ifndef UI_AI_RESPONSES_H
#define UI_AI_RESPONSES_H
#include <stddef.h>
#include <stdint.h>
#include "capability_internal.h"
#define AM_EVENT (256u*1024u)
#define AM_REQUEST (128u*1024u)
#define AM_WIRE (2u*1024u*1024u)
typedef struct am_function {
 char item[257],call[257],name[65];char *args;size_t length;int done;
} am_function_t;
typedef struct am_stream {
 char line[AM_EVENT+1],data[AM_EVENT+1],text[65537],response[257],event[80],last_hash[65];
 size_t line_len,data_len,text_len,wire,args_bytes;uint64_t sequence;
 int sequence_seen,cr,complete,failed;unsigned count;am_function_t functions[64];
 char *final_json;char error[64];
} am_stream_t;
void am_stream_clear(am_stream_t *);
int am_stream_feed(am_stream_t *,const void *,size_t);
int am_stream_eof(am_stream_t *);
int am_json_parse(const char *,size_t,ui_cap_json_doc_t *);
void am_json_free(ui_cap_json_doc_t *);
yyjson_alc *am_memory_enter(yyjson_alc *);
void *am_allocate(size_t);
void *am_resize(void *,size_t);
void am_delete(void *);
char *am_value_write(yyjson_val *);
char *am_mutable_write(yyjson_mut_doc *);
#endif
