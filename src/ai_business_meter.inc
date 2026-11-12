/* BR-2 pure, pinned local metering. No credentials or transport in this process. */
#include "br2_meter_config.h"
#define BM_PROFILE "deepseek-v41-responses-1017-v1"

static int bm_profile_matches(const char *profile,const char *evidence)
{return profile&&evidence&&!strcmp(profile,BM_PROFILE)&&!_stricmp(evidence,BR2_METER_EVIDENCE);}

static int bm_component_path(wchar_t path[4096])
{
 HMODULE module=NULL;
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)BM_PROFILE,&module))return 0;
 DWORD length=GetModuleFileNameW(module,path,4096);
 if(!length||length>=4096)return 0;
 wchar_t *separator=wcsrchr(path,L'\\');if(!separator)return 0;separator[1]=0;
#ifdef UI_AI_FIXTURE
 return wcscat_s(path,4096,L"xuanshu_br2_meter_fixture.exe")==0;
#else
 return wcscat_s(path,4096,L"xuanshu_br2_meter.exe")==0;
#endif
}

static int bm_receipt(const av_policy_t *policy,yyjson_val *receipt,const char *hash,uint64_t *tokens)
{
 yyjson_val *input=yyjson_obj_get(receipt,"input_tokens"),*output=yyjson_obj_get(receipt,"output_limit");
 const char *profile=yyjson_get_str(yyjson_obj_get(receipt,"profile"));
 const char *evidence=yyjson_get_str(yyjson_obj_get(receipt,"evidence"));
 const char *body=yyjson_get_str(yyjson_obj_get(receipt,"body_hash"));
 if(!bm_profile_matches(profile,evidence)||strcmp(policy->meter_profile,profile)||_stricmp(policy->meter_evidence,evidence)||!body||_stricmp(body,hash)||!yyjson_is_uint(input)||!yyjson_get_uint(input)||yyjson_get_uint(input)>policy->input_ceiling||!yyjson_is_uint(output)||yyjson_get_uint(output)!=4096)return 0;
 *tokens=yyjson_get_uint(input);return 1;
}

