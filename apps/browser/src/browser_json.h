#ifndef BROWSER_JSON_H
#define BROWSER_JSON_H
#include <stddef.h>
/* Small JSON data helpers for trusted chrome and public command parameters. */
int browser_json_get(const char *,const char *key,char *out,size_t capacity);
int browser_json_quote(const char *,char *out,size_t capacity);
#endif
