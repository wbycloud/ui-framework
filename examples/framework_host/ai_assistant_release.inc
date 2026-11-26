#include "../../src/ai_assistant_price.h"

static const char *ai_release_string(yyjson_val *o,const char *key,size_t capacity)
{
    yyjson_val *v=yyjson_obj_get(o,key);const char *text=yyjson_get_str(v);
    size_t length=yyjson_get_len(v);
    return text&&length&&length<capacity&&strlen(text)==length?text:NULL;
}

/* Read only the release descriptor beside the actual host executable. It is
 * deployment input for this Service, independent of the profile and its key. */
static yyjson_doc *ai_assistant_release(char evidence[65])
{
    wchar_t path[4096];DWORD n=GetModuleFileNameW(NULL,path,4096);
    wchar_t *name=n&&n<4096?wcsrchr(path,L'\\'):NULL;
    if(!name||wcscpy_s(name+1,4096-(size_t)(name+1-path),L"assistant-release.json"))return NULL;
    FILE *f=_wfopen(path,L"rb");if(!f)return NULL;
    char raw[8193];size_t length=fread(raw,1,sizeof(raw),f);
    int complete=feof(f)&&!ferror(f);fclose(f);
    if(!complete||length>8192||!length)return NULL;
    yyjson_doc *d=yyjson_read(raw,length,0);yyjson_val *root=yyjson_doc_get_root(d);
    if(!yyjson_is_uint(yyjson_obj_get(root,"version"))||yyjson_get_uint(yyjson_obj_get(root,"version"))!=1){yyjson_doc_free(d);return NULL;}
    ui_ai_assistant_attachment_t blob={0};blob.size=sizeof(blob);blob.version=1;
    blob.name_utf8="assistant-release.json";blob.media_type="text/plain";blob.bytes=raw;blob.byte_count=length;
    if(ui_ai_assistant_attachment_hash(&blob,evidence)){yyjson_doc_free(d);return NULL;}
    return d;
}

static void ai_assistant_release_price(yyjson_doc *d,const char *evidence,ui_ai_assistant_policy_t *p)
{
    yyjson_val *price=yyjson_obj_get(yyjson_doc_get_root(d),"price");
    const char *model=ai_release_string(price,"model",128),*protocol=ai_release_string(price,"protocol",64);
    const char *version=ai_release_string(price,"version",65),*profile=ai_release_string(price,"meter_profile",65);
    yyjson_val *input=yyjson_obj_get(price,"input_nano_usd_per_token"),*output=yyjson_obj_get(price,"output_nano_usd_per_token");
    if(!model||strcmp(model,"deepseek-flash")||!protocol||strcmp(protocol,"deepseek-responses-v1")||
       !profile||strcmp(profile,p->meter_profile)||!version||strncmp(version,AI_RELEASE_PRICE_PREFIX,sizeof(AI_RELEASE_PRICE_PREFIX)-1)||
       !ai_assistant_price_current(version)||!ai_release_string(price,"pricing_source",256)||!ai_release_string(price,"protocol_source",256)||
       !yyjson_is_uint(input)||!yyjson_is_uint(output)||!yyjson_get_uint(input)||yyjson_get_uint(input)>1000000||
       !yyjson_get_uint(output)||yyjson_get_uint(output)>1000000)return;
    p->price_version=version;p->price_evidence_sha256=evidence;
    p->input_rate_nano_usd=yyjson_get_uint(input);p->output_rate_nano_usd=yyjson_get_uint(output);
}
