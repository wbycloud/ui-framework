#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui_framework/application.h"
#include "ui_framework/package.h"
#include "ui_framework/shell.h"

static int failed;
#define CHECK(expression) do { if (!(expression)) { \
    fprintf(stderr, "FAIL %d: %s\n", __LINE__, #expression); ++failed; } } while (0)

static ui_app_instance_info_t instance(ui_workspace_t *workspace, uint64_t id)
{
    ui_app_instance_info_t info = {0}; info.size = sizeof(info);
    CHECK(ui_workspace_get_instance(workspace, id, &info) == UI_STATUS_OK);
    return info;
}

static int state_contains(ui_workspace_t *workspace, uint64_t id, const char *value)
{
    ui_app_instance_info_t info = instance(workspace, id);
    const char *json = NULL;
    return ui_assistant_get_state_snapshot(info.assistant, &json) == UI_STATUS_OK &&
           json && strstr(json, value) != NULL;
}

int wmain(int argc, wchar_t **wide_argv)
{
    ui_package_t *package = NULL;
    ui_workspace_config_t config = {0};
    ui_workspace_t *workspace;
    ui_host_config_t host_config = {0};
    ui_host_t *host;
    ui_rect_t rect = {0, 0, 800, 600};
    HWND parent;
    uint64_t old_id = 0, old_two = 0, new_id = 0, new_two = 0, wrong = 0, request;
    ui_app_instance_info_t old_info;
    HWND panel;
    char error[256];
    char *argv[4] = {NULL};
    int arg;
    if (argc != 4) return 2;
    for (arg = 1; arg < 4; ++arg) {
        int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
            wide_argv[arg], -1, NULL, 0, NULL, NULL);
        if (!length) return 2;
        argv[arg] = (char *)malloc((size_t)length);
        if (!argv[arg]) return 2;
        if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide_argv[arg],
                                -1, argv[arg], length, NULL, NULL)) return 2;
    }
    CHECK(ui_framework_supports_api(1) && ui_framework_supports_api(2));
    CHECK(!ui_framework_supports_api(0) && ui_framework_supports_api(4)&&ui_framework_supports_api(5)&&ui_framework_supports_api(6)&&ui_framework_supports_api(7)&&!ui_framework_supports_api(8));
    host_config.size = sizeof(host_config);
    host_config.api_version = 1; host = ui_host_create(&host_config);
    CHECK(host != NULL); ui_host_destroy(host);
    host_config.api_version = 2; host = ui_host_create(&host_config);
    CHECK(host != NULL); ui_host_destroy(host);
    host_config.api_version = 3; host = ui_host_create(&host_config); CHECK(host != NULL); ui_host_destroy(host);
    host_config.api_version = 5; host=ui_host_create(&host_config);CHECK(host!=NULL);ui_host_destroy(host);
    host_config.api_version = 6; host=ui_host_create(&host_config);CHECK(host!=NULL);ui_host_destroy(host); host_config.api_version = 8; CHECK(ui_host_create(&host_config) == NULL);
    CHECK(ui_package_open(argv[1], &package, error, sizeof(error)) == UI_STATUS_OK);
    CHECK(package && ui_package_get_metadata(package)->framework_api_version == 1);
    ui_package_destroy(package); package = NULL;
    CHECK(ui_package_open(argv[2], &package, error, sizeof(error)) == UI_STATUS_OK);
    CHECK(package && ui_package_get_metadata(package)->framework_api_version == 2);
    ui_package_destroy(package);
    parent = CreateWindowExW(0, L"STATIC", L"API compatibility", WS_OVERLAPPEDWINDOW,
        0, 0, 1000, 760, NULL, NULL, GetModuleHandleW(NULL), NULL);
    CHECK(parent != NULL);
    config.size = sizeof(config); config.native_parent = parent;
    config.max_permission = UI_ASSISTANT_PERMISSION_EDIT;
    config.shell_mode = UI_WORKSPACE_SHELL_WEB;
    workspace = ui_workspace_create(&config);
    CHECK(workspace != NULL); if (!workspace) return 1;
    CHECK(ui_workspace_set_rect(workspace, &rect, 96) == UI_STATUS_OK);
    CHECK(ui_workspace_open(workspace, argv[1], &old_id) == UI_STATUS_OK);
    CHECK(ui_workspace_open(workspace, argv[1], &old_two) == UI_STATUS_OK);
    CHECK(old_id && old_two && old_id != old_two);
    old_info = instance(workspace, old_id);
    panel = (HWND)ui_native_shell_panel_handle(old_info.shell, "eda.properties");
    CHECK(IsWindow(panel));
    CHECK(ui_workspace_invoke(workspace, old_id, "eda.add_block", "{}", &request) == UI_STATUS_OK);
    CHECK(state_contains(workspace, old_id, "\"blocks\":1"));
    CHECK(state_contains(workspace, old_two, "\"blocks\":0"));
    CHECK(ui_workspace_open(workspace, argv[2], &new_id) == UI_STATUS_OK);
    CHECK(ui_workspace_open(workspace, argv[2], &new_two) == UI_STATUS_OK);
    CHECK(ui_workspace_invoke(workspace, new_id, "counter.increment", "{}", &request) == UI_STATUS_OK);
    CHECK(state_contains(workspace, new_id, "\"count\":1"));
    CHECK(state_contains(workspace, new_two, "\"count\":0"));
    CHECK(ui_workspace_open(workspace, argv[3], &wrong) == UI_STATUS_UNSUPPORTED);
    CHECK(wrong == 0 && strstr(ui_workspace_last_error(workspace), "API") != NULL);
    CHECK(ui_workspace_count(workspace) == 4);
    CHECK(ui_workspace_activate(workspace, old_id) == UI_STATUS_OK);
    CHECK((HWND)ui_native_shell_panel_handle(old_info.shell, "eda.properties") == panel);
    CHECK(GetMenu(parent) == NULL);
    CHECK(ui_workspace_close_all(workspace) == UI_STATUS_OK);
    ui_workspace_poll(workspace);
    CHECK(ui_workspace_count(workspace) == 0);
    CHECK(ui_workspace_post_event(workspace, old_id, "late.event", "{}") == UI_STATUS_OK);
    ui_workspace_poll(workspace);
    CHECK(ui_workspace_destroy(workspace) == UI_STATUS_OK);
    DestroyWindow(parent);
    for (arg = 1; arg < 4; ++arg) free(argv[arg]);
    printf("application API1/API2 and Web/native contents: %s\n", failed ? "FAILED" : "PASS");
    return failed ? 1 : 0;
}
