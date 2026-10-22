#include "ui_framework/application.h"
#include "ui_framework/light_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%d: %s\n", __LINE__, #x); ++failures; } } while (0)
static int commands, messages, navigations;
static char navigation_url[4096];
static void command(ui_host_t *h, uint64_t id, const char *c, const char *p, const char *s, void *u)
{ (void)c; (void)p; (void)s; (void)u; ++commands; (void)ui_host_reply(h,id,1,"{}"); }
static void message(ui_web_view_t *v, const char *j, void *u)
{ (void)v; (void)j; (void)u; ++messages; }
static void event(ui_web_view_t *v, const ui_document_event_t *e, void *u)
{ (void)v; (void)u; if(e->kind==UI_DOCUMENT_NAVIGATE){++navigations;strcpy_s(navigation_url,sizeof(navigation_url),e->url_utf8);} }
int main(void)
{
    ui_host_config_t hc={0}; ui_light_web_config_t bc={0}; ui_command_desc_t c={0};
    ui_web_backend_t *b; ui_web_view_t *v; ui_host_t *h;
    ui_document_load_t load={0}; ui_document_state_t state={0};
    ui_element_presentation_t p={0}; ui_input_event_t input={0};
    hc.size=sizeof(hc); hc.api_version=UI_FRAMEWORK_API_VERSION;
    h=ui_host_create(&hc); CHECK(h!=NULL); if(!h)return 1;
    c.size=sizeof(c); c.id="escape"; c.title="escape"; c.handler=command;
    CHECK(ui_host_register_command(h,&c)==UI_STATUS_OK);
    bc.size=sizeof(bc); b=ui_light_document_backend_create(&bc); CHECK(b!=NULL);
    v=ui_web_view_create(h,b); CHECK(v!=NULL); if(!v)return 1;
    CHECK(ui_web_view_resize(v,640,360,96)==UI_STATUS_OK);
    CHECK(ui_web_view_load_html(v,"<script>ui.invoke('escape',{});</script>")==UI_STATUS_UNSUPPORTED);
    CHECK(ui_web_view_set_message_callback(v,message,NULL)==UI_STATUS_UNSUPPORTED);
    CHECK(ui_web_view_post_json(v,"{}")==UI_STATUS_UNSUPPORTED);
    CHECK(ui_light_document_set_event_callback(v,event,NULL)==UI_STATUS_OK);
    load.size=sizeof(load); load.navigation_id=7; load.source_url="https://example.test/a/";
    load.html_utf8="<title>隔离测试</title><style>body{color:#123456}</style>"
        "<h1 id='heading'>Hello 中文</h1><p>before <strong>bold</strong> after<br>next</p>"
        "<script>ui.invoke('escape',{});ui.postMessage({escape:true});while(true){}</script>"
        "<a id='link' href='../next?q=中文' onclick=\"ui.invoke('escape',{})\">next page</a>"
        "<a href='javascript:ui.invoke(\"escape\",{})'>blocked</a>"
        "<iframe src='file:///C:/secret'></iframe>";
    CHECK(ui_light_document_load(v,&load)==UI_STATUS_OK);
    CHECK(ui_web_view_flush(v,64)==UI_STATUS_OK);
    state.size=sizeof(state); CHECK(ui_light_document_get_state(v,&state)==UI_STATUS_OK);
    CHECK(state.navigation_id==7 && state.script_runtime==0 && state.restrictions>0);
    CHECK(strstr(state.title_utf8,"隔离")!=NULL);
    p.size=sizeof(p); CHECK(ui_web_view_get_presentation(v,"link",&p)==UI_STATUS_OK);
    input.size=sizeof(input); input.kind=UI_INPUT_POINTER_DOWN; input.pointer_button=1;
    input.x=p.clip.x+2; input.y=p.clip.y+2;
    CHECK(ui_web_view_dispatch_input(v,&input)==UI_STATUS_OK);
    input.kind=UI_INPUT_POINTER_UP; CHECK(ui_web_view_dispatch_input(v,&input)==UI_STATUS_OK);
    CHECK(navigations==1 && commands==0 && messages==0);
    CHECK(strstr(navigation_url,"https://example.test/next?q=%E4%B8%AD%E6%96%87")!=NULL);
    CHECK(ui_web_view_load_html(v,"<p>trusted?</p>")==UI_STATUS_UNSUPPORTED);
    load.navigation_id=8;load.html_utf8="<style>body {background:#123456;color:#ffffff;}</style><p id='color'>Author colors</p>";
    CHECK(ui_light_document_load(v,&load)==UI_STATUS_OK);
    {ui_pixel_buffer_t pixels={0};pixels.size=sizeof(pixels);CHECK(ui_web_view_capture_rgba(v,640,360,96,&pixels)==UI_STATUS_OK);pixels.capacity=pixels.stride*pixels.height;pixels.pixels=(uint8_t *)malloc(pixels.capacity);
        CHECK(pixels.pixels!=NULL);if(pixels.pixels){CHECK(ui_web_view_capture_rgba(v,640,360,96,&pixels)==UI_STATUS_OK);CHECK(pixels.pixels[0]==0x12&&pixels.pixels[1]==0x34&&pixels.pixels[2]==0x56);free(pixels.pixels);}}
    load.navigation_id=9;load.html_utf8="<div></div><script>document.body.textContent='SPA'</script>";
    CHECK(ui_light_document_load(v,&load)==UI_STATUS_UNSUPPORTED);CHECK(commands==0&&messages==0);
    load.html_utf8="<p style='font-size:10000px'>Excessive font</p>";
    CHECK(ui_light_document_load(v,&load)==UI_STATUS_LIMIT_EXCEEDED);
    ui_web_view_destroy(v); ui_light_web_backend_destroy(b); ui_host_destroy(h);
    printf("public document check: failures=%d\n",failures); return failures?1:0;
}
