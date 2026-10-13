/* Real light rendering: explicit zero padding must leave the whole icon visible. */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "ui_framework/ui.h"
#include "ui_framework/light_web.h"
static unsigned text_pixels(const ui_pixel_buffer_t *pixels,const ui_rect_t *rect,uint32_t dpi)
{
 unsigned count=0;int x0=MulDiv(rect->x+9,(int)dpi,96),x1=MulDiv(rect->x+rect->width-9,(int)dpi,96);
 int y0=MulDiv(rect->y+5,(int)dpi,96),y1=MulDiv(rect->y+rect->height-5,(int)dpi,96);
 for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x){const unsigned char *s=pixels->pixels+(size_t)y*pixels->stride+(size_t)x*4;if(s[0]<180&&s[1]<180&&s[2]<180)++count;}
 return count;
}
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
 free(p.pixels);p.pixels=NULL;
 for(uint32_t dpi=96;dpi<=192;dpi+=48){
  int previous_ok=ok;ok=1;
  ui_rect_t automatic={0},reference={0};unsigned actual=0,complete=0;
  ok=ok&&ui_web_view_resize(v,160,80,dpi)==UI_STATUS_OK;
  ok=ok&&ui_web_view_load_html(v,"<style>body{margin:0;background:#ffffff}button{position:absolute;left:0px;height:28px;padding:4px 8px;border:1px solid #000000;background:#ffffff;color:#000000;font-family:Segoe UI;font-size:13px;text-align:center}#auto{top:0px}#reference{top:40px;width:120px}</style><button id='auto'>Submit</button><button id='reference'>Submit</button>")==UI_STATUS_OK;
  p.stride=0;p.capacity=0;ok=ok&&ui_web_view_capture_rgba(v,160,80,dpi,&p)==UI_STATUS_OK;p.capacity=p.stride*p.height;p.pixels=malloc(p.capacity);
  ok=ok&&p.pixels&&ui_web_view_capture_rgba(v,160,80,dpi,&p)==UI_STATUS_OK;
  ok=ok&&ui_web_view_get_element_rect(v,"auto",&automatic)==UI_STATUS_OK&&ui_web_view_get_element_rect(v,"reference",&reference)==UI_STATUS_OK;
  if(p.pixels){actual=text_pixels(&p,&automatic,dpi);complete=text_pixels(&p,&reference,dpi);}
  printf("Auto text dpi%u logicalWidth%d dark%u reference%u\n",dpi,automatic.width,actual,complete);ok=ok&&complete>20&&actual*10>=complete*9;
  free(p.pixels);p.pixels=NULL;
  ok=ok&&previous_ok;
 }
 ui_web_view_destroy(v);ui_light_web_backend_destroy(b);ui_host_destroy(h);return ok?0:1;
}
