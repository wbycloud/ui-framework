/* Application-owned resources and preferences. Framework never writes this file. */
static const char *app_text(state_app_t *s,const char *en,const char *zh)
{ui_language_info_t info={0};info.size=sizeof(info);(void)ui_host_get_language(s->scene->context.host,&info);return !strcmp(info.language,"zh-CN")?zh:en;}
static ui_status_t prepare_language(state_app_t *s,ui_host_t *host)
{
    if(_snwprintf_s(s->language_path,4096,_TRUNCATE,L"%s.language",s->path)<0)return UI_STATUS_INVALID_ARGUMENT;
    s->language_load=UI_STATUS_NOT_FOUND;return ui_host_set_language(host,"en-US");
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
        text.help=app_text(s,i==0?"Enter submits properties; Esc discards local drafts.":i==1?"Display labels change; semantic values stay low/medium/high.":"Drag previews RGBA; Submit commits once.",i==0?"Enter 提交属性；Esc 放弃本地草稿。":i==1?"标签切换语言；业务值仍为 low/medium/high。":"拖动预览 RGBA；提交只执行一次。");
        if(i==1){text.option_labels=labels;text.option_count=3;}(void)ui_component_set_field_text(s->scene->form,&text);
        if(i==0)(void)ui_component_set_field_text(s->scene->dialog,&text);
    }
    ui_field_text_t pixels={0};pixels.size=sizeof(pixels);pixels.id="pixels";pixels.title=app_text(s,"Actual OSMesa frame","实际 OSMesa 帧");(void)ui_component_set_field_text(s->scene->viewport,&pixels);
    struct caption commands[]={ {"state.save","Save interface state","保存界面状态"},{"state.load","Reload interface state","恢复界面状态"},{"state.reset-column","Reset Name width","重置名称列宽"},{"state.reset","Reset widths and layout","重置列宽与布局"},
        {"language.zh","Simplified Chinese","简体中文"},{"language.en","English","English"},{"state.title","Use draft as tab title","用草稿更新标签标题"},{"state.theme-system","Follow system theme","跟随系统主题"}};
    for(size_t i=0;i<8;++i){const char *text=app_text(s,commands[i].en,commands[i].zh);(void)ui_host_set_title(host,UI_TEXT_COMMAND,commands[i].id,text);(void)ui_host_set_title(host,UI_TEXT_MENU_ITEM,commands[i].id,text);}
    (void)ui_host_set_title(host,UI_TEXT_MENU_ITEM,"state.properties",app_text(s,"Edit properties...","编辑属性..."));
    struct caption tools[]={ {"test.render","Refresh GL frame","刷新 GL 帧"},{"test.edit","Edit cell","编辑单元格"},{"test.select","Select rows","选择行"},{"test.sort","Sort data source","数据源排序"},{"test.submit","Submit properties","提交属性"},{"test.reset","Reset layout","重置布局"},{"test.modal","Open dialog","打开对话框"},{"test.modal.close","Close dialog","关闭对话框"},{"test.image.release","Release image","释放图片"},{"test.late","Test late image","测试迟到图片"}};
    for(size_t i=0;i<10;++i)(void)ui_host_set_title(host,UI_TEXT_COMMAND,tools[i].id,app_text(s,tools[i].en,tools[i].zh));
    (void)ui_host_set_title(host,UI_TEXT_MENU_GROUP,"State",app_text(s,"State","状态"));
    (void)ui_host_set_title(host,UI_TEXT_MENU_GROUP,"Language",app_text(s,"Language","语言"));
    ui_language_info_t language={0};language.size=sizeof(language);(void)ui_host_get_language(host,&language);
    const char *ids[]={"language.zh","language.en"};for(size_t i=0;i<2;++i){ui_command_state_t state={0};state.size=sizeof(state);if(ui_host_get_command_state(host,ids[i],&state)==UI_STATUS_OK){state.checked=!strcmp(language.language,i?"en-US":"zh-CN");(void)ui_host_set_command_state(host,ids[i],&state);}}
}
static void language_changed(ui_host_t *host,const ui_language_info_t *info,void *data)
{(void)host;(void)info;refresh_language(data);}
static ui_status_t register_language(state_app_t *s)
{
    ui_host_t *host=s->scene->context.host;const char *ids[]={"language.zh","language.en"},*titles[]={"Simplified Chinese","English"};ui_status_t status;
    const char *paths[]={"State","Language"};for(size_t i=0;i<2;++i){ui_menu_group_desc_t group={0};group.size=sizeof(group);group.path=group.title=paths[i];group.order=(int)i;group.access_key=i?'L':'S';status=ui_host_register_menu_group(host,&group);if(status!=UI_STATUS_OK)return status;}
    for(size_t i=0;i<2;++i){ui_menu_item_desc_t item={0};
        item.size=sizeof(item);item.id=ids[i];item.title=titles[i];item.menu_path="Language";item.command_id=ids[i];item.order=(int)i;status=ui_host_register_menu_item(host,&item);if(status!=UI_STATUS_OK)return status;status=p2_bind_menu(&s->capabilities,ids[i],ids[i],NULL);if(status)return status;}
    refresh_language(s);return UI_STATUS_OK;
}
