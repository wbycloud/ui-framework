#include "ui_framework/assistant.h"
#include <stdio.h>
#include <string.h>

typedef struct model {
    ui_assistant_t *assistant;
    int count, saved, undo, transaction_open, progress, results, allow, commands, failures;
} model_t;
#define CHECK(m,x) do { if (!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); ++(m)->failures; } } while(0)
static void result(const ui_result_t *r, void *data)
{
    model_t *m = data;
    ++m->results;
    CHECK(m, ui_assistant_forget_request(m->assistant, r->request_id) == UI_STATUS_OK);
}
static void command(ui_host_t *host, uint64_t id, const char *name,
    const char *json, const char *source, void *data)
{
    model_t *m = data; (void)json;
    CHECK(m, strcmp(source, "assistant") == 0);
    CHECK(m, ui_assistant_report_progress(m->assistant, id, 50, "changing") == UI_STATUS_OK);
    if (strcmp(name, "eda.clear") == 0) m->count = 0; else ++m->count;
    CHECK(m, ui_host_reply(host, id, 1, "{}") == UI_STATUS_OK);
}
static void progress(uint64_t id, int p, const char *text, void *data)
{ model_t *m = data; (void)id; (void)p; (void)text; ++m->progress; }
static int confirm(const char *id, const char *json, ui_assistant_permission_t p, void *data)
{ (void)id; (void)json; (void)p; return ((model_t *)data)->allow; }
static ui_status_t begin(const char *label, uint64_t *id, void *data)
{ model_t *m = data; (void)label; if (m->transaction_open) return UI_STATUS_ALREADY_EXISTS;
  m->saved = m->count; m->transaction_open = 1; *id = 1; return UI_STATUS_OK; }
static ui_status_t commit(uint64_t id, void *data)
{ model_t *m = data; if (id != 1 || !m->transaction_open) return UI_STATUS_NOT_FOUND;
  m->undo = m->saved; m->transaction_open = 0; return UI_STATUS_OK; }
static ui_status_t rollback(uint64_t id, void *data)
{ model_t *m = data; if (id != 1 || !m->transaction_open) return UI_STATUS_NOT_FOUND;
  m->count = m->saved; m->transaction_open = 0; return UI_STATUS_OK; }
static ui_status_t undo(uint64_t id, void *data)
{ model_t *m = data; if (id != 1) return UI_STATUS_NOT_FOUND; m->count = m->undo; return UI_STATUS_OK; }
static void visit(const ui_assistant_command_desc_t *desc, void *data)
{ model_t *m = data; CHECK(m, desc->params_schema_json != NULL); ++m->commands; }
int main(void)
{
    model_t model = {0}; ui_host_config_t hc = {0}; ui_assistant_config_t ac = {0};
    ui_command_desc_t cd = {0}; ui_assistant_command_desc_t ad = {0};
    ui_host_t *host; uint64_t request, tx;
    hc.size = sizeof(hc); hc.api_version = UI_FRAMEWORK_API_VERSION; hc.user_data = &model; hc.result_callback = result;
    host = ui_host_create(&hc); CHECK(&model, host != NULL); if (!host) return 1;
    cd.size = sizeof(cd); cd.id = "eda.add_block"; cd.handler = command; cd.user_data = &model;
    CHECK(&model, ui_host_register_command(host, &cd) == UI_STATUS_OK);
    cd.id = "eda.clear"; CHECK(&model, ui_host_register_command(host, &cd) == UI_STATUS_OK);
    ac.size = sizeof(ac); ac.max_permission = UI_ASSISTANT_PERMISSION_EDIT; ac.user_data = &model;
    ac.confirm = confirm; ac.progress = progress; ac.transaction_begin = begin; ac.transaction_commit = commit;
    ac.transaction_rollback = rollback; ac.transaction_undo = undo;
    model.assistant = ui_assistant_create(host, &ac); CHECK(&model, model.assistant != NULL);
    ad.size = sizeof(ad); ad.id = "eda.add_block"; ad.permission = UI_ASSISTANT_PERMISSION_EDIT; ad.params_schema_json = "{\"type\":\"object\"}";
    CHECK(&model, ui_assistant_register_command(model.assistant, &ad) == UI_STATUS_OK);
    ad.id = "eda.clear"; ad.permission = UI_ASSISTANT_PERMISSION_DESTRUCTIVE;
    CHECK(&model, ui_assistant_register_command(model.assistant, &ad) == UI_STATUS_OK);
    CHECK(&model, ui_assistant_visit_commands(model.assistant, visit, &model) == UI_STATUS_OK && model.commands == 2);
    CHECK(&model, ui_assistant_begin_transaction(model.assistant, "Add", &tx) == UI_STATUS_OK);
    CHECK(&model, ui_assistant_invoke(model.assistant, "eda.add_block", "{}", &request) == UI_STATUS_OK && request != 0);
    CHECK(&model, model.count == 1 && model.results == 1 && model.progress == 1);
    CHECK(&model, ui_assistant_commit_transaction(model.assistant, tx) == UI_STATUS_OK);
    CHECK(&model, ui_assistant_undo_transaction(model.assistant, tx) == UI_STATUS_OK && model.count == 0);
    CHECK(&model, ui_assistant_begin_transaction(model.assistant, "Rollback", &tx) == UI_STATUS_OK);
    CHECK(&model, ui_assistant_invoke(model.assistant, "eda.add_block", "{}", &request) == UI_STATUS_OK);
    CHECK(&model, ui_assistant_rollback_transaction(model.assistant, tx) == UI_STATUS_OK && model.count == 0);
    model.count = 7;
    CHECK(&model, ui_assistant_invoke(model.assistant, "eda.clear", "{}", &request) == UI_STATUS_PERMISSION_DENIED && model.count == 7 && request == 0);
    model.allow = 1;
    CHECK(&model, ui_assistant_invoke(model.assistant, "eda.clear", "{}", &request) == UI_STATUS_OK && model.count == 0);
    ui_assistant_destroy(model.assistant); ui_host_destroy(host);
    printf("assistant lifecycle and model rollback: %d failures\n", model.failures);
    return model.failures != 0;
}
