/* Actual embedded host resources. Inputs, credentials and transport are synthetic. */
#include <stdlib.h>
#include <string.h>
#include "ui_framework/light_web.h"
static int chat_interrupt;
static ui_status_t chat_load(ui_web_view_t *view,const char *html)
{
    if(!chat_interrupt)return ui_web_view_load_html(view,html);
    const char *needle="body.id=id+'-body';",*at=strstr(html,needle);
    const char *fault="if(id==='ai-message-2'&&!globalThis.chatFault){globalThis.chatFault=true;throw Error('isolated chat build interruption')}";
    if(!at)return UI_STATUS_VALIDATION_FAILED;
    size_t head=(size_t)(at+strlen(needle)-html),size=strlen(html)+strlen(fault)+1;
    char *copy=malloc(size);if(!copy)return UI_STATUS_OUT_OF_MEMORY;
    memcpy(copy,html,head);strcpy(copy+head,fault);strcpy(copy+head+strlen(fault),html+head);
    ui_status_t status=ui_web_view_load_html(view,copy);free(copy);return status;
}
#define ui_web_view_load_html chat_load
#define UHA_MAIN assistant_ui_previous_main
#include "uha_host.c"
#undef ui_web_view_load_html

static const char *development_nodes[] = {
    "ai-developer-toggle", "ai-developer-box", "ai-resource", "ai-plan",
    "ai-send", "ai-continue", "ai-edit-cap", "ai-edit-resource",
    "ai-edit-params", "ai-edit-schema", "ai-edit-catalog", "ai-edit-propose",
    "ai-graph-targets", "ai-graph-plan", "ai-graph-propose", "ai-graph-continue",
    "ai-graph-node", "ai-graph-question", "ai-graph-answer", "ai-graph-answer-send",
    "ai-diagnostic-toggle", "command-diagnostics", "param-inputs", "params", "schema", "log"
};

static void actual_text(host_window_t *s, const char *id, const char *expected)
{
    ui_element_presentation_t p = {0};
    p.size = sizeof(p);
    CHECK(ui_web_view_get_presentation(s->view, id, &p) == UI_STATUS_OK);
    if(strcmp(p.text_utf8,expected))fprintf(stderr,"TEXT id=%s expected=%s actual=%s\n",id,expected,p.text_utf8);
    CHECK(p.text_utf8 && !strcmp(p.text_utf8, expected));
}

static void capture_page(host_window_t *s, const wchar_t *folder, const wchar_t *name)
{
    ui_rect_t rect = {0};
    ui_pixel_buffer_t pixels = {0};
    pixels.size = sizeof(pixels);
    CHECK(ui_web_view_get_rect(s->view, &rect) == UI_STATUS_OK);
    CHECK(ui_web_view_capture_rgba(s->view, rect.width, rect.height, 96, &pixels) == UI_STATUS_OK);
    pixels.capacity = pixels.stride * pixels.height;
    pixels.pixels = malloc(pixels.capacity);
    CHECK(pixels.pixels != NULL);
    if (!pixels.pixels) return;
    CHECK(ui_web_view_capture_rgba(s->view, rect.width, rect.height, 96, &pixels) == UI_STATUS_OK);
    for (size_t i = 0; i < pixels.capacity; i += 4) {
        unsigned char red = pixels.pixels[i];
        pixels.pixels[i] = pixels.pixels[i + 2];
        pixels.pixels[i + 2] = red;
    }
    wchar_t path[4096];
    _snwprintf_s(path, 4096, _TRUNCATE, L"%s/%s.bmp", folder, name);
    FILE *file = _wfopen(path, L"wb");
    CHECK(file != NULL);
    if (file) {
        BITMAPFILEHEADER header = {0};
        BITMAPINFOHEADER image = {0};
        image.biSize = sizeof(image);
        image.biWidth = pixels.width;
        image.biHeight = -(LONG)pixels.height;
        image.biPlanes = 1;
        image.biBitCount = 32;
        header.bfType = 0x4d42;
        header.bfOffBits = sizeof(header) + sizeof(image);
        header.bfSize = header.bfOffBits + (DWORD)pixels.capacity;
        CHECK(fwrite(&header, sizeof(header), 1, file) == 1);
        CHECK(fwrite(&image, sizeof(image), 1, file) == 1);
        CHECK(fwrite(pixels.pixels, 1, pixels.capacity, file) == pixels.capacity);
        CHECK(fclose(file) == 0);
    }
    free(pixels.pixels);
}

