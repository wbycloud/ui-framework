#include "browser_encoding.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int failures;
#define T(x) do{if(!(x)){fprintf(stderr,"encoding line%d: %s\n",__LINE__,#x);++failures;}}while(0)
static void check(const unsigned char *p,size_t bytes,const char *type,size_t limit,const char *expected)
{unsigned char *out=NULL;size_t n=0;int ok=browser_encoding_utf8(p,bytes,type,1,limit,&out,&n);T(ok==(expected!=NULL));if(ok&&expected){T(n==strlen(expected));T(!strcmp((const char *)out,expected));}free(out);}
int main(void)
{
    static const unsigned char gbk[]={0xd6,0xd0,0xce,0xc4};
    static const unsigned char utf16be[]={0xfe,0xff,0x4e,0x2d,0x65,0x87};
    const char *conflict="<meta charset='gb18030'><p>中文</p>";
    check((const unsigned char *)"\xef\xbb\xbf中文",9,"text/html; charset=unknown",256,"中文");
    check(utf16be,sizeof(utf16be),"text/html; charset=utf-8",256,"中文");
    check((const unsigned char *)conflict,strlen(conflict),"text/html; charset=utf-8",256,conflict);
    check(gbk,sizeof(gbk),"text/html; charset=gb18030",256,"中文");
    check((const unsigned char *)"<meta charset='utf-16'><p>中文</p>",strlen("<meta charset='utf-16'><p>中文</p>"),"text/html",256,"<meta charset='utf-16'><p>中文</p>");
    check((const unsigned char *)"<meta charset='unknown'><p>ok</p>",strlen("<meta charset='unknown'><p>ok</p>"),"text/html",256,NULL);
    check((const unsigned char *)"\xe4\xb8",2,"text/html",256,NULL);
    check((const unsigned char *)"x\0y",3,"text/html",256,NULL);
    check(gbk,sizeof(gbk),"text/html; charset=gb18030",5,NULL);
    {unsigned char *out=NULL;size_t bytes=0;T(browser_encoding_convert(gbk,sizeof(gbk),"text/html; charset=gb18030",1,5,&out,&bytes)==BROWSER_ENCODING_LIMIT);T(out==NULL&&bytes==0);T(browser_encoding_convert((const unsigned char *)"a",1,"text/html; charset=unknown",1,64,&out,&bytes)==BROWSER_ENCODING_UNKNOWN);}
    puts("Encoding precedence/strict/converted budget check complete");printf("encoding failures=%d\n",failures);return failures?1:0;
}
