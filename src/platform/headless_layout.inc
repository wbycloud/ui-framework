/* This core-state test adapter has no window/desktop layout manager. Actual
 * Windows UI_RUN_OFFSCREEN uses win32_shell.c's logical layout implementation. */
ui_status_t ui_shell_get_panel_layout(ui_shell_t *s,const char *id,ui_panel_layout_t *v)
{(void)id;(void)v;return s?UI_STATUS_UNSUPPORTED:UI_STATUS_INVALID_ARGUMENT;}
ui_status_t ui_shell_set_panel_layout(ui_shell_t *s,const char *id,const ui_panel_layout_t *v)
{(void)id;(void)v;return s?UI_STATUS_UNSUPPORTED:UI_STATUS_INVALID_ARGUMENT;}
ui_status_t ui_shell_save_layout(ui_shell_t *s,void *p,size_t n,size_t *required)
{(void)p;(void)n;(void)required;return s?UI_STATUS_UNSUPPORTED:UI_STATUS_INVALID_ARGUMENT;}
ui_status_t ui_shell_restore_layout(ui_shell_t *s,const void *p,size_t n)
{(void)p;(void)n;return s?UI_STATUS_UNSUPPORTED:UI_STATUS_INVALID_ARGUMENT;}
ui_status_t ui_shell_reset_layout(ui_shell_t *s)
{return s?UI_STATUS_UNSUPPORTED:UI_STATUS_INVALID_ARGUMENT;}
ui_status_t ui_shell_resize_splitter(ui_shell_t *s,ui_layout_region_t r,const char *id,int n)
{(void)r;(void)id;(void)n;return s?UI_STATUS_UNSUPPORTED:UI_STATUS_INVALID_ARGUMENT;}
ui_status_t ui_shell_begin_splitter_drag(ui_shell_t *s,ui_layout_region_t r,const char *id)
{(void)r;(void)id;return s?UI_STATUS_UNSUPPORTED:UI_STATUS_INVALID_ARGUMENT;}
ui_status_t ui_shell_begin_panel_drag(ui_shell_t *s,const char *id)
{(void)id;return s?UI_STATUS_UNSUPPORTED:UI_STATUS_INVALID_ARGUMENT;}
ui_status_t ui_shell_update_panel_drag(ui_shell_t *s,int x,int y,ui_rect_t *p,ui_layout_region_t *r)
{(void)x;(void)y;(void)p;(void)r;return s?UI_STATUS_UNSUPPORTED:UI_STATUS_INVALID_ARGUMENT;}
ui_status_t ui_shell_end_panel_drag(ui_shell_t *s,int commit)
{(void)commit;return s?UI_STATUS_UNSUPPORTED:UI_STATUS_INVALID_ARGUMENT;}
