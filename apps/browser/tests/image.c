#define WIN32_LEAN_AND_MEAN
#include "browser_image.h"
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int failures;
#define T(x) do{if(!(x)){fprintf(stderr,"image line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static unsigned char *read(const char *path,size_t *bytes)
{FILE *f=fopen(path,"rb");unsigned char *data=NULL;long n;if(!f)return NULL;fseek(f,0,SEEK_END);n=ftell(f);rewind(f);if(n>0&&n<2*1024*1024){data=malloc((size_t)n);if(data&&fread(data,1,(size_t)n,f)!=(size_t)n){free(data);data=NULL;}}fclose(f);*bytes=n>0?(size_t)n:0;return data;}
int main(int argc,char **argv)
{
    char path[32768];if(argc<2)return 2;T(SUCCEEDED(CoInitializeEx(NULL,COINIT_MULTITHREADED)));
    for(int i=1;i<=8;++i){browser_reply_t reply={0};unsigned char *input,*expected,*pixels=NULL;size_t bytes=0,wanted=0;
        snprintf(path,sizeof(path),"%s/orientation-%d.jpg",argv[1],i);input=read(path,&bytes);T(input!=NULL);
        snprintf(path,sizeof(path),"%s/orientation-%d.rgba",argv[1],i);expected=read(path,&wanted);T(expected!=NULL);
        if(input&&expected){unsigned width=0,height=0;memcpy(&width,expected,4);memcpy(&height,expected+4,4);T(browser_image_decode(input,bytes,32u*1024u*1024u,&reply,&pixels));T(reply.width==width&&reply.height==height);T(reply.bytes+8==wanted);T(reply.orientation==(unsigned)i);
            if(pixels&&reply.bytes+8==wanted){int worst=0;for(size_t k=0;k<reply.bytes;++k){int delta=abs((int)pixels[k]-(int)expected[k+8]);if(delta>worst)worst=delta;}T(worst<=8);printf("EXIF%d %ux%u pixel_max_delta=%d\n",i,reply.width,reply.height,worst);}
            free(pixels);pixels=NULL;memset(&reply,0,sizeof(reply));T(!browser_image_decode(input,bytes,4,&reply,&pixels));T(reply.error==ERROR_FILE_TOO_LARGE);T(pixels==NULL);}
        free(input);free(expected);
    }
    {const char *formats[]={"png","bmp","gif"};for(int i=0;i<3;++i){browser_reply_t r={0};unsigned char *input,*pixels=NULL;size_t bytes=0;snprintf(path,sizeof(path),"%s/format.%s",argv[1],formats[i]);input=read(path,&bytes);T(input!=NULL);if(input){T(browser_image_decode(input,bytes,32u*1024u*1024u,&r,&pixels));T(r.width==12&&r.height==8);if(i==2)T(r.frames==2&&r.restrictions>0);}free(input);free(pixels);}}
    {browser_reply_t r={0};unsigned char *pixels=NULL;T(!browser_image_decode((const unsigned char *)"broken",6,1024,&r,&pixels));T(r.error==ERROR_INVALID_DATA&&r.codec_status!=0&&pixels==NULL);}
    CoUninitialize();printf("WIC formats/orientation/pixels/budget failures=%d\n",failures);return failures?1:0;
}
