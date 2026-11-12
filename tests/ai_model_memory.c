/* Shrinking the model's existing bounded pool must preserve adjacent live data. */
#include "ai_responses.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(void)
{
 unsigned char arena[8192];yyjson_alc pool;unsigned failures=0;
 if(!yyjson_alc_pool_init(&pool,arena,sizeof(arena)))return 2;
 yyjson_alc *previous=am_memory_enter(&pool);
 void *hole=am_allocate(16);unsigned char *guard=am_allocate(1024),*data=am_allocate(256);void *blocker=am_allocate(64);
 if(!hole||!guard||!data||!blocker){(void)am_memory_enter(previous);return 2;}
 memset(guard,0x5a,1024);memset(data,0xa5,256);am_delete(hole);
 (void)am_memory_enter(previous);unsigned char *small=am_resize(data,16);(void)am_memory_enter(&pool);if(!small){(void)am_memory_enter(previous);return 2;}
 for(unsigned i=0;i<1024;++i)if(guard[i]!=0x5a){++failures;break;}
 for(unsigned i=0;i<16;++i)if(small[i]!=0xa5){++failures;break;}
 printf("pool shrink: adjacent live guard %s; copied prefix %s\n",failures?"FAILED":"preserved",failures?"check failure":"preserved");
 if(failures){(void)am_memory_enter(previous);return 1;} /* A red corrupted arena must not be freed again. */
 if(am_resize(small,SIZE_MAX)!=NULL)++failures;
 unsigned char *grown=am_resize(small,512);if(!grown)++failures;else{for(unsigned i=0;i<16;++i)if(grown[i]!=0xa5){++failures;break;}small=grown;}
 am_delete(small);am_delete(guard);am_delete(blocker);
 unsigned char *pressure=am_allocate(7000);if(!pressure)++failures;else{memset(pressure,0x66,7000);void *fill=am_allocate(800);if(!fill)++failures;void *attempt=am_resize(pressure,1024);if(attempt){++failures;pressure=attempt;}else if(pressure[0]!=0x66||pressure[6999]!=0x66)++failures;am_delete(fill);am_delete(pressure);}(void)am_memory_enter(previous);
 unsigned char *heap=am_allocate(1024);if(!heap)return 2;memset(heap,0x36,1024);unsigned char *compact=am_resize(heap,8);if(!compact){am_delete(heap);return 2;}for(unsigned i=0;i<8;++i)if(compact[i]!=0x36)++failures;am_delete(compact);
 printf("model allocation checks failures=%u; no Service, credentials or transport\n",failures);return failures?1:0;
}
