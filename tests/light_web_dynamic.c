#include "ui_framework/light_web.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures, messages;
static char last_message[8192];
#define CHECK(condition) do { if (!(condition)) { \
    printf("FAIL %d: %s\n", __LINE__, #condition); ++failures; \
} } while (0)

static void message(ui_web_view_t *view, const char *json, void *user)
{
    (void)view; (void)user;
    ++messages;
    strcpy_s(last_message,sizeof(last_message),json);
}

static ui_status_t input(ui_web_view_t *view, ui_input_kind_t kind,
                          int x, int y, uint32_t key, int wheel)
{
    ui_input_event_t event = {0};
    event.size = sizeof(event); event.kind = kind; event.x = x; event.y = y;
    event.pointer_button = 1; event.key_code = key; event.wheel_delta = wheel;
    return ui_web_view_dispatch_input(view,&event);
}

int main(void)
{
    const char *html =
        "<style>body{margin:0;color:#233347;font-family:Segoe UI;}"
        "#shell{display:flex;flex-direction:row;width:100%;height:100%;padding:10px;gap:8px;}"
        ".main{flex:1;position:relative;}#sidebar{width:200px;height:100%;background:#f5f6fa;}"
        "button{height:32px;border:1px solid #d2d8e2;border-radius:6px;}"
        "button:hover{background:#eeeeff;}button:focus{border-color:#3674d9;}"
        "#box{position:absolute;left:12px;top:80px;width:100px;height:40px;}"
        "#scroll{height:80px;overflow-y:auto;}#scroll div{height:50px;}"
        "@media(max-width:600px){#sidebar{display:none;}}</style>"
        "<div id='shell'><main id='main' class='main'>"
        "<button id='action'>Click</button><input id='text' value='seed'>"
        "<textarea id='notes' style='height:60px'>notes</textarea>"
        "<div id='holder'></div><div id='scroll'><div id='one'>One</div><div>Two</div><div>Three</div></div>"
        "<div id='box'>Absolute</div></main><aside id='sidebar'>Assistant</aside></div>"
        "<script>var presses=0;var edits=0;"
        "document.getElementById('action').addEventListener('click',function(){ui.postMessage({presses:++presses});});"
        "document.getElementById('text').addEventListener('input',function(){edits++;});"
        "ui.onmessage=function(data){var holder=document.getElementById('holder');"
        "if(data.op==='rebuild'){while(holder.firstChild)holder.removeChild(holder.firstChild);"
        "for(var i=0;i<5;i++){var b=document.createElement('button');b.id='dynamic-'+i;b.textContent='Item '+i;"
        "b.style.height='22px';holder.appendChild(b);}}"
        "if(data.op==='value')document.getElementById('box').textContent=data.text;"
        "if(data.op==='disabled')document.getElementById('action').disabled=data.value;"
        "if(data.op==='theme')document.body.className=data.value;"
        "if(data.op==='snapshot')ui.postMessage({value:document.getElementById('text').value,"
        "text:document.getElementById('box').textContent,edits:edits,children:holder.children.length});};"
        "window.addEventListener('message',function(event){if(event.data.op==='window')ui.postMessage({window:true});});"
        "ui.postMessage({ready:true});</script>";
    HWND parent, web_hwnd;
    ui_host_config_t hc = {0};
    ui_light_web_config_t config = {0};
    ui_host_t *host;
    ui_web_backend_t *backend;
    ui_web_view_t *view;
    ui_rect_t before, after, rect;
    uint64_t capabilities = 0;
    int i, count;
    CHECK(ui_framework_initialize() == UI_STATUS_OK);
    parent = CreateWindowExW(0,L"STATIC",L"Dynamic Web probe",WS_OVERLAPPEDWINDOW,
        20,20,900,700,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if (!parent) return 1;
    hc.size = sizeof(hc); hc.api_version = UI_FRAMEWORK_API_VERSION; hc.native_parent = parent;
    host = ui_host_create(&hc); if (!host) return 1;
    config.size = sizeof(config); config.parent_hwnd = parent; config.enable_native_input = 1;
    backend = ui_light_web_backend_create(&config); if (!backend) return 1;
    view = ui_web_view_create(host,backend); if (!view) return 1;
    CHECK(ui_web_view_set_message_callback(view,message,NULL) == UI_STATUS_OK);
    CHECK(ui_web_view_load_html(view,html) == UI_STATUS_OK);
    CHECK(messages == 1 && !strcmp(last_message,"{\"ready\":true}"));
    CHECK(ui_web_view_get_capabilities(view,&capabilities) == UI_STATUS_OK);
    CHECK((capabilities & (UI_WEB_CAP_JSON_MESSAGES | UI_WEB_CAP_DYNAMIC_DOM | UI_WEB_CAP_NATIVE_WINDOW)) ==
        (UI_WEB_CAP_JSON_MESSAGES | UI_WEB_CAP_DYNAMIC_DOM | UI_WEB_CAP_NATIVE_WINDOW));
    CHECK(ui_web_view_resize(view,800,600,96) == UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(view,"sidebar",&rect) == UI_STATUS_OK && rect.width == 200 && rect.x == 590);
    CHECK(ui_web_view_get_element_rect(view,"box",&rect) == UI_STATUS_OK && rect.x == 22 && rect.y == 90 && rect.width == 100);
    CHECK(ui_web_view_get_element_rect(view,"action",&rect) == UI_STATUS_OK);
    CHECK(input(view,UI_INPUT_POINTER_DOWN,rect.x+4,rect.y+4,0,0) == UI_STATUS_OK);
    CHECK(input(view,UI_INPUT_POINTER_UP,rect.x+4,rect.y+4,0,0) == UI_STATUS_OK);
    CHECK(strstr(last_message,"\"presses\":1") != NULL);
    CHECK(ui_web_view_post_json(view,"{\"op\":\"disabled\",\"value\":true}") == UI_STATUS_OK);
    count = messages;
    CHECK(input(view,UI_INPUT_POINTER_DOWN,rect.x+4,rect.y+4,0,0) == UI_STATUS_OK);
    CHECK(input(view,UI_INPUT_POINTER_UP,rect.x+4,rect.y+4,0,0) == UI_STATUS_OK);
    CHECK(messages == count);
    CHECK(ui_web_view_post_json(view,"{\"op\":\"disabled\",\"value\":false}") == UI_STATUS_OK);
    CHECK(ui_web_view_post_json(view,"{\"op\":\"value\",\"text\":\"中文\\\"\\\\\\n<safe>\"}") == UI_STATUS_OK);
    CHECK(ui_web_view_post_json(view,"{\"op\":\"snapshot\"}") == UI_STATUS_OK);
    CHECK(strstr(last_message,"中文\\\"\\\\\\n<safe>") != NULL);
    CHECK(ui_web_view_post_json(view,"invalid JSON") == UI_STATUS_VALIDATION_FAILED);
    CHECK(ui_web_view_post_json(view,"{\"op\":\"window\"}") == UI_STATUS_OK && strstr(last_message,"\"window\":true") != NULL);
    web_hwnd = (HWND)ui_web_view_native_handle(view); CHECK(web_hwnd != NULL);
    CHECK(FindWindowExW(web_hwnd,NULL,L"EDIT",NULL) == NULL);
    ShowWindow(parent,SW_SHOWNOACTIVATE);SetFocus(web_hwnd);
    CHECK(ui_web_view_get_element_rect(view,"text",&rect) == UI_STATUS_OK);
    CHECK(input(view,UI_INPUT_POINTER_DOWN,rect.x+5,rect.y+5,0,0) == UI_STATUS_OK);
    {ui_input_event_t event={0};event.size=sizeof(event);event.kind=UI_INPUT_KEY_DOWN;event.key_code='A';event.modifiers=UI_INPUT_MODIFIER_CONTROL;
     CHECK(ui_web_view_dispatch_input(view,&event)==UI_STATUS_OK);event.kind=UI_INPUT_TEXT;event.text_utf8="中文 input";
     CHECK(ui_web_view_dispatch_input(view,&event)==UI_STATUS_OK);}
    CHECK(ui_web_view_post_json(view,"{\"op\":\"snapshot\"}") == UI_STATUS_OK);
    CHECK(strstr(last_message,"中文 input") != NULL && strstr(last_message,"\"edits\":1") != NULL);
    CHECK(ui_web_view_get_element_rect(view,"scroll",&rect) == UI_STATUS_OK);
    CHECK(input(view,UI_INPUT_WHEEL,rect.x+10,rect.y+10,0,-120) == UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(view,"one",&before) == UI_STATUS_OK);
    for (i = 0; i < 250; ++i) CHECK(ui_web_view_post_json(view,"{\"op\":\"rebuild\"}") == UI_STATUS_OK);
    CHECK(GetFocus() == web_hwnd);
    CHECK(FindWindowExW(web_hwnd,NULL,L"EDIT",NULL) == NULL);
    CHECK(ui_web_view_get_element_rect(view,"one",&after) == UI_STATUS_OK);
    /* Rebuild inserts a fixed 110px sibling before scroll, preserving offset. */
    CHECK(after.y - before.y == 110);
    CHECK(ui_web_view_post_json(view,"{\"op\":\"snapshot\"}") == UI_STATUS_OK);
    CHECK(strstr(last_message,"\"children\":5") != NULL && strstr(last_message,"中文 input") != NULL);
    CHECK(ui_web_view_resize(view,500,600,144) == UI_STATUS_OK);
    CHECK(ui_web_view_get_element_rect(view,"sidebar",&rect) == UI_STATUS_OK && rect.width == 0);
    CHECK(ui_web_view_native_handle(view) == web_hwnd);
    ui_web_view_destroy(view); ui_light_web_backend_destroy(backend); ui_host_destroy(host); DestroyWindow(parent);
    printf("Dynamic Web probe: %d failure(s), %d messages, 250 incremental rebuilds\n",failures,messages);
    return failures ? 1 : 0;
}
