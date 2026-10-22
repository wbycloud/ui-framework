#ifndef BROWSER_ENCODING_H
#define BROWSER_ENCODING_H
#include <stddef.h>
/* Uses fixed Lexbor encoding sniffing/decoders. BOM > HTTP charset > HTML meta
 * (first 1024 bytes) > UTF-8. Strict errors and bounded UTF-8 output. */
int browser_encoding_utf8(const unsigned char *,size_t,const char *content_type,
    int html,size_t limit,unsigned char **out,size_t *out_bytes);
typedef enum browser_encoding_result {
    BROWSER_ENCODING_OK=0,BROWSER_ENCODING_UNKNOWN,BROWSER_ENCODING_INVALID,
    BROWSER_ENCODING_LIMIT,BROWSER_ENCODING_MEMORY
} browser_encoding_result_t;
browser_encoding_result_t browser_encoding_convert(const unsigned char *,size_t,
    const char *,int html,size_t limit,unsigned char **out,size_t *out_bytes);
#endif
