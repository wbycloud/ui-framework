#include "ui_framework/light_web.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexbor/html/html.h"
#include "lexbor/dom/interfaces/element.h"
#include "lexbor/dom/interfaces/attr.h"
#include "lexbor/dom/interfaces/node.h"
#include "quickjs.h"

#include <windows.h>
#include <windowsx.h>

#define LW_MAX_NODES 192
#define LW_TEXT_CAP 1024
#define LW_ATTR_CAP 2048
#define LW_ID_CAP 96
#define LW_HTML_LIMIT (256u * 1024u)
#define LW_MAX_DIMENSION 32767
#define LW_SCRIPT_MILLISECONDS 50

typedef struct lw_style {
    int row;
    int flex;
    int width;
    int height;
    int width_percent;
    int height_percent;
    int hidden;
    int scroll;
    int background_set;
    unsigned background;
} lw_style_t;

typedef struct lw_node {
    int parent, first_child, next_sibling;
    int kind; /* block=1, button=2, input=3 */
    char id[LW_ID_CAP];
    char text[LW_TEXT_CAP];
    char onclick[LW_ATTR_CAP];
    char oninput[LW_ATTR_CAP];
    char value[LW_TEXT_CAP];
    lw_style_t style;
    ui_rect_t rect;
    ui_rect_t clip;
    int visible;
    int scroll_y, content_height;
    HWND edit;
} lw_node_t;

typedef struct lw_backend {
    ui_web_backend_t *common;
    HWND parent;
    int enable_native_input;
    struct lw_backend *next;
} lw_backend_t;

typedef struct lw_view {
    lw_backend_t *backend;
    ui_host_t *host;
    lxb_html_document_t *document;
    JSRuntime *runtime;
    JSContext *context;
    lw_node_t nodes[LW_MAX_NODES];
    int node_count;
    int root;
    int x, y, width, height;
    uint32_t dpi;
    int media_breakpoint;
    int media_hide_assistant;
    int focused, pressed;
    int suppress_edit;
    ULONGLONG script_deadline;
    HWND hwnd;
    HFONT font;
} lw_view_t;

static lw_backend_t *g_backends;
static ui_status_t lw_dispatch_input(void *user, void *data,
                                      const ui_input_event_t *event);
static void lw_layout(lw_view_t *view);

static int lw_name(const char *a, size_t n, const char *b)
{
    return strlen(b) == n && _strnicmp(a, b, n) == 0;
}

static wchar_t *lw_wide(const char *s)
{
    int n;
    wchar_t *w;
    if (s == NULL) return NULL;
    n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, NULL, 0);
    if (n <= 0) return NULL;
    w = (wchar_t *)malloc((size_t)n * sizeof(*w));
    if (w != NULL && !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                         s, -1, w, n)) {
        free(w); return NULL;
    }
    return w;
}

static ui_status_t lw_copy(char *dst, size_t cap, const lxb_char_t *s,
                            size_t length)
{
    if (s == NULL) length = 0;
    if (length >= cap) return UI_STATUS_VALIDATION_FAILED;
    if (length != 0) memcpy(dst, s, length);
    dst[length] = 0;
    return UI_STATUS_OK;
}

static ui_status_t lw_attr(lxb_dom_element_t *element, const char *name,
                            char *out, size_t cap)
{
    size_t length = 0;
    const lxb_char_t *s = lxb_dom_element_get_attribute(element,
        (const lxb_char_t *)name, strlen(name), &length);
    return lw_copy(out, cap, s, length);
}

static ui_status_t lw_validate_attributes(lxb_dom_element_t *element)
{
    lxb_dom_attr_t *attr;
    for (attr = lxb_dom_element_first_attribute(element); attr;
         attr = lxb_dom_element_next_attribute(attr)) {
        size_t length;
        const lxb_char_t *name = lxb_dom_attr_qualified_name(attr, &length);
        if (!lw_name((const char *)name, length, "id") &&
            !lw_name((const char *)name, length, "style") &&
            !lw_name((const char *)name, length, "onclick") &&
            !lw_name((const char *)name, length, "oninput") &&
            !lw_name((const char *)name, length, "value") &&
            !lw_name((const char *)name, length, "type")) return UI_STATUS_UNSUPPORTED;
    }
    return UI_STATUS_OK;
}

static char *lw_trim(char *s)
{
    size_t length;
    while (isspace((unsigned char)*s)) ++s;
    length = strlen(s);
    while (length != 0 && isspace((unsigned char)s[length - 1])) s[--length] = 0;
    return s;
}

static ui_status_t lw_number(const char *s, int *number, int *percent)
{
    char *end;
    long value = strtol(s, &end, 10);
    if (end == s || value < 0 || value > LW_MAX_DIMENSION)
        return UI_STATUS_VALIDATION_FAILED;
    *percent = 0;
    if (*end == '%') { *percent = 1; ++end; }
    else if (end[0] == 'p' && end[1] == 'x') end += 2;
    if (*end != 0 || (*percent && value > 100)) return UI_STATUS_UNSUPPORTED;
    *number = (int)value;
    return UI_STATUS_OK;
}

