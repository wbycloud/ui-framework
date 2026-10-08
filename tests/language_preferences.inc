/* Actual complete DLL startup, independent of the desktop input station. */
static void preference_cases(host_window_t *h,const wchar_t *package,const wchar_t *home)
{
    const char *names[]={"missing","empty","truncated","magic","future","language","profile","reserved","zh-CN","en-US"};
    wchar_t path[4096];unsigned char saved[16],readback[17];DWORD bytes;LARGE_INTEGER length;
    CHECK(_snwprintf_s(path,4096,_TRUNCATE,L"%s\\profile-A.ust.language",home)>0);
    for(unsigned test=0;test<10;++test){
        size_t count=test==1?0:test==2?7:16;memset(saved,0,sizeof(saved));memcpy(saved,"UIL1",4);saved[4]=1;saved[8]=test==9?2:1;saved[9]='A';
        if(test==3)saved[0]='X';if(test==4)saved[4]=2;if(test==5)saved[8]=3;if(test==6)saved[9]='B';if(test==7)saved[15]=1;
        if(test==0){if(!DeleteFileW(path))CHECK(GetLastError()==ERROR_FILE_NOT_FOUND);}
        else{HANDLE file=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(file!=INVALID_HANDLE_VALUE);if(file!=INVALID_HANDLE_VALUE){CHECK(WriteFile(file,saved,(DWORD)count,&bytes,NULL)&&bytes==count);CloseHandle(file);}}
        open_path(h,package);pump(200);ui_app_instance_info_t info={0};CHECK(get_instance(h,ui_workspace_active(h->workspace),&info));if(!info.host)break;
        const char *expected=test==8?"zh-CN":"en-US";language_is(info.host,expected);text_is(h,ui_component_find(info.host,"table"),ui_component_find(info.host,"form"),test==8);
        int width=0;CHECK(ui_component_get_column_width(ui_component_find(info.host,"table"),"name",&width)==UI_STATUS_OK&&width==200);
        if(test>=1&&test<=7)CHECK(strstr(ui_host_status_text(info.host),"State rejected:")!=NULL);
        HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
        if(test==0)CHECK(file==INVALID_HANDLE_VALUE&&GetLastError()==ERROR_FILE_NOT_FOUND);
        else{CHECK(file!=INVALID_HANDLE_VALUE);if(file!=INVALID_HANDLE_VALUE){CHECK(GetFileSizeEx(file,&length)&&length.QuadPart==(LONGLONG)count);CHECK(ReadFile(file,readback,17,&bytes,NULL)&&bytes==count&&!memcmp(readback,saved,count));CloseHandle(file);}}
        if(test==5)screenshot(h->hwnd,home,L"rejected-preference");if(test==8)screenshot(h->hwnd,home,L"restored-chinese");
        CHECK(ui_workspace_close(h->workspace,info.instance_id,UI_APP_CLOSE_TAB)==UI_STATUS_OK);ULONGLONG end=GetTickCount64()+10000;while(ui_workspace_count(h->workspace)&&GetTickCount64()<end)pump(1);CHECK(!ui_workspace_count(h->workspace));pump(100);CHECK(GetModuleHandleW(L"stateful_components_app.dll")==NULL);
        printf("PREFERENCE %s: final=%s file unchanged\n",names[test],expected);
    }
}
