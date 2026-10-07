/* Real light rendering: explicit zero padding must leave the whole icon visible. */
#include <stdio.h>
#include <stdlib.h>
#include "ui_framework/ui.h"
#include "ui_framework/light_web.h"
int main(void)
{
 ui_host_config_t hc={0};ui_light_web_config_t bc={0};ui_pixel_buffer_t p={0};unsigned count=0;int left=20,right=-1;
 if(ui_framework_initialize()!=UI_STATUS_OK)return 1;
 hc.size=sizeof(hc);hc.api_version=UI_FRAMEWORK_API_VERSION;bc.size=sizeof(bc);p.size=sizeof(p);
 ui_host_t *h=ui_host_create(&hc);ui_web_backend_t *b=ui_light_web_backend_create(&bc);ui_web_view_t *v=ui_web_view_create(h,b);
 if(v)(void)ui_web_view_resize(v,40,40,96);
 ui_status_t load=v?ui_web_view_load_html(v,"<!doctype html><html><head><style>body{margin:0;width:40px;height:40px;background:#ffffff}button{width:20px;height:20px;flex:0;padding:0;border:0;background:#ffffff;color:#000000;font-family:Segoe MDL2 Assets;font-size:12px;text-align:center}</style></head><body><button id='icon'>&#xE70D;</button><div style='height:1px'></div></body></html>"):UI_STATUS_PLATFORM_ERROR;int ok=load==UI_STATUS_OK;printf("load=%d host=%p view=%p\n",load,(void*)h,(void*)v);
 if(ok){ui_status_t s=ui_web_view_capture_rgba(v,40,40,96,&p);printf("capture=%d dimensions=%dx%d\n",s,p.width,p.height);ok=s==UI_STATUS_OK;}
 if(ok){p.capacity=p.stride*p.height;p.pixels=malloc(p.capacity);ok=p.pixels&&ui_web_view_capture_rgba(v,40,40,96,&p)==UI_STATUS_OK;}
 ui_element_presentation_t box={0};box.size=sizeof(box);ui_status_t ps=ui_web_view_get_presentation(v,"icon",&box);printf("presentation=%d visible=%d rect=%d,%d,%d,%d text=%s\n",ps,box.visible,box.rect.x,box.rect.y,box.rect.width,box.rect.height,box.text_utf8);
 if(ok)for(int y=2;y<38;++y)for(int x=2;x<18;++x){unsigned char *s=p.pixels+y*p.stride+x*4;if((299u*s[0]+587u*s[1]+114u*s[2])<210000u){++count;if(x<left)left=x;if(x>right)right=x;}}
 ok=ok&&right-left>=6&&count>=10;printf("Explicit zero-padding icon: dark=%u extent=%d..%d %s\n",count,left,right,ok?"PASS":"FAIL");
 free(p.pixels);ui_web_view_destroy(v);ui_light_web_backend_destroy(b);ui_host_destroy(h);return ok?0:1;
}
