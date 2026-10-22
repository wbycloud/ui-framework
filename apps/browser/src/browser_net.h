#ifndef BROWSER_NET_H
#define BROWSER_NET_H
#include "browser_url.h"
#include "browser_diagnostic.h"
#include "ui_framework/app_data.h"
#include <windows.h>
#define BROWSER_REPLY_KIND 1u
#define BROWSER_HTML_LIMIT (256u*1024u)
#define BROWSER_IMAGE_LIMIT (8u*1024u*1024u)
#define BROWSER_TRANSFER_LIMIT (8u*1024u*1024u)
typedef enum browser_resource_kind { BROWSER_HTML=1,BROWSER_IMAGE=2,BROWSER_CSS=3 } browser_resource_kind_t;
typedef struct browser_reply {
    uint64_t navigation,resource;
    uint32_t kind,error,http_status,width,height,restrictions;
    size_t bytes;
    double network_ms,decode_ms;
    uint32_t category,phase,tls_flags,redirects,codec_status,encoding_status,orientation,frames;
    char url[BROWSER_URL_CAP],message[512],redirect_trace[4096];
} browser_reply_t;
#define BROWSER_REPLY_DATA(reply) ((unsigned char *)((reply)+1))
typedef struct browser_net browser_net_t;
browser_net_t *browser_net_create(ui_workspace_t *,uint64_t,const browser_policy_t *);
int browser_net_begin(browser_net_t *,uint64_t,const char *url);
int browser_net_resource(browser_net_t *,uint64_t,uint64_t,browser_resource_kind_t,const char *url);
void browser_net_stop(browser_net_t *);
void browser_net_set_policy(browser_net_t *,const browser_policy_t *); /* Cancels first; locked policy replacement. */
void browser_net_shutdown(browser_net_t *);
int browser_net_finished(browser_net_t *);
int browser_net_idle(browser_net_t *);
long browser_net_take_error(browser_net_t *);
int browser_net_decoding(browser_net_t *);
void browser_net_destroy(browser_net_t *); /* Only after finished, or create failure. */
long browser_net_live_threads(void);
long browser_net_live_callbacks(void);
#endif
