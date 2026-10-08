from pathlib import Path
p=Path('src/platform/win32_shell.c');s=p.read_text(encoding='utf-8')
s=s.replace('#include "visual_style.h"','#include "visual_style.h"\n#include "../language_json.h"')
s=s.replace('get_or_create_menu_path(HMENU root, const char *path)','get_or_create_menu_path(ui_host_t *host, HMENU root, const char *path)')
s=s.replace('title = utf8_to_wide(cursor);','title = utf8_to_wide(ui_menus_group_title(host,copy));')
s=s.replace('cursor = separator + 1;',"*separator='/';cursor = separator + 1;")
s=s.replace('refresh_menu(ui_native_shell_t *shell)','refresh_menu(ui_native_shell_t *shell,int bind)')
s=s.replace('get_or_create_menu_path(menu, entries[index]->menu_path)','get_or_create_menu_path(shell->host,menu, entries[index]->menu_path)')
s=s.replace('native_id = UI_NATIVE_FIRST_COMMAND + (UINT)shell->binding_count;', 'native_id = UI_NATIVE_FIRST_COMMAND + (UINT)(bind?shell->binding_count:index);')
s=s.replace('!add_binding(shell, native_id, entries[index]->command_id)', '(bind&&!add_binding(shell, native_id, entries[index]->command_id))')
s=s.replace('refresh_menu(shell);','refresh_menu(shell,1);').replace('refresh_menu(native);','refresh_menu(native,1);')
s=s.replace("<button style='width:46px", "<button id='panel-dock' style='width:60px")
s=s.replace("ui.onmessage=function(s){document.body.className=s.dark?'dark':''}", "ui.onmessage=function(s){document.body.className=s.dark?'dark':'';if(s.title!==undefined)title.textContent=s.title;if(s.texts)document.getElementById('panel-dock').textContent=s.texts.Dock}")
old='for(i=0;i<shell->panel_count;++i)if(shell->panels[i].popup_view)\n        (void)ui_web_view_post_json(shell->panels[i].popup_view,host->menu_dark?"{\\\"dark\\\":true}":"{\\\"dark\\\":false}");'
new=r'''for(i=0;i<shell->panel_count;++i)if(shell->panels[i].popup_view){
        ui_json_t json={0};ui_panel_entry_t *entry=ui_find_panel(host,shell->panels[i].id);
        uj_add(&json,host->menu_dark?"{\"dark\":true,\"title\":" : "{\"dark\":false,\"title\":");uj_string(&json,entry?entry->title:"");ui_language_json(host,&json);uj_add(&json,"}");
        if(!json.failed)(void)ui_web_view_post_json(shell->panels[i].popup_view,json.text);free(json.text);
    }'''
assert old in s;s=s.replace(old,new)
at=s.index('\nstatic ui_status_t web_set_floating')
new=r'''
void ui_shell_refresh_titles(ui_host_t *host)
{
    ui_native_shell_t *shell=host->shell?host->shell->native:NULL;if(!shell)return;
    if(!shell->offscreen&&!shell->web_chrome)(void)refresh_menu(shell,0);
    for(size_t i=0;i<shell->panel_count;++i){ui_native_panel_t *p=&shell->panels[i];ui_panel_entry_t *entry=ui_find_panel(host,p->id);if(!entry)continue;
        wchar_t *title=utf8_to_wide(entry->title);if(!title)continue;
        if(p->hwnd&&!shell->web_chrome)SetWindowTextW(p->hwnd,title);if(p->tag)SetWindowTextW(p->tag,title);if(p->popup)SetWindowTextW(p->popup,title);free(title);
    }
    if(shell->toolbar){ui_toolbar_entry_t **bars=NULL;size_t count=0,capacity=0;int button_index=0;
        for(ui_toolbar_entry_t *bar=host->toolbars;bar;bar=bar->next)if(bar->visible){if(!ensure_capacity((void **)&bars,&capacity,count,sizeof(*bars))){free(bars);return;}bars[count++]=bar;}
        qsort(bars,count,sizeof(*bars),toolbar_compare);
        for(size_t i=0;i<count;++i){ui_toolbar_item_entry_t **items=NULL;size_t n=0;if(!collect_toolbar_items(shell,bars[i]->id,&items,&n))continue;
            if(i)++button_index;
            for(size_t j=0;j<n;++j){TBBUTTON button;TBBUTTONINFOW info={0};wchar_t *title=utf8_to_wide(items[j]->title);
                if(title&&SendMessageW(shell->toolbar,TB_GETBUTTON,button_index,(LPARAM)&button)){info.cbSize=sizeof(info);info.dwMask=TBIF_TEXT;info.pszText=title;(void)SendMessageW(shell->toolbar,TB_SETBUTTONINFOW,button.idCommand,(LPARAM)&info);}free(title);++button_index;}
            free(items);
        }free(bars);
    }
    ui_shell_sync_visual(host);
}
'''
s=s[:at]+new+s[at:];p.write_text(s,encoding='utf-8')
p=Path('src/components.html');s=p.read_text(encoding='utf-8').replace("(f.value.text||t('选择'))+' ▾'","(f.kind===4?(f.optionLabels[f.options.indexOf(f.value.text)]||f.value.text||t('选择')):(f.value.text||t('选择')))+' ▾'");p.write_text(s,encoding='utf-8')
p=Path('src/components.c');s=p.read_text(encoding='utf-8').replace('if(uj_u64(json,"generation")!=c->generation)return;','if(uj_u64(json,"generation")!=c->generation||uj_u64(json,"languageGeneration")!=c->host->language.generation)return;',1).replace('tip=c->fields[i].options[option];','tip=c->option_labels[i]?c->option_labels[i][option]:c->fields[i].options[option];');p.write_text(s,encoding='utf-8')