static ui_status_t lw_parse_style(lw_style_t *style, char *text)
{
    char *item = text;
    memset(style, 0, sizeof(*style));
    style->width = style->height = -1;
    while (*item) {
        char *end = strchr(item, ';'), *colon, *key, *value;
        ui_status_t status;
        int number, percent;
        if (end != NULL) *end = 0;
        key = lw_trim(item);
        if (*key) {
            colon = strchr(key, ':');
            if (colon == NULL) return UI_STATUS_VALIDATION_FAILED;
            *colon = 0; key = lw_trim(key); value = lw_trim(colon + 1);
            if (_stricmp(key, "display") == 0) {
                if (_stricmp(value, "none") == 0) style->hidden = 1;
                else if (_stricmp(value, "flex") != 0 && _stricmp(value, "block") != 0)
                    return UI_STATUS_UNSUPPORTED;
            } else if (_stricmp(key, "flex-direction") == 0) {
                if (_stricmp(value, "row") == 0) style->row = 1;
                else if (_stricmp(value, "column") != 0) return UI_STATUS_UNSUPPORTED;
            } else if (_stricmp(key, "flex") == 0) {
                status = lw_number(value, &number, &percent);
                if (status != UI_STATUS_OK) return status;
                if (percent || strstr(value, "px") || number > 1000)
                    return UI_STATUS_UNSUPPORTED;
                style->flex = number;
            } else if (_stricmp(key, "width") == 0 || _stricmp(key, "height") == 0) {
                int width = _stricmp(key, "width") == 0;
                status = lw_number(value, &number, &percent);
                if (status != UI_STATUS_OK) return status;
                if (width) { style->width = number; style->width_percent = percent; }
                else { style->height = number; style->height_percent = percent; }
            } else if (_stricmp(key, "overflow-y") == 0) {
                if (_stricmp(value, "auto") != 0 && _stricmp(value, "scroll") != 0)
                    return UI_STATUS_UNSUPPORTED;
                style->scroll = 1;
            } else if (_stricmp(key, "background") == 0 || _stricmp(key, "background-color") == 0) {
                char *color_end;
                unsigned long color;
                if (strlen(value) != 7 || value[0] != '#') return UI_STATUS_UNSUPPORTED;
                color = strtoul(value + 1, &color_end, 16);
                if (*color_end || color > 0xffffffu) return UI_STATUS_VALIDATION_FAILED;
                style->background = (unsigned)color; style->background_set = 1;
            } else return UI_STATUS_UNSUPPORTED;
        }
        if (end == NULL) break;
        item = end + 1;
    }
    return UI_STATUS_OK;
}

static ui_status_t lw_stylesheet(lw_view_t *view, const char *text)
{
    char compact[LW_ATTR_CAP];
    size_t at = 0;
    const char *p;
    char *end;
    long breakpoint;
    while (*text) {
        if (!isspace((unsigned char)*text)) {
            if (at + 1 >= sizeof(compact)) return UI_STATUS_VALIDATION_FAILED;
            compact[at++] = *text;
        }
        ++text;
    }
    compact[at] = 0;
    if (at == 0) return UI_STATUS_OK;
    if (view->media_hide_assistant || strncmp(compact, "@media(max-width:", 17) != 0)
        return UI_STATUS_UNSUPPORTED;
    p = compact + 17;
    breakpoint = strtol(p, &end, 10);
    if (end == p || breakpoint <= 0 || breakpoint > LW_MAX_DIMENSION)
        return UI_STATUS_VALIDATION_FAILED;
    if (strcmp(end, "px){#assistant{display:none}}") != 0 &&
        strcmp(end, "px){#assistant{display:none;}}") != 0)
        return UI_STATUS_UNSUPPORTED;
    view->media_breakpoint = (int)breakpoint;
    view->media_hide_assistant = 1;
    return UI_STATUS_OK;
}

