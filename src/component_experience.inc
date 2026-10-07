ui_status_t ui_component_set_selection(ui_component_t *c,const uint64_t *ids,size_t count)
{
    uint64_t *copy=NULL;size_t i,j;if(!c||(count&&!ids))return UI_STATUS_INVALID_ARGUMENT;
    if(count>512)return UI_STATUS_LIMIT_EXCEEDED;if(count>1&&!(c->desc.selection_flags&UI_SELECTION_MULTIPLE))return UI_STATUS_UNSUPPORTED;
    if(c->host->dispatch_blocked)return UI_STATUS_CANCELLED;
    for(i=0;i<count;++i){if(!ids[i])return UI_STATUS_INVALID_ARGUMENT;for(j=0;j<i;++j)if(ids[j]==ids[i])return UI_STATUS_INVALID_ARGUMENT;}
    if(cache_bytes(c)-c->selection_count*sizeof(uint64_t)+count*sizeof(uint64_t)>2u*1024u*1024u)return UI_STATUS_LIMIT_EXCEEDED;
    if(count){copy=(uint64_t *)malloc(count*sizeof(uint64_t));if(!copy)return UI_STATUS_OUT_OF_MEMORY;memcpy(copy,ids,count*sizeof(uint64_t));}
    free(c->selection);c->selection=copy;c->selection_count=count;c->selected=count?ids[count-1]:0;c->selection_request=0;render_component(c);return UI_STATUS_OK;
}
ui_status_t ui_component_get_selection(const ui_component_t *c,uint64_t *ids,size_t capacity,size_t *count)
{
    if(!c||!count)return UI_STATUS_INVALID_ARGUMENT;*count=c->selection_count;if(!ids)return UI_STATUS_OK;
    if(capacity<c->selection_count)return UI_STATUS_LIMIT_EXCEEDED;if(c->selection_count)memcpy(ids,c->selection,c->selection_count*sizeof(uint64_t));return UI_STATUS_OK;
}
ui_status_t ui_component_select_range(ui_component_t *c,uint64_t first,size_t count,int extend)
{
    ui_component_query_t q={0};component_page_t *root;if(!c||!count||!c->desc.source)return UI_STATUS_INVALID_ARGUMENT;
    if(!(c->desc.selection_flags&UI_SELECTION_MULTIPLE))return UI_STATUS_UNSUPPORTED;if(count>512)return UI_STATUS_LIMIT_EXCEEDED;
    if(c->host->dispatch_blocked)return UI_STATUS_CANCELLED;root=page(c,0,0);
    if(!root||first>root->total||count>root->total-first)return UI_STATUS_INVALID_ARGUMENT;
    c->selection_request=++c->next_request;c->selection_requested=count;c->selection_extend=extend!=0;
    q.size=sizeof(q);q.kind=UI_QUERY_SELECTION;q.first=first;q.count=count;q.component_generation=c->generation;q.request_id=c->selection_request;
    q.sort_column=c->sort_column;q.sort_direction=c->sort_direction;q.dpi=c->host->dpi;
    ui_dispatch_enter(c->host);c->desc.source(c,&q,c->desc.user_data);ui_dispatch_leave(c->host);return UI_STATUS_OK;
}
ui_status_t ui_component_submit_selection(ui_component_t *c,uint64_t generation,uint64_t request,const uint64_t *ids,size_t count)
{
    uint64_t merged[512];size_t n=0,i,j;if(!c||(count&&!ids))return UI_STATUS_INVALID_ARGUMENT;
    if(c->host->dispatch_blocked||generation!=c->generation||!request||request!=c->selection_request)return UI_STATUS_CANCELLED;
    if(count!=c->selection_requested)return UI_STATUS_INVALID_ARGUMENT;
    if(c->selection_extend){n=c->selection_count;if(n)memcpy(merged,c->selection,n*sizeof(uint64_t));}
    for(i=0;i<count;++i){if(!ids[i])return UI_STATUS_INVALID_ARGUMENT;for(j=0;j<i;++j)if(ids[j]==ids[i])return UI_STATUS_INVALID_ARGUMENT;
        for(j=0;j<n&&merged[j]!=ids[i];++j){}if(j==n){if(n==512)return UI_STATUS_LIMIT_EXCEEDED;merged[n++]=ids[i];}}
    return ui_component_set_selection(c,merged,n);
}
ui_status_t ui_component_set_sort(ui_component_t *c,const char *column,int direction)
{
    size_t i;ui_command_state_t state={0};ui_json_t params={0};ui_status_t status;
    if(!c||!column||(direction!=-1&&direction!=0&&direction!=1))return UI_STATUS_INVALID_ARGUMENT;
    if(c->desc.kind!=UI_COMPONENT_TABLE||!c->desc.source||!c->desc.sort_command||!*c->desc.sort_command)return UI_STATUS_UNSUPPORTED;
    if(c->host->dispatch_blocked||c->host->modal_component)return UI_STATUS_CANCELLED;
    if(direction){for(i=0;i<c->desc.column_count&&strcmp(c->columns[i].id,column);++i){}if(i==c->desc.column_count)return UI_STATUS_NOT_FOUND;}
    state.size=sizeof(state);if(ui_host_get_command_state(c->host,c->desc.sort_command,&state)!=UI_STATUS_OK||!state.enabled||state.busy)return UI_STATUS_CANCELLED;
    if(c->sort_direction==direction&&(!direction||!strcmp(c->sort_column,column)))return UI_STATUS_OK;
    ui_dispatch_enter(c->host);cancel_column_resize(c,0);free_pages(c);free_thumbs(c);release_previews(c,1);c->selection_request=0;c->selection_anchor=0;
    c->selection_anchor_id=0;c->generation=++c->host->component_generation;c->sort_direction=direction;strcpy(c->sort_column,direction?column:"");c->first=0;
    uj_add(&params,"{\"column\":");uj_string(&params,c->sort_column);uj_fmt(&params,",\"direction\":%d,\"generation\":\"%llu\"}",direction,(unsigned long long)c->generation);
    if(!params.failed)invoke_component(c,c->desc.sort_command,params.data);free(params.data);
    status=query_viewport(c,0,c->first_column,0);render_component(c);ui_dispatch_leave(c->host);return status;
}
static int parse_color(const char *text,uint32_t *out)
{
    size_t i,n;if(!text||text[0]!='#')return 0;n=strlen(text);if(n!=7&&n!=9)return 0;*out=0;
    for(i=1;i<n;++i){unsigned char c=(unsigned char)text[i];int v=c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:c>='A'&&c<='F'?c-'A'+10:-1;if(v<0)return 0;*out=(*out<<4)|(uint32_t)v;}
    if(n==7)*out=(*out<<8)|255u;return 1;
}
static void user_selection(ui_component_t *c,const char *json,uint64_t id)
{
    char flag[16];uint64_t ids[512],position=uj_u64(json,"index");size_t n=0,i;int control=uj_get(json,"ctrl",flag,sizeof(flag))&&!strcmp(flag,"true");
    int shift=uj_get(json,"shift",flag,sizeof(flag))&&!strcmp(flag,"true");
    if(!cached_row(c,id))return;
    if((c->desc.selection_flags&UI_SELECTION_MULTIPLE)&&shift&&c->selection_count&&c->selection_anchor_id){uint64_t first=position<c->selection_anchor?position:c->selection_anchor,last=position>c->selection_anchor?position:c->selection_anchor;
        if(last-first<512)(void)ui_component_select_range(c,first,(size_t)(last-first+1),control);
    }else if((c->desc.selection_flags&UI_SELECTION_MULTIPLE)&&control){
        for(i=0;i<c->selection_count;++i)if(c->selection[i]!=id)ids[n++]=c->selection[i];
        if(n==c->selection_count&&n<512)ids[n++]=id;(void)ui_component_set_selection(c,ids,n);c->selection_anchor=position;c->selection_anchor_id=id;
    }else{(void)ui_component_select(c,id);c->selection_anchor=position;c->selection_anchor_id=id;}
    invoke_component(c,c->desc.commands.select,json);
}
