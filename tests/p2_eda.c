#include "ui_internal.h"
#include "../examples/minimal_eda/app.c"
#include "p2_example_logic.h"
static void route_eda(eda_state_t *e,ui_cap_input_kind_t kind,void *owner,ui_input_event_t *event,int admit)
{if(admit)ui_capabilities_human_begin(e->context.host,e);CHECK(ui_inputs_route(e->context.host,kind,owner,"",event,e));if(admit)ui_capabilities_human_end(e->context.host);}
static void adapters(eda_state_t *e)
{
 ui_surface_desc_t canvas={0};ui_input_event_t event={0};uint64_t revision,id;char text[256];
 e->context.host->native_parent=e->properties;canvas.size=sizeof(canvas);canvas.id="eda.canvas";canvas.kind=UI_SURFACE_NATIVE;canvas.visible=1;canvas.rect=(ui_rect_t){0,0,400,300};e->canvas=ui_surface_create(e->context.host,&canvas);CHECK(e->canvas!=NULL);if(!e->canvas)return;
 e->submit=CreateWindowExW(0,L"BUTTON",L"Submit",WS_CHILD|WS_VISIBLE,0,0,70,25,e->properties,(HMENU)(uintptr_t)3,module_instance,NULL);
 e->cancel=CreateWindowExW(0,L"BUTTON",L"Cancel",WS_CHILD|WS_VISIBLE,70,0,70,25,e->properties,(HMENU)(uintptr_t)4,module_instance,NULL);
 e->keep=CreateWindowExW(0,L"BUTTON",L"Keep",WS_CHILD|WS_VISIBLE,140,0,70,25,e->properties,(HMENU)(uintptr_t)5,module_instance,NULL);
 CHECK(e->submit&&e->cancel&&e->keep);ShowWindow(e->properties,SW_SHOWNOACTIVATE);CHECK(register_ui(e)==UI_STATUS_OK);CHECK(eda_bindings(e)==UI_STATUS_OK);
 /* Actual existing menu/toolbar binding and fixed-geometry business output. */
 revision=e->revision;ui_capabilities_human_begin(e->context.host,e);CHECK(ui_capabilities_ui_action(e->context.host,"eda.clear",e)==UI_STATUS_OK);ui_capabilities_human_end(e->context.host);CHECK(e->block_count==0&&e->zoom==1&&e->revision==revision+1);p2_poll(&e->context,&e->capabilities);
 event.size=sizeof(event);event.kind=UI_INPUT_KEY_DOWN;event.key_code='A';revision=e->revision;route_eda(e,UI_CAP_INPUT_SURFACE,e->canvas,&event,0);CHECK(e->revision==revision&&e->block_count==0);
 route_eda(e,UI_CAP_INPUT_SURFACE,e->canvas,&event,1);CHECK(e->block_count==1&&e->revision==revision+1&&e->blocks[0].width==.22f);id=e->blocks[0].id;p2_poll(&e->context,&e->capabilities);
 event.key_code='Z';revision=e->revision;route_eda(e,UI_CAP_INPUT_SURFACE,e->canvas,&event,1);CHECK(e->zoom>1&&e->revision==revision);p2_poll(&e->context,&e->capabilities);
 event.kind=UI_INPUT_POINTER_UP;event.pointer_button=1;event.key_code=0;
 event.x=(int)(((e->blocks[0].x+e->blocks[0].width/2)*e->zoom/(400.0/300.0)+1)*200);event.y=(int)((1-(e->blocks[0].y+e->blocks[0].height/2)*e->zoom)*150);
 route_eda(e,UI_CAP_INPUT_SURFACE,e->canvas,&event,1);CHECK(e->selected==id&&e->revision==revision);p2_poll(&e->context,&e->capabilities);
 ui_capabilities_human_begin(e->context.host,e);CHECK(ui_inputs_route(e->context.host,UI_CAP_INPUT_TOOLBAR,NULL,"eda.add_block",&event,e));ui_capabilities_human_end(e->context.host);CHECK(e->block_count==2&&e->blocks[1].id>id&&e->revision==revision+1);p2_poll(&e->context,&e->capabilities);
 SendMessageW(e->note,EM_SETSEL,0,-1);SendMessageW(e->note,EM_REPLACESEL,TRUE,(LPARAM)L"actual native collector note");revision=e->revision;CHECK(strcmp(e->committed_note,"actual native collector note")&&e->note_dirty);
 route_eda(e,UI_CAP_INPUT_NATIVE_BUTTON,e->submit,&event,0);CHECK(e->revision==revision&&e->note_dirty);
 EnableWindow(e->submit,FALSE);route_eda(e,UI_CAP_INPUT_NATIVE_BUTTON,e->submit,&event,1);CHECK(e->revision==revision&&e->note_dirty);EnableWindow(e->submit,TRUE);
 route_eda(e,UI_CAP_INPUT_NATIVE_BUTTON,e->submit,&event,1);CHECK(!strcmp(e->committed_note,"actual native collector note")&&e->revision==revision+1&&!e->note_dirty);p2_poll(&e->context,&e->capabilities);
 SendMessageW(e->note,EM_SETSEL,0,-1);SendMessageW(e->note,EM_REPLACESEL,TRUE,(LPARAM)L"preserve conflicted native note");event.kind=UI_INPUT_KEY_DOWN;event.key_code='A';event.pointer_button=0;route_eda(e,UI_CAP_INPUT_SURFACE,e->canvas,&event,1);CHECK(e->note_conflict);p2_poll(&e->context,&e->capabilities);revision=e->revision;
 event.kind=UI_INPUT_POINTER_UP;event.key_code=0;event.pointer_button=1;route_eda(e,UI_CAP_INPUT_NATIVE_BUTTON,e->submit,&event,1);CHECK(e->revision==revision&&e->note_dirty&&!strcmp(e->committed_note,"actual native collector note"));p2_poll(&e->context,&e->capabilities);
 route_eda(e,UI_CAP_INPUT_NATIVE_BUTTON,e->keep,&event,1);CHECK(!e->note_conflict&&e->note_dirty&&e->note_base==revision);p2_poll(&e->context,&e->capabilities);
 route_eda(e,UI_CAP_INPUT_NATIVE_BUTTON,e->submit,&event,1);CHECK(!strcmp(e->committed_note,"preserve conflicted native note")&&e->revision==revision+1);p2_poll(&e->context,&e->capabilities);
 SendMessageW(e->note,EM_SETSEL,0,-1);SendMessageW(e->note,EM_REPLACESEL,TRUE,(LPARAM)L"discard via actual button");revision=e->revision;route_eda(e,UI_CAP_INPUT_NATIVE_BUTTON,e->cancel,&event,1);CHECK(!e->note_dirty&&e->revision==revision&&!strcmp(e->committed_note,"preserve conflicted native note"));p2_poll(&e->context,&e->capabilities);
 snprintf(text,sizeof(text),"SIM EDA actual surface/button collectors: object=%llu note=%s revision=%llu\n",(unsigned long long)id,e->committed_note,(unsigned long long)e->revision);fputs(text,stdout);
}
int main(void)
{
 ui_host_config_t config={0};eda_state_t e={0};char params[512];uint64_t first,revision;
 config.size=sizeof(config);config.api_version=10;e.context.size=sizeof(e.context);e.context.host=ui_host_create(&config);CHECK(e.context.host!=NULL);if(!e.context.host)return 1;e.zoom=1;e.committed_note=_strdup("initial committed note");CHECK(eda_capabilities(&e,&e.context)==UI_STATUS_OK);
 CHECK(register_properties_class()==UI_STATUS_OK);e.properties=CreateWindowExW(0,properties_class_name,L"EDA fixture",WS_OVERLAPPEDWINDOW,-16000,-16000,300,300,NULL,NULL,module_instance,&e);CHECK(e.properties!=NULL);e.note=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|ES_MULTILINE,0,0,200,200,e.properties,(HMENU)(uintptr_t)2,module_instance,NULL);CHECK(e.note!=NULL);CHECK(eda_note_reload(&e)==UI_STATUS_OK);
 CHECK(logic_invoke(&e.capabilities,"eda.add_block","{}")==UI_CAP2_OK);CHECK(e.block_count==1&&e.blocks[0].width==.22f&&e.blocks[0].height==.12f);first=e.blocks[0].id;revision=e.revision;CHECK(strstr(logic_result,"\"objects\":[{\"height\":")!=NULL||strstr(logic_result,"\"objects\":[{\"id\":")!=NULL);
 CHECK(logic_invoke(&e.capabilities,"eda.zoom_in","{}")==UI_CAP2_OK);CHECK(e.zoom>1&&e.revision==revision);
 snprintf(params,sizeof(params),"{\"ids\":[\"%llu\"]}",(unsigned long long)first);CHECK(logic_invoke(&e.capabilities,"eda.select",params)==UI_CAP2_OK&&e.selected==first);
 SendMessageW(e.note,EM_SETSEL,0,-1);SendMessageW(e.note,EM_REPLACESEL,TRUE,(LPARAM)L"private note draft");CHECK(e.note_dirty&&e.note_base==e.revision);CHECK(!strcmp(e.committed_note,"initial committed note"));
 CHECK(logic_invoke(&e.capabilities,"eda.add_block","{}")==UI_CAP2_OK);CHECK(e.note_dirty&&e.note_conflict);snprintf(params,sizeof(params),"{\"text\":\"private note draft\",\"base_revision\":\"doc-%llu\",\"generation\":\"%llu\"}",(unsigned long long)e.note_base,(unsigned long long)e.note_generation);
 CHECK(logic_invoke(&e.capabilities,"eda.note.submit",params)==UI_CAP2_FAILED);CHECK(!strcmp(e.committed_note,"initial committed note"));CHECK(logic_invoke(&e.capabilities,"eda.note.keep","{}")==UI_CAP2_OK&&e.note_dirty&&!e.note_conflict);
 snprintf(params,sizeof(params),"{\"text\":\"private note draft\",\"base_revision\":\"doc-%llu\",\"generation\":\"%llu\"}",(unsigned long long)e.note_base,(unsigned long long)e.note_generation);CHECK(logic_invoke(&e.capabilities,"eda.note.submit",params)==UI_CAP2_OK);CHECK(!strcmp(e.committed_note,"private note draft")&&!e.note_dirty);CHECK(strstr(logic_result,"private note draft")!=NULL);
 SendMessageW(e.note,EM_SETSEL,0,-1);SendMessageW(e.note,EM_REPLACESEL,TRUE,(LPARAM)L"discard this");CHECK(e.note_dirty);CHECK(logic_invoke(&e.capabilities,"eda.note.cancel","{}")==UI_CAP2_OK&&!e.note_dirty);{wchar_t actual[128];GetWindowTextW(e.note,actual,128);CHECK(!wcscmp(actual,L"private note draft"));}
 CHECK(logic_invoke(&e.capabilities,"eda.clear","{}")==UI_CAP2_OK);CHECK(e.block_count==0&&e.zoom==1&&e.selected==0);CHECK(logic_invoke(&e.capabilities,"eda.add_block","{}")==UI_CAP2_OK);CHECK(e.blocks[0].id>first+1);snprintf(params,sizeof(params),"{\"ids\":[\"%llu\"]}",(unsigned long long)first);CHECK(logic_invoke(&e.capabilities,"eda.select",params)==UI_CAP2_FAILED);
 {uint64_t before=e.revision;(void)ui_host_invoke(e.context.host,"eda.add_block","{}","human");CHECK(e.revision==before);}
 adapters(&e);
 DestroyWindow(e.properties);e.properties=e.note=NULL;ui_host_destroy(e.context.host);p2_dispose(&e.capabilities);free(e.committed_note);CHECK(eda_module_shutdown()==UI_STATUS_OK);
 printf("P2 EDA actual objects/note/drafts (not physical): checks=%u failures=%u\n",checks,failures);return failures?1:0;
}