static ui_status_t lw_collect(lw_view_t *view, lxb_dom_node_t *parent,
                               int parent_index)
{
    lxb_dom_node_t *child;
    int previous = -1;
    for (child = lxb_dom_node_first_child(parent); child; child = lxb_dom_node_next(child)) {
        const lxb_char_t *name;
        size_t length;
        lw_node_t *node;
        char style[LW_ATTR_CAP], type[32];
        ui_status_t status;
        int index;
        if (child->type != LXB_DOM_NODE_TYPE_ELEMENT) continue;
        name = lxb_dom_node_name(child, &length);
        if (lw_name((const char *)name, length, "script") ||
            lw_name((const char *)name, length, "style")) continue;
        if (view->node_count >= LW_MAX_NODES) return UI_STATUS_VALIDATION_FAILED;
        index = view->node_count++;
        node = &view->nodes[index];
        memset(node, 0, sizeof(*node));
        node->parent = parent_index; node->first_child = node->next_sibling = -1;
        if (lw_name((const char *)name, length, "button")) node->kind = 2;
        else if (lw_name((const char *)name, length, "input")) node->kind = 3;
        else if (lw_name((const char *)name, length, "div") ||
                 lw_name((const char *)name, length, "aside") ||
                 lw_name((const char *)name, length, "section") ||
                 lw_name((const char *)name, length, "main") ||
                 lw_name((const char *)name, length, "p") ||
                 lw_name((const char *)name, length, "span")) node->kind = 1;
        else return UI_STATUS_UNSUPPORTED;
        status = lw_validate_attributes(lxb_dom_interface_element(child));
        if (status != UI_STATUS_OK) return status;
#define LW_READ_ATTR(name_, field_) do { \
    status = lw_attr(lxb_dom_interface_element(child), name_, field_, sizeof(field_)); \
    if (status != UI_STATUS_OK) return status; \
} while (0)
        LW_READ_ATTR("id", node->id);
        LW_READ_ATTR("onclick", node->onclick);
        LW_READ_ATTR("oninput", node->oninput);
        LW_READ_ATTR("value", node->value);
        LW_READ_ATTR("style", style);
        LW_READ_ATTR("type", type);
#undef LW_READ_ATTR
        if (node->kind == 3 && *type && _stricmp(type, "text") != 0)
            return UI_STATUS_UNSUPPORTED;
        status = lw_parse_style(&node->style, style);
        if (status != UI_STATUS_OK) return status;
        if (*node->id) {
            int j;
            for (j = 0; j < index; ++j)
                if (strcmp(view->nodes[j].id, node->id) == 0) return UI_STATUS_ALREADY_EXISTS;
        }
        if (parent_index < 0) {
            if (view->root >= 0) return UI_STATUS_UNSUPPORTED;
            view->root = index;
        } else if (previous < 0) view->nodes[parent_index].first_child = index;
        else view->nodes[previous].next_sibling = index;
        previous = index;
        status = lw_collect(view, child, index);
        if (status != UI_STATUS_OK) return status;
        if (node->first_child < 0 && node->kind != 3) {
            lxb_char_t *text = lxb_dom_node_text_content(child, &length);
            if (text == NULL) return UI_STATUS_OUT_OF_MEMORY;
            status = lw_copy(node->text, sizeof(node->text), text, length);
            lxb_dom_document_destroy_text(child->owner_document, text);
            if (status != UI_STATUS_OK) return status;
        }
        if (node->kind != 1 && node->first_child >= 0) return UI_STATUS_UNSUPPORTED;
    }
    return UI_STATUS_OK;
}

static int lw_pixel(int logical, uint32_t dpi)
{
    int64_t value = (int64_t)logical * dpi;
    return (int)((value + (value < 0 ? -48 : 48)) / 96);
}

static ui_rect_t lw_intersect(ui_rect_t a, ui_rect_t b)
{
    ui_rect_t r;
    int right = a.x + a.width < b.x + b.width ? a.x + a.width : b.x + b.width;
    int bottom = a.y + a.height < b.y + b.height ? a.y + a.height : b.y + b.height;
    r.x = a.x > b.x ? a.x : b.x; r.y = a.y > b.y ? a.y : b.y;
    r.width = right > r.x ? right - r.x : 0;
    r.height = bottom > r.y ? bottom - r.y : 0;
    return r;
}

static int lw_size(const lw_node_t *node, int available, int horizontal)
{
    int value = horizontal ? node->style.width : node->style.height;
    int percent = horizontal ? node->style.width_percent : node->style.height_percent;
    if (value < 0) return horizontal ? 120 : 32;
    return percent ? available * value / 100 : value;
}

static void lw_layout_node(lw_view_t *view, int index, ui_rect_t rect, ui_rect_t clip)
{
    lw_node_t *node = &view->nodes[index];
    int child, fixed = 0, weights = 0, cursor, axis;
    node->rect = rect; node->clip = lw_intersect(rect, clip);
    if (!node->visible) { memset(&node->rect, 0, sizeof(node->rect)); return; }
    axis = node->style.row ? rect.width : rect.height;
    for (child = node->first_child; child >= 0; child = view->nodes[child].next_sibling) {
        lw_node_t *c = &view->nodes[child];
        if (!c->visible) continue;
        if (c->style.flex) weights += c->style.flex;
        else fixed += lw_size(c, axis, node->style.row);
    }
    node->content_height = node->style.row ? rect.height : fixed;
    if (weights && node->content_height < rect.height) node->content_height = rect.height;
    if (node->scroll_y > node->content_height - rect.height)
        node->scroll_y = node->content_height - rect.height;
    if (node->scroll_y < 0) node->scroll_y = 0;
    cursor = node->style.row ? rect.x : rect.y - node->scroll_y;
    for (child = node->first_child; child >= 0; child = view->nodes[child].next_sibling) {
        lw_node_t *c = &view->nodes[child];
        ui_rect_t child_rect;
        int size, cross;
        if (!c->visible) { memset(&c->rect, 0, sizeof(c->rect)); continue; }
        size = c->style.flex ? ((axis > fixed ? axis - fixed : 0) * c->style.flex / weights)
                             : lw_size(c, axis, node->style.row);
        cross = node->style.row ? rect.height : rect.width;
        if (node->style.row && c->style.height >= 0) cross = lw_size(c, rect.height, 0);
        if (!node->style.row && c->style.width >= 0) cross = lw_size(c, rect.width, 1);
        child_rect.x = node->style.row ? cursor : rect.x;
        child_rect.y = node->style.row ? rect.y : cursor;
        child_rect.width = node->style.row ? size : cross;
        child_rect.height = node->style.row ? cross : size;
        lw_layout_node(view, child, child_rect, node->clip);
        cursor += size;
    }
}

