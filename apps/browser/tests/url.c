#include "browser_url.h"
#include <stdio.h>
#include <string.h>
int main(void)
{
    browser_url_t u;browser_policy_t p;char form[BROWSER_URL_CAP];int fail=0;
#define T(x) do{if(!(x)){fprintf(stderr,"line%d: %s\n",__LINE__,#x);++fail;}}while(0)
    T(browser_policy_read(&p,"# controlled origins\nhttps://EXAMPLE.test:443\nhttp://127.0.0.1:8765\n"));
    T(browser_url_parse("https://example.test/a/b","../中文?q=1#部分",&u));T(browser_url_allowed(&p,&u));
    T(strstr(u.text,"%E4%B8%AD")!=NULL);T(strstr(u.fragment,"%E9%83%A8")!=NULL);
    T(!browser_url_parse(NULL,"file:///C:/test",&u));T(!browser_url_parse(NULL,"javascript:alert(1)",&u));
    T(!browser_url_parse(NULL,"https://user:password@example.test/",&u));
    T(browser_url_parse(NULL,"https://example.test.evil/",&u));T(!browser_url_allowed(&p,&u));
    T(browser_url_parse(NULL,"https://example.test:444/",&u));T(!browser_url_allowed(&p,&u));
    T(browser_url_parse("http://127.0.0.1:8765/a","/next",&u));printf("relative local URL=%s origin=%s port=%u\n",u.text,u.origin,u.port);T(!strcmp(u.origin,"http://127.0.0.1:8765"));
    {const char *names[]={"q","x"},*values[]={"中 &文","a+b"};T(browser_url_form("https://example.test/a?old=1#f",names,values,2,form));T(strstr(form,"q=%E4%B8%AD+%26%E6%96%87&x=a%2Bb")!=NULL);}
    T(browser_policy_read(&p,"https://example.test"));
    T(!browser_policy_read(&p,"https://*.example.test"));
    T(!browser_url_parse(NULL,"https://example.test:0/",&u));
    T(!browser_policy_read(&p,"https://example.test/path"));
    T(p.count==1&&!strcmp(p.origins[0],"https://example.test")); /* Invalid edit is atomic. */
    {const char *names[]={"same","same","line"},*values[]={"","+ &中文","a\nb"};T(browser_url_form("https://example.test/a?old=1#末尾",names,values,3,form));T(strstr(form,"same=&same=%2B+%26%E4%B8%AD%E6%96%87&line=a%0D%0Ab")!=NULL);T(strstr(form,"#%E6%9C%AB%E5%B0%BE")!=NULL);}
    printf("URL/policy check: failures=%d\n",fail);return fail?1:0;
}
