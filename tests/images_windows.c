#include "../src/ui_internal.h"
#include "png_fixture.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
static int failed;
#define CHECK(x) do{if(!(x)){printf("FAIL %d: %s\n",__LINE__,#x);++failed;}}while(0)
int main(void)
{
    ui_host_config_t config={0};ui_host_t *host,*other;ui_image_id_t id,other_id;ui_image_info_t info={0};ui_image_stats_t stats={0};uint8_t bytes[4]={255,0,0,128};ui_rgba_desc_t r={0};
    HDC dc;HBITMAP bitmap;HGDIOBJ old;RECT rect={0,0,1,1};COLORREF color;ui_fill_style_t style={0};size_t i,quota;uint8_t corrupt[sizeof(png_fixture)];
    config.size=sizeof(config);config.api_version=3;host=ui_host_create(&config);other=ui_host_create(&config);CHECK(host&&other);
    info.size=sizeof(info);CHECK(ui_image_load_png(host,png_fixture,sizeof(png_fixture),&id)==UI_STATUS_OK);CHECK(ui_image_get_info(host,id,&info)==UI_STATUS_OK&&info.width==16&&info.height==16);
    memcpy(corrupt,png_fixture,sizeof(corrupt));memset(corrupt+16,0,sizeof(corrupt)-16);CHECK(ui_image_load_png(host,corrupt,sizeof(corrupt),&other_id)==UI_STATUS_VALIDATION_FAILED);
    CHECK(ui_image_release(host,id)==UI_STATUS_OK);r.size=sizeof(r);r.width=r.height=1;r.stride=r.bytes=4;r.pixels=bytes;CHECK(ui_image_create(host,&r,&id)==UI_STATUS_OK);
    dc=CreateCompatibleDC(NULL);{BITMAPINFO header={0};void *pixels;header.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);header.bmiHeader.biWidth=1;header.bmiHeader.biHeight=-1;header.bmiHeader.biPlanes=1;header.bmiHeader.biBitCount=32;
        bitmap=CreateDIBSection(dc,&header,DIB_RGB_COLORS,&pixels,NULL,0);}old=SelectObject(dc,bitmap);SetPixel(dc,0,0,RGB(255,255,255));
    CHECK(ui_image_draw(host,id,dc,&rect,0));color=GetPixel(dc,0,0);CHECK(GetRValue(color)==255&&GetGValue(color)>=126&&GetGValue(color)<=128&&GetBValue(color)>=126&&GetBValue(color)<=128);
    CHECK(!ui_image_draw(other,id,dc,&rect,0));SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);
    stats.size=sizeof(stats);CHECK(ui_image_get_stats(host,&stats)==UI_STATUS_OK&&stats.bytes>4);quota=stats.bytes*5;CHECK(ui_image_set_limit(host,quota)==UI_STATUS_OK);for(i=0;i<100;++i){CHECK(ui_image_create(host,&r,&other_id)==UI_STATUS_OK);CHECK(ui_image_set_evictable(host,other_id)==UI_STATUS_OK);}
    stats.size=sizeof(stats);CHECK(ui_image_get_stats(host,&stats)==UI_STATUS_OK&&stats.bytes<=quota&&stats.evictions>=95);CHECK(ui_image_get_info(host,id,&info)==UI_STATUS_OK);
    style.size=sizeof(style);style.fill_rgba=0xaabbccff;style.border_rgba=0x223344ff;style.border_width=1;style.line_style=UI_LINE_DOT;style.dot_spacing=4;style.dot_radius=1;
    CHECK(ui_image_create_preview(host,&style,64,32,&other_id)==UI_STATUS_LIMIT_EXCEEDED);CHECK(ui_image_release(host,id)==UI_STATUS_OK);ui_host_destroy(host);ui_host_destroy(other);
    printf("PNG/GDI/cache: %d failures\n",failed);return failed?1:0;
}
