#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include "browser_image.h"
#include <wincodec.h>
#include <objbase.h>
#include <stdlib.h>
#include <string.h>

static unsigned orientation(IWICBitmapFrameDecode *frame,browser_reply_t *reply)
{
    IWICMetadataQueryReader *reader=NULL;PROPVARIANT value;HRESULT hr;unsigned result=1;
    PropVariantInit(&value);hr=IWICBitmapFrameDecode_GetMetadataQueryReader(frame,&reader);
    if(SUCCEEDED(hr)){
        hr=IWICMetadataQueryReader_GetMetadataByName(reader,L"/app1/ifd/{ushort=274}",&value);
        if(SUCCEEDED(hr)){unsigned number=value.vt==VT_UI2?value.uiVal:value.vt==VT_UI4?value.ulVal:0;
            if(number>=1&&number<=8)result=number;else{++reply->restrictions;result=0;}}
        IWICMetadataQueryReader_Release(reader);
    }
    PropVariantClear(&value);return result;
}
static size_t destination(size_t at,unsigned w,unsigned h,unsigned direction)
{
    size_t x=at%w,y=at/w;
    switch(direction){case 2:return y*w+w-1-x;case 3:return (h-1-y)*w+w-1-x;
    case 4:return (h-1-y)*w+x;case 5:return x*h+y;case 6:return x*h+h-1-y;
    case 7:return (w-1-x)*h+h-1-y;case 8:return (w-1-x)*h+y;default:return at;}
}
/* Decode exactly once. Apply the EXIF permutation in the same RGBA allocation;
 * only orientations5..8 need a bounded one-bit-per-pixel visitation map. This
 * avoids the WIC flip-rotator's repeated scanline decoding on an unbuffered
 * source, and avoids a second full RGBA buffer. Scratch is in the 8MiB gate. */
static void orient(uint32_t *pixels,unsigned w,unsigned h,unsigned direction,unsigned char *seen)
{
    size_t count=(size_t)w*h;
    for(size_t start=0;start<count;++start){
        if(direction<5){size_t next=destination(start,w,h,direction);if(next>start){uint32_t temp=pixels[next];pixels[next]=pixels[start];pixels[start]=temp;}continue;}
        if(seen[start/8]&(1u<<(start%8)))continue;
        size_t at=start;uint32_t carried=pixels[at];
        do{size_t next=destination(at,w,h,direction);uint32_t temp=pixels[next];pixels[next]=carried;carried=temp;seen[at/8]|=(unsigned char)(1u<<(at%8));at=next;}while(at!=start);
    }
}
int browser_image_decode(const unsigned char *data,size_t bytes,size_t remaining,browser_reply_t *reply,unsigned char **pixels)
{
    IWICImagingFactory *factory=NULL;IWICStream *stream=NULL;IWICBitmapDecoder *decoder=NULL;IWICBitmapFrameDecode *frame=NULL;IWICFormatConverter *converter=NULL;
    HRESULT hr;UINT width=0,height=0,frames=0;GUID format;size_t size=0,scratch=0;unsigned direction=1;unsigned char *seen=NULL;int result=0;*pixels=NULL;
    if(bytes>2u*1024u*1024u){reply->error=ERROR_FILE_TOO_LARGE;return 0;}
    hr=CoCreateInstance(&CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,&IID_IWICImagingFactory,(void **)&factory);
    if(SUCCEEDED(hr))hr=IWICImagingFactory_CreateStream(factory,&stream);
    if(SUCCEEDED(hr))hr=IWICStream_InitializeFromMemory(stream,(BYTE *)data,(DWORD)bytes);
    if(SUCCEEDED(hr))hr=IWICImagingFactory_CreateDecoderFromStream(factory,(IStream *)stream,NULL,WICDecodeMetadataCacheOnDemand,&decoder);
    if(SUCCEEDED(hr))hr=IWICBitmapDecoder_GetContainerFormat(decoder,&format);
    if(SUCCEEDED(hr)&&!IsEqualGUID(&format,&GUID_ContainerFormatPng)&&!IsEqualGUID(&format,&GUID_ContainerFormatJpeg)&&!IsEqualGUID(&format,&GUID_ContainerFormatGif)&&!IsEqualGUID(&format,&GUID_ContainerFormatBmp))hr=WINCODEC_ERR_UNKNOWNIMAGEFORMAT;
    if(SUCCEEDED(hr))hr=IWICBitmapDecoder_GetFrameCount(decoder,&frames);
    if(SUCCEEDED(hr))hr=IWICBitmapDecoder_GetFrame(decoder,0,&frame);
    if(SUCCEEDED(hr))hr=IWICBitmapFrameDecode_GetSize(frame,&width,&height);
    if(SUCCEEDED(hr)&&(!width||!height||width>4096||height>4096||(uint64_t)width*height*4>BROWSER_IMAGE_LIMIT-sizeof(*reply)-1024u)){hr=E_INVALIDARG;reply->error=ERROR_FILE_TOO_LARGE;}
    if(SUCCEEDED(hr)){direction=orientation(frame,reply);size=(size_t)width*height*4;scratch=direction>=5?((size_t)width*height+7)/8:0;
        if(size+scratch>BROWSER_IMAGE_LIMIT-sizeof(*reply)-1024u||size>remaining){hr=E_INVALIDARG;reply->error=ERROR_FILE_TOO_LARGE;}}
    if(SUCCEEDED(hr))hr=IWICImagingFactory_CreateFormatConverter(factory,&converter);
    if(SUCCEEDED(hr))hr=IWICFormatConverter_Initialize(converter,(IWICBitmapSource *)frame,&GUID_WICPixelFormat32bppRGBA,WICBitmapDitherTypeNone,NULL,0,WICBitmapPaletteTypeCustom);
    if(SUCCEEDED(hr)){*pixels=malloc(size);if(scratch)seen=calloc(scratch,1);if(!*pixels||(scratch&&!seen))hr=E_OUTOFMEMORY;}
    if(SUCCEEDED(hr))hr=IWICFormatConverter_CopyPixels(converter,NULL,width*4,(UINT)size,*pixels);
    if(SUCCEEDED(hr)){if(direction>1)orient((uint32_t *)*pixels,width,height,direction,seen);reply->width=direction>=5?height:width;reply->height=direction>=5?width:height;reply->bytes=size;reply->orientation=direction;reply->frames=frames;reply->restrictions+=frames>1?1u:0u;result=1;}
    else{free(*pixels);*pixels=NULL;reply->codec_status=(uint32_t)hr;if(!reply->error)reply->error=hr==E_OUTOFMEMORY?ERROR_NOT_ENOUGH_MEMORY:ERROR_INVALID_DATA;}
    free(seen);if(converter)IWICFormatConverter_Release(converter);if(frame)IWICBitmapFrameDecode_Release(frame);if(decoder)IWICBitmapDecoder_Release(decoder);if(stream)IWICStream_Release(stream);if(factory)IWICImagingFactory_Release(factory);return result;
}