static void lw_layout(lw_view_t *view)
{
    int i;
    ui_rect_t viewport = {0, 0, view->width, view->height};
    for (i = 0; i < view->node_count; ++i) {
        lw_node_t *n = &view->nodes[i];
        n->visible = !n->style.hidden;
        if (view->media_hide_assistant && view->width <= view->media_breakpoint &&
            strcmp(n->id, "assistant") == 0) n->visible = 0;
        if (n->parent >= 0 && !view->nodes[n->parent].visible) n->visible = 0;
        if (!n->visible) memset(&n->rect, 0, sizeof(n->rect));
    }
    if (view->root >= 0) lw_layout_node(view, view->root, viewport, viewport);
    for (i = 0; i < view->node_count; ++i) {
        lw_node_t *n = &view->nodes[i];
        if (n->edit == NULL) continue;
        ShowWindow(n->edit, n->visible && n->clip.width && n->clip.height ? SW_SHOW : SW_HIDE);
        SetWindowPos(n->edit, NULL, lw_pixel(n->rect.x, view->dpi),
            lw_pixel(n->rect.y, view->dpi), lw_pixel(n->rect.width, view->dpi),
            lw_pixel(n->rect.height, view->dpi), SWP_NOZORDER | SWP_NOACTIVATE);
        {
            HRGN clip = CreateRectRgn(lw_pixel(n->clip.x - n->rect.x, view->dpi),
                lw_pixel(n->clip.y - n->rect.y, view->dpi),
                lw_pixel(n->clip.x + n->clip.width - n->rect.x, view->dpi),
                lw_pixel(n->clip.y + n->clip.height - n->rect.y, view->dpi));
            if (clip && !SetWindowRgn(n->edit, clip, TRUE)) DeleteObject(clip);
        }
        SendMessageW(n->edit, WM_SETFONT, (WPARAM)view->font, TRUE);
    }
    if (view->hwnd) InvalidateRect(view->hwnd, NULL, FALSE);
}

static int lw_interrupt(JSRuntime *runtime, void *opaque)
{
    lw_view_t *view = (lw_view_t *)opaque;
    (void)runtime;
    return GetTickCount64() >= view->script_deadline;
}

static JSValue lw_js_invoke(JSContext *ctx, JSValueConst this_value,
                             int argc, JSValueConst *argv)
{
    lw_view_t *view = (lw_view_t *)JS_GetContextOpaque(ctx);
    const char *command, *params;
    JSValue json = JS_UNDEFINED;
    uint64_t request;
    (void)this_value;
    if (argc < 1) return JS_ThrowTypeError(ctx, "ui.invoke requires command id");
    command = JS_ToCString(ctx, argv[0]);
    if (command == NULL) return JS_EXCEPTION;
    if (argc > 1) {
        if (JS_IsString(argv[1])) json = JS_DupValue(ctx, argv[1]);
        else json = JS_JSONStringify(ctx, argv[1], JS_UNDEFINED, JS_UNDEFINED);
    } else json = JS_NewString(ctx, "{}");
    params = JS_ToCString(ctx, json);
    if (params == NULL) { JS_FreeCString(ctx, command); JS_FreeValue(ctx, json); return JS_EXCEPTION; }
    request = ui_host_invoke(view->host, command, params, "light-web");
    JS_FreeCString(ctx, params); JS_FreeCString(ctx, command); JS_FreeValue(ctx, json);
    return JS_NewInt64(ctx, (int64_t)request);
}

static JSValue lw_js_value(JSContext *ctx, JSValueConst this_value,
                            int argc, JSValueConst *argv)
{
    lw_view_t *view = (lw_view_t *)JS_GetContextOpaque(ctx);
    const char *id;
    int i;
    (void)this_value;
    if (argc != 1) return JS_ThrowTypeError(ctx, "ui.value(id) is a getter");
    id = JS_ToCString(ctx, argv[0]);
    if (id == NULL) return JS_EXCEPTION;
    for (i = 0; i < view->node_count; ++i)
        if (view->nodes[i].kind == 3 && strcmp(view->nodes[i].id, id) == 0) {
            JS_FreeCString(ctx, id); return JS_NewString(ctx, view->nodes[i].value);
        }
    JS_FreeCString(ctx, id);
    return JS_ThrowReferenceError(ctx, "input id not found");
}

static void lw_free_js(lw_view_t *view)
{
    if (view->context) JS_FreeContext(view->context);
    if (view->runtime) JS_FreeRuntime(view->runtime);
    view->context = NULL; view->runtime = NULL;
}

static ui_status_t lw_setup_js(lw_view_t *view)
{
    JSValue global, ui;
    view->runtime = JS_NewRuntime();
    if (view->runtime == NULL) return UI_STATUS_OUT_OF_MEMORY;
    JS_SetMemoryLimit(view->runtime, 8u * 1024u * 1024u);
    JS_SetMaxStackSize(view->runtime, 256u * 1024u);
    JS_SetInterruptHandler(view->runtime, lw_interrupt, view);
    view->script_deadline = GetTickCount64() + LW_SCRIPT_MILLISECONDS;
    view->context = JS_NewContext(view->runtime);
    if (view->context == NULL) { lw_free_js(view); return UI_STATUS_OUT_OF_MEMORY; }
    JS_SetContextOpaque(view->context, view);
    global = JS_GetGlobalObject(view->context); ui = JS_NewObject(view->context);
    JS_SetPropertyStr(view->context, ui, "invoke", JS_NewCFunction(view->context, lw_js_invoke, "invoke", 2));
    JS_SetPropertyStr(view->context, ui, "value", JS_NewCFunction(view->context, lw_js_value, "value", 1));
    JS_SetPropertyStr(view->context, global, "ui", ui);
    JS_FreeValue(view->context, global);
    return UI_STATUS_OK;
}