static void check_resource(host_window_t *s, int development)
{
    char *page = load_page();
    CHECK(page != NULL);
    if (!page) return;
    for (unsigned i = 0; i < sizeof(development_nodes) / sizeof(development_nodes[0]); ++i) {
        ui_element_presentation_t p = {0};
        p.size = sizeof(p);
        ui_status_t status = ui_web_view_get_presentation(s->view, development_nodes[i], &p);
        CHECK(development ? status == UI_STATUS_OK : status != UI_STATUS_OK);
        if (!development) {
            char reference[96];
            snprintf(reference, sizeof(reference), "element('%s')", development_nodes[i]);
            CHECK(strstr(page, reference) == NULL);
        }
    }
    free(page);
}

static void reveal_control(host_window_t *s, const char *id)
{
    ui_element_presentation_t p = {0};
    p.size = sizeof(p);
    for (unsigned step = 0; step < 32; ++step) {
        CHECK(ui_web_view_get_presentation(s->view, id, &p) == UI_STATUS_OK);
        if (p.visible && p.clip.height == p.rect.height) return;
        ui_rect_t area = {0}, developer = {0};
        ui_input_event_t input = {0};
        input.size = sizeof(input);
        input.kind = UI_INPUT_WHEEL;
        const char *scroll = ui_web_view_get_element_rect(s->view, "ai-developer-toggle", &developer) == UI_STATUS_OK ? "assistant" : "ai-scroll";
        CHECK(ui_web_view_get_element_rect(s->view, scroll, &area) == UI_STATUS_OK);
        input.x = area.x + area.width - 4;
        input.y = area.y + area.height / 2;
        input.wheel_delta = p.rect.y < p.clip.y ? 480 : -480;
        CHECK(ui_web_view_dispatch_input(s->view, &input) == UI_STATUS_OK);
        pump(s);
    }
    CHECK(0);
}

static void chat_geometry(host_window_t *s)
{
    ui_element_presentation_t input = {0}, send = {0}, scroll = {0};
    input.size = sizeof(input);
    send.size = sizeof(send);
    scroll.size = sizeof(scroll);
    CHECK(ui_web_view_get_presentation(s->view, "ai-input", &input) == UI_STATUS_OK);
    CHECK(ui_web_view_get_presentation(s->view, "ai-model-propose", &send) == UI_STATUS_OK);
    CHECK(ui_web_view_get_presentation(s->view, "ai-scroll", &scroll) == UI_STATUS_OK);
    CHECK(input.visible && input.clip.height == input.rect.height && input.rect.height >= 60);
    CHECK(send.visible && send.clip.height == send.rect.height && send.clip.width == send.rect.width);
    CHECK(scroll.rect.height >= 150 && scroll.rect.y + scroll.rect.height <= input.rect.y);
    ui_rect_t view = {0}, assistant = {0};
    CHECK(ui_web_view_get_rect(s->view, &view) == UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(s->view, "assistant", &assistant) == UI_STATUS_OK);
    CHECK(assistant.width == (view.width < 640 ? 280 : 380));
    CHECK(assistant.x == 0 && assistant.width < view.width);
    CHECK(input.rect.width >= assistant.width - 70);
    CHECK(input.rect.width <= assistant.width);
    ui_element_presentation_t welcome = {0};
    welcome.size = sizeof(welcome);
    CHECK(ui_web_view_get_presentation(s->view, "ai-empty-title", &welcome) == UI_STATUS_OK);
    if (welcome.visible) {
        CHECK(welcome.rect.width >= 200 && welcome.clip.width == welcome.rect.width);
        actual_text(s, "ai-empty-title", "有什么可以帮你？");
    }
    actual_text(s, "ai-model-propose", "发送消息");
    actual_text(s, "ai-input-label", "消息输入");
}

static const char *reading_reply = "**已读取所选的合成数据。**\n\n当前值为 10，版本为 r1。本次只读取你选定的 memory.document，没有修改应用数据。\n\n- 读取能力：memory.read\n- 返回字段：value、revision\n- 读取范围：仅当前固定目标\n\n可以继续提问，也可以展开任务范围查看这次读取的依据。";

