/* Isolated rendering-only interruption: never authorizes or sends. */
#include <stdlib.h>
#include <string.h>
#include "ui_framework/light_web.h"
static char *choices_replace(char *text,const char *needle,const char *insert)
{char *at=text?strstr(text,needle):NULL;if(!at){free(text);return NULL;}size_t n=(size_t)(at-text);char *out=malloc(strlen(text)+strlen(insert)+1);if(out){memcpy(out,text,n);strcpy(out+n,insert);strcpy(out+n+strlen(insert),at+strlen(needle));}free(text);return out;}
static ui_status_t choices_load(ui_web_view_t *view,const char *html)
{
 char *copy=_strdup(html);
 copy=choices_replace(copy,"clear(caps);","clear(caps);if(choicesPoint==='clear'){choicesPoint='';throw Error('isolated choice clear');}");
 copy=choices_replace(copy,"root.appendChild(b);return b;","root.appendChild(b);if(choicesPoint==='append'&&root===element('ai-model-projection')){choicesPoint='';throw Error('isolated choice append');}return b;");
 copy=choices_replace(copy,"ui.onmessage=function(state) {","var choicesPoint='';ui.onmessage=function(state) {if(state.type==='body-choice-test'){choicesPoint=state.point||'';realResource='browser.view';realInstance='1';realCaps={'browser.body':true};realFields={body:true,revision:true};try{paintChoices(state.ai)}catch(e){}return;}");
 if(!copy)return UI_STATUS_VALIDATION_FAILED;ui_status_t status=ui_web_view_load_html(view,copy);free(copy);return status;
}
#define ui_web_view_load_html choices_load
#define UHA_MAIN choices_previous_main
#include "uha_host.c"
#undef ui_web_view_load_html
static void project_choices(host_window_t *s,const char *point,const char *name)
{char state[900];snprintf(state,sizeof(state),"{\"type\":\"body-choice-test\",\"point\":\"%s\",\"ai\":{\"messages\":{\"items\":[]},\"choices\":{\"instance\":\"1\",\"resources\":[\"browser.view\"],\"capabilities\":[{\"id\":\"browser.body\",\"name\":\"%s\",\"fields\":[\"body\",\"revision\"]}]}}}",point,name);CHECK(ui_web_view_post_json(s->view,state)==UI_STATUS_OK);}
int wmain(int argc,wchar_t **argv)
{
 if(argc!=3)return 2;host_window_t s={0};host_action_t action={0};wchar_t db[4096],id[40];GUID guid;CHECK(CreateDirectoryW(argv[1],NULL)||GetLastError()==ERROR_ALREADY_EXISTS);CoCreateGuid(&guid);StringFromGUID2(&guid,id,40);_snwprintf_s(db,4096,_TRUNCATE,L"%s/choices-%s.db",argv[1],id);CHECK(SetEnvironmentVariableW(L"UI_AI_DB_PATH",db));CHECK(ui_framework_initialize()==UI_STATUS_OK);CHECK(create_host(&s,GetModuleHandleW(NULL))!=NULL);if(!s.ai)return 1;CHECK(model_wait(&s,NULL));ShowWindow(s.hwnd,SW_HIDE);action.action="assistant";process_action(&s,&action);pump(&s);
 project_choices(&s,"","A");const char *point=!wcscmp(argv[2],L"clear")?"clear":"append";project_choices(&s,point,"B");project_choices(&s,"","B");
 const char *nodes[]={"ai-resource-select-0","ai-cap-select-0","ai-field-select-0-0","ai-field-select-0-1"};const char *texts[]={"✓ browser.view","✓ browser.body · B","✓ 字段 body","✓ 字段 revision"};for(unsigned i=0;i<4;++i){ui_element_presentation_t p={0};p.size=sizeof(p);CHECK(ui_web_view_get_presentation(s.view,nodes[i],&p)==UI_STATUS_OK);CHECK(!strcmp(p.text_utf8,texts[i]));}
 CHECK(!s.ai_run[0]&&!s.ai_preview_ready&&!s.ai_model_bindings[1]&&s.ai->model_config->requests==0);CHECK(close_host(&s));SetEnvironmentVariableW(L"UI_AI_DB_PATH",NULL);printf("PB-1 choices checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
