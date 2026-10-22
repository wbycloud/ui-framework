#include "browser_url.h"
#include "lexbor/url/url.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct buffer { char *text; size_t length,capacity; } buffer_t;
static lxb_status_t append(const lxb_char_t *s,size_t length,void *data)
{
    buffer_t *b=(buffer_t *)data;if(length>=b->capacity-b->length)return LXB_STATUS_ERROR;
    memcpy(b->text+b->length,s,length);b->length+=length;b->text[b->length]=0;return LXB_STATUS_OK;
}
browser_url_result_t browser_url_parse_ex(const char *base,const char *input,browser_url_t *out)
{
    lxb_url_parser_t *p; lxb_url_t *b=NULL,*u=NULL;buffer_t text,host,path,fragment;browser_url_result_t result=BROWSER_URL_INVALID;
    if(!input||!*input||!out)return BROWSER_URL_INVALID;if(strlen(input)>=BROWSER_URL_CAP)return BROWSER_URL_LIMIT;memset(out,0,sizeof(*out));
    p=lxb_url_parser_create();if(!p)return BROWSER_URL_MEMORY;
    if(lxb_url_parser_init(p,NULL)!=LXB_STATUS_OK){lxb_url_parser_destroy(p,true);return BROWSER_URL_MEMORY;}
    if(base&&*base){b=lxb_url_parse(p,NULL,(const lxb_char_t *)base,strlen(base));lxb_url_parser_clean(p);if(!b)goto done;}
    u=lxb_url_parse(p,b,(const lxb_char_t *)input,strlen(input));
    /* Fixed Lexbor 2.5 host_copy copies numeric address bits but omits the
       destination type for inherited IPv4/IPv6 hosts. Preserve that metadata;
       parsing/resolution and all address bits remain owned by Lexbor. */
    if(u&&b&&u->host.type==LXB_URL_HOST_TYPE__UNDEF&&(b->host.type==LXB_URL_HOST_TYPE_IPV4||b->host.type==LXB_URL_HOST_TYPE_IPV6))u->host.type=b->host.type;
    if(!u)goto done;
    if(u->scheme.type!=LXB_URL_SCHEMEL_TYPE_HTTP&&u->scheme.type!=LXB_URL_SCHEMEL_TYPE_HTTPS){result=BROWSER_URL_SCHEME;goto done;}
    if(u->username.length||u->password.length){result=BROWSER_URL_CREDENTIALS;goto done;}
    if(u->has_port&&!u->port){result=BROWSER_URL_PORT;goto done;} /* WinHTTP port0 means default, not origin port0. */
    result=BROWSER_URL_LIMIT;
    out->secure=u->scheme.type==LXB_URL_SCHEMEL_TYPE_HTTPS;
    out->port=u->has_port?u->port:(unsigned short)(out->secure?443:80);
    text=(buffer_t){out->text,0,sizeof(out->text)};host=(buffer_t){out->host,0,sizeof(out->host)};
    path=(buffer_t){out->path,0,sizeof(out->path)};fragment=(buffer_t){out->fragment,0,sizeof(out->fragment)};
    if(lxb_url_serialize(u,append,&text,false)!=LXB_STATUS_OK||lxb_url_serialize_host(&u->host,append,&host)!=LXB_STATUS_OK||
       lxb_url_serialize_path(&u->path,append,&path)!=LXB_STATUS_OK)goto done;
    if(u->query.data){if(append((const lxb_char_t *)"?",1,&path)!=LXB_STATUS_OK||append(u->query.data,u->query.length,&path)!=LXB_STATUS_OK)goto done;}
    if(u->fragment.data&&append(u->fragment.data,u->fragment.length,&fragment)!=LXB_STATUS_OK)goto done;
    if(u->has_port)snprintf(out->origin,sizeof(out->origin),"%s://%s:%u",out->secure?"https":"http",out->host,(unsigned)out->port);
    else snprintf(out->origin,sizeof(out->origin),"%s://%s",out->secure?"https":"http",out->host);
    result=BROWSER_URL_OK;
done:lxb_url_parser_memory_destroy(p);lxb_url_parser_destroy(p,true);return result;
}
int browser_url_parse(const char *base,const char *input,browser_url_t *out)
{return browser_url_parse_ex(base,input,out)==BROWSER_URL_OK;}
void browser_url_public(const char *input,char out[BROWSER_URL_CAP])
{
    browser_url_t url;if(!strcmp(input,"browser:start")||!strcmp(input,"browser:help")){strcpy(out,input);return;}
    if(!browser_url_parse(NULL,input,&url)){strcpy(out,"[无效或含凭据的地址已隐藏]");return;}
    snprintf(out,BROWSER_URL_CAP,"%s%.*s",url.origin,(int)strcspn(url.path,"?"),url.path);
}
int browser_policy_read(browser_policy_t *policy,const char *text)
{
    const char *at=text;browser_policy_t parsed={0};
    while(at&&*at){const char *end=strchr(at,'\n');size_t size=end?(size_t)(end-at):strlen(at);char line[BROWSER_URL_CAP];browser_url_t url;
        while(size&&(at[size-1]=='\r'||at[size-1]==' '||at[size-1]=='\t'))--size;
        while(size&&(*at==' '||*at=='\t')){++at;--size;}
        if(size&&*at!='#'){if(size>=sizeof(line)||parsed.count==BROWSER_MAX_ORIGINS)return 0;memcpy(line,at,size);line[size]=0;
            if(!browser_url_parse(NULL,line,&url)||strcmp(url.path,"/")||*url.fragment||strchr(url.host,'*'))return 0;
            {size_t i;for(i=0;i<parsed.count;++i)if(!strcmp(parsed.origins[i],url.origin))break;
             if(i==parsed.count)strcpy(parsed.origins[parsed.count++],url.origin);}}
        if(!end)break;at=end+1;
    }*policy=parsed;return text!=NULL;
}
int browser_url_allowed(const browser_policy_t *p,const browser_url_t *url)
{for(size_t i=0;i<p->count;++i)if(!strcmp(p->origins[i],url->origin))return 1;return 0;}
static int encode(buffer_t *b,const char *s)
{
    static const char hex[]="0123456789ABCDEF";for(;*s;++s){unsigned char c=(unsigned char)*s;char part[3];size_t n=1;part[0]=(char)c;
        if(c=='\r'||c=='\n'){if(c=='\r'&&s[1]=='\n')++s;if(append((const lxb_char_t *)"%0D%0A",6,b)!=LXB_STATUS_OK)return 0;continue;}
        if(c==' ')part[0]='+';else if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='*'||c=='-'||c=='.'||c=='_')){part[0]='%';part[1]=hex[c>>4];part[2]=hex[c&15];n=3;}
        if(append((const lxb_char_t *)part,n,b)!=LXB_STATUS_OK)return 0;}return 1;
}
int browser_url_form(const char *url,const char *const *names,const char *const *values,size_t count,char out[BROWSER_URL_CAP])
{
    browser_url_t parsed;buffer_t b={out,0,BROWSER_URL_CAP};char *query,*fragment;
    if(!browser_url_parse(NULL,url,&parsed)||count>64)return 0;
    fragment=strchr(parsed.text,'#');if(fragment)*fragment=0;query=strchr(parsed.text,'?');if(query)*query=0;
    if(append((const lxb_char_t *)parsed.text,strlen(parsed.text),&b)!=LXB_STATUS_OK||append((const lxb_char_t *)"?",1,&b)!=LXB_STATUS_OK)return 0;
    for(size_t i=0;i<count;++i){if(i&&append((const lxb_char_t *)"&",1,&b)!=LXB_STATUS_OK)return 0;
        if(!encode(&b,names[i])||append((const lxb_char_t *)"=",1,&b)!=LXB_STATUS_OK||!encode(&b,values[i]))return 0;}
    if(*parsed.fragment&&(append((const lxb_char_t *)"#",1,&b)!=LXB_STATUS_OK||append((const lxb_char_t *)parsed.fragment,strlen(parsed.fragment),&b)!=LXB_STATUS_OK))return 0;
    return 1;
}
