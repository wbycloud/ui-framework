#include "ui_framework/components.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static int failures,queries;static ui_component_query_t request;
#define CHECK(x) do{if(!(x)){printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(0)
static void source(ui_component_t *c,const ui_component_query_t *q,void *data)
{(void)c;(void)data;request=*q;++queries;}
static ui_host_t *host(void)
{ui_host_config_t h={0};h.size=sizeof(h);h.api_version=3;return ui_host_create(&h);}
int main(void)
{
    ui_host_t *a=host(),*b=host();ui_component_t *table,*tree;ui_component_desc_t d={0};ui_column_desc_t columns[16]={0};
    ui_row_t rows[10]={0};ui_cell_t cells[10]={0};ui_component_batch_t batch={0};ui_component_state_t state={0};
    char names[16][16],titles[10][32];size_t i;uint64_t old_generation,removed=9007199254741001ULL;uint8_t rgba[24]={255,0,0,128};
    ui_rgba_desc_t image={0};ui_image_id_t id;ui_image_info_t info={0};ui_image_stats_t stats={0};
    CHECK(a&&b);CHECK(ui_framework_supports_api(1)&&ui_framework_supports_api(2)&&ui_framework_supports_api(3)&&ui_framework_supports_api(4)&&ui_framework_supports_api(5)&&!ui_framework_supports_api(6));
    for(i=0;i<16;++i){snprintf(names[i],16,"c%zu",i);columns[i].size=sizeof(columns[i]);columns[i].id=names[i];columns[i].title=names[i];columns[i].kind=UI_VALUE_TEXT;columns[i].width=120;}
    d.size=sizeof(d);d.id="table";d.title="100k table";d.kind=UI_COMPONENT_TABLE;d.columns=columns;d.column_count=16;d.source=source;
    CHECK(ui_component_register(a,&d,&table)==UI_STATUS_OK);CHECK(ui_component_register(b,&d,&tree)==UI_STATUS_OK);
    strcpy(names[0],"mutated");CHECK(ui_component_query(table,0,90000,10,2,4)==UI_STATUS_OK);CHECK(request.first==90000&&request.count==10&&request.first_column==2&&request.column_count==4);
    for(i=0;i<10;++i){snprintf(titles[i],32,"row %zu",i);rows[i].size=sizeof(rows[i]);rows[i].id=removed+i;rows[i].content_version=1;rows[i].title=titles[i];
        cells[i].size=sizeof(cells[i]);cells[i].column_id="c2";cells[i].kind=UI_VALUE_TEXT;cells[i].text="value";rows[i].cells=&cells[i];rows[i].cell_count=1;}
    batch.size=sizeof(batch);batch.component_generation=request.component_generation;batch.request_id=request.request_id;batch.first=request.first;batch.total_count=100000;batch.rows=rows;batch.row_count=10;
    CHECK(ui_component_submit(table,&batch)==UI_STATUS_OK);strcpy(titles[0],"changed stack");state.size=sizeof(state);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.total_count==100000&&state.row_count==10);
    CHECK(ui_component_select(table,removed)==UI_STATUS_OK);CHECK(ui_component_get_state(tree,&state)==UI_STATUS_OK&&state.selected_id==0);
    CHECK(ui_component_remove_rows(table,&removed,1)==UI_STATUS_OK);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.row_count==9&&state.total_count==99999&&state.selected_id==0);
    CHECK(ui_component_insert_rows(table,0,90000,rows,1)==UI_STATUS_OK);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.row_count==10&&state.total_count==100000);
    old_generation=batch.component_generation;CHECK(ui_component_set_source(table,source,NULL)==UI_STATUS_OK);CHECK(ui_component_submit(table,&batch)==UI_STATUS_CANCELLED);
    CHECK(ui_component_query(table,0,0,10,0,1)==UI_STATUS_OK);CHECK(request.component_generation!=old_generation);batch.component_generation=request.component_generation;batch.request_id=request.request_id;batch.first=0;
    rows[1].id=rows[0].id;CHECK(ui_component_submit(table,&batch)==UI_STATUS_INVALID_ARGUMENT);rows[1].id=removed+1;
    {ui_row_t *large=(ui_row_t *)calloc(512,sizeof(*large));ui_cell_t *values=(ui_cell_t *)calloc(512,sizeof(*values));char text[4096];
        memset(text,'x',4095);text[4095]=0;CHECK(large&&values);CHECK(ui_component_query(table,0,0,512,0,1)==UI_STATUS_OK);
        if(large&&values){for(i=0;i<512;++i){large[i].size=sizeof(large[i]);large[i].id=i+1;large[i].title=text;large[i].cells=&values[i];large[i].cell_count=1;
                values[i].size=sizeof(values[i]);values[i].kind=UI_VALUE_TEXT;values[i].column_id="c0";values[i].text=text;}
            batch.component_generation=request.component_generation;batch.request_id=request.request_id;batch.rows=large;batch.row_count=512;
            CHECK(ui_component_submit(table,&batch)==UI_STATUS_LIMIT_EXCEEDED);CHECK(ui_component_get_state(table,&state)==UI_STATUS_OK&&state.cached_bytes<=2u*1024u*1024u);}
        free(large);free(values);}
    image.size=sizeof(image);image.width=2;image.height=2;image.stride=12;image.bytes=20;image.pixels=rgba;
    CHECK(ui_image_create(a,&image,&id)==UI_STATUS_OK);memset(rgba,0,sizeof(rgba));info.size=sizeof(info);CHECK(ui_image_get_info(a,id,&info)==UI_STATUS_OK&&info.bytes==16&&info.version==1);
    CHECK(ui_image_get_info(b,id,&info)==UI_STATUS_NOT_FOUND);CHECK(ui_image_update(a,id,&image)==UI_STATUS_OK);CHECK(ui_image_get_info(a,id,&info)==UI_STATUS_OK&&info.version==2);
    image.stride=7;CHECK(ui_image_create(a,&image,&id)==UI_STATUS_INVALID_ARGUMENT);image.stride=SIZE_MAX;CHECK(ui_image_create(a,&image,&id)==UI_STATUS_INVALID_ARGUMENT);
    stats.size=sizeof(stats);CHECK(ui_image_get_stats(a,&stats)==UI_STATUS_OK&&stats.bytes>=16&&stats.bytes<1024);CHECK(ui_image_set_limit(a,8)==UI_STATUS_LIMIT_EXCEEDED);
    CHECK(ui_image_load_png(a,"bad png",7,&id)==UI_STATUS_INVALID_ARGUMENT);CHECK(ui_component_unregister(table)==UI_STATUS_OK);CHECK(ui_component_find(a,"table")==NULL);
    ui_host_destroy(a);ui_host_destroy(b);printf("API3 components core: %d failures, %d bounded queries\n",failures,queries);return failures?1:0;
}