static void reading_response(host_window_t *s)
{
    yyjson_mut_doc *d = yyjson_mut_doc_new(NULL);
    yyjson_mut_doc_set_root(d, yyjson_mut_str(d, reading_reply));
    char *encoded = yyjson_mut_write(d, 0, NULL);
    char wire[4096];
    snprintf(wire, sizeof(wire), "event: response.created\r\ndata: {\"type\":\"response.created\",\"sequence_number\":0,\"response\":{\"id\":\"resp-reading\"}}\r\n\r\n"
        "event: response.output_text.delta\r\ndata: {\"type\":\"response.output_text.delta\",\"sequence_number\":1,\"delta\":%s}\r\n\r\n"
        "event: response.completed\r\ndata: {\"type\":\"response.completed\",\"sequence_number\":2,\"response\":{\"id\":\"resp-reading\",\"status\":\"completed\",\"output\":[{\"type\":\"message\",\"role\":\"assistant\",\"content\":[{\"type\":\"output_text\",\"text\":%s}]}],\"usage\":{\"input_tokens\":10,\"output_tokens\":100}}}\r\n\r\n", encoded, encoded);
    free(encoded);
    yyjson_mut_val *responses = yyjson_mut_arr(d);
    yyjson_mut_doc_set_root(d, responses);
    yyjson_mut_arr_add_strcpy(d, responses, rp_read);
    yyjson_mut_arr_add_strcpy(d, responses, wire);
    char *json = yyjson_mut_write(d, 0, NULL);
    CHECK(ui_ai_model_fixture(s->ai, 1, json) == UI_STATUS_OK);
    free(json);
    yyjson_mut_doc_free(d);
}

static void conversation_path(host_window_t *s, const wchar_t *folder, int reading)
{
    const char *input = "请读取选定的合成值，保持当前版本。";
    const char *draft = "下一条未发送消息\n查看对话和范围后继续编辑。";
    response(s, 1);
    if (reading) reading_response(s);
    text(s, "ai-input", input);
    click(s, "ai-model-propose");
    CHECK(model_wait(s, "\"phase\":3"));
    CHECK(bound(s));
    CHECK(s->ai->model_config->requests == 0);
    DWORD saved = GetTickCount();
    ui_element_presentation_t message = {0};
    message.size = sizeof(message);
    do {
        pump(s);
        if (ui_web_view_get_presentation(s->view, "ai-message-1-body", &message) == UI_STATUS_OK &&
            message.text_utf8 && !strcmp(message.text_utf8, input)) break;
    } while (GetTickCount() - saved < 5000);
    actual_text(s, "ai-message-1-role", "你");
    actual_text(s, "ai-message-1-body", input);
    text(s, "ai-input", draft);
    char run[65];
    strcpy_s(run, sizeof(run), s->ai_run);
    char *preview = _strdup(s->ai_preview);
    CHECK(preview != NULL);
    click(s, "ai-range-close");
    CHECK(!s->ai_preview_ready);
    CHECK(ui_ai_model_fixture_light(s->ai, s->view, "ai-model-confirm") != UI_STATUS_OK);
    chat_geometry(s);
    capture_page(s, folder, L"chat-local-message");
    click(s, "ai-view-scope");
    CHECK(bound(s));
    CHECK(preview && s->ai_preview && !strcmp(preview, s->ai_preview) && !strcmp(run, s->ai_run));
    actual_text(s, "ai-input", draft);
    CHECK(ui_ai_model_fixture_light(s->ai, s->view, "ai-model-confirm") == UI_STATUS_OK);
    CHECK(ui_ai_model_fixture_light(s->ai, s->view, "ai-model-confirm") != UI_STATUS_OK);
    CHECK(model_wait(s, "\"outcome\":\"completed\""));
    CHECK(model_wait(s, "\"committed\":true"));
    DWORD begin = GetTickCount();
    ui_element_presentation_t reply = {0};
    reply.size = sizeof(reply);
    do {
        pump(s);
        if(chat_interrupt)schedule_refresh(s);
        if (ui_web_view_get_presentation(s->view, reading ? "ai-message-2-body-block-0" : "ai-message-2-body", &reply) == UI_STATUS_OK &&
            reply.text_utf8 && (reading ? strstr(reply.text_utf8, "已读取所选的合成数据。") != NULL : !strcmp(reply.text_utf8, "你好，value 10"))) break;
    } while (GetTickCount() - begin < 5000);
    actual_text(s, "ai-message-2-role", "玄枢");
    if (reading) {
        actual_text(s, "ai-message-2-body-block-0", "已读取所选的合成数据。");
        actual_text(s, "ai-message-2-body-block-1", "当前值为 10，版本为 r1。本次只读取你选定的 memory.document，没有修改应用数据。");
        actual_text(s, "ai-message-2-body-block-2", "• 读取能力：memory.read\n• 返回字段：value、revision\n• 读取范围：仅当前固定目标");
    } else actual_text(s, "ai-message-2-body", "你好，value 10");
    actual_text(s, "ai-message-1-body", input);
    actual_text(s, "ai-input", draft);
    CHECK(s->ai->model_config->requests == 2);
    click(s, "ai-range-close");
    chat_geometry(s);
    capture_page(s, folder, L"chat-conversation");
    if (reading && !chat_interrupt && s->chrome_width >= 1000) {
        host_action_t theme = {0};
        theme.action = "theme";
        process_action(s,&theme); pump(s);
        CHECK(s->dark);
        actual_text(s,"ai-input",draft);
        actual_text(s,"ai-message-1-body",input);
        capture_page(s,folder,L"chat-conversation-dark");
        process_action(s,&theme); pump(s);
        CHECK(!s->dark);
    }
    click(s, "ai-model-toggle");
    chat_geometry(s);
    actual_text(s, "ai-input", draft);
    click(s, "ai-model-toggle");
    click(s, "ai-history-toggle");
    chat_geometry(s);
    actual_text(s, "ai-input", draft);
    click(s, "ai-history-toggle");
    if (!reading) actual_text(s, "ai-message-2-body", "你好，value 10");
    CHECK(!strcmp(run, s->ai_run));
    free(preview);
}

