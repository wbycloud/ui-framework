/* Framework-painted overflow bars: 12 logical pixels, 24 pixel minimum thumb.
 * No system scrollbar controls and no extra DOM nodes. */
static void lw_scroll_rect(lw_node_t *n,int horizontal,ui_rect_t *track,ui_rect_t *thumb,int *maximum)
{
    ui_rect_t r=n->viewport;int extent=horizontal?r.width:r.height,content=horizontal?n->content_width:n->content_height;
    int offset=horizontal?n->scroll_x:n->scroll_y,size;
    *track=horizontal?(ui_rect_t){r.x,r.y+r.height,r.width,12}:(ui_rect_t){r.x+r.width,r.y,12,r.height};
    *maximum=content>extent?content-extent:0;size=content?(int)((int64_t)extent*extent/content):extent;
    if(size<24)size=24;if(size>extent)size=extent;*thumb=*track;
    if(horizontal){thumb->width=size;thumb->x+=*maximum?(int)((int64_t)(extent-size)*offset / *maximum):0;}
    else{thumb->height=size;thumb->y+=*maximum?(int)((int64_t)(extent-size)*offset / *maximum):0;}
}
static int lw_scroll_contains(ui_rect_t r,int x,int y)
{return x>=r.x&&y>=r.y&&x<r.x+r.width&&y<r.y+r.height;}
static void lw_scroll_cancel(lw_view_t *v,int restore)
{
    unsigned uid=v->scroll_uid;int index;v->scroll_uid=0;if(!uid)return;index=lw_uid_index(v,uid);
    if(restore&&index>=0){if(v->scroll_axis)v->nodes[index].scroll_x=v->scroll_origin;else v->nodes[index].scroll_y=v->scroll_origin;}
    if(v->hwnd&&GetCapture()==v->hwnd)ReleaseCapture();if(restore)lw_layout(v);
}
static int lw_scroll_input(lw_view_t *v,const ui_input_event_t *e)
{
    int index=lw_uid_index(v,v->scroll_uid);
    if(v->scroll_uid){
        if(index<0){lw_scroll_cancel(v,0);return 1;}
        {lw_node_t *n=&v->nodes[index];int maximum=(v->scroll_axis?n->content_width-n->viewport.width:n->content_height-n->viewport.height);if(maximum!=v->scroll_max){lw_scroll_cancel(v,0);return 1;}}
        if(e->kind==UI_INPUT_KEY_DOWN&&e->key_code==VK_ESCAPE){lw_scroll_cancel(v,1);return 1;}
        if(e->kind==UI_INPUT_POINTER_UP){lw_scroll_cancel(v,0);return 1;}
        if(e->kind==UI_INPUT_POINTER_MOVE){int at=(v->scroll_axis?e->x:e->y)-v->scroll_start+v->scroll_position,value;
            if(at<0)at=0;if(at>v->scroll_travel)at=v->scroll_travel;
            value=v->scroll_travel?(int)((int64_t)at*v->scroll_max/v->scroll_travel):0;
            if(v->scroll_axis)v->nodes[index].scroll_x=value;else v->nodes[index].scroll_y=value;lw_layout(v);return 1;}
    }
    if(e->kind==UI_INPUT_KEY_DOWN&&v->scroll_focus>=0&&v->scroll_focus<(int)v->node_count){
        lw_node_t *n=&v->nodes[v->scroll_focus];int axis=v->scroll_axis,extent=axis?n->viewport.width:n->viewport.height,*offset=axis?&n->scroll_x:&n->scroll_y,maximum=(axis?n->content_width:n->content_height)-extent;
        if(n->used&&(axis?n->bar_x:n->bar_y)){switch(e->key_code){case VK_HOME:*offset=0;break;case VK_END:*offset=maximum;break;case VK_PRIOR:*offset-=extent;break;case VK_NEXT:*offset+=extent;break;case VK_LEFT:case VK_UP:*offset-=24;break;case VK_RIGHT:case VK_DOWN:*offset+=24;break;default:return 0;}lw_layout(v);return 1;}
    }
    if(e->kind==UI_INPUT_POINTER_DOWN&&e->pointer_button<=1){
        int hit=-1,axis=0;ui_rect_t track={0},thumb={0};int maximum=0;
        for(int i=v->root;i>=0;i=lw_next_node(v,i)){lw_node_t *n=&v->nodes[i];if(!n->used||!n->visible||lw_disabled(v,i)||!lw_scroll_contains(n->clip,e->x,e->y))continue;
            for(int a=0;a<2;++a)if(a?n->bar_x:n->bar_y){ui_rect_t t,b;int m;lw_scroll_rect(n,a,&t,&b,&m);if(lw_scroll_contains(t,e->x,e->y)){hit=i;axis=a;track=t;thumb=b;maximum=m;}}}
        if(hit>=0){lw_node_t *n=&v->nodes[hit];int *offset=axis?&n->scroll_x:&n->scroll_y,at=axis?e->x:e->y;
            v->scroll_focus=hit;v->scroll_axis=axis;if(v->hwnd)SetFocus(v->hwnd);
            if(lw_scroll_contains(thumb,e->x,e->y)){v->scroll_uid=n->uid;v->scroll_origin=*offset;v->scroll_start=at;v->scroll_position=axis?thumb.x-track.x:thumb.y-track.y;v->scroll_travel=axis?track.width-thumb.width:track.height-thumb.height;v->scroll_max=maximum;if(v->hwnd)SetCapture(v->hwnd);}
            else{*offset+=(at<(axis?thumb.x:thumb.y)?-1:1)*(axis?n->viewport.width:n->viewport.height);lw_layout(v);}return 1;}
    }
    return 0;
}
static void lw_scroll_paint(lw_view_t *v,lw_node_t *n,HDC dc)
{
    for(int a=0;a<2;++a)if(a?n->bar_x:n->bar_y){ui_rect_t track,thumb;RECT r;int maximum;HBRUSH brush;int dark=n->style.color>0x808080;
        lw_scroll_rect(n,a,&track,&thumb,&maximum);r=lw_pixel_rect(v,track);brush=CreateSolidBrush(dark?RGB(24,30,40):RGB(234,236,240));FillRect(dc,&r,brush);DeleteObject(brush);
        r=lw_pixel_rect(v,thumb);brush=CreateSolidBrush(v->scroll_uid==n->uid||v->hovered==(int)(n-v->nodes)?RGB(91,139,200):dark?RGB(90,108,135):RGB(151,160,172));FillRect(dc,&r,brush);DeleteObject(brush);
        if(v->scroll_focus==(int)(n-v->nodes)){InflateRect(&r,-1,-1);DrawFocusRect(dc,&r);}
    }
}
