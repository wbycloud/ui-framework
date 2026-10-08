/* Application-owned resources and preferences. Framework never writes this file. */
static const char *app_text(state_app_t *s,const char *en,const char *zh)
{ui_language_info_t info={0};info.size=sizeof(info);(void)ui_host_get_language(s->scene->context.host,&info);return !strcmp(info.language,"zh-CN")?zh:en;}
static ui_status_t prepare_language(state_app_t *s,ui_host_t *host)
{
    unsigned char data[16]={0};DWORD bytes=0;LARGE_INTEGER length;const char *language="en-US";
    if(_snwprintf_s(s->language_path,4096,_TRUNCATE,L"%s.language",s->path)<0)return UI_STATUS_INVALID_ARGUMENT;
    HANDLE file=CreateFileW(s->language_path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    s->language_load=UI_STATUS_NOT_FOUND;
    if(file!=INVALID_HANDLE_VALUE){
        s->language_load=UI_STATUS_INVALID_ARGUMENT;
        if(GetFileSizeEx(file,&length)&&length.QuadPart==16&&ReadFile(file,data,16,&bytes,NULL)&&bytes==16&& !memcmp(data,"UIL1",4)){
            if(read32(data+4)!=1)s->language_load=UI_STATUS_UNSUPPORTED;
            else if((data[8]==1||data[8]==2)&&data[9]==s->profile&&!data[10]&&!data[11]&&!read32(data+12)){language=data[8]==1?"zh-CN":"en-US";s->language_load=UI_STATUS_OK;}
        }CloseHandle(file);
    }else if(GetLastError()!=ERROR_FILE_NOT_FOUND)s->language_load=UI_STATUS_PLATFORM_ERROR;
    /* Missing/rejected preference uses this application's documented English default. */
    return ui_host_set_language(host,language);
}
static ui_status_t save_language(state_app_t *s,const char *language)
{
    unsigned char data[16]={0};DWORD bytes=0;wchar_t temporary[4096];ui_status_t status;
    memcpy(data,"UIL1",4);write32(data+4,1);data[8]=!strcmp(language,"zh-CN")?1:2;data[9]=s->profile;
    if(_snwprintf_s(temporary,4096,_TRUNCATE,L"%s.tmp",s->language_path)<0)return UI_STATUS_INVALID_ARGUMENT;
    HANDLE file=CreateFileW(temporary,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);if(file==INVALID_HANDLE_VALUE)return UI_STATUS_PLATFORM_ERROR;
    status=WriteFile(file,data,16,&bytes,NULL)&&bytes==16&&FlushFileBuffers(file)?UI_STATUS_OK:UI_STATUS_PLATFORM_ERROR;CloseHandle(file);
    if(status==UI_STATUS_OK&&!MoveFileExW(temporary,s->language_path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))status=UI_STATUS_PLATFORM_ERROR;return status;
}
static void refresh_language(state_app_t *s)
{
    ui_host_t *host=s->scene->context.host;
    struct caption {const char *id,*en,*zh;} panels[]={ {"tree","Items","项目树"},{"viewport","GL viewport","GL 视口"},{"form","Properties","属性"}};
    for(size_t i=0;i<3;++i)(void)ui_host_set_title(host,UI_TEXT_PANEL,panels[i].id,app_text(s,panels[i].en,panels[i].zh));
    (void)ui_component_set_title(s->scene->tree,app_text(s,"Items and asynchronous images","项目与异步图片"));
    (void)ui_component_set_title(s->scene->table,app_text(s,"Complete 100000 row source","完整十万行数据源"));
    (void)ui_component_set_column_title(s->scene->table,"name",app_text(s,"Name","名称"));
    (void)ui_component_set_column_title(s->scene->table,"value",app_text(s,"Value","数值"));
    (void)ui_component_set_title(s->scene->form,app_text(s,"Properties","属性"));
    (void)ui_component_set_title(s->scene->dialog,app_text(s,"Edit properties","编辑属性"));
    const char *labels[]={app_text(s,"Low","低"),app_text(s,"Medium","中"),app_text(s,"High","高")};
    struct caption fields[]={ {"name","Draft","草稿"},{"mode","Quality","质量"},{"color","RGBA color","RGBA 颜色"}};
    for(size_t i=0;i<3;++i){ui_field_text_t text={0};text.size=sizeof(text);text.id=fields[i].id;text.title=app_text(s,fields[i].en,fields[i].zh);text.group=app_text(s,"Appearance","外观");
        text.help=app_text(s,i==0?"Enter keeps your draft; Esc cancels.":i==1?"Display labels change; semantic values stay low/medium/high.":"Drag previews RGBA; Submit commits once.",i==0?"Enter 保留草稿；Esc 取消。":i==1?"标签切换语言；业务值仍为 low/medium/high。":"拖动预览 RGBA；提交只执行一次。");
        if(i==1){text.option_labels=labels;text.option_count=3;}(void)ui_component_set_field_text(s->scene->form,&text);
        if(i==0)(void)ui_component_set_field_text(s->scene->dialog,&text);
    }
    ui_field_text_t pixels={0};pixels.size=sizeof(pixels);pixels.id="pixels";pixels.title=app_text(s,"Actual OSMesa frame","实际 OSMesa 帧");(void)ui_component_set_field_text(s->scene->viewport,&pixels);
    struct caption commands[]={ {"state.save","Save interface state","保存界面状态"},{"state.load","Reload interface state","恢复界面状态"},{"state.reset-column","Reset Name width","重置名称列宽"},{"state.reset","Reset widths and layout","重置列宽与布局"},
        {"language.zh","Simplified Chinese","简体中文"},{"language.en","English","English"}};
    for(size_t i=0;i<6;++i){const char *text=app_text(s,commands[i].en,commands[i].zh);(void)ui_host_set_title(host,UI_TEXT_COMMAND,commands[i].id,text);(void)ui_host_set_title(host,UI_TEXT_MENU_ITEM,commands[i].id,text);}
    struct caption tools[]={ {"test.render","Refresh GL frame","刷新 GL 帧"},{"test.edit","Edit cell","编辑单元格"},{"test.select","Select rows","选择行"},{"test.sort","Sort data source","数据源排序"},{"test.submit","Submit properties","提交属性"},{"test.reset","Reset layout","重置布局"},{"test.modal","Open dialog","打开对话框"},{"test.modal.close","Close dialog","关闭对话框"},{"test.image.release","Release image","释放图片"},{"test.late","Test late image","测试迟到图片"}};
    for(size_t i=0;i<10;++i)(void)ui_host_set_title(host,UI_TEXT_COMMAND,tools[i].id,app_text(s,tools[i].en,tools[i].zh));
    (void)ui_host_set_title(host,UI_TEXT_MENU_GROUP,"State",app_text(s,"State","状态"));
    (void)ui_host_set_title(host,UI_TEXT_MENU_GROUP,"Language",app_text(s,"Language","语言"));
    ui_language_info_t language={0};language.size=sizeof(language);(void)ui_host_get_language(host,&language);
    const char *ids[]={"language.zh","language.en"};for(size_t i=0;i<2;++i){ui_command_state_t state={0};state.size=sizeof(state);if(ui_host_get_command_state(host,ids[i],&state)==UI_STATUS_OK){state.checked=!strcmp(language.language,i?"en-US":"zh-CN");(void)ui_host_set_command_state(host,ids[i],&state);}}
}
static void language_changed(ui_host_t *host,const ui_language_info_t *info,void *data)
{(void)host;(void)info;refresh_language(data);}
static void language_command(ui_host_t *host,uint64_t request,const char *id,const char *params,const char *origin,void *data)
{
    state_app_t *s=data;const char *language=!strcmp(id,"language.zh")?"zh-CN":"en-US";ui_status_t status;(void)params;(void)origin;
    status=ui_host_set_language(host,language);if(status==UI_STATUS_OK)status=save_language(s,language);
    if(status!=UI_STATUS_OK)(void)ui_component_set_text(s->scene->status,app_text(s,"Language preference could not be saved; runtime language is retained.","语言偏好保存失败；保留当前运行语言。"));
    (void)ui_host_reply(host,request,status==UI_STATUS_OK,"{}");
}
static ui_status_t register_language(state_app_t *s)
{
    ui_host_t *host=s->scene->context.host;const char *ids[]={"language.zh","language.en"},*titles[]={"Simplified Chinese","English"};ui_status_t status;
    const char *paths[]={"State","Language"};for(size_t i=0;i<2;++i){ui_menu_group_desc_t group={0};group.size=sizeof(group);group.path=group.title=paths[i];group.order=(int)i;group.access_key=i?'L':'S';status=ui_host_register_menu_group(host,&group);if(status!=UI_STATUS_OK)return status;}
    for(size_t i=0;i<2;++i){ui_command_desc_t command={0};ui_menu_item_desc_t item={0};command.size=sizeof(command);command.id=ids[i];command.title=titles[i];command.handler=language_command;command.user_data=s;status=ui_host_register_command(host,&command);if(status!=UI_STATUS_OK)return status;
        item.size=sizeof(item);item.id=ids[i];item.title=titles[i];item.menu_path="Language";item.command_id=ids[i];item.order=(int)i;status=ui_host_register_menu_item(host,&item);if(status!=UI_STATUS_OK)return status;}
    refresh_language(s);return UI_STATUS_OK;
}
