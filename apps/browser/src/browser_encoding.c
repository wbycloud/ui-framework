#include "browser_encoding.h"
#include "lexbor/html/encoding.h"
#include "lexbor/encoding/encoding.h"
#include <stdlib.h>
#include <string.h>
browser_encoding_result_t browser_encoding_convert(const unsigned char *data,size_t length,const char *type,int html,size_t limit,unsigned char **out,size_t *out_bytes)
{
    const lxb_encoding_data_t *encoding=NULL,*utf8=lxb_encoding_data(LXB_ENCODING_UTF_8);
    const lxb_char_t *at,*end,*name_end=NULL,*name;
    lxb_codepoint_t *points=NULL;const lxb_codepoint_t *cp;lxb_encoding_decode_t decoder;lxb_encoding_encode_t encoder;
    unsigned char *result=NULL;lxb_status_t status;browser_encoding_result_t result_code=BROWSER_ENCODING_INVALID;
    *out=NULL;*out_bytes=0;if(!data)return BROWSER_ENCODING_INVALID;if(length>256u*1024u||limit>256u*1024u)return BROWSER_ENCODING_LIMIT;at=data;end=data+length;
    if(length>=3&&data[0]==0xef&&data[1]==0xbb&&data[2]==0xbf){encoding=utf8;at+=3;}
    else if(length>=2&&data[0]==0xff&&data[1]==0xfe){encoding=lxb_encoding_data(LXB_ENCODING_UTF_16LE);at+=2;}
    else if(length>=2&&data[0]==0xfe&&data[1]==0xff){encoding=lxb_encoding_data(LXB_ENCODING_UTF_16BE);at+=2;}
    if(!encoding&&type){name=lxb_html_encoding_content((const lxb_char_t *)type,(const lxb_char_t *)type+strlen(type),&name_end);
        if(name){encoding=lxb_encoding_data_by_pre_name(name,(size_t)(name_end-name));if(!encoding)return BROWSER_ENCODING_UNKNOWN;}}
    if(!encoding&&html){lxb_html_encoding_t scan={0};if(lxb_html_encoding_init(&scan)!=LXB_STATUS_OK)return BROWSER_ENCODING_MEMORY;
        status=lxb_html_encoding_determine(&scan,data,data+(length<1024?length:1024));
        if(status==LXB_STATUS_OK&&lxb_html_encoding_meta_length(&scan)){lxb_html_encoding_entry_t *entry=lxb_html_encoding_meta_entry(&scan,0);encoding=lxb_encoding_data_by_pre_name(entry->name,(size_t)(entry->end-entry->name));
            if(!encoding){lxb_html_encoding_destroy(&scan,false);return BROWSER_ENCODING_UNKNOWN;}}
        lxb_html_encoding_destroy(&scan,false);
        /* HTML meta labels cannot switch an ASCII pre-scan to UTF-16. */
        if(encoding==lxb_encoding_data(LXB_ENCODING_UTF_16LE)||encoding==lxb_encoding_data(LXB_ENCODING_UTF_16BE))encoding=utf8;}
    if(!encoding)encoding=utf8;
    points=(lxb_codepoint_t *)malloc((length+8)*sizeof(*points));result=(unsigned char *)malloc(limit+1);if(!points||!result){result_code=BROWSER_ENCODING_MEMORY;goto done;}
    if(lxb_encoding_decode_init(&decoder,encoding,points,length+8)!=LXB_STATUS_OK)goto done;
    status=encoding->decode(&decoder,&at,end);
    if((status!=LXB_STATUS_OK&&status!=LXB_STATUS_CONTINUE)||lxb_encoding_decode_finish(&decoder)!=LXB_STATUS_OK||at!=end)goto done;
    for(size_t i=0;i<decoder.buffer_used;++i)if(!points[i])goto done;
    if(lxb_encoding_encode_init(&encoder,utf8,result,limit)!=LXB_STATUS_OK)goto done;
    cp=points;status=utf8->encode(&encoder,&cp,points+decoder.buffer_used);if(status!=LXB_STATUS_OK){if(status==LXB_STATUS_SMALL_BUFFER||status==LXB_STATUS_ERROR_SMALL_BUFFER)result_code=BROWSER_ENCODING_LIMIT;goto done;}if(lxb_encoding_encode_finish(&encoder)!=LXB_STATUS_OK)goto done;
    result[encoder.buffer_used]=0;*out=result;*out_bytes=encoder.buffer_used;result=NULL;result_code=BROWSER_ENCODING_OK;
done:free(points);free(result);return result_code;
}

int browser_encoding_utf8(const unsigned char *data,size_t length,const char *type,int html,size_t limit,unsigned char **out,size_t *out_bytes)
{return browser_encoding_convert(data,length,type,html,limit,out,out_bytes)==BROWSER_ENCODING_OK;}
