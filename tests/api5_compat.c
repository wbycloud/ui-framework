#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/components.h"
static int failures,queries;
_Static_assert(sizeof(ui_component_desc_t)==144&&offsetof(ui_component_desc_t,web_backend)==136,"SDK5 description frozen");
_Static_assert(sizeof(ui_component_state_t)==120&&sizeof(ui_component_query_t)==96,"SDK5 state/query frozen");
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SDK5 line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void source(ui_component_t *c,const ui_component_query_t *q,void *user)
{ui_component_query_t copied;ui_component_batch_t b={0};ui_row_t row={0};(void)user;memcpy(&copied,q,sizeof(copied));++queries;
 row.size=sizeof(row);row.id=9007199254741001ULL;row.title="API5 caller";row.content_version=1;b.size=sizeof(b);b.component_generation=copied.component_generation;b.request_id=copied.request_id;b.first=copied.first;b.total_count=1;b.rows=&row;b.row_count=1;CHECK(ui_component_submit(c,&b)==UI_STATUS_OK);}
int main(void)
{
    SYSTEM_INFO si;DWORD previous;unsigned char *memory;ui_component_desc_t *d;ui_component_state_t *state;ui_host_config_t hc={0};ui_host_t *h;ui_component_t *c;
    GetSystemInfo(&si);memory=(unsigned char *)VirtualAlloc(NULL,(size_t)si.dwPageSize*2,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(!memory)return 2;
    CHECK(VirtualProtect(memory+si.dwPageSize,si.dwPageSize,PAGE_NOACCESS,&previous));hc.size=sizeof(hc);hc.api_version=5;h=ui_host_create(&hc);CHECK(h!=NULL);if(!h)return 1;
    d=(ui_component_desc_t *)(memory+si.dwPageSize-sizeof(*d));memset(d,0,sizeof(*d));d->size=sizeof(*d);d->id="sdk5";d->kind=UI_COMPONENT_LIST;d->source=source;
    CHECK(ui_component_register(h,d,&c)==UI_STATUS_OK);CHECK(ui_component_query(c,0,0,1,0,0)==UI_STATUS_OK&&queries==1);
    state=(ui_component_state_t *)(memory+si.dwPageSize-sizeof(*state));memset(state,0,sizeof(*state));state->size=sizeof(*state);CHECK(ui_component_get_state(c,state)==UI_STATUS_OK&&state->row_count==1);
    CHECK(ui_component_select(c,9007199254741001ULL)==UI_STATUS_OK);CHECK(ui_component_get_state(c,state)==UI_STATUS_OK&&state->selected_id==9007199254741001ULL);
    ui_host_destroy(h);VirtualFree(memory,0,MEM_RELEASE);printf("frozen SDK5 guarded descriptor/state/source: %d failures\n",failures);return failures?1:0;
}
