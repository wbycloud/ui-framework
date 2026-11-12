/* Included only by the isolated test DLL. No credentials or arbitrary controls. */
typedef struct br1_input_record {char stage[32],text[256];unsigned uid;int caret,anchor;const void *address;} br1_input_record;
static br1_input_record br1_inputs[128];static unsigned br1_input_count,br1_input_overflow,br1_input_verified;
static void br1_input_note(lw_view_t *v,lw_node_t *n,const char *stage,const char *text)
{
 (void)v;if(strcmp(n->id,"ai-model-name"))return;if(br1_input_count==128){br1_input_overflow=1;return;}
 br1_input_record *r=&br1_inputs[br1_input_count++];strcpy_s(r->stage,sizeof(r->stage),stage);strcpy_s(r->text,sizeof(r->text),text);
 r->uid=n->uid;r->caret=n->caret;r->anchor=n->anchor;r->address=n->value;
}
static void br1_input_wide(lw_view_t *v,lw_node_t *n,const char *stage,const wchar_t *text)
{char utf8[256];if(strcmp(n->id,"ai-model-name"))return;if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text,-1,utf8,sizeof(utf8),NULL,NULL)){br1_input_overflow=1;return;}br1_input_note(v,n,stage,utf8);}
static void br1_input_after(lw_view_t *v,unsigned uid,const char *stage)
{for(unsigned i=0;i<(unsigned)v->node_count;++i)if(v->nodes[i].used&&v->nodes[i].uid==uid){br1_input_note(v,&v->nodes[i],stage,v->nodes[i].value);return;}}
__declspec(dllexport) int ui_br1_input_verify(const char *expected,int moved)
{
 if(br1_input_overflow||br1_input_count!=br1_input_verified+6)return 0;br1_input_verified=br1_input_count;unsigned at=br1_input_count-6;br1_input_record *r=br1_inputs+at;
 const char *stages[]={"raw","converted","temporary","written","callback","layout"};
 for(unsigned i=0;i<6;++i)if(strcmp(r[i].stage,stages[i])||r[i].uid!=r[0].uid)return 0;
 if(strcmp(r[0].text,r[1].text))return 0;
 for(unsigned i=2;i<6;++i)if(strcmp(r[i].text,expected))return 0;
 return !moved||r[3].address!=r[4].address;
}
__declspec(dllexport) void ui_br1_input_dump(void)
{
 fprintf(stderr,"INPUT-MATRIX records=%u overflow=%u\n",br1_input_count,br1_input_overflow);
 for(unsigned i=0;i<br1_input_count;++i){br1_input_record *r=&br1_inputs[i];fprintf(stderr,"INPUT seq=%u stage=%s uid=%u ptr=%p caret=%d anchor=%d bytes=%zu hex=",i,r->stage,r->uid,r->address,r->caret,r->anchor,strlen(r->text));for(size_t j=0;j<=strlen(r->text);++j)fprintf(stderr,"%02x",(unsigned char)r->text[j]);fputc('\n',stderr);}
}
typedef struct br1_write_record {
 unsigned seq,uid,kind,key,message;DWORD thread;uint64_t generation;
 const void *view,*address;char stage[32],old_value[128],value[128];
 void *stack[12];USHORT depth;
} br1_write_record;
static br1_write_record br1_writes[256];
static volatile LONG br1_write_count,br1_write_overflow;
static br1_write_record *br1_write_allocate(void)
{LONG index=InterlockedIncrement(&br1_write_count)-1;if(index>=256){InterlockedExchange(&br1_write_overflow,1);return NULL;}br1_write_record *r=&br1_writes[index];r->seq=(unsigned)index+1;return r;}
static uint64_t br1_node_epoch;
__declspec(dllexport) uint64_t ui_br1_write_node_epoch(void){return br1_node_epoch;}
static const void *br1_last_view;static unsigned br1_last_uid;
static char br1_last_value[128];
static __declspec(thread) UINT br1_window_message;
static void br1_write_note(lw_view_t *v,lw_node_t *n,const char *stage)
{
 if(strcmp(n->id,"ai-model-name"))return;
 if(!strcmp(stage,"initialize")||!strcmp(stage,"release"))++br1_node_epoch;
 if(br1_last_view!=v||br1_last_uid!=n->uid){br1_last_view=v;br1_last_uid=n->uid;br1_last_value[0]=0;}
 if(!strcmp(br1_last_value,n->value)&&strcmp(stage,"release")&&strcmp(stage,"initialize"))return;
 br1_write_record *r=br1_write_allocate();if(!r)return;r->uid=n->uid;r->view=v;r->address=n->value;r->generation=v->dom_generation;
 r->thread=GetCurrentThreadId();r->kind=v->semantic_input.kind;r->key=v->semantic_input.key_code;r->message=br1_window_message;
 strcpy_s(r->stage,sizeof(r->stage),stage);strcpy_s(r->old_value,sizeof(r->old_value),br1_last_value);strcpy_s(r->value,sizeof(r->value),n->value);
 r->depth=CaptureStackBackTrace(1,12,r->stack,NULL);strcpy_s(br1_last_value,sizeof(br1_last_value),n->value);
 if(!strcmp(stage,"release")){br1_last_view=NULL;br1_last_uid=0;br1_last_value[0]=0;}
}
void ui_br1_model_copy(char *destination,size_t capacity,const char *source,const char *stage)
{
 br1_write_record *r=br1_write_allocate();if(r)strcpy_s(r->old_value,sizeof(r->old_value),destination);
 strcpy_s(destination,capacity,source);
 if(r){r->address=destination;r->thread=GetCurrentThreadId();strcpy_s(r->stage,sizeof(r->stage),stage);strcpy_s(r->value,sizeof(r->value),destination);r->depth=CaptureStackBackTrace(1,12,r->stack,NULL);}
}
__declspec(dllexport) void ui_br1_write_trace_dump(void)
{
 fprintf(stderr,"WRITE-TRACE records=%ld overflow=%ld\n",br1_write_count,br1_write_overflow);
 for(unsigned i=0;i<(unsigned)br1_write_count&&i<256;++i){br1_write_record *r=&br1_writes[i];
  fprintf(stderr,"WRITE seq=%u thread=%lu uid=%u gen=%llu view=%p address=%p stage=%s kind=%u key=%u msg=%u old[%zu]=%s new[%zu]=%s\n",r->seq,r->thread,r->uid,(unsigned long long)r->generation,r->view,r->address,r->stage,r->kind,r->key,r->message,strlen(r->old_value),r->old_value,strlen(r->value),r->value);
  for(unsigned j=0;j<r->depth;++j){HMODULE module=NULL;char path[MAX_PATH];
   if(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCSTR)r->stack[j],&module)&&GetModuleFileNameA(module,path,MAX_PATH)){const char *base=strrchr(path,'\\');fprintf(stderr,"  %s+0x%llx\n",base?base+1:path,(unsigned long long)((uintptr_t)r->stack[j]-(uintptr_t)module));}
   else fprintf(stderr,"  address=%p\n",r->stack[j]);
  }
 }
}