static int bm_measure_bound(ui_ai_service_t *s,ai_run_t *run,av_policy_t *policy,uint64_t *tokens)
{
 const char *error="business_meter_unavailable";DWORD system_error=0,exit_code=0;
 HANDLE binary=INVALID_HANDLE_VALUE,mapping=NULL,input_read=NULL,input_write=NULL,output_read=NULL,output_write=NULL;
 PROCESS_INFORMATION process={0};STARTUPINFOEXW startup={0};LPPROC_THREAD_ATTRIBUTE_LIST attributes=NULL;
 wchar_t path[4096],command[4100],environment[2]={0,0};char receipt[1024]={0};SIZE_T attribute_bytes=0;
 const char *body=run->model->body;size_t body_length=body?strlen(body):0,received=0;int overflow=0,ok=0;
 if(!body_length||body_length>AM_REQUEST)goto done;
#ifdef UI_AI_FIXTURE
 const char *approved=BR2_METER_FIXTURE_SHA256;
#else
 const char *approved=BR2_METER_SHA256;
#endif
 if(strlen(approved)!=64||!bm_component_path(path))goto done;
#ifdef UI_AI_FIXTURE
 if(s->model_config->fixture_meter_fault==1)wcscat_s(path,4096,L".fixture-missing");
#endif
 /* Keep the no-write/no-delete file handle through child exit. */
 binary=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
 if(binary==INVALID_HANDLE_VALUE){system_error=GetLastError();goto done;}
 LARGE_INTEGER size;
 if(!GetFileSizeEx(binary,&size)||size.QuadPart<=0||size.QuadPart>64*1024*1024)goto done;
 mapping=CreateFileMappingW(binary,NULL,PAGE_READONLY,0,0,NULL);
 if(!mapping){system_error=GetLastError();goto done;}
 const void *bytes=MapViewOfFile(mapping,FILE_MAP_READ,0,0,0);
 if(!bytes){system_error=GetLastError();goto done;}
 char actual[65];ui_cap_hash_bytes(bytes,(size_t)size.QuadPart,actual);UnmapViewOfFile(bytes);
#ifdef UI_AI_FIXTURE
 if(s->model_config->fixture_meter_fault==3)actual[0]=actual[0]=='0'?'1':'0';
#endif
 error="business_meter_identity";
 if(_stricmp(actual,approved))goto done;
 SECURITY_ATTRIBUTES security={sizeof(security),NULL,TRUE};
 error="business_meter_pipe";
 if(!CreatePipe(&input_read,&input_write,&security,0)||!CreatePipe(&output_read,&output_write,&security,0)||!SetHandleInformation(input_write,HANDLE_FLAG_INHERIT,0)||!SetHandleInformation(output_read,HANDLE_FLAG_INHERIT,0)){system_error=GetLastError();goto done;}
 InitializeProcThreadAttributeList(NULL,1,0,&attribute_bytes);
 attributes=HeapAlloc(GetProcessHeap(),0,attribute_bytes);
 if(!attributes)goto done;
 if(!InitializeProcThreadAttributeList(attributes,1,0,&attribute_bytes)){system_error=GetLastError();HeapFree(GetProcessHeap(),0,attributes);attributes=NULL;goto done;}
 HANDLE handles[2]={input_read,output_write};
 if(!UpdateProcThreadAttribute(attributes,0,PROC_THREAD_ATTRIBUTE_HANDLE_LIST,handles,sizeof(handles),NULL,NULL)){system_error=GetLastError();goto done;}
 startup.StartupInfo.cb=sizeof(startup);startup.StartupInfo.dwFlags=STARTF_USESTDHANDLES;
 startup.StartupInfo.hStdInput=input_read;startup.StartupInfo.hStdOutput=output_write;startup.StartupInfo.hStdError=output_write;startup.lpAttributeList=attributes;
 _snwprintf_s(command,4100,_TRUNCATE,L"\"%s\"",path);
 error="business_meter_start";
 if(!CreateProcessW(path,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_UNICODE_ENVIRONMENT|EXTENDED_STARTUPINFO_PRESENT,environment,NULL,&startup.StartupInfo,&process)){system_error=GetLastError();goto done;}
 CloseHandle(input_read);input_read=NULL;CloseHandle(output_write);output_write=NULL;
 CloseHandle(process.hThread);process.hThread=NULL;
#ifdef UI_AI_FIXTURE
 if(s->model_config->fixture_meter_enter){
  SetEvent(s->model_config->fixture_meter_enter);
  if(WaitForSingleObject(s->model_config->fixture_meter_release,5000)!=WAIT_OBJECT_0){error="business_meter_fixture_barrier";body_length=0;}
 }
#endif
 error="business_meter_io";
#ifdef UI_AI_FIXTURE
 if(s->model_config->fixture_meter_fault==4){body="{";body_length=1;}
#endif
 size_t sent=0;int read_ok=1;while(sent<body_length){DWORD wrote=0;if(!WriteFile(input_write,body+sent,(DWORD)(body_length-sent),&wrote,NULL)||!wrote){system_error=GetLastError();break;}sent+=wrote;}
 CloseHandle(input_write);input_write=NULL;
 for(;;){char block[256];DWORD count=0;if(!ReadFile(output_read,block,sizeof(block),&count,NULL)){DWORD code=GetLastError();if(code!=ERROR_BROKEN_PIPE){read_ok=0;system_error=code;}break;}if(!count)break;if(count>=sizeof(receipt)-received)overflow=1;if(!overflow){memcpy(receipt+received,block,count);received+=count;}}
 /* Never consume a result while callbacks/child execution can still be live. */
 if(WaitForSingleObject(process.hProcess,INFINITE)!=WAIT_OBJECT_0||!GetExitCodeProcess(process.hProcess,&exit_code)){system_error=GetLastError();goto done;}
 if(sent!=body_length||!body_length||exit_code||overflow||!received||!read_ok)goto done;
 receipt[received]=0;
 ui_cap_json_doc_t parsed={0};int parsed_ok=am_json_parse(receipt,received,&parsed);
 yyjson_val *root=parsed_ok?yyjson_doc_get_root(parsed.doc):NULL;
 char hash[65];ui_cap_hash_bytes(body,strlen(body),hash);
 error="business_meter_receipt";
#ifdef UI_AI_FIXTURE
 if(s->model_config->fixture_meter_fault==2)hash[0]=hash[0]=='0'?'1':'0';
#endif
 ok=parsed_ok&&yyjson_get_bool(yyjson_obj_get(root,"ok"))&&bm_receipt(policy,root,hash,tokens);
 yyjson_val *peak=yyjson_obj_get(root,"peak_commit_bytes");
 ok=ok&&yyjson_is_uint(peak)&&yyjson_get_uint(peak)>0&&yyjson_get_uint(peak)<=128u*1024u*1024u;
 am_json_free(&parsed);
done:
 int started=process.hProcess!=NULL;
 if(input_write)CloseHandle(input_write);if(input_read)CloseHandle(input_read);
 if(output_read)CloseHandle(output_read);if(output_write)CloseHandle(output_write);
 if(process.hProcess){/* All paths after launch wait for real exit, without force. */WaitForSingleObject(process.hProcess,INFINITE);GetExitCodeProcess(process.hProcess,&exit_code);CloseHandle(process.hProcess);}
 if(process.hThread)CloseHandle(process.hThread);
 if(attributes){DeleteProcThreadAttributeList(attributes);HeapFree(GetProcessHeap(),0,attributes);}
 if(mapping)CloseHandle(mapping);if(binary!=INVALID_HANDLE_VALUE)CloseHandle(binary);
 EnterCriticalSection(&s->lock);policy->meter_started=started;policy->meter_exit=exit_code;policy->meter_system_error=system_error;
 if(ok)policy->meter_last=*tokens;else{policy->stopped=1;if(!policy->error[0])strcpy_s(policy->error,96,error);if(policy==s->validation)av_stop_locked(s,error);}
 LeaveCriticalSection(&s->lock);return ok;
}

static int bm_measure(ui_ai_service_t *s,ai_run_t *run,uint64_t *tokens)
{return bm_measure_bound(s,run,s->validation,tokens);}