static void maintenance_path(host_window_t *s, const wchar_t *package, const wchar_t *folder)
{
    CHECK(phase(s, UI_AI_EDIT_IDLE));
    DWORD begin = GetTickCount();
    while (ai_panel_edit_number(s->ai_edit, "schema") != 1 && GetTickCount() - begin < 5000) pump(s);
    CHECK(ai_panel_edit_number(s->ai_edit, "schema") == 1);
    actual_text(s, "ai-edit-scope", "");
    click(s, "ai-maintenance-toggle");
    reveal_control(s, "ai-edit-migrate");
    ui_element_presentation_t p = {0};
    p.size = sizeof(p);
    CHECK(ui_web_view_get_presentation(s->view, "ai-edit-migrate", &p) == UI_STATUS_OK && p.visible && p.enabled);
    capture_page(s, folder, L"maintenance-migration");
    accepted(s, "ai-edit-migrate");
    begin = GetTickCount();
    while (ai_panel_edit_number(s->ai_edit, "schema") != 2 && GetTickCount() - begin < 5000) pump(s);
    CHECK(ai_panel_edit_number(s->ai_edit, "schema") == 2);
    click(s, "ai-maintenance-toggle");
    open_path(s, package);
    pump(s);
    uint64_t instance = ui_workspace_active(s->workspace);
    ui_ai_edit_request_t request = {0};
    request.size = sizeof(request);
    request.version = 1;
    request.client_request_id = "production-maintenance-unknown";
    request.capability_id = "counter.increment";
    request.parameter_constraint_schema = "{\"type\":\"object\",\"properties\":{},\"additionalProperties\":false}";
    request.params_json = "{}";
    request.target.size = sizeof(request.target);
    request.target.version = 2;
    char operation[65];
    /* Public setup API, then the unchanged production controls and private
       isolated qualified-input fixture. This is not a physical-source claim. */
    CHECK(ui_ai_edit_bind_target(s->ai, instance, "counter.document", &request.target) == UI_STATUS_OK);
    CHECK(ui_ai_edit_propose(s->ai, &request, operation) == UI_STATUS_OK);
    CHECK(phase(s, UI_AI_EDIT_SCOPE_WAIT));
    click(s, "ai-maintenance-toggle");
    CHECK(ui_web_view_get_presentation(s->view, "ai-edit-scope", &p) == UI_STATUS_OK);
    CHECK(p.text_utf8 && strstr(p.text_utf8, "counter.increment") && strstr(p.text_utf8, operation));
    ui_ai_edit_fixture(s->ai, 4, UI_AI_EDIT_READBACK);
    accepted(s, "ai-edit-confirm");
    CHECK(phase(s, UI_AI_EDIT_READBACK));
    ui_ai_edit_fixture(s->ai, 2, 4);
    ui_ai_edit_fixture(s->ai, 4, 0);
    CHECK(phase(s, UI_AI_EDIT_UNKNOWN));
    reveal_control(s, "ai-edit-check");
    CHECK(ui_web_view_get_presentation(s->view, "ai-edit-check", &p) == UI_STATUS_OK && p.visible && p.enabled);
    CHECK(count(s, instance) == 1);
    capture_page(s, folder, L"maintenance-unknown");
    accepted(s, "ai-edit-check");
    CHECK(phase(s, UI_AI_EDIT_DONE));
    CHECK(strstr(snapshot, "reconciled") && count(s, instance) == 1);
    CHECK(s->ai->model_config->requests == 0);
}

