/* Example-local reader for the flat semantic edit payload; no backend dependency. */
#include <stdint.h>
#include <string.h>
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
