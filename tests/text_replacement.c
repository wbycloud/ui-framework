/* Public input contract. Desktop SendInput/visibility is validated separately. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <string.h>
#include "ui_framework/light_web.h"
#include "ui_framework/webview2.h"
#include "ui_framework/components.h"
static int failures, replies;
static char response[8192];
#define CHECK(x) do { if (!(x)) { printf("replacement line%d: %s\n",__LINE__,#x); ++failures; } } while(0)
static void pump(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}Sleep(1);}
static void received(ui_web_view_t *v,const char *json,void *data)
{(void)v;(void)data;strcpy_s(response,sizeof(response),json);++replies;}
static void settle(ui_web_view_t *v)
{ULONGLONG end=GetTickCount64()+10000;ui_status_t s;do{s=ui_web_view_flush(v,64);if(s==UI_STATUS_OK)return;pump();}while(GetTickCount64()<end);CHECK(s==UI_STATUS_OK);}
static void web_key(ui_web_view_t *v,uint32_t k,uint32_t mods)
{ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_KEY_DOWN;e.key_code=k;e.modifiers=mods;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);settle(v);}
static void web_text(ui_web_view_t *v,const char *text)
{ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_TEXT;e.text_utf8=text;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);settle(v);}
static void expect(ui_web_view_t *v,const char *value,const char *label)
{char want[8192];size_t at=0;int before=replies;ULONGLONG end=GetTickCount64()+10000;want[at++]='"';for(const char *s=value;*s&&at<sizeof(want)-3;++s){if(*s=='\n'){want[at++]='\\';want[at++]='n';}else{if(*s=='"'||*s=='\\')want[at++]='\\';want[at++]=*s;}}want[at++]='"';want[at]=0;CHECK(ui_web_view_post_json(v,"{}") == UI_STATUS_OK);do{pump();}while(replies==before&&GetTickCount64()<end);CHECK(replies>before);printf("%s observed=%s expected=%s\n",label,response,want);CHECK(!strcmp(response,want));}
static void focus(ui_web_view_t *v,const char *id)
{ui_rect_t r;ui_input_event_t e={0};ui_status_t s;ULONGLONG end=GetTickCount64()+10000;do{s=ui_web_view_get_element_rect(v,id,&r);if(s==UI_STATUS_OK)break;pump();}while(GetTickCount64()<end);CHECK(s==UI_STATUS_OK);if(s!=UI_STATUS_OK)return;e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.x=r.x+6;e.y=r.y+6;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);settle(v);}
static void expect_inputs(ui_web_view_t *v,const char *count)
{int before=replies;ULONGLONG end=GetTickCount64()+10000;CHECK(ui_web_view_post_json(v,"{\"stats\":true}")==UI_STATUS_OK);do{pump();}while(replies==before&&GetTickCount64()<end);CHECK(replies>before);printf("input events observed=%s expected=%s\n",response,count);CHECK(!strcmp(response,count));}
int main(int argc,char **argv)
{
    int runtime=argc>1&&!strcmp(argv[1],"--webview2");HWND root;ui_host_config_t hc={0};ui_host_t *h;ui_web_backend_t *b;ui_web_view_t *v;
    ui_light_web_config_t lc={0};ui_webview2_backend_config_t wc={0};ui_status_t status;
    const char *page="<body><input id='edit' value='0'><textarea id='multi'>中</textarea><script>var edits=0;document.getElementById('edit').addEventListener('input',function(){edits++});ui.onmessage=function(m){if(m.same)document.activeElement.value=document.activeElement.value;ui.postMessage(m.stats?String(edits):JSON.stringify(document.activeElement.value))};</script></body>";
    CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));CHECK(ui_framework_initialize()==UI_STATUS_OK);
    root=CreateWindowW(L"STATIC",L"Public replacement",WS_OVERLAPPEDWINDOW,40,40,600,400,NULL,NULL,NULL,NULL);CHECK(root);
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;hc.native_parent=root;h=ui_host_create(&hc);CHECK(h);if(!h)return 1;
    if(runtime){
#ifdef UI_REPLACEMENT_WEBVIEW2
        wc.size=sizeof(wc);wc.parent_window=root;wc.framework_components=1;b=ui_webview2_backend_create(&wc,&status);CHECK(status==UI_STATUS_OK);
#else
        (void)wc;return 2;
#endif
    }
    else{lc.size=sizeof(lc);lc.parent_hwnd=root;b=ui_light_web_backend_create(&lc);}CHECK(b);if(!b)return 1;
    v=ui_web_view_create(h,b);CHECK(v);if(!v)return 1;CHECK(ui_web_view_set_message_callback(v,received,NULL)==UI_STATUS_OK);CHECK(ui_web_view_load_html(v,page)==UI_STATUS_OK);CHECK(ui_web_view_resize(v,540,300,96)==UI_STATUS_OK);settle(v);
    focus(v,"edit");web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"0");expect_inputs(v,"1");web_text(v,".");web_text(v,"0");web_text(v,"0");web_text(v,"2");expect(v,"0.002","same first character");
    web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"abc");web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"abc");web_text(v,"!");expect(v,"abc!","identical selection");
    web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"00");web_key(v,VK_HOME,0);web_key(v,VK_RIGHT,UI_INPUT_MODIFIER_SHIFT);web_text(v,"0");web_text(v,"x");expect(v,"0x0","partial identical selection");
    web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"abcd");web_key(v,VK_HOME,0);web_key(v,VK_RIGHT,0);web_text(v,"X");expect(v,"aXbcd","middle insertion");web_key(v,VK_BACK,0);web_key(v,VK_DELETE,0);expect(v,"acd","backspace and delete");
    web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"");expect(v,"","empty replacement");web_text(v,"7");web_key(v,VK_BACK,0);web_key(v,VK_BACK,0);expect(v,"","empty backspace");
    focus(v,"multi");web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"中");web_text(v,"文");expect(v,"中文","Chinese identical selection");
    web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"甲\n乙");web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"甲\n乙");web_text(v,"\n末");expect(v,"甲\n乙\n末","identical multiline");
    /* A content-changing edit remains undoable/redoable; no-op replacement is
       followed by actual text, rather than pretending a getter sees selection. */
    web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"undo");web_key(v,VK_HOME,0);web_text(v,"X");web_key(v,'Z',UI_INPUT_MODIFIER_CONTROL);expect(v,"undo","undo");web_key(v,'Y',UI_INPUT_MODIFIER_CONTROL);expect(v,"Xundo","redo");
    CHECK(ui_web_view_load_html(v,page)==UI_STATUS_OK);settle(v);focus(v,"edit");web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);web_text(v,"0");web_key(v,'Z',UI_INPUT_MODIFIER_CONTROL);expect(v,"0","identical replacement undo value");web_key(v,'Y',UI_INPUT_MODIFIER_CONTROL);web_text(v,".002");expect(v,"0.002","identical replacement redo caret");
    web_key(v,'A',UI_INPUT_MODIFIER_CONTROL);{int before=replies;ULONGLONG end=GetTickCount64()+10000;CHECK(ui_web_view_post_json(v,"{\"same\":true}")==UI_STATUS_OK);do{pump();}while(replies==before&&GetTickCount64()<end);CHECK(replies>before);}web_text(v,"x");expect(v,"x","equal script setter preserves existing selection");
    focus(v,"multi");focus(v,"edit");web_key(v,VK_END,0);web_text(v,"y");expect(v,"xy","focus switch and next input");
    for(uint32_t dpi=96;dpi<=192;dpi+=48){
        ui_element_presentation_t p={0};ULONGLONG end;
        CHECK(ui_web_view_load_html(v,"<body><div style='height:110px;width:220px;overflow:auto'><div style='height:220px;overflow:auto'><input id='focus-first' value='first'><div style='height:220px'></div><input id='focus-last' value='LAST FOCUS'></div></div></body>")==UI_STATUS_OK);
        CHECK(ui_web_view_resize(v,240,180,dpi)==UI_STATUS_OK);settle(v);focus(v,"focus-first");web_key(v,VK_TAB,0);p.size=sizeof(p);end=GetTickCount64()+10000;
        do{status=ui_web_view_get_presentation(v,"focus-last",&p);if(status!=UI_STATUS_PENDING)break;pump();}while(GetTickCount64()<end);
        printf("nested focus dpi%u focused%d visible%d rect%d,%d,%d,%d clip%d,%d,%d,%d\n",dpi,p.focused,p.visible,p.rect.x,p.rect.y,p.rect.width,p.rect.height,p.clip.x,p.clip.y,p.clip.width,p.clip.height);
        CHECK(status==UI_STATUS_OK&&p.focused&&p.visible&&p.clip.height>=20);
    }
    ui_web_view_destroy(v);
    {ui_component_desc_t d={0};ui_field_desc_t f={0};ui_component_t *c=NULL;ui_native_shell_t *shell=NULL;ui_cell_t value={0},got={0};ui_element_presentation_t p={0};ui_input_event_t e={0};ULONGLONG end;
     if(runtime){ui_native_shell_config_t sc={0};ui_layout_desc_t layout={0};layout.size=sizeof(layout);CHECK(ui_host_set_layout(h,&layout)==UI_STATUS_OK);CHECK(ui_host_resize(h,540,300)==UI_STATUS_OK);sc.size=sizeof(sc);sc.host=h;sc.native_parent=root;shell=ui_native_shell_create_web(&sc);CHECK(shell);}
     f.size=sizeof(f);f.id="value";f.title="Number";f.kind=UI_VALUE_TEXT;d.size=sizeof(d);d.id="replace";d.title="Public FORM";d.kind=UI_COMPONENT_FORM;d.fields=&f;d.field_count=1;d.web_backend=runtime?b:NULL;
     CHECK(ui_component_register(h,&d,&c)==UI_STATUS_OK);value.size=sizeof(value);value.kind=UI_VALUE_TEXT;value.text="0";CHECK(ui_component_set_field(c,"value",&value)==UI_STATUS_OK);CHECK((runtime?ui_component_mount(c,ui_shell_get_content_slot(ui_host_get_shell(h),NULL)):ui_component_mount_offscreen(c,360,240,96))==UI_STATUS_OK);
     p.size=sizeof(p);end=GetTickCount64()+10000;do{status=ui_component_get_presentation(c,"field-value",&p);if(status==UI_STATUS_OK)break;pump();}while(GetTickCount64()<end);CHECK(status==UI_STATUS_OK&&p.visible);
     if(status==UI_STATUS_OK){e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.x=p.rect.x+5;e.y=p.rect.y+5;CHECK(ui_component_dispatch_input(c,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;CHECK(ui_component_dispatch_input(c,&e)==UI_STATUS_OK);e.kind=UI_INPUT_KEY_DOWN;e.key_code='A';e.modifiers=UI_INPUT_MODIFIER_CONTROL;CHECK(ui_component_dispatch_input(c,&e)==UI_STATUS_OK);
      for(const char *at="0.002";*at;++at){char one[2]={*at,0};e.kind=UI_INPUT_TEXT;e.modifiers=0;e.text_utf8=one;CHECK(ui_component_dispatch_input(c,&e)==UI_STATUS_OK);end=GetTickCount64()+10000;while(ui_component_flush(c,64)==UI_STATUS_PENDING&&GetTickCount64()<end)pump();}
      got.size=sizeof(got);CHECK(ui_component_get_field(c,"value",&got)==UI_STATUS_OK);printf("FORM observed=%s expected=0.002\n",got.text);CHECK(!strcmp(got.text,"0.002"));}
     CHECK(ui_component_unregister(c)==UI_STATUS_OK);if(shell)ui_native_shell_destroy(shell);}
    ui_host_destroy(h);
#ifdef UI_REPLACEMENT_WEBVIEW2
    if(runtime)ui_webview2_backend_destroy(b);else
#endif
    ui_light_web_backend_destroy(b);DestroyWindow(root);{ULONGLONG end=GetTickCount64()+100;while(GetTickCount64()<end)pump();}CoUninitialize();printf("Replacement %s failures=%d\n",runtime?"Runtime":"Light",failures);return failures?1:0;
}