static ui_status_t lw_eval(lw_view_t *view, const char *script)
{
    JSValue result;
    ui_status_t status = UI_STATUS_OK;
    if (script == NULL || *script == 0) return status;
    view->script_deadline = GetTickCount64() + LW_SCRIPT_MILLISECONDS;
    result = JS_Eval(view->context, script, strlen(script), "application-inline", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(view->context);
        JS_FreeValue(view->context, exception);
        status = UI_STATUS_VALIDATION_FAILED;
    }
    JS_FreeValue(view->context, result);
    return status;
}

static ui_status_t lw_handler(lw_view_t *view, lw_node_t *node, const char *script)
{
    JSContext *ctx = view->context;
    JSValue global, event, target;
    ui_status_t status;
    global = JS_GetGlobalObject(ctx); event = JS_NewObject(ctx); target = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, target, "id", JS_NewString(ctx, node->id));
    JS_SetPropertyStr(ctx, target, "value", JS_NewString(ctx, node->value));
    JS_SetPropertyStr(ctx, event, "target", target);
    JS_SetPropertyStr(ctx, global, "event", event); JS_FreeValue(ctx, global);
    status = lw_eval(view, script);
    global = JS_GetGlobalObject(ctx); JS_SetPropertyStr(ctx, global, "event", JS_UNDEFINED); JS_FreeValue(ctx, global);
    return status;
}

static ui_status_t lw_collect_scripts(lw_view_t *view, lxb_dom_node_t *parent,
                                       int execute, int depth)
{
    lxb_dom_node_t *child;
    if (depth > LW_MAX_NODES) return UI_STATUS_VALIDATION_FAILED;
    for (child = lxb_dom_node_first_child(parent); child; child = lxb_dom_node_next(child)) {
        size_t length;
        const lxb_char_t *name = lxb_dom_node_name(child, &length);
        int style = name && lw_name((const char *)name, length, "style");
        int script = name && lw_name((const char *)name, length, "script");
        ui_status_t status;
        if (script) {
            char src[LW_ATTR_CAP];
            status = lw_attr(lxb_dom_interface_element(child), "src", src, sizeof(src));
            if (status != UI_STATUS_OK) return status;
            if (*src) return UI_STATUS_UNSUPPORTED;
        }
        if ((style && !execute) || (script && execute)) {
            lxb_char_t *text = lxb_dom_node_text_content(child, &length);
            char *copy;
            if (length > 64u * 1024u) return UI_STATUS_VALIDATION_FAILED;
            if (text == NULL) return UI_STATUS_OUT_OF_MEMORY;
            copy = (char *)malloc(length + 1);
            if (copy == NULL) { lxb_dom_document_destroy_text(child->owner_document, text); return UI_STATUS_OUT_OF_MEMORY; }
            memcpy(copy, text, length); copy[length] = 0;
            lxb_dom_document_destroy_text(child->owner_document, text);
            status = style ? lw_stylesheet(view, copy) : lw_eval(view, copy);
            free(copy);
        } else status = lw_collect_scripts(view, child, execute, depth + 1);
        if (status != UI_STATUS_OK) return status;
    }
    return UI_STATUS_OK;
}

static void lw_free_document(lw_view_t *view)
{
    int i;
    for (i = 0; i < view->node_count; ++i) if (view->nodes[i].edit) DestroyWindow(view->nodes[i].edit);
    if (view->document) lxb_html_document_destroy(view->document);
    view->document = NULL; view->node_count = 0; view->root = -1;
    view->media_breakpoint = 0; view->media_hide_assistant = 0;
    view->focused = view->pressed = -1;
    lw_free_js(view);
}

static RECT lw_pixel_rect(const lw_view_t *view, ui_rect_t rect)
{
    RECT r = {lw_pixel(rect.x, view->dpi), lw_pixel(rect.y, view->dpi),
              lw_pixel(rect.x + rect.width, view->dpi), lw_pixel(rect.y + rect.height, view->dpi)};
    return r;
}

static ui_status_t lw_edit_changed(lw_view_t *view, int index)
{
    lw_node_t *node = &view->nodes[index];
    wchar_t wide[LW_TEXT_CAP];
    int bytes, length = GetWindowTextLengthW(node->edit);
    if (length >= LW_TEXT_CAP) return UI_STATUS_VALIDATION_FAILED;
    GetWindowTextW(node->edit, wide, LW_TEXT_CAP);
    bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide, -1, NULL, 0, NULL, NULL);
    if (bytes <= 0 || bytes > LW_TEXT_CAP) return UI_STATUS_VALIDATION_FAILED;
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide, -1, node->value, LW_TEXT_CAP, NULL, NULL);
    view->focused = index;
    return lw_handler(view, node, node->oninput);
}

