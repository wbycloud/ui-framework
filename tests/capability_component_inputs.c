#define UI_CAP_INPUT_LIGHT
#define UI_CAP_INPUT_WORKSPACE
#include "capability_inputs.c"
#include "json_ui.h"
typedef struct editor_model {model_t base;ui_component_t *table;char text[256];int collect_change,readonly_in_check,readonly;} editor_model_t;
static void editor_rows(ui_component_t *component,const ui_component_query_t *q,void *data)
{
    editor_model_t *m=data;ui_component_batch_t b={0};ui_row_t row={0};ui_cell_t cell={0};if(q->kind!=UI_QUERY_ROWS)return;
    b.size=sizeof(b);b.component_generation=q->component_generation;b.request_id=q->request_id;b.parent_id=q->parent_id;b.first=q->first;b.total_count=1;
    cell.size=sizeof(cell);cell.column_id="name";cell.kind=UI_VALUE_TEXT;cell.text=m->text;cell.flags=m->readonly?UI_VALUE_READONLY:0;row.size=sizeof(row);row.id=UINT64_MAX;row.title="editor";row.cells=&cell;row.cell_count=1;
    if(q->first==0){b.rows=&row;b.row_count=1;}CHECK(ui_component_submit(component,&b)==UI_STATUS_OK);
}
static void editor_execute(uint64_t call,const ui_cap_target_v2_t *target,const char *params,void *data)
{
    editor_model_t *m=data;ui_cap_completion_v2_t c={0};char value[256];(void)target;
    CHECK(call==m->base.call);CHECK(uj_get(params,"text",value,sizeof(value)));++m->base.runs;strcpy_s(m->text,sizeof(m->text),value);++m->base.rev;
    CHECK(ui_component_query(m->table,0,0,4,0,1)==UI_STATUS_OK);c.size=sizeof(c);c.version=2;c.outcome=UI_CAP2_OK;c.effect=UI_CAP2_EFFECT_APPLIED;c.output_json="{}";
    CHECK(ui_capability_complete_v2(m->base.host,call,&c)==UI_STATUS_OK);
}
static ui_status_t editor_collect(const ui_cap_input_context_t *ctx,char *out,size_t capacity,size_t *length,void *data)
{
    ui_json_t j={0};editor_model_t *m=data;CHECK(!strcmp(ctx->control_id,"edit-18446744073709551615-name"));CHECK(ctx->value_utf8!=NULL);
    uj_add(&j,"{\"text\":");uj_string(&j,ctx->value_utf8?ctx->value_utf8:"");uj_add(&j,"}");
    if(j.failed||j.length>=capacity){free(j.data);return UI_STATUS_LIMIT_EXCEEDED;}memcpy(out,j.data,j.length+1);*length=j.length;free(j.data);
    if(m->collect_change)ui_inputs_editor_event(m->base.host,m->table,"editor-change",ctx->control_id);return UI_STATUS_OK;
}
static ui_status_t editor_check(const ui_cap_target_v2_t *target,const char *params,void *data)
{editor_model_t *m=data;(void)target;(void)params;if(m->readonly_in_check){m->readonly=1;CHECK(ui_component_query(m->table,0,0,4,0,1)==UI_STATUS_OK);}return UI_STATUS_OK;}
static void key(ui_web_view_t *view,unsigned code,int qualified)
{ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code=code;(void)ui_inputs_fixture_light(view,&e,qualified);}
static void click_editor(ui_web_view_t *view)
{ui_rect_t rect={0};ui_input_event_t e={0};CHECK(ui_web_view_get_element_rect(view,"cell-18446744073709551615-name",&rect)==UI_STATUS_OK);e.size=sizeof(e);e.pointer_button=1;e.x=rect.x+4;e.y=rect.y+4;e.kind=UI_INPUT_POINTER_DOWN;(void)ui_web_view_dispatch_input(view,&e);e.kind=UI_INPUT_POINTER_UP;(void)ui_web_view_dispatch_input(view,&e);}
int main(void)
{
    editor_model_t m={0};ui_component_desc_t component={0};ui_column_desc_t column={0};ui_capability_desc_v2_t cap={0};ui_cap_component_input_desc_t d={0};
    ui_cap_input_binding_desc_t old;ui_cap_editor_state_t state={0};ui_cap_result_v2_t result={0};ui_web_view_t *view;uint64_t id,duplicate;ui_input_event_t text={0};
    init(&m.base);strcpy_s(m.text,sizeof(m.text),"initial");CHECK(ui_capability_begin_unregistration(m.base.host,"change")==UI_STATUS_OK);CHECK(ui_capability_finish_unregistration(m.base.host,"change")==UI_STATUS_OK);
    cap.size=sizeof(cap);cap.version=2;cap.capability_version=1;cap.id="change";cap.name_utf8="Edit";cap.description_utf8="";cap.category="fixture";cap.target_kind=UI_CAP_DOCUMENT;cap.effect="write";
    cap.input_schema_json="{\"type\":\"object\",\"properties\":{\"text\":{\"type\":\"string\",\"maxLength\":255}},\"required\":[\"text\"],\"additionalProperties\":false}";
    cap.output_schema_json=cap.reconcile_schema_json="{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";cap.execute=editor_execute;cap.check=editor_check;cap.reconcile=reconcile;cap.user_data=&m;CHECK(ui_capability_register_v2(m.base.host,&cap)==UI_STATUS_OK);
    column.size=sizeof(column);column.id="name";column.title="Name";column.kind=UI_VALUE_TEXT;column.width=180;
    component.size=sizeof(component);component.id="table";component.kind=UI_COMPONENT_TABLE;component.columns=&column;component.column_count=1;component.source=editor_rows;component.user_data=&m;component.commands.edit="change";
    CHECK(ui_component_register(m.base.host,&component,&m.table)==UI_STATUS_OK);CHECK(ui_component_mount_offscreen(m.table,320,240,96)==UI_STATUS_OK);view=ui_components_view(m.table);CHECK(view!=NULL);
    d.size=sizeof(d);d.version=1;d.component=m.table;d.selector=UI_CAP_CELL_EDIT;d.capability_id="change";d.resource.size=sizeof(d.resource);d.resource.version=2;
    CHECK(ui_capability_resource_ref_v2(m.base.host,"document",&d.resource)==UI_STATUS_OK);d.collect=editor_collect;d.observe=observe;d.user_data=&m.base;
    CHECK(ui_capability_component_input_bind(m.base.host,&d,&id)==UI_STATUS_OK);CHECK(ui_capability_component_input_bind(m.base.host,&d,&duplicate)==UI_STATUS_ALREADY_EXISTS);
    old=descriptor(&m.base,UI_CAP_INPUT_COMPONENT);old.component=m.table;old.control_id="edit-18446744073709551615-name";old.trigger=UI_INPUT_KEY_DOWN;old.key=13;old.pointer_button=0;
    CHECK(ui_capability_input_bind(m.base.host,&old,&duplicate)==UI_STATUS_ALREADY_EXISTS);
    click_editor(view);state.size=sizeof(state);state.version=1;CHECK(ui_capability_component_editor_state(m.base.host,id,"edit-18446744073709551615-name",&state)==UI_STATUS_OK&&!state.conflict&&!strcmp(state.base_revision,"r0"));
    text.size=sizeof(text);text.kind=UI_INPUT_TEXT;text.text_utf8="!";(void)ui_web_view_dispatch_input(view,&text);CHECK(m.base.runs==0);
    strcpy_s(m.text,sizeof(m.text),"external");++m.base.rev;CHECK(ui_component_query(m.table,0,0,4,0,1)==UI_STATUS_OK);
    CHECK(ui_capability_component_editor_state(m.base.host,id,"edit-18446744073709551615-name",&state)==UI_STATUS_OK&&state.conflict);
    key(view,13,1);result.size=sizeof(result);result.version=2;CHECK(m.base.runs==0);CHECK(ui_capability_get_result_v2(m.base.host,m.base.call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_STALE);forget(&m.base);
    key(view,27,0);CHECK(ui_capability_component_editor_state(m.base.host,id,"edit-18446744073709551615-name",&state)==UI_STATUS_OK);key(view,27,1);
    CHECK(ui_capability_component_editor_state(m.base.host,id,"edit-18446744073709551615-name",&state)==UI_STATUS_NOT_FOUND);
    click_editor(view);key(view,VK_END,0);text.text_utf8="?";(void)ui_web_view_dispatch_input(view,&text);key(view,13,0);CHECK(m.base.runs==0);key(view,13,1);
    if(m.base.runs!=1||strcmp(m.text,"external?"))fprintf(stderr,"editor actual runs=%u text=%s status=%d code=%s call=%llu\n",m.base.runs,m.text,(int)m.base.status,m.base.code,(unsigned long long)m.base.call);
    CHECK(m.base.runs==1&&!strcmp(m.text,"external?"));CHECK(ui_capability_component_editor_state(m.base.host,id,"edit-18446744073709551615-name",&state)==UI_STATUS_NOT_FOUND);forget(&m.base);
    click_editor(view);key(view,VK_END,0);text.text_utf8="#";(void)ui_web_view_dispatch_input(view,&text);m.collect_change=1;key(view,13,1);m.collect_change=0;
    CHECK(m.base.runs==2&&!strcmp(m.text,"external?#"));CHECK(ui_capability_component_editor_state(m.base.host,id,"edit-18446744073709551615-name",&state)==UI_STATUS_OK&&state.conflict);forget(&m.base);
    key(view,27,1);click_editor(view);m.readonly_in_check=1;key(view,13,1);CHECK(m.base.runs==2);CHECK(ui_capability_get_result_v2(m.base.host,m.base.call,&result)==UI_STATUS_OK&&result.outcome==UI_CAP2_REJECTED);forget(&m.base);
    unbind(&m.base,id);CHECK(ui_component_unregister(m.table)==UI_STATUS_OK);ui_host_destroy(m.base.host);
    printf("CI-1 component editor logic (not physical): checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
