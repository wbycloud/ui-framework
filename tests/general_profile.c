/* Stable profile identity across task databases; isolated store only. */
#define GENERAL_ASSISTANT_MAIN general_core_previous_main
#include "general_assistant.c"
#undef GENERAL_ASSISTANT_MAIN
static void profile_service(const char *database,const char *profile,const char *account)
{
    ui_ai_service_config_t q={0};q.size=sizeof(q);q.version=1;q.workspace=w;q.storage_path_utf8=database;
    CHECK(ui_ai_service_create(&q,&s)==UI_STATUS_OK);CHECK(general_open(s)==UI_STATUS_OK);
    ui_ai_service_state_t service_state={0};service_state.size=sizeof(service_state);service_state.version=1;DWORD begin=GetTickCount();
    do{pump();CHECK(ui_ai_service_get_state(s,&service_state)==UI_STATUS_OK);}while(service_state.phase!=UI_AI_READY&&GetTickCount()-begin<5000);
    CHECK(service_state.phase==UI_AI_READY);CHECK(ui_ai_model_open_profile(s,profile,account)==UI_STATUS_OK);CHECK(ready_model());
}
int wmain(int argc,wchar_t **argv)
{
    if(argc!=3)return 2;CHECK(ui_framework_initialize()==UI_STATUS_OK);setup(argv[1],argv[2]);
    char profile[4096],first[128],second[128],database[4096];snprintf(profile,sizeof(profile),"%s.shared-profile.json",dbpath);
    CHECK(ui_ai_model_open_profile(s,NULL,"isolated-account")==UI_STATUS_PERMISSION_DENIED);
    CHECK(ui_ai_model_open_profile(s,profile,"isolated-account")==UI_STATUS_OK);CHECK(ready_model());
    char *raw=NULL;CHECK(ui_ai_model_get_state(s,&raw)==UI_STATUS_OK);str(raw,"credential_ref",first,sizeof(first));ui_ai_release(raw);
    CHECK(strstr(first,"ui-framework/xuanshu/profile/")==first);
    ui_ai_model_profile_t p={0};p.size=sizeof(p);p.version=1;p.profile_id="deepseek";p.protocol_profile="deepseek-responses-v1";
    p.origin="https://api.deepseek.com";p.model_id="deepseek-flash";p.credential_ref=first;p.revision=1;
    CHECK(ui_ai_model_configure(s,&p,"save-shared-model")==UI_STATUS_OK);CHECK(ready_model());CHECK(!s->model_config->credential.readable);
    close_service();snprintf(database,sizeof(database),"%s.second.db",dbpath);profile_service(database,profile,"isolated-account");
    CHECK(ui_ai_model_get_state(s,&raw)==UI_STATUS_OK);str(raw,"credential_ref",second,sizeof(second));CHECK(!strcmp(first,second));
    CHECK(strstr(raw,"deepseek-flash")&&strstr(raw,"\"revision\":1"));CHECK(s->model_config->requests==0&&!s->model_config->credential.readable);ui_ai_release(raw);close_service();
    snprintf(database,sizeof(database),"%s.third.db",dbpath);profile_service(database,profile,"other-isolated-account");
    CHECK(ui_ai_model_get_state(s,&raw)==UI_STATUS_OK);str(raw,"credential_ref",second,sizeof(second));CHECK(strcmp(first,second));CHECK(!s->model_config->ready&&s->model_config->error[0]);CHECK(s->model_config->requests==0);ui_ai_release(raw);close_service();
    CHECK(ui_workspace_close_all(w)==UI_STATUS_OK);DWORD begin=GetTickCount();while(ui_workspace_count(w)&&GetTickCount()-begin<5000)ui_workspace_poll(w);CHECK(ui_workspace_destroy(w)==UI_STATUS_OK);
    printf("General profile checks=%u failures=%u; no credential access\n",checks,failures);return failures?1:0;
}