static LRESULT CALLBACK lw_wnd_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    lw_view_t *view = (lw_view_t *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    int i;
    if (message == WM_NCCREATE) {
        view = (lw_view_t *)((CREATESTRUCTW *)lp)->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)view); view->hwnd = hwnd; return TRUE;
    }
    if (view == NULL) return DefWindowProcW(hwnd, message, wp, lp);
    if (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_MOUSEWHEEL) {
        ui_input_event_t event = {0};
        POINT point = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        if (message == WM_MOUSEWHEEL) ScreenToClient(hwnd, &point);
        event.size = sizeof(event); event.x = point.x * 96 / (int)view->dpi;
        event.y = point.y * 96 / (int)view->dpi; event.pointer_button = 1;
        event.kind = message == WM_LBUTTONDOWN ? UI_INPUT_POINTER_DOWN :
                     message == WM_LBUTTONUP ? UI_INPUT_POINTER_UP : UI_INPUT_WHEEL;
        event.wheel_delta = GET_WHEEL_DELTA_WPARAM(wp);
        (void)lw_dispatch_input(NULL, view, &event); return 0;
    }
    if (message == WM_COMMAND && HIWORD(wp) == EN_CHANGE && !view->suppress_edit) {
        for (i = 0; i < view->node_count; ++i)
            if (view->nodes[i].edit == (HWND)lp) {
                if (lw_edit_changed(view, i) != UI_STATUS_OK) {
                    wchar_t *old = lw_wide(view->nodes[i].value);
                    view->suppress_edit = 1; if (old) SetWindowTextW((HWND)lp, old);
                    view->suppress_edit = 0; free(old);
                }
                break;
            }
        return 0;
    }
    if (message == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT client;
        HBRUSH brush = CreateSolidBrush(RGB(32, 34, 38));
        GetClientRect(hwnd, &client); FillRect(dc, &client, brush); DeleteObject(brush);
        SelectObject(dc, view->font); SetBkMode(dc, TRANSPARENT);
        for (i = 0; i < view->node_count; ++i) {
            lw_node_t *n = &view->nodes[i];
            RECT r, clip;
            int saved;
            wchar_t *text;
            if (!n->visible || !n->clip.width || !n->clip.height || n->kind == 3) continue;
            r = lw_pixel_rect(view, n->rect); clip = lw_pixel_rect(view, n->clip);
            saved = SaveDC(dc); IntersectClipRect(dc, clip.left, clip.top, clip.right, clip.bottom);
            if (n->kind == 2 || n->style.background_set) {
                unsigned color = n->kind == 2 ? 0x3968a8u : n->style.background;
                brush = CreateSolidBrush(RGB((color >> 16) & 255, (color >> 8) & 255, color & 255));
                FillRect(dc, &r, brush); DeleteObject(brush);
            }
            SetTextColor(dc, RGB(235, 235, 235));
            text = lw_wide(n->text);
            if (text) {
                r.left += lw_pixel(8, view->dpi); r.right -= lw_pixel(8, view->dpi);
                DrawTextW(dc, text, -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                free(text);
            }
            RestoreDC(dc, saved);
        }
        EndPaint(hwnd, &ps); return 0;
    }
    return DefWindowProcW(hwnd, message, wp, lp);
}

static ui_status_t lw_create_edits(lw_view_t *view)
{
    int i;
    if (!view->backend->enable_native_input || !view->hwnd) return UI_STATUS_OK;
    view->suppress_edit = 1;
    for (i = 0; i < view->node_count; ++i) if (view->nodes[i].kind == 3) {
        lw_node_t *node = &view->nodes[i];
        wchar_t *value = lw_wide(node->value);
        if (!value) { view->suppress_edit = 0; return UI_STATUS_VALIDATION_FAILED; }
        node->edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", value,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 1, 1,
            view->hwnd, (HMENU)(INT_PTR)(1000 + i), GetModuleHandleW(NULL), NULL);
        free(value);
        if (!node->edit) { view->suppress_edit = 0; return UI_STATUS_PLATFORM_ERROR; }
        SendMessageW(node->edit, EM_SETLIMITTEXT, (LW_TEXT_CAP - 1) / 3, 0);
    }
    view->suppress_edit = 0;
    return UI_STATUS_OK;
}

static ui_status_t lw_create_view(void *user, ui_host_t *host, void **out)
{
    lw_backend_t *backend = (lw_backend_t *)user;
    lw_view_t *view;
    WNDCLASSW wc = {0};
    if (!backend || !out) return UI_STATUS_INVALID_ARGUMENT;
    view = (lw_view_t *)calloc(1, sizeof(*view));
    if (!view) return UI_STATUS_OUT_OF_MEMORY;
    view->backend = backend; view->host = host; view->dpi = 96;
    view->root = view->focused = view->pressed = -1;
    view->font = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, L"Segoe UI");
    if (backend->parent) {
        wc.lpfnWndProc = lw_wnd_proc; wc.hInstance = GetModuleHandleW(NULL);
        wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512)); wc.lpszClassName = L"UIFrameworkLightWeb";
        if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            DeleteObject(view->font); free(view); return UI_STATUS_PLATFORM_ERROR;
        }
        view->hwnd = CreateWindowExW(0, wc.lpszClassName, L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
            0, 0, 1, 1, backend->parent, NULL, wc.hInstance, view);
        if (!view->hwnd) { DeleteObject(view->font); free(view); return UI_STATUS_PLATFORM_ERROR; }
    }
    *out = view; return UI_STATUS_OK;
}

