#include "ui_framework/capability.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int failures, checks;
#define CHECK(x) do { ++checks; if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);++failures;} } while(0)
static void canonical(const char *input,const char *expected)
{
    char *out=NULL;size_t n=0;ui_status_t s=ui_cap_json_canonicalize(input,strlen(input),&out,&n);
    CHECK(s==UI_STATUS_OK);if(!s){CHECK(n==strlen(expected));CHECK(!strcmp(out,expected));}ui_capability_release(out);
}
static void invalid(const char *input,ui_status_t expected)
{char *out=NULL;size_t n=0;CHECK(ui_cap_json_canonicalize(input,strlen(input),&out,&n)==expected);CHECK(out==NULL);}
static void valid(const char *schema,const char *input,ui_status_t expected)
{CHECK(ui_cap_json_validate(schema,input,strlen(input))==expected);}
int main(int argc,char **argv)
{
    char hash[65],*large;size_t i,n;char a[2048];
    if(argc==2&&!strcmp(argv[1],"--canonical")){
        char *line=malloc(UI_CAPABILITY_OUTPUT_BYTES+2u);int rc=0;
        if(!line)return 2;
        while(fgets(line,UI_CAPABILITY_OUTPUT_BYTES+2u,stdin)){
            char *out=NULL;size_t len=0;ui_status_t status=ui_cap_json_canonicalize(line,strlen(line),&out,&len);
            if(status){printf("ERROR:%d\n",status);rc=1;}else printf("%s\n",out);
            ui_capability_release(out);
        }
        free(line);return rc;
    }
    const char *properties="{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\",\"minLength\":1,\"maxLength\":4},\"mode\":{\"type\":\"string\",\"enum\":[\"auto\",\"manual\"]},\"rgba\":{\"type\":\"array\",\"items\":{\"type\":\"number\",\"minimum\":0,\"maximum\":1},\"minItems\":4,\"maxItems\":4}},\"required\":[\"name\",\"mode\",\"rgba\"],\"additionalProperties\":false}";
    canonical(" {\"b\":2, \"a\": [true, null, \"\\u0000\\n/\\\\\\\"\"]}","{\"a\":[true,null,\"\\u0000\\n/\\\\\\\"\"],\"b\":2}");
    canonical("{\"\\ue000\":1,\"\\ud800\\udc00\":2,\"a\":3}","{\"a\":3,\"\xf0\x90\x80\x80\":2,\"\xee\x80\x80\":1}");
    canonical("{\"\\u00e9\":1,\"e\\u0301\":2}","{\"e\xcc\x81\":2,\"\xc3\xa9\":1}");
    canonical("[-0,-0.0,1.0,1e-6,1e-7,1e20,1e21]","[0,0,1,0.000001,1e-7,100000000000000000000,1e+21]");
    canonical("[333333333.33333329,1E30,4.50,2e-3,0.000000000000000000000000001]","[333333333.3333333,1e+30,4.5,0.002,1e-27]");
    canonical("[5e-324,-5e-324,1.7976931348623157e308,9.999999999999997e22]","[5e-324,-5e-324,1.7976931348623157e+308,9.999999999999997e+22]");
    canonical("9007199254740992","9007199254740992");
    canonical("9007199254740993","9007199254740992");
    invalid("{\"a\":1,\"\\u0061\":2}",UI_STATUS_VALIDATION_FAILED);
    invalid("[{\"nested\":1,\"nested\":2}]",UI_STATUS_VALIDATION_FAILED);
    invalid("true false",UI_STATUS_VALIDATION_FAILED);invalid("[1,]",UI_STATUS_VALIDATION_FAILED);
    invalid("/*x*/{}",UI_STATUS_VALIDATION_FAILED);invalid("NaN",UI_STATUS_VALIDATION_FAILED);
    invalid("1e400",UI_STATUS_VALIDATION_FAILED);invalid("\"\\ud800\"",UI_STATUS_VALIDATION_FAILED);
    invalid("\"\xc0\xaf\"",UI_STATUS_VALIDATION_FAILED);
    memset(a,'[',17);a[17]='0';memset(a+18,']',17);a[35]=0;invalid(a,UI_STATUS_LIMIT_EXCEEDED);
    {char *out=NULL;size_t len=0;memset(a,'[',16);a[16]='0';memset(a+17,']',16);a[33]=0;CHECK(ui_cap_json_canonicalize(a,33,&out,&len)==UI_STATUS_OK);ui_capability_release(out);}
    a[0]='[';n=1;for(i=0;i<513;++i){if(i)a[n++]=',';a[n++]='0';}a[n++]=']';a[n]=0;invalid(a,UI_STATUS_LIMIT_EXCEEDED);
    n=0;a[n++]='{';for(i=0;i<129;++i)n+=(size_t)snprintf(a+n,sizeof(a)-n,"%s\"k%zu\":0",i?",":"",i);a[n++]='}';a[n]=0;invalid(a,UI_STATUS_LIMIT_EXCEEDED);
    large=malloc(UI_CAPABILITY_OUTPUT_BYTES+2u);CHECK(large!=NULL);if(large){memset(large,' ',UI_CAPABILITY_OUTPUT_BYTES+1u);large[UI_CAPABILITY_OUTPUT_BYTES+1u]=0;invalid(large,UI_STATUS_LIMIT_EXCEEDED);free(large);}
    /* 4105 nodes while every individual array remains within 512 items. */
    large=malloc(10000);CHECK(large!=NULL);if(large){size_t j;n=0;large[n++]='[';for(i=0;i<8;++i){if(i)large[n++]=',';large[n++]='[';for(j=0;j<512;++j){if(j)large[n++]=',';large[n++]='0';}large[n++]=']';}large[n++]=']';large[n]=0;invalid(large,UI_STATUS_LIMIT_EXCEEDED);free(large);}
    {const char bytes[]={'{','}',0,'x'};char *out=NULL;size_t len=0;CHECK(ui_cap_json_canonicalize(bytes,sizeof(bytes),&out,&len)==UI_STATUS_VALIDATION_FAILED);ui_capability_release(out);}
    valid(properties,"{\"name\":\"\xf0\x9f\x98\x80\",\"mode\":\"auto\",\"rgba\":[0,1,0.5,1]}",UI_STATUS_OK);
    valid(properties,"{\"name\":\"abcde\",\"mode\":\"auto\",\"rgba\":[0,1,0,1]}",UI_STATUS_VALIDATION_FAILED);
    valid(properties,"{\"name\":\"ok\",\"mode\":\"bad\",\"rgba\":[0,1,0,1]}",UI_STATUS_VALIDATION_FAILED);
    valid(properties,"{\"name\":\"ok\",\"mode\":\"auto\",\"rgba\":[0,1,0,2]}",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"integer\"}","9007199254740992",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"integer\"}","9007199254740991",UI_STATUS_OK);
    valid("{\"type\":\"integer\"}","1.1",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"boolean\",\"const\":true}","false",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"object\",\"additionalProperties\":false}","{\"extra\":1}",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"object\"}","{}",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"string\",\"format\":\"uri\"}","\"a\"",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"string\",\"pattern\":\"a\"}","\"a\"",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"string\",\"anyOf\":[]}","\"a\"",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"string\",\"$ref\":\"#\"}","\"a\"",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"array\"}","[]",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"string\",\"minimum\":1}","\"a\"",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"number\",\"minimum\":2,\"maximum\":1}","1",UI_STATUS_VALIDATION_FAILED);
    valid("{\"type\":\"string\",\"enum\":[\"a\",\"a\"]}","\"a\"",UI_STATUS_VALIDATION_FAILED);
#ifdef _WIN32
    CHECK(ui_cap_json_fingerprint("{}",2,hash)==UI_STATUS_OK);
    CHECK(!strcmp(hash,"44136fa355b3678a1146ad16f7e8649e94fb4fc21fe77e8310c060f61caaff8a"));
    {char other[65];CHECK(ui_cap_json_fingerprint("{\"b\":2,\"a\":1}",13,hash)==UI_STATUS_OK);CHECK(ui_cap_json_fingerprint("{\"a\":1.0,\"b\":2}",15,other)==UI_STATUS_OK);CHECK(!strcmp(hash,other));}
#else
    (void)hash;
#endif
    printf("capability JSON: %d checks, %d failures\n",checks,failures);return failures?1:0;
}
