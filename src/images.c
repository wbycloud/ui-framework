#include "ui_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
#define COBJMACROS
#include <windows.h>
#include <objbase.h>
#include <wincodec.h>
#endif

#define IMAGE_MAX_BYTES (8u * 1024u * 1024u)
typedef struct ui_image_entry {
    ui_host_t *host;
    uint64_t id, version, used;
    uint32_t width, height;
    size_t bytes;
    uint8_t *pixels;
    int evictable;
    void *bitmap;
    struct ui_image_entry *next, *global_next;
} ui_image_entry_t;
static ui_image_entry_t *all_images;
static size_t image_charge(const ui_image_entry_t *p){return p->bytes+sizeof(*p);}
static uint64_t use_sequence;
#ifdef _WIN32
static volatile LONG64 image_sequence;
static SRWLOCK image_lock = SRWLOCK_INIT;
#define LOCK() AcquireSRWLockExclusive(&image_lock)
#define UNLOCK() ReleaseSRWLockExclusive(&image_lock)
static uint64_t next_id(void) { return (uint64_t)InterlockedIncrement64(&image_sequence); }
#else
static uint64_t image_sequence;
#define LOCK() ((void)0)
#define UNLOCK() ((void)0)
static uint64_t next_id(void) { return ++image_sequence; }
#endif

