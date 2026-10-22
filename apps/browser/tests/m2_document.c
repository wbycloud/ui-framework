/* Reading compatibility gate. Only public C framework headers and calls. */
#include "ui_framework/application.h"
#include "ui_framework/light_document.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int failures,submits;
static char fields[4096];static uint64_t resource;
#define T(x) do{if(!(x)){fprintf(stderr,"M2 line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void event(ui_web_view_t *v,const ui_document_event_t *e,void *u)
{(void)v;(void)u;if(e->kind==UI_DOCUMENT_STYLESHEET)resource=e->resource_id;if(e->kind==UI_DOCUMENT_SUBMIT){size_t at=0;++submits;fields[0]=0;for(size_t i=0;i<e->field_count;++i)at+=(size_t)snprintf(fields+at,sizeof(fields)-at,"%s=%s;",e->fields[i].name_utf8,e->fields[i].value_utf8);}}
static ui_rect_t rect(ui_web_view_t *v,const char *id)
{ui_rect_t r={0};T(ui_web_view_get_element_rect(v,id,&r)==UI_STATUS_OK);return r;}
static void click(ui_web_view_t *v,const char *id)
{ui_rect_t r=rect(v,id);ui_input_event_t e={0};e.size=sizeof(e);e.kind=UI_INPUT_POINTER_DOWN;e.pointer_button=1;e.x=r.x+2;e.y=r.y+2;T(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);e.kind=UI_INPUT_POINTER_UP;T(ui_web_view_dispatch_input(v,&e)==UI_STATUS_OK);}
static void load(ui_web_view_t *v,const char *html)
{static uint64_t nav;ui_document_load_t d={0};d.size=sizeof(d);d.navigation_id=++nav;d.source_url="https://fixture.test/docs/page";d.html_utf8=html;T(ui_light_document_load(v,&d)==UI_STATUS_OK);T(ui_web_view_flush(v,64)==UI_STATUS_OK);}
int main(void)
{
    ui_host_config_t hc={0};ui_light_web_config_t bc={0};ui_host_t *h;ui_web_backend_t *b;ui_web_view_t *v;ui_rect_t a,c,d;
    hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;h=ui_host_create(&hc);T(h!=NULL);if(!h)return 1;
    bc.size=sizeof(bc);b=ui_light_document_backend_create(&bc);v=ui_web_view_create(h,b);T(v!=NULL);if(!v)return 1;T(ui_web_view_resize(v,640,420,96)==UI_STATUS_OK);T(ui_light_document_set_event_callback(v,event,NULL)==UI_STATUS_OK);
    load(v,"<ol start='7'><li>Alpha<li value='11'>Beta<li>Gamma<ul><li>Nested</ul></ol><ol reversed><li>Delta<li>Epsilon<li value='-2'>Zeta<li>Eta</ol>");
    T(ui_light_document_find(v,"7. Alpha",0)==UI_STATUS_OK);T(ui_light_document_find(v,"11. Beta",0)==UI_STATUS_OK);T(ui_light_document_find(v,"12. Gamma",0)==UI_STATUS_OK);T(ui_light_document_find(v,"4. Delta",0)==UI_STATUS_OK);T(ui_light_document_find(v,"-3. Eta",0)==UI_STATUS_OK);
    load(v,"<table><caption id='cap'>Revenue / 收入</caption><thead><tr><th id='a' rowspan='2'>Region<th id='b' colspan='2'>Year<tr><th id='c'>2025<th id='d'>2026</thead><tbody><tr><td id='empty'></td><td>10<td>20</table>");
    a=rect(v,"a");c=rect(v,"c");d=rect(v,"d");T(c.x>=a.x+a.width);T(d.x>=c.x+c.width);T(a.y+a.height>=c.y+c.height);T(rect(v,"b").width==c.width+d.width);T(rect(v,"empty").height>0);T(rect(v,"cap").height>0);
    load(v,"<p id='p'>A <strong>B <em>C</em></strong> D &amp; 中文 e&#x301; &#x1f600;</p><pre id='pre'> one\n  two\n</pre>");
    T(ui_light_document_find(v,"A B C D & 中文 é 😀",0)==UI_STATUS_OK);T(ui_light_document_find(v," one\n  two",0)==UI_STATUS_OK);
    load(v,"<style>.hide{display:none}.mixed{color:#123456;unknown-read:1;background:#abcdef}a{color:#654321}</style><p class='hide'>HIDDEN_SENTINEL</p><p class='mixed' id='color'>Valid CSS survives</p><a id='author' href='/next'>Link</a>");
    T(ui_light_document_find(v,"HIDDEN_SENTINEL",0)==UI_STATUS_NOT_FOUND);
    {ui_pixel_buffer_t p={0};p.size=sizeof(p);T(ui_web_view_capture_rgba(v,640,420,96,&p)==UI_STATUS_OK);p.capacity=p.stride*p.height;p.pixels=malloc(p.capacity);T(p.pixels!=NULL);if(p.pixels){T(ui_web_view_capture_rgba(v,640,420,96,&p)==UI_STATUS_OK);a=rect(v,"color");size_t pos=(size_t)(a.y+1)*p.stride+(size_t)(a.x+1)*4;T(pos+3<p.capacity);if(pos+3<p.capacity)T(p.pixels[pos]==0xab&&p.pixels[pos+1]==0xcd&&p.pixels[pos+2]==0xef);free(p.pixels);}}
    load(v,"<link rel='stylesheet' href='/external.css'><p id='ext'>External visible</p>");T(resource!=0);T(ui_light_document_resource(v,5,resource,0,"#ext{display:none} body{color:#222222}")==UI_STATUS_UNSUPPORTED);
    /* An externally hidden entire document is an explicit unsupported result. */
    load(v,"<form action='/result'><input name='empty' value=''><input name='same' value='first'><input name='same' value='second'><input name='no' disabled value='secret'><button id='disabled' disabled name='no' value='bad'>Disabled</button><input id='send' type='submit' name='submitter' value='Go'></form>");
    click(v,"disabled");T(submits==0);click(v,"send");T(submits==1);T(!strcmp(fields,"empty=;same=first;same=second;submitter=Go;"));
    load(v,"<div id='spaced' style='padding:20px;margin-top:30px'><span id='inside'>Spacing</span></div>");a=rect(v,"spaced");c=rect(v,"inside");T(c.x>=a.x+20);T(c.y>=a.y+20);
    load(v,"<p><span id='plain'>MMMMMMMM</span><b id='normal' style='font-weight:normal'>MMMMMMMM</b></p>");a=rect(v,"plain");c=rect(v,"normal");T(a.width==c.width);
    load(v,"<p style='white-space:nowrap'>" "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz" "</p>");{ui_document_state_t st={0};st.size=sizeof(st);T(ui_light_document_get_state(v,&st)==UI_STATUS_OK);T(st.content_width>640);}
    load(v,"<pre style='white-space:pre-wrap'>" "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz" "</pre>");{ui_document_state_t st={0};st.size=sizeof(st);T(ui_light_document_get_state(v,&st)==UI_STATUS_OK);T(st.content_width<=640);}
    load(v,"<link rel='stylesheet' href='/early.css'><style>#cascade{background:#00ff00}</style><p id='cascade'>Cascade</p>");{ui_document_state_t st={0};st.size=sizeof(st);T(ui_light_document_get_state(v,&st)==UI_STATUS_OK);T(ui_light_document_resource(v,st.navigation_id,resource,0,"#cascade{background:#ff0000}")==UI_STATUS_OK);ui_pixel_buffer_t px={0};px.size=sizeof(px);T(ui_web_view_capture_rgba(v,640,420,96,&px)==UI_STATUS_OK);px.capacity=px.stride*px.height;px.pixels=malloc(px.capacity);if(px.pixels){T(ui_web_view_capture_rgba(v,640,420,96,&px)==UI_STATUS_OK);a=rect(v,"cascade");size_t off=(size_t)(a.y+1)*px.stride+(size_t)(a.x+1)*4;T(off+3<px.capacity);if(off+3<px.capacity)T(px.pixels[off]==0&&px.pixels[off+1]==255&&px.pixels[off+2]==0);free(px.pixels);}}
    load(v,"<p hidden>HTML_HIDDEN_M2</p><p>Visible</p>");T(ui_light_document_find(v,"HTML_HIDDEN_M2",0)==UI_STATUS_NOT_FOUND);
    load(v,"<p style='visibility:hidden'>CSS_HIDDEN_M2</p><p>Visible</p>");T(ui_light_document_find(v,"CSS_HIDDEN_M2",0)==UI_STATUS_NOT_FOUND);
    load(v,"<p style='font-size:32px;line-height:1px'><span id='line-one'>First</span><br><span id='line-two'>Second</span></p>");a=rect(v,"line-one");c=rect(v,"line-two");T(c.y>=a.y+32);
    load(v,"<style>*{color:#123456}p > span{color:#ff0000}p{background:#aabbcc}</style><p id='universal'>Readable partial selector</p>");{ui_document_state_t st={0};st.size=sizeof(st);T(ui_light_document_get_state(v,&st)==UI_STATUS_OK);T(st.restrictions>0);}
    load(v,"<table><tbody><tr><td id='zero' rowspan='0'>Across group<td>A<tr><td id='zlast'>B</tbody><tbody><tr><td id='free'>New group<td>C</tbody></table>");a=rect(v,"zero");c=rect(v,"zlast");d=rect(v,"free");T(c.x>=a.x+a.width);T(a.y+a.height>=c.y+c.height);T(d.x==a.x&&d.y>=a.y+a.height);
    load(v,"<img alt='Long fallback text' src='/broken.png'>");{ui_document_state_t st={0};st.size=sizeof(st);T(ui_light_document_get_state(v,&st)==UI_STATUS_OK);T(ui_light_document_cancel_resources(v)==UI_STATUS_OK);T(ui_light_document_find(v,"image failed",0)==UI_STATUS_OK);T(ui_light_document_find(v,"Long fallback text",0)==UI_STATUS_OK);}
    T(ui_web_view_load_html(v,"<script>ui.invoke('x',{})</script>")==UI_STATUS_UNSUPPORTED);T(ui_web_view_post_json(v,"{}")==UI_STATUS_UNSUPPORTED);
    ui_web_view_destroy(v);ui_light_web_backend_destroy(b);ui_host_destroy(h);printf("M2 public reading failures=%d\n",failures);return failures?1:0;
}
