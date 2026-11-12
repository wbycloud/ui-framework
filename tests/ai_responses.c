#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ai_responses.h"
#include "ai_model_wire.h"
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"line %d: %s\n",__LINE__,#x);}}while(0)
static int parse(const char *wire,size_t cut,int good)
{
 void *arena=malloc(2u*1024u*1024u);yyjson_alc a;CHECK(arena&&yyjson_alc_pool_init(&a,arena,2u*1024u*1024u));yyjson_alc *old=am_memory_enter(&a);am_stream_t *s=am_allocate(sizeof(*s));CHECK(s!=NULL);if(!s){am_memory_enter(old);free(arena);return 0;}memset(s,0,sizeof(*s));size_t n=strlen(wire);int ok=am_stream_feed(s,wire,cut)&&am_stream_feed(s,wire+cut,n-cut)&&am_stream_eof(s);CHECK(ok==good);if(ok){CHECK(s->complete&&!s->failed);if(strstr(wire,"xs_read_call"))CHECK(s->count>0&&s->functions[0].done);else CHECK(!strcmp(s->text,"你好，value 10"));}am_stream_clear(s);am_delete(s);am_memory_enter(old);free(arena);return ok;
}
int main(void)
{
 for(size_t cut=0;cut<=strlen(rp_text);++cut)(void)parse(rp_text,cut,1);
 for(size_t cut=0;cut<=strlen(rp_two_reads);++cut)(void)parse(rp_two_reads,cut,1);
 (void)rp_read;(void)rp_write;(void)rp_bad_batch;
 am_stream_t *s=calloc(1,sizeof(*s));CHECK(s!=NULL);for(size_t i=0;i<strlen(rp_text);++i)CHECK(am_stream_feed(s,rp_text+i,1));CHECK(am_stream_eof(s));CHECK(!strcmp(s->text,"你好，value 10"));am_stream_clear(s);
 char *broken=malloc(strlen(rp_text)+1);strcpy_s(broken,strlen(rp_text)+1,rp_text);char *q=strstr(broken,"你好");CHECK(q!=NULL);if(q){((unsigned char *)q)[0]=255;(void)parse(broken,0,0);}free(broken);
 CHECK(!am_stream_feed(s,"data: {\"type\":\"x\",\"type\":\"y\"}\n\n",strlen("data: {\"type\":\"x\",\"type\":\"y\"}\n\n")));am_stream_clear(s);
 CHECK(am_stream_feed(s,rp_read,strlen(rp_read)-3));CHECK(!am_stream_eof(s));am_stream_clear(s);
 CHECK(!am_stream_feed(s,"data: [DONE]\n\n",14));am_stream_clear(s);
 char *large=malloc(AM_EVENT+2);memset(large,'x',AM_EVENT+1);CHECK(!am_stream_feed(s,large,AM_EVENT+1));free(large);am_stream_clear(s);
 /* Three SSE data lines reach the exact event budget; the next separator
  * must fail before writing beyond the data buffer. */
 char *boundary=malloc(AM_EVENT+1);CHECK(boundary!=NULL);memset(boundary,'x',AM_EVENT);memcpy(boundary,"data: ",6);boundary[AM_EVENT]=0;CHECK(am_stream_feed(s,boundary,AM_EVENT));CHECK(am_stream_feed(s,"\ndata: xxxxx\n",13));CHECK(s->data_len==AM_EVENT);CHECK(!am_stream_feed(s,"data: \n",7));am_stream_clear(s);free(boundary);
 ui_cap_json_doc_t d={0};CHECK(!am_json_parse("{\"a\":1,\"a\":2}",13,&d));CHECK(!am_json_parse("{\"a\":\"\\u0000\"}",14,&d));CHECK(am_json_parse("{\"a\":1}",7,&d));am_json_free(&d);free(s);
 printf("RP protocol checks=%u failures=%u; offline only\n",checks,failures);return failures?1:0;
}
