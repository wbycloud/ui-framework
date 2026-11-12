/* Minimal public integration client. The application owns its actual business
 * model/capabilities/Profile. This client creates no model configuration. */
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include "ui_framework/ai_assistant.h"

int wmain(int argc,wchar_t **argv)
{
    if(argc!=3){fprintf(stderr,"usage: catalog app.uapp new-database-path\n");return 2;}
    if(GetFileAttributesW(argv[2])!=INVALID_FILE_ATTRIBUTES){fprintf(stderr,"new database required\n");return 2;}
    char package[4096],database[4096];
    if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[1],-1,package,sizeof(package),NULL,NULL)||
       !WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,argv[2],-1,database,sizeof(database),NULL,NULL))return 2;
    ui_status_t status=ui_framework_initialize();if(status)return 1;
    ui_workspace_config_t wc={0};wc.size=sizeof(wc);wc.run_mode=UI_RUN_OFFSCREEN;wc.max_permission=UI_ASSISTANT_PERMISSION_EDIT;
    ui_workspace_t *workspace=ui_workspace_create(&wc);if(!workspace)return 1;
    uint64_t instance=0;status=ui_workspace_open(workspace,package,&instance);
    ui_ai_service_t *service=NULL;
    ui_ai_service_config_t config={0};config.size=sizeof(config);config.version=1;config.workspace=workspace;config.storage_path_utf8=database;
    if(!status)status=ui_ai_service_create(&config,&service);
    ui_ai_assistant_policy_t p={0};p.size=sizeof(p);p.version=1;p.request_limit=8;p.deadline_ms=600000;p.budget_nano_usd=100000000;
    p.price_version=p.price_evidence_sha256=p.meter_profile=p.meter_evidence_sha256="";
    if(!status)status=ui_ai_assistant_enable(service,&p);
    if(!status)status=ui_ai_service_open(service);
    ui_ai_service_state_t state={0};state.size=sizeof(state);state.version=1;DWORD begin=GetTickCount();
    while(!status&&GetTickCount()-begin<5000){ui_ai_service_poll(service,64);ui_workspace_poll(workspace);ui_ai_service_get_state(service,&state);if(state.phase==UI_AI_READY||state.phase==UI_AI_UNAVAILABLE)break;MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}
    if(!status&&state.phase!=UI_AI_READY)status=UI_STATUS_PLATFORM_ERROR;
    char *catalog=NULL;if(!status)status=ui_ai_assistant_catalog(service,instance,&catalog);
    if(catalog){printf("%s\n",catalog);ui_ai_release(catalog);}
    if(service){ui_ai_service_request_close(service);begin=GetTickCount();while(GetTickCount()-begin<5000){ui_ai_service_poll(service,64);ui_workspace_poll(workspace);if(!ui_ai_service_destroy(service)){service=NULL;break;}MsgWaitForMultipleObjects(0,NULL,FALSE,1,QS_ALLINPUT);}if(service)return 1;}
    ui_workspace_close_all(workspace);begin=GetTickCount();while(ui_workspace_count(workspace)&&GetTickCount()-begin<5000)ui_workspace_poll(workspace);
    if(ui_workspace_destroy(workspace))return 1;
    return status?1:0;
}