static ui_image_entry_t *find_image(const ui_host_t *host, uint64_t id)
{
    ui_image_entry_t *p;
    for (p = host->images; p; p = p->next) if (p->id == id) return p;
    return NULL;
}
static ui_status_t rgba_valid(const ui_rgba_desc_t *r, size_t *bytes)
{
    size_t row;
    if (!r || r->size < sizeof(*r) || !r->pixels || !r->width || !r->height ||
        r->width > 4096 || r->height > 4096) return UI_STATUS_INVALID_ARGUMENT;
    row = (size_t)r->width * 4;
    if (r->stride < row || r->stride > SIZE_MAX / r->height ||
        r->bytes < r->stride * (r->height - 1) + row) return UI_STATUS_INVALID_ARGUMENT;
    *bytes = row * r->height;
    return *bytes > IMAGE_MAX_BYTES ? UI_STATUS_LIMIT_EXCEEDED : UI_STATUS_OK;
}
static void free_image(ui_image_entry_t *p)
{
    ui_image_entry_t **link;
    for (link = &p->host->images; *link && *link != p; link = &(*link)->next) {}
    if (*link) *link = p->next;
    for (link = &all_images; *link && *link != p; link = &(*link)->global_next) {}
    if (*link) *link = p->global_next;
    p->host->image_bytes -= image_charge(p);
#ifdef _WIN32
    if (p->bitmap) DeleteObject((HBITMAP)p->bitmap);
    else free(p->pixels);
#else
    free(p->pixels);
#endif
    free(p);
}
static int make_room(ui_host_t *host, size_t bytes, const ui_image_entry_t *keep)
{
    while (host->image_metadata_bytes>host->image_limit || host->image_bytes > host->image_limit-host->image_metadata_bytes || bytes > host->image_limit-host->image_metadata_bytes-host->image_bytes) {
        ui_image_entry_t *p, *old = NULL;
        for (p = host->images; p; p = p->next)
            if (p != keep && p->evictable && (!old || p->used < old->used)) old = p;
        if (!old) return 0;
        free_image(old); ++host->image_evictions;
    }
    return 1;
}
ui_status_t ui_images_reserve_metadata(ui_host_t *host,size_t bytes)
{LOCK();if(!make_room(host,bytes,NULL)){UNLOCK();return UI_STATUS_LIMIT_EXCEEDED;}host->image_metadata_bytes+=bytes;UNLOCK();return UI_STATUS_OK;}
void ui_images_release_metadata(ui_host_t *host,size_t bytes)
{LOCK();host->image_metadata_bytes-=bytes;UNLOCK();}
static ui_image_entry_t *copy_rgba(ui_host_t *host, const ui_rgba_desc_t *r, size_t bytes)
{
    ui_image_entry_t *p = (ui_image_entry_t *)calloc(1, sizeof(*p));
    uint32_t x, y;
    if (!p) return NULL;
    p->host = host; p->width = r->width; p->height = r->height; p->bytes = bytes;
#ifdef _WIN32
    {
        BITMAPINFO info;
        memset(&info, 0, sizeof(info)); info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth = (LONG)r->width; info.bmiHeader.biHeight = -(LONG)r->height;
        info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32; info.bmiHeader.biCompression = BI_RGB;
        p->bitmap = CreateDIBSection(NULL, &info, DIB_RGB_COLORS, (void **)&p->pixels, NULL, 0);
    }
#else
    p->pixels = (uint8_t *)malloc(bytes);
#endif
    if (!p->pixels) { free(p); return NULL; }
    for (y = 0; y < r->height; ++y) for (x = 0; x < r->width; ++x) {
        const uint8_t *s = r->pixels + y * r->stride + x * 4;
        uint8_t *d = p->pixels + ((size_t)y * r->width + x) * 4;
        d[0] = (uint8_t)((s[2] * s[3] + 127) / 255);
        d[1] = (uint8_t)((s[1] * s[3] + 127) / 255);
        d[2] = (uint8_t)((s[0] * s[3] + 127) / 255); d[3] = s[3];
    }
    return p;
}
void ui_image_changed(ui_host_t *host, uint64_t id)
{
    ui_web_view_t *view;
    char json[64];
    for (view = host->web_views; view; view = view->host_next) ui_web_image_changed(view,id);
    (void)snprintf(json, sizeof(json), "{\"image_id\":\"%llu\"}", (unsigned long long)id);
    (void)ui_host_emit_event(host, "ui.image.changed", json);
}
ui_status_t ui_image_create(ui_host_t *host, const ui_rgba_desc_t *r, ui_image_id_t *id)
{
    size_t bytes; ui_image_entry_t *p; ui_status_t status;
    if (!host || !id) return UI_STATUS_INVALID_ARGUMENT;
    *id = 0; status = rgba_valid(r, &bytes); if (status != UI_STATUS_OK) return status;
    LOCK();
    if (!make_room(host, bytes+sizeof(*p), NULL)) { UNLOCK(); return UI_STATUS_LIMIT_EXCEEDED; }
    p = copy_rgba(host, r, bytes);
    if (!p) { UNLOCK(); return UI_STATUS_OUT_OF_MEMORY; }
    p->id = next_id(); p->version = 1; p->used = ++use_sequence;
    p->next = host->images; host->images = p; host->image_bytes += image_charge(p);
    p->global_next = all_images; all_images = p; *id = p->id; UNLOCK();
    ui_image_changed(host, *id); return UI_STATUS_OK;
}
ui_status_t ui_image_update(ui_host_t *host, ui_image_id_t id, const ui_rgba_desc_t *r)
{
    size_t bytes; ui_image_entry_t *p, *replacement; ui_status_t status;
    if (!host) return UI_STATUS_INVALID_ARGUMENT;
    status = rgba_valid(r, &bytes); if (status != UI_STATUS_OK) return status;
    LOCK(); p = find_image(host, id);
    if (!p) { UNLOCK(); return UI_STATUS_NOT_FOUND; }
    if (!make_room(host, bytes > p->bytes ? bytes - p->bytes : 0, p)) { UNLOCK(); return UI_STATUS_LIMIT_EXCEEDED; }
    replacement = copy_rgba(host, r, bytes);
    if (!replacement) { UNLOCK(); return UI_STATUS_OUT_OF_MEMORY; }
    replacement->id = id; replacement->version = p->version + 1;
    replacement->evictable = p->evictable; replacement->used = ++use_sequence;
    free_image(p); replacement->next = host->images; host->images = replacement;
    replacement->global_next = all_images; all_images = replacement; host->image_bytes += image_charge(replacement);
    UNLOCK(); ui_image_changed(host, id); return UI_STATUS_OK;
}
ui_status_t ui_image_release(ui_host_t *host, ui_image_id_t id)
{
    ui_image_entry_t *p;
    if (!host) return UI_STATUS_INVALID_ARGUMENT;
    LOCK(); p = find_image(host, id);
    if (!p) { UNLOCK(); return UI_STATUS_NOT_FOUND; }
    free_image(p); UNLOCK(); ui_image_changed(host, id); return UI_STATUS_OK;
}
ui_status_t ui_image_get_info(ui_host_t *host, ui_image_id_t id, ui_image_info_t *info)
{
    ui_image_entry_t *p;
    if (!host || !info || info->size < sizeof(*info)) return UI_STATUS_INVALID_ARGUMENT;
    LOCK(); p = find_image(host, id);
    if (!p) { UNLOCK(); return UI_STATUS_NOT_FOUND; }
    info->width = p->width; info->height = p->height; info->bytes = p->bytes;
    info->version = p->version; p->used = ++use_sequence; UNLOCK(); return UI_STATUS_OK;
}
ui_status_t ui_image_set_limit(ui_host_t *host, size_t bytes)
{
    ui_image_entry_t *p; size_t pinned = 0;
    if (!host || !bytes) return UI_STATUS_INVALID_ARGUMENT;
    LOCK(); pinned=host->image_metadata_bytes;for (p = host->images; p; p = p->next) if (!p->evictable) pinned += image_charge(p);
    if (pinned > bytes) { UNLOCK(); return UI_STATUS_LIMIT_EXCEEDED; }
    host->image_limit = bytes; (void)make_room(host, 0, NULL); UNLOCK(); return UI_STATUS_OK;
}
ui_status_t ui_image_get_stats(const ui_host_t *host, ui_image_stats_t *stats)
{
    ui_image_entry_t *p;
    if (!host || !stats || stats->size < sizeof(*stats)) return UI_STATUS_INVALID_ARGUMENT;
    LOCK(); stats->count = 0;
    for (p = host->images; p; p = p->next) ++stats->count;
    stats->bytes = host->image_bytes+host->image_metadata_bytes; stats->limit = host->image_limit;
    stats->evictions = host->image_evictions; UNLOCK(); return UI_STATUS_OK;
}
ui_status_t ui_image_set_evictable(ui_host_t *host, uint64_t id)
{
    ui_image_entry_t *p; LOCK(); p = find_image(host, id);
    if (p) p->evictable = 1;
    UNLOCK(); return p ? UI_STATUS_OK : UI_STATUS_NOT_FOUND;
}
void ui_images_destroy(ui_host_t *host)
{ LOCK(); while (host->images) free_image(host->images); UNLOCK(); }
int ui_image_draw(const ui_host_t *host,uint64_t id, void *context, const void *rectangle, int disabled)
{
#ifdef _WIN32
    ui_image_entry_t *p; const RECT *rect = (const RECT *)rectangle;
    HDC dc = (HDC)context, source; HGDIOBJ previous;
    int width, height, x, y;
    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    if (!dc || !rect) return 0;
    LOCK(); for (p = all_images; p && p->id != id; p = p->global_next) {}
    if (!p || (p->host!=host && p->host!=host->image_source)) { UNLOCK(); return 0; }
    width = rect->right - rect->left; height = rect->bottom - rect->top;
    if (width <= 0 || height <= 0) { UNLOCK(); return 0; }
    if ((int64_t)width * p->height > (int64_t)height * p->width) width = (int)((int64_t)height * p->width / p->height);
    else height = (int)((int64_t)width * p->height / p->width);
    x = rect->left + (rect->right - rect->left - width) / 2;
    y = rect->top + (rect->bottom - rect->top - height) / 2;
    source = CreateCompatibleDC(dc); previous = SelectObject(source, (HBITMAP)p->bitmap);
    if (disabled) blend.SourceConstantAlpha = 96;
    AlphaBlend(dc, x, y, width, height, source, 0, 0, (int)p->width, (int)p->height, blend);
    SelectObject(source, previous); DeleteDC(source); p->used = ++use_sequence; UNLOCK(); return 1;
#else
    (void)host; (void)id; (void)context; (void)rectangle; (void)disabled; return 0;
#endif
}
ui_status_t ui_image_png_stream(const ui_host_t *host,uint64_t id,void **output)
{
#ifdef _WIN32
    ui_image_entry_t *p;IWICImagingFactory *factory=NULL;IWICBitmapEncoder *encoder=NULL;IWICBitmapFrameEncode *frame=NULL;IStream *stream=NULL;
    uint8_t *pixels=NULL;UINT width,height;size_t bytes;HRESULT hr;WICPixelFormatGUID format=GUID_WICPixelFormat32bppBGRA;
    if(!host||!output)return UI_STATUS_INVALID_ARGUMENT;*output=NULL;
    LOCK();for(p=all_images;p&&p->id!=id;p=p->global_next){}
    if(!p||(p->host!=host&&p->host!=host->image_source)){UNLOCK();return UI_STATUS_NOT_FOUND;}
    width=p->width;height=p->height;bytes=p->bytes;pixels=(uint8_t *)malloc(bytes);if(!pixels){UNLOCK();return UI_STATUS_OUT_OF_MEMORY;}
    for(size_t i=0;i<bytes;i+=4){unsigned a=p->pixels[i+3];for(size_t k=0;k<3;++k)pixels[i+k]=a?(uint8_t)((p->pixels[i+k]*255u+a/2)/a):0;pixels[i+3]=(uint8_t)a;}
    p->used=++use_sequence;UNLOCK();
    hr=CreateStreamOnHGlobal(NULL,TRUE,&stream);
    if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,&IID_IWICImagingFactory,(void **)&factory);
    if(SUCCEEDED(hr))hr=IWICImagingFactory_CreateEncoder(factory,&GUID_ContainerFormatPng,NULL,&encoder);
    if(SUCCEEDED(hr))hr=IWICBitmapEncoder_Initialize(encoder,stream,WICBitmapEncoderNoCache);
    if(SUCCEEDED(hr))hr=IWICBitmapEncoder_CreateNewFrame(encoder,&frame,NULL);
    if(SUCCEEDED(hr))hr=IWICBitmapFrameEncode_Initialize(frame,NULL);
    if(SUCCEEDED(hr))hr=IWICBitmapFrameEncode_SetSize(frame,width,height);
    if(SUCCEEDED(hr))hr=IWICBitmapFrameEncode_SetPixelFormat(frame,&format);
    if(SUCCEEDED(hr)&&!IsEqualGUID(&format,&GUID_WICPixelFormat32bppBGRA))hr=E_FAIL;
    if(SUCCEEDED(hr))hr=IWICBitmapFrameEncode_WritePixels(frame,height,width*4,(UINT)bytes,pixels);
    if(SUCCEEDED(hr))hr=IWICBitmapFrameEncode_Commit(frame);
    if(SUCCEEDED(hr))hr=IWICBitmapEncoder_Commit(encoder);
    if(SUCCEEDED(hr)){LARGE_INTEGER zero={0};hr=IStream_Seek(stream,zero,STREAM_SEEK_SET,NULL);}
    free(pixels);if(frame)IWICBitmapFrameEncode_Release(frame);if(encoder)IWICBitmapEncoder_Release(encoder);if(factory)IWICImagingFactory_Release(factory);
    if(FAILED(hr)){if(stream)IStream_Release(stream);return UI_STATUS_PLATFORM_ERROR;}*output=stream;return UI_STATUS_OK;
