/* Materialize clipboard handles before a test replaces them. Retaining an OLE
   proxy and flushing it after replacement can re-enter an expired data owner. */
typedef struct clipboard_snapshot { UINT formats[64]; HANDLE data[64]; size_t count; } clipboard_snapshot;
static void clipboard_discard(clipboard_snapshot *saved)
{
    for(size_t i=0;i<saved->count;++i)if(saved->data[i]){
        UINT f=saved->formats[i];HANDLE h=saved->data[i];
        if(f==CF_BITMAP||f==CF_DSPBITMAP||f==CF_PALETTE)DeleteObject(h);
        else if(f==CF_ENHMETAFILE||f==CF_DSPENHMETAFILE)DeleteEnhMetaFile(h);
        else {if(f==CF_METAFILEPICT||f==CF_DSPMETAFILEPICT){METAFILEPICT *p=GlobalLock(h);if(p){DeleteMetaFile(p->hMF);GlobalUnlock(h);}}GlobalFree(h);}
    }
    saved->count=0;
}
static int clipboard_capture(HWND owner,clipboard_snapshot *saved)
{
    UINT format=0;int ok=1;
    if(!OpenClipboard(owner))return 0;
    while((format=EnumClipboardFormats(format))!=0){
        HANDLE source=GetClipboardData(format),copy;
        /* Delayed formats can be advertised after their owner has gone away.
           No data exists to preserve for such an entry. */
        if(!source){fprintf(stderr,"clipboard original format=%u has no readable data\n",format);continue;}
        if(format==CF_OWNERDISPLAY||saved->count==64){ok=0;break;}
        if(format==CF_BITMAP||format==CF_DSPBITMAP)copy=CopyImage(source,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION);
        else if(format==CF_ENHMETAFILE||format==CF_DSPENHMETAFILE)copy=CopyEnhMetaFileW(source,NULL);
        else copy=OleDuplicateData(source,(CLIPFORMAT)(format==CF_DSPMETAFILEPICT?CF_METAFILEPICT:format),0);
        if(!copy){fprintf(stderr,"clipboard duplicate format=%u size=%zu error=%lu\n",format,(size_t)GlobalSize(source),GetLastError());ok=0;break;}
        saved->formats[saved->count]=format;saved->data[saved->count++]=copy;
    }
    CloseClipboard();if(!ok)clipboard_discard(saved);return ok;
}
static int clipboard_restore(HWND owner,clipboard_snapshot *saved)
{
    int ok=1;if(!OpenClipboard(owner))return 0;
    if(!EmptyClipboard()){CloseClipboard();return 0;}
    for(size_t i=0;i<saved->count;++i){
        if(SetClipboardData(saved->formats[i],saved->data[i]))saved->data[i]=NULL;else ok=0;
    }
    CloseClipboard();clipboard_discard(saved);return ok;
}
