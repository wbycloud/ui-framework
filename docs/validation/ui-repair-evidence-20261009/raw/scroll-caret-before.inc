/* Text belongs to the Web document; Win32 supplies only IME, clipboard and caret. */
typedef struct lw_history { char *text; int caret,anchor; struct lw_history *next; } lw_history_t;
static void lw_history_free(lw_history_t **head)
{while(*head){lw_history_t *p=*head;*head=p->next;free(p->text);free(p);}}
static void lw_text_free(lw_node_t *n)
{lw_history_free(&n->undo);lw_history_free(&n->redo);}
static void lw_history_push(lw_history_t **head,lw_node_t *n)
{
    lw_history_t *p=(lw_history_t *)calloc(1,sizeof(*p)),*at;int count=0;
    if(!p)return;p->text=ui_strdup(n->value);if(!p->text){free(p);return;}
    p->caret=n->caret;p->anchor=n->anchor;p->next=*head;*head=p;
    for(at=*head;at;at=at->next)if(++count==16){lw_history_free(&at->next);break;}
}
static SCRIPT_STRING_ANALYSIS lw_shape(HDC dc,const wchar_t *text,int count)
{
    SCRIPT_STRING_ANALYSIS analysis=NULL;
    if(count>0&&FAILED(ScriptStringAnalyse(dc,text,count,count*3/2+16,-1,
        SSA_GLYPHS|SSA_FALLBACK|SSA_LINK|SSA_BREAK,0,NULL,NULL,NULL,NULL,NULL,&analysis)))return NULL;
    return analysis;
}
static int lw_grapheme(lw_node_t *n,const wchar_t *wide,int position,int direction)
{
    HDC dc=GetDC(NULL);SCRIPT_STRING_ANALYSIS a;const SCRIPT_LOGATTR *attrs;int length=(int)wcslen(wide);
    HGDIOBJ old=n->font?SelectObject(dc,n->font):NULL;a=lw_shape(dc,wide,length);attrs=a?ScriptString_pLogAttr(a):NULL;
    position+=direction;while(position>0&&position<length&&
        ((attrs&&!attrs[position].fCharStop)||(!attrs&&wide[position]>=0xdc00&&wide[position]<=0xdfff)))position+=direction;
    if(a)ScriptStringFree(&a);if(old)SelectObject(dc,old);ReleaseDC(NULL,dc);
    return position<0?0:position>length?length:position;
}
static int lw_line_height(lw_view_t *view,lw_node_t *n)
{return lw_pixel(n->style.font_size+6,view->dpi);}
static void lw_text_position(lw_view_t *view,lw_node_t *n,int *x,int *y)
{
    wchar_t *w=lw_wide(n->value);int first=0,i,line=0;HDC dc;SCRIPT_STRING_ANALYSIS a;HGDIOBJ old;
    *x=0;*y=0;if(!w)return;
    for(i=0;i<n->caret&&w[i];++i)if(w[i]==L'\n'){first=i+1;++line;}
    for(i=first;w[i]&&w[i]!=L'\n';++i){}
    dc=GetDC(view->hwnd);old=n->font?SelectObject(dc,n->font):NULL;a=lw_shape(dc,w+first,i-first);
    if(a){(void)ScriptStringCPtoX(a,n->caret==i?i-first-1:n->caret-first,n->caret==i,x);ScriptStringFree(&a);}
    if(old)SelectObject(dc,old);ReleaseDC(view->hwnd,dc);*y=line*lw_line_height(view,n);free(w);
}
static void lw_caret(lw_view_t *view)
{
    lw_node_t *n;int x,y,width,height;POINT point;HIMC imc;
    if(!view->hwnd||GetFocus()!=view->hwnd||view->focused<0)return;
    n=&view->nodes[view->focused];DestroyCaret();
    if(!lw_input(n)||n->disabled||!n->visible)return;
    lw_text_position(view,n,&x,&y);width=lw_pixel(n->kind==4?n->viewport.width:n->rect.width-12,view->dpi);height=lw_pixel(n->kind==4?n->viewport.height:n->rect.height-8,view->dpi);
    if(!view->text_scrolling){
        if(x-n->text_scroll_x>=width)n->text_scroll_x=x-width+lw_pixel(2,view->dpi);
        if(x<n->text_scroll_x)n->text_scroll_x=x;
        if(y-n->text_scroll_y+lw_line_height(view,n)>height)n->text_scroll_y=y+lw_line_height(view,n)-height;
        if(y<n->text_scroll_y)n->text_scroll_y=y;
        if(n->text_scroll_x<0)n->text_scroll_x=0;if(n->text_scroll_y<0)n->text_scroll_y=0;
        if(n->kind==4){n->scroll_x=MulDiv(n->text_scroll_x,96,(int)view->dpi);n->scroll_y=MulDiv(n->text_scroll_y,96,(int)view->dpi);}}
    point.x=lw_pixel(n->kind==4?n->viewport.x:n->rect.x+5,view->dpi)+x-n->text_scroll_x;
    point.y=lw_pixel(n->kind==4?n->viewport.y:n->rect.y+4,view->dpi)+y-n->text_scroll_y;
    if(x-n->text_scroll_x<0||x-n->text_scroll_x>=width||y-n->text_scroll_y<0||y-n->text_scroll_y+lw_line_height(view,n)>height)return;
    CreateCaret(view->hwnd,NULL,lw_pixel(1,view->dpi),lw_line_height(view,n));SetCaretPos(point.x,point.y);ShowCaret(view->hwnd);
    imc=ImmGetContext(view->hwnd);if(imc){COMPOSITIONFORM cf={0};CANDIDATEFORM candidate={0};
        cf.dwStyle=CFS_POINT;cf.ptCurrentPos=point;ImmSetCompositionWindow(imc,&cf);
        candidate.dwStyle=CFS_CANDIDATEPOS;candidate.ptCurrentPos=point;candidate.ptCurrentPos.y+=lw_line_height(view,n);
        ImmSetCandidateWindow(imc,&candidate);ImmReleaseContext(view->hwnd,imc);}
}
static ui_status_t lw_text_replace(lw_view_t *view,lw_node_t *n,const wchar_t *insert)
{
    wchar_t *w=lw_wide(n->value),buffer[LW_TEXT_CAP];char utf8[LW_TEXT_CAP];int a,b,len,added,at=0,i;
    if(!w)return UI_STATUS_VALIDATION_FAILED;len=(int)wcslen(w);a=n->anchor<n->caret?n->anchor:n->caret;b=n->anchor>n->caret?n->anchor:n->caret;
    if(a>len)a=len;if(b>len)b=len;
    for(i=0;insert[i]&&at<LW_TEXT_CAP-1;++i){if(insert[i]==L'\r'){if(insert[i+1]==L'\n')++i;if(n->kind==4)buffer[at++]=L'\n';}
        else if(insert[i]!=L'\n'||n->kind==4)buffer[at++]=insert[i];}
    added=at;if(a+added+len-b>=LW_TEXT_CAP){free(w);return UI_STATUS_VALIDATION_FAILED;}
    memmove(buffer+a,buffer,(size_t)added*sizeof(wchar_t));memcpy(buffer,w,(size_t)a*sizeof(wchar_t));
    memcpy(buffer+a+added,w+b,(size_t)(len-b+1)*sizeof(wchar_t));free(w);
    if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,buffer,-1,utf8,sizeof(utf8),NULL,NULL))return UI_STATUS_VALIDATION_FAILED;
    if(!strcmp(utf8,n->value))return UI_STATUS_OK;
    lw_history_push(&n->undo,n);lw_history_free(&n->redo);strcpy(n->value,utf8);n->caret=n->anchor=a+added;
    view->dirty=1;(void)lw_event(view,(int)(n-view->nodes),1,0);lw_layout(view);return UI_STATUS_OK;
}
static ui_status_t lw_text_insert(lw_view_t *view,const char *text)
{
    wchar_t *w;ui_status_t status;lw_node_t *n;
    if(view->focused<0||!text)return UI_STATUS_NOT_FOUND;n=&view->nodes[view->focused];
    if(!lw_input(n)||n->disabled||n->readonly)return UI_STATUS_NOT_FOUND;
    w=lw_wide(text);if(!w)return UI_STATUS_VALIDATION_FAILED;status=lw_text_replace(view,n,w);free(w);return status;
}
static void lw_clipboard(lw_view_t *view,lw_node_t *n,int paste,int cut)
{
    wchar_t *w;int a,b,attempt;HGLOBAL memory;wchar_t *target;
    for(attempt=0;attempt<6;++attempt){if(OpenClipboard(view->hwnd))break;Sleep(5);}
    if(attempt==6)return;
    if(paste){memory=GetClipboardData(CF_UNICODETEXT);target=memory?(wchar_t *)GlobalLock(memory):NULL;
        if(target){if(!n->readonly)(void)lw_text_replace(view,n,target);GlobalUnlock(memory);}}
    else{w=lw_wide(n->value);a=n->caret<n->anchor?n->caret:n->anchor;b=n->caret>n->anchor?n->caret:n->anchor;
        if(w&&a!=b){memory=GlobalAlloc(GMEM_MOVEABLE,(size_t)(b-a+1)*sizeof(wchar_t));target=memory?(wchar_t *)GlobalLock(memory):NULL;
            if(target){memcpy(target,w+a,(size_t)(b-a)*sizeof(wchar_t));target[b-a]=0;GlobalUnlock(memory);EmptyClipboard();if(!SetClipboardData(CF_UNICODETEXT,memory))GlobalFree(memory);}
            if(cut&&!n->readonly)(void)lw_text_replace(view,n,L"");}free(w);}
    CloseClipboard();
}
static int lw_text_key(lw_view_t *view,const ui_input_event_t *event)
{
    lw_node_t *n;wchar_t *w;int length,position,control=(event->modifiers&UI_INPUT_MODIFIER_CONTROL)!=0,shift=(event->modifiers&UI_INPUT_MODIFIER_SHIFT)!=0;
    if(view->focused<0)return 0;n=&view->nodes[view->focused];if(!lw_input(n)||n->disabled)return 0;
    w=lw_wide(n->value);if(!w)return 0;length=(int)wcslen(w);position=n->caret;
    if(control&&(event->key_code=='C'||event->key_code=='X'||event->key_code=='V')){free(w);lw_clipboard(view,n,event->key_code=='V',event->key_code=='X');return 1;}
    if(control&&(event->key_code=='Z'||event->key_code=='Y')){lw_history_t **from=event->key_code=='Y'||shift?&n->redo:&n->undo,**to=from==&n->undo?&n->redo:&n->undo;
        if(*from&&!n->readonly){lw_history_t *h=*from;lw_history_push(to,n);*from=h->next;strcpy(n->value,h->text);n->caret=h->caret;n->anchor=h->anchor;free(h->text);free(h);view->dirty=1;(void)lw_event(view,view->focused,1,0);}free(w);lw_layout(view);return 1;}
    if(control&&event->key_code=='A'){n->anchor=0;n->caret=length;free(w);lw_layout(view);return 1;}
    if(event->key_code==VK_BACK||event->key_code==VK_DELETE){
        if(!n->readonly){if(n->anchor==n->caret)n->anchor=lw_grapheme(n,w,n->caret,event->key_code==VK_BACK?-1:1);(void)lw_text_replace(view,n,L"");}free(w);return 1;}
    if(event->key_code==VK_RETURN&&n->kind==4&&!control){free(w);if(!n->readonly)(void)lw_text_replace(view,n,L"\n");return 1;}
    if(event->key_code==VK_LEFT||event->key_code==VK_RIGHT){int direction=event->key_code==VK_LEFT?-1:1;
        position=lw_grapheme(n,w,position,direction);if(control)while(position>0&&position<length&&w[position]!=L' '&&w[position]!=L'\n')position=lw_grapheme(n,w,position,direction);}
    else if(event->key_code==VK_HOME){position=control?0:position;while(position>0&&w[position-1]!=L'\n')--position;}
    else if(event->key_code==VK_END){position=control?length:position;while(position<length&&w[position]!=L'\n')++position;}
    else if(event->key_code==VK_UP||event->key_code==VK_DOWN){int start=position,end,column;
        while(start>0&&w[start-1]!=L'\n')--start;column=position-start;
        if(event->key_code==VK_UP){end=start?start-1:0;start=end;while(start>0&&w[start-1]!=L'\n')--start;}
        else{end=position;while(end<length&&w[end]!=L'\n')++end;start=end<length?end+1:length;end=start;while(end<length&&w[end]!=L'\n')++end;}
        position=start+column;if(position>end)position=end;}
    else{free(w);return 0;}
    n->caret=position;if(!shift)n->anchor=position;free(w);lw_layout(view);return 1;
}
static void lw_text_pointer(lw_view_t *view,int index,int x,int y,int dragging)
{
    lw_node_t *n;wchar_t *w;int line,start=0,end=0,cp=0,trailing=0,i;HDC dc;HGDIOBJ old;SCRIPT_STRING_ANALYSIS a;
    if(index<0)return;n=&view->nodes[index];if(!lw_input(n)||n->disabled)return;
    if(!dragging){view->dragging=1;if(view->hwnd)SetCapture(view->hwnd);}
    w=lw_wide(n->value);if(!w)return;
    line=(lw_pixel(y-(n->kind==4?n->viewport.y:n->rect.y+4),view->dpi)+n->text_scroll_y)/lw_line_height(view,n);
    for(i=0;w[i]&&line>0;++i)if(w[i]==L'\n'){--line;start=i+1;}
    end=start;while(w[end]&&w[end]!=L'\n')++end;
    dc=GetDC(view->hwnd);old=n->font?SelectObject(dc,n->font):NULL;a=lw_shape(dc,w+start,end-start);
    if(a){ScriptStringXtoCP(a,lw_pixel(x-(n->kind==4?n->viewport.x:n->rect.x+5),view->dpi)+n->text_scroll_x,&cp,&trailing);ScriptStringFree(&a);}
    if(cp<0)cp=0;if(cp+trailing>end-start){cp=end-start;trailing=0;}
    n->caret=start+cp+trailing;if(!dragging&&!(view->event_modifiers&UI_INPUT_MODIFIER_SHIFT))n->anchor=n->caret;
    if(old)SelectObject(dc,old);ReleaseDC(view->hwnd,dc);free(w);lw_layout(view);
}
static void lw_text_paint(lw_view_t *view,lw_node_t *n,HDC dc,RECT rect)
{
    wchar_t *w=lw_wide(n->value);int start=0,end=0,line=0,sel_a=n->anchor<n->caret?n->anchor:n->caret,sel_b=n->anchor>n->caret?n->anchor:n->caret;
    int saved=0;
    int height=lw_line_height(view,n),x=rect.left+lw_pixel(5,view->dpi)-n->text_scroll_x,y=rect.top+lw_pixel(4,view->dpi)-n->text_scroll_y;
    if(n->kind==4){saved=SaveDC(dc);rect=lw_pixel_rect(view,n->viewport);IntersectClipRect(dc,rect.left,rect.top,rect.right,rect.bottom);x=rect.left-n->text_scroll_x;y=rect.top-n->text_scroll_y;}
    if(!w){if(saved)RestoreDC(dc,saved);return;}if(n->font)SelectObject(dc,n->font);SetTextColor(dc,lw_rgb(n->disabled?0x8490a0:n->style.color));
    do{SCRIPT_STRING_ANALYSIS a;end=start;while(w[end]&&w[end]!=L'\n')++end;
        a=lw_shape(dc,w+start,end-start);if(a){int a_sel=sel_a>start?sel_a-start:0,b_sel=sel_b<end?sel_b-start:end-start;
            if(a_sel>end-start)a_sel=end-start;if(b_sel<0)b_sel=0;if(b_sel<a_sel)b_sel=a_sel;
            ScriptStringOut(a,x,y+line*height,0,NULL,a_sel,b_sel,FALSE);ScriptStringFree(&a);}
        if(!w[end])break;start=end+1;++line;
    }while(y+line*height<rect.bottom);
    if(*n->composition){int cx,cy;RECT composition;lw_text_position(view,n,&cx,&cy);composition.left=x+cx;composition.top=y+cy;composition.right=rect.right;composition.bottom=composition.top+height;
        DrawTextW(dc,n->composition,-1,&composition,DT_SINGLELINE|DT_NOPREFIX);MoveToEx(dc,composition.left,composition.bottom-2,NULL);LineTo(dc,composition.right,composition.bottom-2);}
    free(w);if(saved)RestoreDC(dc,saved);
}
static int lw_text_message(lw_view_t *view,UINT message,WPARAM wp,LPARAM lp)
{
    lw_node_t *n=view->focused>=0?&view->nodes[view->focused]:NULL;
    if(message==WM_KILLFOCUS){HIMC imc=ImmGetContext(view->hwnd);if(imc){ImmNotifyIME(imc,NI_COMPOSITIONSTR,CPS_CANCEL,0);ImmReleaseContext(view->hwnd,imc);}view->composing=0;if(n)*n->composition=0;DestroyCaret();return 0;}
    if(message==WM_SETFOCUS){lw_caret(view);return 0;}
    if(!n||!lw_input(n)||n->disabled)return 0;
    if(message==WM_CHAR){wchar_t chars[3]={0};char utf8[12];if(wp<32)return 1;
        if(wp>=0xd800&&wp<=0xdbff){view->high_surrogate=(wchar_t)wp;return 1;}
        if(wp>=0xdc00&&wp<=0xdfff&&view->high_surrogate){chars[0]=view->high_surrogate;chars[1]=(wchar_t)wp;}else chars[0]=(wchar_t)wp;
        view->high_surrogate=0;if(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,chars,-1,utf8,sizeof(utf8),NULL,NULL))(void)lw_text_insert(view,utf8);return 1;}
    if(message==WM_IME_STARTCOMPOSITION){view->composing=1;*n->composition=0;return 1;}
    if(message==WM_IME_ENDCOMPOSITION){view->composing=0;*n->composition=0;lw_layout(view);return 1;}
    if(message==WM_IME_COMPOSITION){HIMC imc=ImmGetContext(view->hwnd);LONG bytes;
        if(!imc)return 1;
        if(lp&GCS_RESULTSTR){wchar_t result[LW_TEXT_CAP];bytes=ImmGetCompositionStringW(imc,GCS_RESULTSTR,result,sizeof(result)-sizeof(wchar_t));if(bytes>=0){result[bytes/2]=0;if(!n->readonly)(void)lw_text_replace(view,n,result);}*n->composition=0;}
        if(lp&GCS_COMPSTR){bytes=ImmGetCompositionStringW(imc,GCS_COMPSTR,n->composition,sizeof(n->composition)-sizeof(wchar_t));if(bytes>=0)n->composition[bytes/2]=0;}
        ImmReleaseContext(view->hwnd,imc);lw_layout(view);return 1;}
    return 0;
}
