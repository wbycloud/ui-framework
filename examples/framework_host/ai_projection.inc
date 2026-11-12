/* One bounded state group, never a page-supplied authorization. */
#include <bcrypt.h>
#pragma comment(lib,"bcrypt.lib")
#define AI_HOST_STATE_BYTES (2u*1024u*1024u)
#define AI_HOST_FRAGMENT_BYTES 8192u
static void ai_projection_invalidate(host_window_t *s)
{s->ai_display_valid=0;s->ai_preview_ready=0;s->ai_projection_rendered=0;}
static int ai_projection_hash(const char *text,size_t length,char hash[65])
{
 BCRYPT_ALG_HANDLE a=NULL;BCRYPT_HASH_HANDLE h=NULL;unsigned char digest[32];NTSTATUS st=BCryptOpenAlgorithmProvider(&a,BCRYPT_SHA256_ALGORITHM,NULL,0);
 if(st>=0)st=BCryptCreateHash(a,&h,NULL,0,NULL,0,0);if(st>=0)st=BCryptHashData(h,(PUCHAR)text,(ULONG)length,0);if(st>=0)st=BCryptFinishHash(h,digest,32,0);
 if(h)BCryptDestroyHash(h);if(a)BCryptCloseAlgorithmProvider(a,0);if(st<0)return 0;
 for(unsigned i=0;i<32;++i){hash[2*i]="0123456789abcdef"[digest[i]>>4];hash[2*i+1]="0123456789abcdef"[digest[i]&15];}hash[64]=0;return 1;
}
static int ai_projection_marker(host_window_t *s)
{
 char expected[110];ui_element_presentation_t p={0};p.size=sizeof(p);snprintf(expected,sizeof(expected),"%llu:%s",(unsigned long long)s->ai_display_generation,s->ai_state_hash);
 return s->view&&!ui_web_view_get_presentation(s->view,"ai-projection-proof",&p)&&!strcmp(p.text_utf8,expected);
}
static int ai_projection_send(host_window_t *s,const char *text)
{
 size_t length=strlen(text),at=0,count=0,index=0;char hash[65],chunk[AI_HOST_FRAGMENT_BYTES+1];
 if(s->ai_state_sent&&!strcmp(s->ai_state_sent,text)&&s->ai_display_valid&&ai_projection_marker(s)&&(!s->ai_preview||ai_model_preview_displayed(s))){if(ui_web_view_post_json(s->view,"{\"type\":\"repaint\"}")==UI_STATUS_OK)return 1;ai_projection_invalidate(s);return 0;}
 ai_projection_invalidate(s);
 if(UI_UHA_PROJECTION_PROBE(3)||s->ai_display_generation==UINT64_MAX||length>AI_HOST_STATE_BYTES||!ai_projection_hash(text,length,hash))return 0;
 ++s->ai_display_generation;
 while(at<length){size_t n=length-at;if(n>AI_HOST_FRAGMENT_BYTES)n=AI_HOST_FRAGMENT_BYTES;while(at+n<length&&((unsigned char)text[at+n]&0xc0)==0x80)--n;if(!n)return 0;at+=n;++count;}
 for(at=0;at<length;){size_t n=length-at;if(n>AI_HOST_FRAGMENT_BYTES)n=AI_HOST_FRAGMENT_BYTES;while(at+n<length&&((unsigned char)text[at+n]&0xc0)==0x80)--n;memcpy(chunk,text+at,n);chunk[n]=0;
  json_buffer_t packet={0};json_format(&packet,"{\"type\":\"projection\",\"generation\":\"%llu\",\"hash\":\"%s\",\"count\":%u,\"index\":%u,\"bytes\":%u,\"text\":",(unsigned long long)s->ai_display_generation,hash,(unsigned)count,(unsigned)index,(unsigned)length);json_string(&packet,chunk);json_format(&packet,",\"chunk_bytes\":%u}",(unsigned)n);
  int ok=!packet.failed&&packet.length<=256u*1024u&&ui_web_view_post_json(s->view,packet.text)==UI_STATUS_OK;free(packet.text);if(!ok)return 0;at+=n;++index;
 }
 if(s->ai_general)for(unsigned part=0;part<2;++part){
  char packet[220];snprintf(packet,sizeof(packet),"{\"type\":\"general-projection\",\"generation\":\"%llu\",\"hash\":\"%s\",\"part\":%u}",(unsigned long long)s->ai_display_generation,hash,part);
  if(ui_web_view_post_json(s->view,packet)!=UI_STATUS_OK)return 0;
 }
 strcpy_s(s->ai_state_hash,65,hash);if(!ai_projection_marker(s))return 0;
 char *copy=_strdup(text);if(!copy)return 0;free(s->ai_state_sent);s->ai_state_sent=copy;s->ai_display_valid=1;return 1;
}
