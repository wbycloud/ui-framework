#define WIN32_LEAN_AND_MEAN
#include "browser_json.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
static const char *space(const char *s){while(isspace((unsigned char)*s))++s;return s;}
static int hex(char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;}
static const char *string(const char *s,char *out,size_t capacity)
{
    size_t used=0;if(*s++!='"')return NULL;
    while(*s&&*s!='"'){char bytes[8];size_t count=1;unsigned char c=(unsigned char)*s++;bytes[0]=(char)c;
        if(c<32)return NULL;
        if(c=='\\'){char escape=*s++;if(escape=='"'||escape=='\\'||escape=='/')bytes[0]=escape;
            else if(escape=='n')bytes[0]='\n';else if(escape=='r')bytes[0]='\r';else if(escape=='t')bytes[0]='\t';else if(escape=='b')bytes[0]='\b';else if(escape=='f')bytes[0]='\f';
            else if(escape=='u'){wchar_t w[3]={0};unsigned value=0;int digits;
                if(strlen(s)<4)return NULL;
                for(digits=0;digits<4;++digits){int h=hex(*s++);if(h<0)return NULL;value=value*16+(unsigned)h;}w[0]=(wchar_t)value;
                if(value>=0xd800&&value<=0xdbff){unsigned tail=0;if(strlen(s)<6||s[0]!='\\'||s[1]!='u')return NULL;s+=2;for(digits=0;digits<4;++digits){int h=hex(*s++);if(h<0)return NULL;tail=tail*16+(unsigned)h;}if(tail<0xdc00||tail>0xdfff)return NULL;w[1]=(wchar_t)tail;}
                if(!value)return NULL;{int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,w,w[1]?2:1,bytes,sizeof(bytes),NULL,NULL);if(!n)return NULL;count=(size_t)n;}}
            else return NULL;}
        if(count>=capacity-used)return NULL;memcpy(out+used,bytes,count);used+=count;
    }if(*s!='"')return NULL;out[used]=0;return s+1;
}
int browser_json_get(const char *json,const char *key,char *out,size_t capacity)
{
    char name[64],value[4096];const char *p=space(json);int found=0;if(*p++!='{')return 0;
    p=space(p);while(*p&&*p!='}'){
        p=string(p,name,sizeof(name));if(!p)return 0;p=space(p);if(*p++!=':')return 0;p=space(p);
        p=string(p,value,sizeof(value));if(!p)return 0;
        if(!strcmp(name,key)){if(found||strlen(value)>=capacity)return 0;strcpy(out,value);found=1;}
        p=space(p);if(*p==','){p=space(p+1);if(*p=='}')return 0;}else if(*p!='}')return 0;
    }if(*p++!='}'||*space(p))return 0;return found;
}
int browser_json_quote(const char *s,char *out,size_t capacity)
{
    size_t used=0;if(capacity<3)return 0;out[used++]='"';
    for(;*s;++s){unsigned char c=(unsigned char)*s;char piece[7];size_t count=1;piece[0]=(char)c;
        if(c=='"'||c=='\\'){piece[0]='\\';piece[1]=(char)c;count=2;}
        else if(c<32){snprintf(piece,sizeof(piece),"\\u%04x",(unsigned)c);count=6;}
        if(count+2>=capacity-used)return 0;memcpy(out+used,piece,count);used+=count;}
    out[used++]='"';out[used]=0;return 1;
}
