#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "browser_body.h"
#include "lexbor/html/html.h"
#include <string.h>
typedef struct body_text {char *out;size_t used;unsigned nodes;char pending;} body_text_t;
static ui_status_t body_append(body_text_t *b,const unsigned char *s,size_t len)
{
 for(size_t i=0;i<len;){unsigned char c=s[i];if(c==' '||c=='\t'||c=='\r'||c=='\n'||c=='\f'){if(b->used&&!b->pending)b->pending=' ';++i;continue;}
  size_t bytes=c<0x80?1:c<0xe0?2:c<0xf0?3:4,space=b->pending?1:0;
  if(bytes>len-i||bytes+space>BROWSER_BODY_BYTES-b->used)return UI_STATUS_LIMIT_EXCEEDED;
  if(space)b->out[b->used++]=b->pending;b->pending=0;memcpy(b->out+b->used,s+i,bytes);b->used+=bytes;i+=bytes;
 }return UI_STATUS_OK;
}
static ui_status_t body_tree(body_text_t *b,lxb_dom_node_t *node,unsigned depth)
{
 if(depth>64||++b->nodes>1024)return UI_STATUS_LIMIT_EXCEEDED;
 if(node->type==LXB_DOM_NODE_TYPE_TEXT){size_t length=0;lxb_char_t *s=lxb_dom_node_text_content(node,&length);if(!s)return UI_STATUS_OUT_OF_MEMORY;ui_status_t status=body_append(b,s,length);lxb_dom_document_destroy_text(node->owner_document,s);return status;}
 int block=0;if(node->type==LXB_DOM_NODE_TYPE_ELEMENT){size_t length=0;const lxb_char_t *name=lxb_dom_element_local_name(lxb_dom_interface_element(node),&length);
  const char *skip[]={"script","style","template","noscript","input","textarea","select","button"};for(unsigned i=0;i<sizeof(skip)/sizeof(skip[0]);++i)if(strlen(skip[i])==length&&!memcmp(skip[i],name,length))return UI_STATUS_OK;
  const char *blocks[]={"p","div","section","article","main","header","footer","nav","aside","address","blockquote","ul","ol","li","pre","table","tr","td","th","caption","br","hr","h1","h2","h3","h4","h5","h6"};for(unsigned i=0;i<sizeof(blocks)/sizeof(blocks[0]);++i)if(strlen(blocks[i])==length&&!memcmp(blocks[i],name,length)){block=1;break;}
 }
 if(block&&b->used)b->pending='\n';
 for(lxb_dom_node_t *n=lxb_dom_node_first_child(node);n;n=lxb_dom_node_next(n)){ui_status_t status=body_tree(b,n,depth+1);if(status)return status;}
 if(block&&b->used)b->pending='\n';return UI_STATUS_OK;
}
ui_status_t browser_body_extract(const char *html,size_t length,char out[BROWSER_BODY_BYTES+1])
{
 if(!out)return UI_STATUS_INVALID_ARGUMENT;out[0]=0;
 if(!html||!length||length>256u*1024u||memchr(html,0,length))return UI_STATUS_VALIDATION_FAILED;
 if(!MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,html,(int)length,NULL,0))return UI_STATUS_VALIDATION_FAILED;
 lxb_html_document_t *doc=lxb_html_document_create();if(!doc)return UI_STATUS_OUT_OF_MEMORY;
 ui_status_t status=UI_STATUS_VALIDATION_FAILED;
 if(lxb_html_document_parse(doc,(const lxb_char_t *)html,length)==LXB_STATUS_OK){lxb_html_body_element_t *body=lxb_html_document_body_element(doc);if(body){body_text_t b={out,0,0,0};status=body_tree(&b,lxb_dom_interface_node(body),0);if(!status)out[b.used]=0;}}
 if(status)out[0]=0;lxb_html_document_destroy(doc);return status;
}