int wmain(int argc, wchar_t **argv)
{
    if (argc != 4) return 2;
    printf("assistant UI PID=%lu\n", (unsigned long)GetCurrentProcessId());
    int disabled = !wcscmp(argv[3], L"disabled");
    int development = !wcscmp(argv[3], L"development");
    int general_home = !wcscmp(argv[3], L"general-home");
    chat_interrupt = !wcscmp(argv[3], L"interrupted");
    int reading = chat_interrupt || !wcscmp(argv[3], L"reading") || !wcscmp(argv[3], L"narrow");
    int conversation = reading || !wcscmp(argv[3], L"conversation") || !wcscmp(argv[3], L"compact");
    host_window_t s = {0};
    host_action_t action = {0};
    GUID guid;
    wchar_t unique[40], database[4096];
    CHECK(CreateDirectoryW(argv[2], NULL) || GetLastError() == ERROR_ALREADY_EXISTS);
    CHECK(CoCreateGuid(&guid) == S_OK);
    CHECK(StringFromGUID2(&guid, unique, 40) > 0);
    _snwprintf_s(database, 4096, _TRUNCATE, L"%s/assistant-ui-%s.db", argv[2], unique);
    if (disabled) {
        wchar_t invalid[5000];
        for (unsigned i = 0; i < 4999; ++i) invalid[i] = L'x';
        invalid[4999] = 0;
        CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH", invalid));
    } else CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH", database));
    if (general_home) CHECK(SetEnvironmentVariableW(L"UI_AI_GENERAL_ASSISTANT", L"1"));
    CHECK(ui_framework_initialize() == UI_STATUS_OK);
    HWND created = create_host(&s, GetModuleHandleW(NULL));
    CHECK(created != NULL);
    if (!created) return 1;
    CHECK(MoveWindow(s.hwnd, -16000, -16000, 1280, 1000, FALSE));
    if (!wcscmp(argv[3], L"compact")) CHECK(MoveWindow(s.hwnd, -16000, -16000, 960, 640, FALSE));
    if (!wcscmp(argv[3], L"narrow")) CHECK(MoveWindow(s.hwnd, -16000, -16000, 480, 800, FALSE));
    ShowWindow(s.hwnd, SW_HIDE);
    action.action = "assistant";
    process_action(&s, &action);
    pump(&s);
    check_resource(&s, development);
    if (disabled) {
        CHECK(s.ai == NULL);
        ui_element_presentation_t p = {0};
        p.size = sizeof(p);
        CHECK(ui_web_view_get_presentation(s.view, "ai-service-failure", &p) == UI_STATUS_OK);
        CHECK(p.visible && p.text_utf8 && strstr(p.text_utf8, "UI_AI_DB_PATH is too long"));
        capture_page(&s, argv[2], L"service-unavailable");
    } else if (general_home) {
        CHECK(model_wait(&s, NULL));
        actual_text(&s, "ai-main-title", "玄枢 · 应用助手");
        chat_geometry(&s);
        capture_page(&s, argv[2], L"general-daily-home");
        CHECK(s.ai->model_config->requests == 0 && !s.ai_run[0]);
    } else if (!wcscmp(argv[3], L"maintenance")) {
        CHECK(model_wait(&s, NULL));
        maintenance_path(&s, argv[1], argv[2]);
    } else if (development) {
        CHECK(model_wait(&s, NULL));
        click(&s, "ai-developer-toggle");
        text(&s, "ai-input", "development-only synthetic stub");
        text(&s, "ai-plan", "{\"steps\":[]}");
        click(&s, "ai-send");
        CHECK(model_wait(&s, "\"terminal\":true"));
        CHECK(s.ai->model_config->requests == 0);
        capture_page(&s, argv[2], L"development-only");
    } else {
        CHECK(model_wait(&s, NULL));
        capture_page(&s, argv[2], L"daily-home");
        chat_geometry(&s);
        ui_element_presentation_t p = {0};
        p.size = sizeof(p);
        CHECK(ui_web_view_get_presentation(s.view, "ai-output", &p) == UI_STATUS_OK && !p.visible);
        click(&s, "ai-model-toggle");
        text(&s, "ai-model-name", "offline-fixture");
        click(&s, "ai-model-save");
        CHECK(model_wait(&s, NULL));
        open_path(&s, argv[1]);
        pump(&s);
        click(&s, "ai-draft-range-toggle");
        click(&s, "ai-model-catalog");
        click(&s, "ai-resource-select-0");
        int capability = cap_index(&s);
        CHECK(capability >= 0);
        char id[80];
        snprintf(id, sizeof(id), "ai-cap-select-%d", capability);
        click(&s, id);
        for (unsigned i = 0; i < 2; ++i) {
            snprintf(id, sizeof(id), "ai-field-select-%d-%u", capability, i);
            click(&s, id);
        }
        if (conversation) {
            click(&s, "ai-model-toggle");
            click(&s, "ai-draft-range-toggle");
            conversation_path(&s, argv[2], reading);
            CHECK(close_host(&s));
            CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH", NULL));
            printf("chat checks=%u failures=%u\n", checks, failures);
            return failures ? 1 : 0;
        }
        text(&s, "ai-input", "正式助手界面回归：仅读取本轮合成内存。");
        click(&s, "ai-model-propose");
        CHECK(model_wait(&s, "\"phase\":3"));
        CHECK(bound(&s));
        char run[65];
        strcpy_s(run, sizeof(run), s.ai_run);
        char *preview = _strdup(s.ai_preview);
        CHECK(preview && strstr(preview, "memory.read") && strstr(preview, "memory.document"));
        CHECK(preview && strstr(preview, "revision") && strstr(preview, "value"));
        capture_page(&s, argv[2], L"saved-range");
        text(&s, "ai-input", "未提交草稿：查看范围后保留。");
        click(&s, "ai-detail-toggle");
        CHECK(ui_ai_model_fixture_light(s.ai, s.view, "ai-model-confirm") != UI_STATUS_OK);
        CHECK(ui_web_view_get_presentation(s.view, "ai-model-confirm", &p) == UI_STATUS_OK && !p.enabled);
        capture_page(&s, argv[2], L"details-hidden");
        click(&s, "ai-detail-toggle");
        CHECK(bound(&s));
        actual_text(&s, "ai-input", "未提交草稿：查看范围后保留。");
        CHECK(!strcmp(run, s.ai_run) && preview && s.ai_preview && !strcmp(preview, s.ai_preview));
        CHECK(s.ai->model_config->requests == 0);
        capture_page(&s, argv[2], L"details-restored");
        free(preview);
        click(&s, "ai-cancel");
        CHECK(model_wait(&s, "\"outcome\":\"cancelled\""));
        CHECK(model_wait(&s, "\"committed\":true"));
        actual_text(&s, "ai-input", "未提交草稿：查看范围后保留。");
        capture_page(&s, argv[2], L"cancelled");
        CHECK(s.ai->model_config->requests == 0);
    }
    CHECK(close_host(&s));
    CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH", NULL));
    printf("assistant UI checks=%u failures=%u development=%d disabled=%d\n", checks, failures, development, disabled);
    return failures ? 1 : 0;
}
