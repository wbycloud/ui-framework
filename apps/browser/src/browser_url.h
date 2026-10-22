#ifndef BROWSER_URL_H
#define BROWSER_URL_H
#include <stddef.h>
#define BROWSER_URL_CAP 4096
#define BROWSER_MAX_ORIGINS 16
typedef struct browser_url {
    char text[BROWSER_URL_CAP], origin[BROWSER_URL_CAP], host[1024];
    char path[BROWSER_URL_CAP], fragment[BROWSER_URL_CAP];
    unsigned short port;
    int secure;
} browser_url_t;
typedef struct browser_policy { char origins[BROWSER_MAX_ORIGINS][BROWSER_URL_CAP]; size_t count; } browser_policy_t;
typedef enum browser_url_result { BROWSER_URL_OK=0,BROWSER_URL_INVALID,BROWSER_URL_SCHEME,BROWSER_URL_CREDENTIALS,BROWSER_URL_LIMIT,BROWSER_URL_MEMORY,BROWSER_URL_PORT } browser_url_result_t;
browser_url_result_t browser_url_parse_ex(const char *base,const char *input,browser_url_t *);
int browser_url_parse(const char *base,const char *input,browser_url_t *);
void browser_url_public(const char *url,char out[BROWSER_URL_CAP]); /* No credentials, query or fragment. */
int browser_policy_read(browser_policy_t *,const char *text);
int browser_url_allowed(const browser_policy_t *,const browser_url_t *);
int browser_url_form(const char *url,const char *const *names,const char *const *values,size_t count,char out[BROWSER_URL_CAP]);
#endif
