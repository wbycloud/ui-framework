#ifndef UI_JSON_UI_H
#define UI_JSON_UI_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
typedef struct ui_json { char *data; size_t length, capacity; int failed; } ui_json_t;
static void uj_add(ui_json_t *b, const char *s)
{
    size_t n = strlen(s), capacity; char *p;
    if (b->failed) return;
    if (n > 256u * 1024u || b->length > 256u * 1024u - n) { b->failed=1; return; }
    if (b->length+n+1 > b->capacity) {
        capacity=b->capacity?b->capacity:512;
        while(capacity<b->length+n+1)capacity*=2;
        p=(char *)realloc(b->data,capacity);if(!p){b->failed=1;return;}b->data=p;b->capacity=capacity;
    }
    memcpy(b->data+b->length,s,n+1);b->length+=n;
}
static void uj_fmt(ui_json_t *b,const char *format,...)
{
    char text[256];int n;va_list args;va_start(args,format);n=vsnprintf(text,sizeof(text),format,args);va_end(args);
    if(n<0||(size_t)n>=sizeof(text)){b->failed=1;return;}uj_add(b,text);
}
static void uj_string(ui_json_t *b,const char *s)
{
    const unsigned char *p=(const unsigned char *)(s?s:"");char text[8];uj_add(b,"\"");
    for(;*p;++p){if(*p=='"')uj_add(b,"\\\"");else if(*p=='\\')uj_add(b,"\\\\");
        else if(*p<32){(void)snprintf(text,sizeof(text),"\\u%04x",(unsigned)*p);uj_add(b,text);}
        else{text[0]=(char)*p;text[1]=0;uj_add(b,text);}}
    uj_add(b,"\"");
}
/* Controlled component messages only contain a flat object of strings/numbers.
 * Decode JSON escapes (including UTF-16 pairs); never interpolate as script. */
static const char *uj_space(const char *p){while(*p==' '||*p=='\n'||*p=='\r'||*p=='\t')++p;return p;}
static int uj_hex(char c){return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1;}
static const char *uj_read_string(const char *p,char *out,size_t size)
{
    size_t at=0;if(*p++!='"')return NULL;
    while(*p&&*p!='"'){
        unsigned v=(unsigned char)*p++;
        if(v<32)return NULL;
        if(v=='\\'){
            char c=*p++;if(!c)return NULL;
            if(c=='u'){
                int k,h;v=0;for(k=0;k<4;++k){h=uj_hex(*p++);if(h<0)return NULL;v=v*16+(unsigned)h;}
                if(v>=0xd800&&v<=0xdbff){unsigned low=0;if(*p++!='\\'||*p++!='u')return NULL;
                    for(k=0;k<4;++k){h=uj_hex(*p++);if(h<0)return NULL;low=low*16+(unsigned)h;}
                    if(low<0xdc00||low>0xdfff)return NULL;v=0x10000+((v-0xd800)<<10)+low-0xdc00;
                }else if(v>=0xdc00&&v<=0xdfff)return NULL;
                if(!v)return NULL;
                if(v>=0x80){unsigned char bytes[4];size_t n,i;
                    if(v<0x800){bytes[0]=(unsigned char)(0xc0|(v>>6));bytes[1]=(unsigned char)(0x80|(v&63));n=2;}
                    else if(v<0x10000){bytes[0]=(unsigned char)(0xe0|(v>>12));bytes[1]=(unsigned char)(0x80|((v>>6)&63));bytes[2]=(unsigned char)(0x80|(v&63));n=3;}
                    else{bytes[0]=(unsigned char)(0xf0|(v>>18));bytes[1]=(unsigned char)(0x80|((v>>12)&63));bytes[2]=(unsigned char)(0x80|((v>>6)&63));bytes[3]=(unsigned char)(0x80|(v&63));n=4;}
                    for(i=0;i<n;++i){if(at+1>=size)return NULL;out[at++]=(char)bytes[i];}continue;
                }
            }else if(c=='n')v='\n';else if(c=='r')v='\r';else if(c=='t')v='\t';else if(c=='b')v='\b';else if(c=='f')v='\f';
            else if(c=='"'||c=='\\'||c=='/')v=(unsigned char)c;else return NULL;
        }
        if(at+1>=size)return NULL;out[at++]=(char)v;
    }
    if(*p!='"')return NULL;out[at]=0;return p+1;
}
static int uj_get(const char *json,const char *wanted,char *out,size_t size)
{
    const char *p=uj_space(json);char key[128],value[4096];int found=0;
    if(*p++!='{')return 0;
    for(;;){p=uj_space(p);if(*p=='}'){++p;break;}p=uj_read_string(p,key,sizeof(key));if(!p)return 0;
        p=uj_space(p);if(*p++!=':')return 0;p=uj_space(p);
        if(*p=='"'){p=uj_read_string(p,value,sizeof(value));if(!p)return 0;}
        else{size_t n=0;while(*p&&*p!=','&&*p!='}'&&*p!=' '&&*p!='\n'){if(n+1>=sizeof(value)||*p=='['||*p=='{')return 0;value[n++]=*p++;}value[n]=0;if(!n)return 0;}
        if(!strcmp(key,wanted)){if(found||strlen(value)>=size)return 0;strcpy(out,value);found=1;}
        p=uj_space(p);if(*p=='}'){++p;break;}if(*p++!=',')return 0;
    }
    return *uj_space(p)==0&&found;
}
static uint64_t uj_u64(const char *json,const char *key)
{
    char text[32];const char *p;uint64_t n=0;if(!uj_get(json,key,text,sizeof(text)))return 0;
    for(p=text;*p;++p){unsigned d=(unsigned)(*p-'0');if(d>9||n>(UINT64_MAX-d)/10)return 0;n=n*10+d;}return n;
}
#endif
