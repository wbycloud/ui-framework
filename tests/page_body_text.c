#include "browser_body.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned checks,failures;
#define CHECK(x) do{++checks;if(!(x)){++failures;fprintf(stderr,"body text line%d: %s\n",__LINE__,#x);}}while(0)
static void exact(const char *html,const char *expected)
{char out[BROWSER_BODY_BYTES+1];CHECK(browser_body_extract(html,strlen(html),out)==UI_STATUS_OK);CHECK(!strcmp(out,expected));}
int main(void)
{
 exact("<title>not body</title><body><h1>中文交接</h1><p>设备  12\t台。</p><p>完成9台；待处理3台。</p><div>林青 &amp; 周明<br>下一步</div></body>","中文交接\n设备 12 台。\n完成9台；待处理3台。\n林青 & 周明\n下一步");
 exact("<body><p>  A <span> B </span> C </p><script>not text</script><style>not text</style><template>not text</template><noscript>not text</noscript><input value='secret'><textarea>draft</textarea><select><option>draft</option></select><button>not body</button></body>","A B C");
 exact("<body> \t\n </body>","");exact("<body><p style='display:none'>DOM隐藏文本</p><p>&#x1f600; &lt;正文&gt;</p></body>","DOM隐藏文本\n😀 <正文>");
 exact("<body><ol><li>一</li><li>二</li></ol><table><tr><td>三</td><td>四</td></tr></table></body>","一\n二\n三\n四");
 char *html=malloc(20000),out[BROWSER_BODY_BYTES+1];CHECK(html!=NULL);if(!html)return 1;
 strcpy(html,"<body>");memset(html+6,'x',8192);strcpy(html+8198,"</body>");CHECK(browser_body_extract(html,strlen(html),out)==UI_STATUS_OK);CHECK(strlen(out)==8192);
 html[8198]='x';strcpy(html+8199,"</body>");CHECK(browser_body_extract(html,strlen(html),out)==UI_STATUS_LIMIT_EXCEEDED&&out[0]==0);
 strcpy(html,"<body>");for(unsigned i=0;i<2730;++i)memcpy(html+6+i*3,"中",3);strcpy(html+8196,"ab</body>");CHECK(browser_body_extract(html,strlen(html),out)==UI_STATUS_OK&&strlen(out)==8192);strcpy(html+8196,"中文</body>");CHECK(browser_body_extract(html,strlen(html),out)==UI_STATUS_LIMIT_EXCEEDED&&out[0]==0);
 const char invalid[]="<body>\xff</body>",nul[]="<body>a\0b</body>";CHECK(browser_body_extract(invalid,sizeof(invalid)-1,out)==UI_STATUS_VALIDATION_FAILED&&out[0]==0);CHECK(browser_body_extract(nul,sizeof(nul)-1,out)==UI_STATUS_VALIDATION_FAILED&&out[0]==0);
 html[0]=0;for(unsigned i=0;i<66;++i)strcat(html,"<div>");strcat(html,"deep");for(unsigned i=0;i<66;++i)strcat(html,"</div>");CHECK(browser_body_extract(html,strlen(html),out)==UI_STATUS_LIMIT_EXCEEDED&&out[0]==0);
 html[0]=0;for(unsigned i=0;i<1025;++i)strcat(html,"<br>");CHECK(browser_body_extract(html,strlen(html),out)==UI_STATUS_LIMIT_EXCEEDED&&out[0]==0);free(html);
 printf("PB-1 text checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