#else
    (void)host;(void)id;(void)output;return UI_STATUS_UNSUPPORTED;
#endif
}
ui_status_t ui_image_load_png(ui_host_t *host, const void *data, size_t bytes, ui_image_id_t *id)
{
#ifdef _WIN32
    IWICImagingFactory *factory = NULL; IWICStream *stream = NULL;
    IWICBitmapDecoder *decoder = NULL; IWICBitmapFrameDecode *frame = NULL;
    IWICFormatConverter *converter = NULL;
    HRESULT hr, initialized; UINT width = 0, height = 0;
    uint8_t *pixels = NULL; ui_status_t status = UI_STATUS_VALIDATION_FAILED;
    static const uint8_t magic[] = {137,80,78,71,13,10,26,10};
    ui_rgba_desc_t rgba;
    if (!host || !data || !id || bytes < 8 || bytes > IMAGE_MAX_BYTES || memcmp(data, magic, 8)) return UI_STATUS_INVALID_ARGUMENT;
    *id = 0; initialized = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(initialized) && initialized != RPC_E_CHANGED_MODE) return UI_STATUS_PLATFORM_ERROR;
    hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
        &IID_IWICImagingFactory, (void **)&factory);
    if (SUCCEEDED(hr)) hr = IWICImagingFactory_CreateStream(factory, &stream);
    if (SUCCEEDED(hr)) hr = IWICStream_InitializeFromMemory(stream, (BYTE *)data, (DWORD)bytes);
    if (SUCCEEDED(hr)) hr = IWICImagingFactory_CreateDecoder(factory, &GUID_ContainerFormatPng, NULL, &decoder);
    if (SUCCEEDED(hr)) hr = IWICBitmapDecoder_Initialize(decoder, (IStream *)stream, WICDecodeMetadataCacheOnLoad);
    if (SUCCEEDED(hr)) hr = IWICBitmapDecoder_GetFrame(decoder, 0, &frame);
    if (SUCCEEDED(hr)) hr = IWICBitmapFrameDecode_GetSize(frame, &width, &height);
    if (SUCCEEDED(hr) && (!width || !height || width > 4096 || height > 4096 || (uint64_t)width * height * 4 > IMAGE_MAX_BYTES)) {
        status = UI_STATUS_LIMIT_EXCEEDED; hr = E_INVALIDARG;
    }
    if (SUCCEEDED(hr)) hr = IWICImagingFactory_CreateFormatConverter(factory, &converter);
    if (SUCCEEDED(hr)) hr = IWICFormatConverter_Initialize(converter, (IWICBitmapSource *)frame,
        &GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
    if (SUCCEEDED(hr)) { pixels = (uint8_t *)malloc((size_t)width * height * 4); if (!pixels) { status = UI_STATUS_OUT_OF_MEMORY; hr = E_OUTOFMEMORY; } }
    if (SUCCEEDED(hr)) hr = IWICFormatConverter_CopyPixels(converter, NULL, width * 4, width * height * 4, pixels);
    if (SUCCEEDED(hr)) {
        memset(&rgba, 0, sizeof(rgba)); rgba.size = sizeof(rgba); rgba.width = width; rgba.height = height;
        rgba.stride = width * 4; rgba.bytes = rgba.stride * height; rgba.pixels = pixels;
        status = ui_image_create(host, &rgba, id);
    }
    free(pixels);
    if (converter) IWICFormatConverter_Release(converter);
    if (frame) IWICBitmapFrameDecode_Release(frame);
    if (decoder) IWICBitmapDecoder_Release(decoder);
    if (stream) IWICStream_Release(stream);
    if (factory) IWICImagingFactory_Release(factory);
    if (SUCCEEDED(initialized)) CoUninitialize();
    return status;
#else
    (void)host; (void)data; (void)bytes; (void)id; return UI_STATUS_UNSUPPORTED;
#endif
}
ui_status_t ui_image_load_resource(const struct ui_app_context *context, const char *name, ui_image_id_t *id)
{
#if defined(UI_FRAMEWORK_BUILD_SHARED)
    void *bytes; size_t length; ui_status_t status;
    if (!context || !name || !id) return UI_STATUS_INVALID_ARGUMENT;
    status = ui_app_resource_read(context, name, &bytes, &length);
    if (status != UI_STATUS_OK) return status;
    status = ui_image_load_png(context->host, bytes, length, id);
    ui_app_resource_release(bytes); return status;
#else
    (void)context; (void)name; (void)id; return UI_STATUS_UNSUPPORTED;
#endif
}
ui_status_t ui_image_create_preview(ui_host_t *host, const ui_fill_style_t *s,
    uint32_t width, uint32_t height, ui_image_id_t *id)
{
    uint8_t *pixels; uint32_t x, y; ui_rgba_desc_t r; ui_status_t status;
    if (!host || !s || s->size < sizeof(*s) || !id || !width || !height || width > 512 || height > 512 ||
        s->border_width < 0 || s->dot_spacing < 0 || s->dot_radius < 0 || s->line_style > UI_LINE_DOT) return UI_STATUS_INVALID_ARGUMENT;
    if (s->image_id) { ui_image_info_t info; memset(&info,0,sizeof(info)); info.size=sizeof(info);
        status=ui_image_get_info(host,s->image_id,&info); if(status==UI_STATUS_OK)*id=s->image_id; return status; }
    pixels = (uint8_t *)malloc((size_t)width * height * 4); if (!pixels) return UI_STATUS_OUT_OF_MEMORY;
    for (y = 0; y < height; ++y) for (x = 0; x < width; ++x) {
        uint32_t color = s->fill_rgba; uint8_t *p = pixels + ((size_t)y * width + x) * 4;
        int border = x < (uint32_t)s->border_width || y < (uint32_t)s->border_width ||
            width - x <= (uint32_t)s->border_width || height - y <= (uint32_t)s->border_width;
        if (border && (s->line_style == UI_LINE_SOLID || (x + y) % (s->line_style == UI_LINE_DASH ? 10u : 4u) < (s->line_style == UI_LINE_DASH ? 6u : 1u))) color = s->border_rgba;
        else if (s->dot_spacing && x % (uint32_t)s->dot_spacing < (uint32_t)s->dot_radius && y % (uint32_t)s->dot_spacing < (uint32_t)s->dot_radius) color = s->dot_rgba;
        p[0]=(uint8_t)(color>>24);p[1]=(uint8_t)(color>>16);p[2]=(uint8_t)(color>>8);p[3]=(uint8_t)color;
    }
    memset(&r,0,sizeof(r));r.size=sizeof(r);r.width=width;r.height=height;r.stride=(size_t)width*4;r.bytes=r.stride*height;r.pixels=pixels;
    status=ui_image_create(host,&r,id);free(pixels);return status;
}