static void lw_destroy_view(void *user, void *data)
{
    lw_view_t *view = (lw_view_t *)data;
    (void)user;
    if (!view) return;
    lw_free_document(view);
    if (view->hwnd) DestroyWindow(view->hwnd);
    DeleteObject(view->font); free(view);
}

static ui_status_t lw_load_html(void *user, void *data, const char *html)
{
    lw_view_t *view = (lw_view_t *)data;
    lxb_html_parser_t *parser;
    lxb_dom_node_t *body;
    size_t length;
    ui_status_t status;
    wchar_t *valid;
    (void)user;
    if (!view || !html) return UI_STATUS_INVALID_ARGUMENT;
    length = strlen(html);
    if (length > LW_HTML_LIMIT) return UI_STATUS_VALIDATION_FAILED;
    valid = lw_wide(html); if (!valid) return UI_STATUS_VALIDATION_FAILED; free(valid);
    lw_free_document(view);
    parser = lxb_html_parser_create();
    if (!parser) return UI_STATUS_OUT_OF_MEMORY;
    if (lxb_html_parser_init(parser) != LXB_STATUS_OK) {
        lxb_html_parser_destroy(parser); return UI_STATUS_OUT_OF_MEMORY;
    }
    view->document = lxb_html_parse(parser, (const lxb_char_t *)html, length);
    lxb_html_parser_destroy(parser);
    if (!view->document) return UI_STATUS_VALIDATION_FAILED;
    status = lw_setup_js(view);
    if (status == UI_STATUS_OK) status = lw_collect_scripts(view, lxb_dom_interface_node(view->document), 0, 0);
    body = lxb_dom_interface_node(lxb_html_document_body_element(view->document));
    if (status == UI_STATUS_OK) status = lw_collect(view, body, -1);
    if (status == UI_STATUS_OK) status = lw_collect_scripts(view, lxb_dom_interface_node(view->document), 1, 0);
    if (status == UI_STATUS_OK) status = lw_create_edits(view);
    if (status != UI_STATUS_OK) lw_free_document(view);
    lw_layout(view);
    return status;
}

static ui_status_t lw_set_rect(void *user, void *data, const ui_rect_t *rect,
                                uint32_t dpi)
{
    lw_view_t *view = (lw_view_t *)data;
    HFONT font, old_font;
    int pixel_x, pixel_y, pixel_width, pixel_height;
    (void)user;
    if (!view || !rect || rect->width < 0 || rect->height < 0 || !dpi || dpi > 768 ||
        rect->width > LW_MAX_DIMENSION || rect->height > LW_MAX_DIMENSION ||
        rect->x < -LW_MAX_DIMENSION || rect->x > LW_MAX_DIMENSION ||
        rect->y < -LW_MAX_DIMENSION || rect->y > LW_MAX_DIMENSION)
        return UI_STATUS_INVALID_ARGUMENT;
    pixel_x = lw_pixel(rect->x, dpi); pixel_y = lw_pixel(rect->y, dpi);
    pixel_width = lw_pixel(rect->x + rect->width, dpi) - pixel_x;
    pixel_height = lw_pixel(rect->y + rect->height, dpi) - pixel_y;
    font = CreateFontW(-lw_pixel(14, dpi), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, L"Segoe UI");
    if (!font) return UI_STATUS_PLATFORM_ERROR;
    if (view->hwnd && !SetWindowPos(view->hwnd, NULL, pixel_x, pixel_y,
        pixel_width, pixel_height, SWP_NOZORDER | SWP_NOACTIVATE)) {
        DeleteObject(font); return UI_STATUS_PLATFORM_ERROR;
    }
    view->x = rect->x; view->y = rect->y;
    view->width = rect->width; view->height = rect->height; view->dpi = dpi;
    old_font = view->font; view->font = font;
    lw_layout(view); DeleteObject(old_font); return UI_STATUS_OK;
}

static ui_status_t lw_resize(void *user, void *data, int width, int height, uint32_t dpi)
{
    lw_view_t *view = (lw_view_t *)data;
    ui_rect_t rect;
    if (!view) return UI_STATUS_INVALID_ARGUMENT;
    rect.x = view->x; rect.y = view->y; rect.width = width; rect.height = height;
    return lw_set_rect(user, view, &rect, dpi);
}

static int lw_hit(const lw_view_t *view, int x, int y)
{
    int i;
    for (i = view->node_count - 1; i >= 0; --i) {
        const lw_node_t *n = &view->nodes[i];
        if (n->visible && x >= n->clip.x && y >= n->clip.y &&
            x < n->clip.x + n->clip.width && y < n->clip.y + n->clip.height) return i;
    }
    return -1;
}

