from pathlib import Path
import re
p=Path('examples/framework_host/main.c');s=p.read_text(encoding='utf-8')
s=s.replace('uint64_t popup_token;', 'uint64_t popup_token,language_instance,language_revision;\n    ui_language_info_t popup_language;')
# Framework strings only: neither dynamic app captions nor user/results enter the lookup.
s=re.sub(r'(?<![A-Za-z0-9_])"(?:[^"\\\r\n]|\\.)*"',lambda m:'tr(s,'+m[0]+')' if re.search('[\u4e00-\u9fff]',m[0]) and '\\0' not in m[0] else m[0],s)
at=s.index('\nstatic void refresh_workspace(void *data);')
s=s[:at]+'''
static const char *tr(host_window_t *s,const char *key)
{return ui_language_text(s->chrome_host,key);}
static void language_json(json_buffer_t *json,const char *language,uint64_t revision)
{
    json_append(json,",\\\"language\\\":");json_string(json,language);json_append(json,",\\\"languageRevision\\\":");json_id(json,revision);json_append(json,",\\\"texts\\\":{");
    for(size_t i=0;i<ui_language_catalog_count();++i){const char *key,*value;ui_language_catalog_language(language,i,&key,&value);if(i)json_append(json,",");json_string(json,key);json_append(json,":");json_string(json,value);}json_append(json,"}");
}
''' +s[at:]
# Helpers above json implementation require declarations, not implicit functions.
s=s.replace('static const char *tr(', 'static void json_append(json_buffer_t *,const char *);\nstatic void json_string(json_buffer_t *,const char *);\nstatic void json_id(json_buffer_t *,uint64_t);\nstatic const char *tr(',1)
s=s.replace('if(!s->popup_hwnd)return 0;s->popup_kind=kind;++s->popup_token;', 'if(!s->popup_hwnd)return 0;s->popup_kind=kind;s->popup_language=s->chrome_host->language;++s->popup_token;')
s=s.replace('json_text_chunks(json,detail);json_append(json,",\\\"items\\\":[");', 'json_text_chunks(json,detail);language_json(json,s->popup_language.language,s->popup_token);json_append(json,",\\\"items\\\":[");')
# Active instance generation cannot order A/B chrome snapshots; own revision is monotonic.
key='GetClientRect(s->hwnd,&client);width=MulDiv(client.right,96,(int)s->dpi);schema=refresh_command(s,&target);'
assert key in s
s=s.replace(key,'''ui_language_info_t language={0};language.size=sizeof(language);
    if(active&&get_instance(s,active,&info))(void)ui_host_get_language(info.host,&language);else (void)ui_host_get_system_language(&language);
    if(s->language_instance!=active||strcmp(language.language,s->chrome_host->language.language)||language.source!=s->chrome_host->language.source||language.generation!=s->chrome_host->language.generation){
        if(s->popup_kind&&s->popup_kind!=3)close_popup(s);s->language_instance=active;++s->language_revision;s->chrome_host->language=language;
    }
    '''+key)
key='json_format(&json,",\\\"dpi\\\":%u,\\\"dark\\\":%s,\\\"assistant\\\":%s,\\\"tabs\\\":[",'
assert key in s;s=s.replace(key,'language_json(&json,s->chrome_host->language.language,s->language_revision);\n    '+key)
# Confirmation uses the command's owner at open time, including background targets.
s=s.replace('(void)permission;if(!create_popup(s,3,560,300))return 0;', '(void)permission;if(!create_popup(s,3,560,300))return 0;\n    ui_app_instance_info_t owner;if(get_instance(s,id,&owner))(void)ui_host_get_language(owner.host,&s->popup_language);')
s=s.replace('tr(s,"目标实例 #%llu\\n命令：")','ui_language_lookup(s->popup_language.language,"目标实例 #%llu\\n命令：")').replace('tr(s,"\\n参数：")','ui_language_lookup(s->popup_language.language,"\\n参数：")').replace('tr(s,"允许助手执行此操作？")','ui_language_lookup(s->popup_language.language,"允许助手执行此操作？")')
p.write_text(s,encoding='utf-8')