static ui_status_t lw_dispatch_input(void *user, void *data, const ui_input_event_t *event)
{
    lw_view_t *view = (lw_view_t *)data;
    int hit;
    (void)user;
    if (!view || !event || event->size < sizeof(*event)) return UI_STATUS_INVALID_ARGUMENT;
    if (!view->context) return UI_STATUS_NOT_FOUND;
    hit = lw_hit(view, event->x, event->y);
    if (event->kind == UI_INPUT_POINTER_DOWN || event->kind == UI_INPUT_POINTER_UP) {
        if (event->pointer_button > 1) return UI_STATUS_UNSUPPORTED;
        if (event->kind == UI_INPUT_POINTER_DOWN) {
            view->pressed = hit;
            view->focused = hit >= 0 && view->nodes[hit].kind == 3 ? hit : -1;
            if (view->focused >= 0 && view->nodes[hit].edit) SetFocus(view->nodes[hit].edit);
            return UI_STATUS_OK;
        }
        if (hit >= 0 && hit == view->pressed && view->nodes[hit].kind == 2) {
            view->pressed = -1; return lw_handler(view, &view->nodes[hit], view->nodes[hit].onclick);
        }
        view->pressed = -1; return UI_STATUS_OK;
    }
    if (event->kind == UI_INPUT_POINTER_MOVE) return UI_STATUS_OK;
    if (event->kind == UI_INPUT_WHEEL) {
        while (hit >= 0 && !view->nodes[hit].style.scroll) hit = view->nodes[hit].parent;
        if (hit < 0) return UI_STATUS_UNSUPPORTED;
        {
            int64_t scroll = (int64_t)view->nodes[hit].scroll_y - (int64_t)event->wheel_delta * 48 / 120;
            view->nodes[hit].scroll_y = scroll < 0 ? 0 : scroll > LW_MAX_DIMENSION * LW_MAX_NODES
                ? LW_MAX_DIMENSION * LW_MAX_NODES : (int)scroll;
        }
        lw_layout(view); return UI_STATUS_OK;
    }
    if (event->kind == UI_INPUT_TEXT || (event->kind == UI_INPUT_KEY_DOWN && event->key_code == VK_BACK)) {
        lw_node_t *n;
        size_t length;
        wchar_t *wide;
        if (view->focused < 0) return UI_STATUS_NOT_FOUND;
        n = &view->nodes[view->focused]; length = strlen(n->value);
        if (event->kind == UI_INPUT_TEXT) {
            size_t added;
            if (!event->text_utf8) return UI_STATUS_INVALID_ARGUMENT;
            added = strlen(event->text_utf8);
            wide = lw_wide(event->text_utf8);
            if (!wide || length + added >= sizeof(n->value)) { free(wide); return UI_STATUS_VALIDATION_FAILED; }
            free(wide); memcpy(n->value + length, event->text_utf8, added + 1);
        } else if (length) {
            do { --length; } while (length && ((unsigned char)n->value[length] & 0xc0) == 0x80);
            n->value[length] = 0;
        }
        if (n->edit) {
            wide = lw_wide(n->value); view->suppress_edit = 1;
            if (wide) SetWindowTextW(n->edit, wide);
            view->suppress_edit = 0; free(wide);
        }
        return lw_handler(view, n, n->oninput);
    }
    return UI_STATUS_UNSUPPORTED;
}

static ui_status_t lw_invalidate(void *user, void *data)
{
    lw_view_t *view = (lw_view_t *)data;
    (void)user;
    if (!view) return UI_STATUS_INVALID_ARGUMENT;
    if (view->hwnd) InvalidateRect(view->hwnd, NULL, FALSE);
    return UI_STATUS_OK;
}

static ui_status_t lw_get_element_rect(void *user, void *data, const char *id,
                                       ui_rect_t *rect)
{
    lw_view_t *view = (lw_view_t *)data;
    int i;
    (void)user;
    if (!view || !id || !rect) return UI_STATUS_INVALID_ARGUMENT;
    for (i = 0; i < view->node_count; ++i) if (strcmp(view->nodes[i].id, id) == 0) {
        *rect = view->nodes[i].rect; return UI_STATUS_OK;
    }
    return UI_STATUS_NOT_FOUND;
}

static const ui_web_backend_ops_t lw_ops = {
    sizeof(ui_web_backend_ops_t), lw_create_view, lw_destroy_view, lw_load_html,
    lw_resize, lw_dispatch_input, lw_invalidate, lw_get_element_rect, lw_set_rect
};

ui_web_backend_t *ui_light_web_backend_create(const ui_light_web_config_t *config)
{
    lw_backend_t *backend;
    ui_web_backend_desc_t desc = {0};
    if (config && config->size < sizeof(*config)) return NULL;
    backend = (lw_backend_t *)calloc(1, sizeof(*backend));
    if (!backend) return NULL;
    backend->enable_native_input = config ? config->enable_native_input : 1;
    backend->parent = config ? (HWND)config->parent_hwnd : NULL;
    desc.size = sizeof(desc); desc.ops = &lw_ops; desc.user_data = backend;
    backend->common = ui_web_backend_create(&desc);
    if (!backend->common) { free(backend); return NULL; }
    backend->next = g_backends; g_backends = backend;
    return backend->common;
}

void ui_light_web_backend_destroy(ui_web_backend_t *handle)
{
    lw_backend_t **link = &g_backends;
    while (*link) {
        if ((*link)->common == handle) {
            lw_backend_t *backend = *link;
            *link = backend->next; ui_web_backend_destroy(backend->common); free(backend); return;
        }
        link = &(*link)->next;
    }
}

const char *ui_light_web_backend_capabilities(void)
{
    return "html:lexbor;js:quickjs-ng,50ms,8MiB;layout:row,column,px,percent,flex;controls:button,input;events:ui.invoke,ui.value,event.target;scroll:vertical;css:assistant-max-width;thread:ui";
}
