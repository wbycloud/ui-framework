#include "ui_framework/light_web.h"
#include "../ui_internal.h"

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
#include <imm.h>
#include <usp10.h>

#define LW_MAX_NODES 1024
#define LW_EVENTS 15
#define LW_MAX_RULES 256
#define LW_TEXT_CAP 4096
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
    int display_flex, absolute, left, top, right, bottom;
    int min_width, max_width, min_height, max_height;
    int padding[4], margin[4], gap, align, justify;
    int border, radius, font_size, font_weight, font_family, nowrap, text_align, scroll_x;
    unsigned color, border_color;
} lw_style_t;

typedef struct lw_rule {
    char selector[256];
    char declarations[LW_ATTR_CAP];
    int min_width, max_width, specificity;
} lw_rule_t;

typedef struct lw_node {
    int parent, first_child, next_sibling;
    int kind; /* block=1, button=2, input=3 */
    char id[LW_ID_CAP];
    char tag[24], classes[512], inline_style[LW_ATTR_CAP];
    unsigned uid;
    int used, attached, disabled, readonly, natural_width, natural_height;
    char text[LW_TEXT_CAP];
    char onclick[LW_ATTR_CAP];
    char onmousedown[LW_ATTR_CAP];
    char oninput[LW_ATTR_CAP];
    char value[LW_TEXT_CAP];
    lw_style_t style;
    ui_rect_t rect;
    ui_rect_t clip;
    int visible;
    int scroll_y, content_height, scroll_x, content_width;
    ui_rect_t viewport;
    int bar_x,bar_y;
    int caret, anchor, text_scroll_x, text_scroll_y;
    wchar_t composition[1024];
    struct lw_history *undo, *redo;
    uint64_t image;
    HFONT font;
    int font_size, font_weight, font_family;
    uint32_t font_dpi;
    JSValue object;
    JSValue listeners[LW_EVENTS]; /* click, input, keydown, focus, blur, mouseenter, mouseleave, mousedown */
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
    lw_node_t *nodes;
    int node_capacity;
    int node_count;
    int root;
    int x, y, width, height;
    uint32_t dpi;
    int media_breakpoint;
    int media_hide_assistant;
    int focused, pressed;
    int dragging, composing;
    wchar_t high_surrogate;
    uint32_t event_modifiers;
    int wheel_delta;
    int hovered, dispatch_depth, dirty;
    int default_prevented;
    int layout_depth;
    int pointer_x,pointer_y;
    unsigned captured;
    unsigned scroll_uid;
    int scroll_axis,scroll_origin,scroll_start,scroll_position,scroll_travel,scroll_max,scroll_focus;
    unsigned next_uid;
    lw_rule_t *rules;
    int rule_count, rule_capacity;
    ui_web_backend_message_fn message_handler;
    void *message_user;
    ULONGLONG script_deadline;
    HWND hwnd;
    HFONT font;
} lw_view_t;

static lw_backend_t *g_backends;
static void lw_report_exception(lw_view_t *view)
{
    JSValue exception=JS_GetException(view->context);const char *message=JS_ToCString(view->context,exception);
    if(GetEnvironmentVariableW(L"UI_LIGHT_SCRIPT_DIAGNOSTICS",NULL,0))fprintf(stderr,"Light script: %s\n",message?message:"unknown");
    if(message)JS_FreeCString(view->context,message);JS_FreeValue(view->context,exception);
}
static ui_status_t lw_dispatch_input(void *user, void *data,
                                      const ui_input_event_t *event);
static void lw_layout(lw_view_t *view);
static void lw_scroll_cancel(lw_view_t *,int);
static int lw_hit(const lw_view_t *view, int x, int y);
static int lw_disabled(const lw_view_t *,int);
static void lw_text_paint(lw_view_t *, lw_node_t *, HDC, RECT);
static void lw_text_free(lw_node_t *);
static void lw_caret(lw_view_t *);
static int lw_text_key(lw_view_t *, const ui_input_event_t *);
static ui_status_t lw_text_insert(lw_view_t *, const char *);
static void lw_text_pointer(lw_view_t *, int, int, int, int);
static int lw_input(const lw_node_t *n) { return n->kind == 3 || n->kind == 4; }
static ui_status_t lw_event(lw_view_t *view, int index, int kind, uint32_t key);

static void lw_cancel_composition(lw_view_t *view)
{
    if(view->composing){HIMC imc=ImmGetContext(view->hwnd);if(imc){ImmNotifyIME(imc,NI_COMPOSITIONSTR,CPS_CANCEL,0);ImmReleaseContext(view->hwnd,imc);}
        if(view->focused>=0)*view->nodes[view->focused].composition=0;view->composing=0;}
}
static ui_status_t lw_reserve_nodes(lw_view_t *view, int count)
{
    lw_node_t *nodes;
    int capacity = view->node_capacity ? view->node_capacity : 32;
    if (count > LW_MAX_NODES) return UI_STATUS_VALIDATION_FAILED;
    if (count <= view->node_capacity) return UI_STATUS_OK;
    while (capacity < count) capacity *= 2;
    if (capacity > LW_MAX_NODES) capacity = LW_MAX_NODES;
    nodes = (lw_node_t *)realloc(view->nodes,(size_t)capacity * sizeof(*nodes));
    if (!nodes) return UI_STATUS_OUT_OF_MEMORY;
    memset(nodes + view->node_capacity,0,(size_t)(capacity - view->node_capacity) * sizeof(*nodes));
    view->nodes = nodes; view->node_capacity = capacity;
    return UI_STATUS_OK;
}

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
            !lw_name((const char *)name, length, "class") &&
            !lw_name((const char *)name, length, "disabled") &&
            !lw_name((const char *)name, length, "readonly") &&
            !lw_name((const char *)name, length, "placeholder") &&
            !lw_name((const char *)name, length, "title") &&
            !lw_name((const char *)name, length, "aria-label") &&
            !lw_name((const char *)name, length, "style") &&
            !lw_name((const char *)name, length, "onclick") &&
            !lw_name((const char *)name, length, "onmousedown") &&
            !lw_name((const char *)name, length, "oninput") &&
            !lw_name((const char *)name, length, "value") &&
            !lw_name((const char *)name, length, "src") &&
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

static int lw_color(const char *value, unsigned *color)
{
    char *end;
    unsigned long parsed;
    if (!strcmp(value, "transparent")) return 2;
    if (*value != '#' || (strlen(value) != 7 && strlen(value) != 4)) return 0;
    parsed = strtoul(value + 1, &end, 16);
    if (*end) return 0;
    if (strlen(value) == 4) parsed = ((parsed >> 8) & 15) * 0x110000u +
        ((parsed >> 4) & 15) * 0x1100u + (parsed & 15) * 0x11u;
    *color = (unsigned)parsed;
    return 1;
}

static ui_status_t lw_box(const char *text, int values[4])
{
    char copy[128], *at, *end;
    int count = 0, numbers[4], percent;
    if (strlen(text) >= sizeof(copy)) return UI_STATUS_VALIDATION_FAILED;
    strcpy(copy, text); at = copy;
    while (*at && count < 4) {
        while (isspace((unsigned char)*at)) ++at;
        end = at;
        while (*end && !isspace((unsigned char)*end)) ++end;
        if (*end) *end++ = 0;
        if (lw_number(at, &numbers[count], &percent) != UI_STATUS_OK || percent)
            return UI_STATUS_UNSUPPORTED;
        ++count; at = end;
    }
    if (*at || !count) return UI_STATUS_UNSUPPORTED;
    values[0] = numbers[0]; values[1] = count > 1 ? numbers[1] : numbers[0];
    values[2] = count > 2 ? numbers[2] : numbers[0];
    values[3] = count > 3 ? numbers[3] : values[1];
    return UI_STATUS_OK;
}

static void lw_style_default(lw_style_t *style)
{
    memset(style, 0, sizeof(*style));
    style->width = style->height = -1;
    style->max_width = style->max_height = LW_MAX_DIMENSION;
    style->left = style->top = style->right = style->bottom = -1;
    style->font_size = 14; style->font_weight = FW_NORMAL;
    style->color = 0xebebebu; style->border_color = 0x687282u;
}

/* Apply declarations without clearing prior rules: defaults, cascade, inline. */
static ui_status_t lw_apply_style(lw_style_t *style, char *text)
{
    char *item = text;
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
                else if (_stricmp(value, "flex") == 0) { style->hidden = 0; style->display_flex = 1; }
                else if (_stricmp(value, "block") == 0) { style->hidden = 0; style->display_flex = 0; }
                else return UI_STATUS_UNSUPPORTED;
            } else if (_stricmp(key, "flex-direction") == 0) {
                if (_stricmp(value, "row") == 0) style->row = 1;
                else if (_stricmp(value, "column") == 0) style->row = 0;
                else return UI_STATUS_UNSUPPORTED;
            } else if (_stricmp(key, "flex") == 0 || _stricmp(key, "flex-grow") == 0) {
                status = lw_number(value, &number, &percent);
                if (status != UI_STATUS_OK) return status;
                if (percent || strstr(value, "px") || number > 1000)
                    return UI_STATUS_UNSUPPORTED;
                style->flex = number;
            } else if (_stricmp(key, "width") == 0 || _stricmp(key, "height") == 0) {
                int width = _stricmp(key, "width") == 0;
                if (!strcmp(value, "auto")) { if (width) style->width = -1; else style->height = -1; goto next; }
                status = lw_number(value, &number, &percent);
                if (status != UI_STATUS_OK) return status;
                if (width) { style->width = number; style->width_percent = percent; }
                else { style->height = number; style->height_percent = percent; }
            } else if (_stricmp(key, "overflow-y") == 0 || _stricmp(key, "overflow-x") == 0 || _stricmp(key, "overflow") == 0) {
                int scrolling;
                if (!strcmp(value, "auto") || !strcmp(value, "scroll")) scrolling = 1;
                else if (!strcmp(value, "hidden") || !strcmp(value, "visible")) scrolling = 0;
                else return UI_STATUS_UNSUPPORTED;
                if (strcmp(key,"overflow-x")) style->scroll = scrolling;
                if (strcmp(key,"overflow-y")) style->scroll_x = scrolling;
            } else if (_stricmp(key, "background") == 0 || _stricmp(key, "background-color") == 0) {
                int result = lw_color(value, &style->background);
                if (!result) return UI_STATUS_UNSUPPORTED;
                style->background_set = result == 1;
            } else if (!strcmp(key, "color") || !strcmp(key, "border-color")) {
                if (lw_color(value, !strcmp(key, "color") ? &style->color : &style->border_color) != 1)
                    return UI_STATUS_UNSUPPORTED;
            } else if (!strcmp(key, "padding") || !strcmp(key, "margin")) {
                status = lw_box(value, !strcmp(key, "padding") ? style->padding : style->margin);
                if (status != UI_STATUS_OK) return status;
            } else if (!strncmp(key, "padding-", 8) || !strncmp(key, "margin-", 7)) {
                int *box = !strncmp(key, "padding-", 8) ? style->padding : style->margin;
                const char *side = key + (!strncmp(key, "padding-", 8) ? 8 : 7);
                int slot = !strcmp(side,"top") ? 0 : !strcmp(side,"right") ? 1 :
                    !strcmp(side,"bottom") ? 2 : !strcmp(side,"left") ? 3 : -1;
                if (slot < 0 || lw_number(value, &number, &percent) != UI_STATUS_OK || percent) return UI_STATUS_UNSUPPORTED;
                box[slot] = number;
            } else if (!strcmp(key, "align-items") || !strcmp(key, "justify-content") || !strcmp(key, "text-align")) {
                int alignment = !strcmp(value,"center") ? 1 : (!strcmp(value,"end") || !strcmp(value,"flex-end") || !strcmp(value,"right")) ? 2 :
                    !strcmp(value,"space-between") ? 3 : !strcmp(value,"stretch") ? 4 :
                    (!strcmp(value,"start") || !strcmp(value,"flex-start") || !strcmp(value,"left")) ? 0 : -1;
                if (alignment < 0) return UI_STATUS_UNSUPPORTED;
                if (!strcmp(key,"align-items")) style->align = alignment;
                else if (!strcmp(key,"justify-content")) style->justify = alignment;
                else style->text_align = alignment;
            } else if (!strcmp(key,"position")) {
                if (!strcmp(value,"absolute")) style->absolute = 1;
                else if (!strcmp(value,"relative") || !strcmp(value,"static")) style->absolute = 0;
                else return UI_STATUS_UNSUPPORTED;
            } else if (!strcmp(key,"white-space")) {
                if (!strcmp(value,"nowrap")) style->nowrap = 1;
                else if (!strcmp(value,"normal") || !strcmp(value,"pre-wrap")) style->nowrap = 0;
                else return UI_STATUS_UNSUPPORTED;
            } else if (!strcmp(key,"font-family")) {
                if (strstr(value,"Segoe MDL2 Assets")) style->font_family = 2;
                else if (strstr(value,"Microsoft YaHei UI")) style->font_family = 1;
                else if (strstr(value,"Segoe UI") || !strcmp(value,"sans-serif")) style->font_family = 0;
                else return UI_STATUS_UNSUPPORTED;
            } else if (!strcmp(key,"font-weight")) {
                if (!strcmp(value,"normal")) style->font_weight = FW_NORMAL;
                else if (!strcmp(value,"bold")) style->font_weight = FW_BOLD;
                else { number = atoi(value); if (number < 100 || number > 900) return UI_STATUS_UNSUPPORTED; style->font_weight = number; }
            } else if (!strcmp(key,"border") || !strcmp(key,"border-left") || !strcmp(key,"border-right") || !strcmp(key,"border-top") || !strcmp(key,"border-bottom")) {
                char border[128], *space, *color;
                if (!strcmp(value,"none") || !strcmp(value,"0")) { style->border = 0; goto next; }
                if (strlen(value) >= sizeof(border)) return UI_STATUS_VALIDATION_FAILED;
                strcpy(border,value); space = strchr(border,' ');
                if (!space) return UI_STATUS_UNSUPPORTED;
                *space++ = 0; space = lw_trim(space); color = strchr(space,' ');
                if (!color) return UI_STATUS_UNSUPPORTED;
                *color++ = 0;
                if (strcmp(space,"solid") || lw_number(border,&number,&percent) != UI_STATUS_OK || percent || lw_color(lw_trim(color),&style->border_color) != 1) return UI_STATUS_UNSUPPORTED;
                style->border = number;
            } else if (!strcmp(key,"box-sizing")) {
                if (strcmp(value,"border-box")) return UI_STATUS_UNSUPPORTED;
            } else if (!strcmp(key,"cursor")) {
                if (strcmp(value,"pointer") && strcmp(value,"default") && strcmp(value,"text")) return UI_STATUS_UNSUPPORTED;
            } else {
                int *destination = !strcmp(key,"gap") ? &style->gap :
                    !strcmp(key,"border-width") ? &style->border : !strcmp(key,"border-radius") ? &style->radius :
                    !strcmp(key,"font-size") ? &style->font_size : !strcmp(key,"min-width") ? &style->min_width :
                    !strcmp(key,"max-width") ? &style->max_width : !strcmp(key,"min-height") ? &style->min_height :
                    !strcmp(key,"max-height") ? &style->max_height : !strcmp(key,"left") ? &style->left :
                    !strcmp(key,"top") ? &style->top : !strcmp(key,"right") ? &style->right : !strcmp(key,"bottom") ? &style->bottom : NULL;
                if (!destination || lw_number(value,&number,&percent) != UI_STATUS_OK || percent) return UI_STATUS_UNSUPPORTED;
                *destination = number;
            }
        }
next:
        if (end == NULL) break;
        item = end + 1;
    }
    return UI_STATUS_OK;
}

static ui_status_t lw_parse_style(lw_style_t *style, char *text)
{
    lw_style_default(style);
    return lw_apply_style(style,text);
}

static ui_status_t lw_stylesheet(lw_view_t *view, const char *text)
{
    const char *at = text;
    int media_min = 0, media_max = LW_MAX_DIMENSION, in_media = 0;
    while (*at) {
        const char *open, *close;
        char selector[256], declarations[LW_ATTR_CAP], *piece, *comma;
        size_t length;
        while (isspace((unsigned char)*at)) ++at;
        if (!*at) break;
        if (at[0] == '/' && at[1] == '*') {
            close = strstr(at + 2,"*/"); if (!close) return UI_STATUS_VALIDATION_FAILED;
            at = close + 2; continue;
        }
        if (*at == '}' && in_media) { in_media = 0; media_min = 0; media_max = LW_MAX_DIMENSION; ++at; continue; }
        open = strchr(at,'{'); if (!open) return UI_STATUS_VALIDATION_FAILED;
        length = (size_t)(open - at);
        if (length >= sizeof(selector)) return UI_STATUS_VALIDATION_FAILED;
        memcpy(selector,at,length); selector[length] = 0;
        piece = lw_trim(selector);
        if (!strncmp(piece,"@media",6)) {
            char *condition = strchr(piece,'('), *colon, *end;
            int value, percent;
            if (in_media || !condition || !(colon = strchr(condition,':')) || !(end = strchr(colon,')'))) return UI_STATUS_UNSUPPORTED;
            *colon = 0; *end = 0;
            if (lw_number(lw_trim(colon + 1),&value,&percent) != UI_STATUS_OK || percent) return UI_STATUS_UNSUPPORTED;
            if (!strcmp(lw_trim(condition + 1),"max-width")) media_max = value;
            else if (!strcmp(lw_trim(condition + 1),"min-width")) media_min = value;
            else return UI_STATUS_UNSUPPORTED;
            in_media = 1; at = open + 1; continue;
        }
        close = strchr(open + 1,'}'); if (!close) return UI_STATUS_VALIDATION_FAILED;
        length = (size_t)(close - open - 1);
        if (length >= sizeof(declarations)) return UI_STATUS_VALIDATION_FAILED;
        memcpy(declarations,open + 1,length); declarations[length] = 0;
        {
            char validate[LW_ATTR_CAP]; lw_style_t style;
            strcpy(validate,declarations);
            if (lw_parse_style(&style,validate) != UI_STATUS_OK) return UI_STATUS_UNSUPPORTED;
        }
        while (piece && *piece) {
            lw_rule_t *rule; const char *s;
            comma = strchr(piece,','); if (comma) *comma++ = 0;
            if (view->rule_count >= LW_MAX_RULES) return UI_STATUS_VALIDATION_FAILED;
            if (view->rule_count == view->rule_capacity) {
                int capacity = view->rule_capacity ? view->rule_capacity * 2 : 16;
                lw_rule_t *rules = (lw_rule_t *)realloc(view->rules,(size_t)capacity * sizeof(*rules));
                if (!rules) return UI_STATUS_OUT_OF_MEMORY;
                view->rules = rules; view->rule_capacity = capacity;
            }
            rule = &view->rules[view->rule_count++];
            strcpy(rule->selector,lw_trim(piece)); strcpy(rule->declarations,declarations);
            rule->min_width = media_min; rule->max_width = media_max; rule->specificity = 0;
            for (s = rule->selector; *s; ++s) {
                if (*s == '#') rule->specificity += 100;
                else if (*s == '.' || *s == ':') rule->specificity += 10;
                else if (s == rule->selector || isspace((unsigned char)s[-1])) ++rule->specificity;
                if (*s == '>' || *s == '+' || *s == '[' || *s == '*') return UI_STATUS_UNSUPPORTED;
            }
            piece = comma;
        }
        at = close + 1;
    }
    if (in_media) return UI_STATUS_VALIDATION_FAILED;
    {
        int i, j;
        for (i = 1; i < view->rule_count; ++i) {
            lw_rule_t rule = view->rules[i]; j = i;
            while (j > 0 && view->rules[j - 1].specificity > rule.specificity) {
                view->rules[j] = view->rules[j - 1]; --j;
            }
            view->rules[j] = rule;
        }
    }
    return UI_STATUS_OK;
}

static int lw_has_class(const char *classes, const char *name)
{
    size_t n = strlen(name);
    while (*classes) {
        const char *end;
        while (isspace((unsigned char)*classes)) ++classes;
        end = classes; while (*end && !isspace((unsigned char)*end)) ++end;
        if ((size_t)(end - classes) == n && !strncmp(classes,name,n)) return 1;
        classes = end;
    }
    return 0;
}

static int lw_matches_piece(lw_view_t *view, int index, const char *piece)
{
    char token[128]; const char *start = piece; lw_node_t *node = &view->nodes[index];
    while (*piece) {
        char kind = *piece;
        size_t length;
        if (kind == '#' || kind == '.' || kind == ':') ++piece;
        start = piece;
        while (*piece && *piece != '#' && *piece != '.' && *piece != ':') ++piece;
        length = (size_t)(piece - start);
        if (!length || length >= sizeof(token)) return 0;
        memcpy(token,start,length); token[length] = 0;
        if (kind == '#' && strcmp(node->id,token)) return 0;
        if (kind == '.' && !lw_has_class(node->classes,token)) return 0;
        if (kind == ':') {
            if (!strcmp(token,"hover")) { int hovered=view->hovered;while(hovered>=0&&hovered!=index)hovered=view->nodes[hovered].parent;if(hovered<0)return 0; }
            else if (!strcmp(token,"active")) {
                int pressed=view->pressed;
                while(pressed>=0&&pressed!=index)pressed=view->nodes[pressed].parent;
                if(pressed<0||lw_disabled(view,index))return 0;
            }
            else if (!strcmp(token,"focus")) { if (view->focused != index) return 0; }
            else if (!strcmp(token,"disabled")) { if (!node->disabled) return 0; }
            else return 0;
        }
        if (kind != '#' && kind != '.' && kind != ':' && _stricmp(node->tag,token)) return 0;
    }
    return 1;
}

static int lw_matches(lw_view_t *view, int index, const char *selector)
{
    char copy[256], *end, *start;
    strcpy(copy,selector); end = copy + strlen(copy);
    while (end > copy && isspace((unsigned char)end[-1])) --end;
    *end = 0;
    start = end; while (start > copy && !isspace((unsigned char)start[-1])) --start;
    if (!lw_matches_piece(view,index,start)) return 0;
    while (start > copy) {
        end = start; while (end > copy && isspace((unsigned char)end[-1])) --end;
        *end = 0; start = end; while (start > copy && !isspace((unsigned char)start[-1])) --start;
        index = view->nodes[index].parent;
        while (index >= 0 && !lw_matches_piece(view,index,start)) index = view->nodes[index].parent;
        if (index < 0) return 0;
    }
    return 1;
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
        status = lw_reserve_nodes(view,view->node_count + 1);
        if (status != UI_STATUS_OK) return status;
        index = view->node_count++;
        node = &view->nodes[index];
        memset(node, 0, sizeof(*node));
        node->object = JS_UNDEFINED;
        { int k; for (k = 0; k < LW_EVENTS; ++k) node->listeners[k] = JS_UNDEFINED; }
        node->used = node->attached = 1; node->uid = ++view->next_uid;
        if (length >= sizeof(node->tag)) return UI_STATUS_UNSUPPORTED;
        memcpy(node->tag,name,length); node->tag[length] = 0;
        node->parent = parent_index; node->first_child = node->next_sibling = -1;
        if (lw_name((const char *)name, length, "button")) node->kind = 2;
        else if (lw_name((const char *)name, length, "input")) node->kind = 3;
        else if (lw_name((const char *)name, length, "textarea")) node->kind = 4;
        else if (lw_name((const char *)name, length, "img")) node->kind = 5;
        else if (lw_name((const char *)name, length, "div") ||
                 lw_name((const char *)name, length, "header") ||
                 lw_name((const char *)name, length, "footer") ||
                 lw_name((const char *)name, length, "nav") ||
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
        LW_READ_ATTR("class", node->classes);
        LW_READ_ATTR("onclick", node->onclick);
        LW_READ_ATTR("onmousedown", node->onmousedown);
        LW_READ_ATTR("oninput", node->oninput);
        LW_READ_ATTR("value", node->value);
        LW_READ_ATTR("style", node->inline_style);
        LW_READ_ATTR("type", type);
        { char image[64]; LW_READ_ATTR("src", image); node->image = _strtoui64(image,NULL,10); }
#undef LW_READ_ATTR
        node->disabled = lxb_dom_element_has_attribute(lxb_dom_interface_element(child),
            (const lxb_char_t *)"disabled",8);
        node->readonly = lxb_dom_element_has_attribute(lxb_dom_interface_element(child),
            (const lxb_char_t *)"readonly",8);
        strcpy(style,node->inline_style);
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
        node = &view->nodes[index];
        if (node->first_child < 0 && node->kind != 3) {
            lxb_char_t *text = lxb_dom_node_text_content(child, &length);
            if (text == NULL) return UI_STATUS_OUT_OF_MEMORY;
            status = lw_copy(node->text, sizeof(node->text), text, length);
            lxb_dom_document_destroy_text(child->owner_document, text);
            if (status != UI_STATUS_OK) return status;
            if (node->kind == 4) strcpy(node->value,node->text);
        }
        if (lw_input(node) && node->first_child >= 0) return UI_STATUS_UNSUPPORTED;
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
    int minimum = horizontal ? node->style.min_width : node->style.min_height;
    int maximum = horizontal ? node->style.max_width : node->style.max_height;
    if (value < 0) value = horizontal ? (node->natural_width ? node->natural_width : 120) :
        (node->natural_height ? node->natural_height : node->kind == 1 && !*node->text ?
         node->style.padding[0] + node->style.padding[2] + 2 * node->style.border : node->kind == 4 ? 96 : 32);
    else if (percent) value = available * value / 100;
    if (value < minimum) value = minimum;
    if (value > maximum) value = maximum;
    return value;
}

static void lw_compute_style(lw_view_t *view, int index)
{
    lw_node_t *node = &view->nodes[index];
    int r, child;
    char declarations[LW_ATTR_CAP];
    lw_style_default(&node->style);
    if (node->parent >= 0) {
        lw_style_t *parent = &view->nodes[node->parent].style;
        node->style.color = parent->color; node->style.font_size = parent->font_size;
        node->style.font_weight = parent->font_weight;
        node->style.font_family = parent->font_family;
    }
    /* Stable specificity order; equal specificity retains source ordering. */
    for (r = 0; r < view->rule_count; ++r) {
            lw_rule_t *rule = &view->rules[r];
            if (view->width < rule->min_width ||
                view->width > rule->max_width || !lw_matches(view,index,rule->selector)) continue;
            strcpy(declarations,rule->declarations);
            (void)lw_apply_style(&node->style,declarations);
    }
    strcpy(declarations,node->inline_style); (void)lw_apply_style(&node->style,declarations);
    /* A single body child is the legacy page root and fills the viewport. */
    if (node->parent == view->root && view->nodes[view->root].first_child == index && node->next_sibling < 0)
        node->style.flex = 1;
    node->visible = node->attached && !node->style.hidden &&
        (node->parent < 0 || view->nodes[node->parent].visible);
    if (!node->visible) memset(&node->rect,0,sizeof(node->rect));
    if (!node->font || node->font_size != node->style.font_size ||
        node->font_weight != node->style.font_weight || node->font_family != node->style.font_family || node->font_dpi != view->dpi) {
        if (node->font) DeleteObject(node->font);
        node->font = CreateFontW(-lw_pixel(node->style.font_size,view->dpi),0,0,0,
            node->style.font_weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,node->style.font_family==2?L"Segoe MDL2 Assets":node->style.font_family?L"Microsoft YaHei UI":L"Segoe UI");
        node->font_size = node->style.font_size; node->font_weight = node->style.font_weight;
        node->font_dpi = view->dpi;
        node->font_family = node->style.font_family;
    }
    for (child = node->first_child; child >= 0; child = view->nodes[child].next_sibling)
        lw_compute_style(view,child);
    node->natural_width = node->natural_height = 0;
    if (node->kind == 2 && *node->text) {
        wchar_t *text = lw_wide(node->text);
        HDC dc = CreateCompatibleDC(NULL);
        if (text && dc) {
            SIZE size = {0}; HGDIOBJ previous = SelectObject(dc,node->font);
            if (GetTextExtentPoint32W(dc,text,(int)wcslen(text),&size))
                node->natural_width = MulDiv(size.cx,96,(int)view->dpi) +
                    (node->style.padding[1] ? node->style.padding[1] : 8) +
                    (node->style.padding[3] ? node->style.padding[3] : 8) + 2 * node->style.border;
            SelectObject(dc,previous);
        }
        if (dc) DeleteDC(dc); free(text);
    }
    {
        int count = 0;
        for (child = node->first_child; child >= 0; child = view->nodes[child].next_sibling) {
            lw_node_t *c = &view->nodes[child]; int w, h;
            if (!c->visible || c->style.absolute) continue;
            w = lw_size(c,view->width,1) + c->style.margin[1] + c->style.margin[3];
            h = lw_size(c,view->height,0) + c->style.margin[0] + c->style.margin[2];
            if (node->style.row) { node->natural_width += w; if (h > node->natural_height) node->natural_height = h; }
            else { node->natural_height += h; if (w > node->natural_width) node->natural_width = w; }
            ++count;
        }
        if (count > 1) { if (node->style.row) node->natural_width += node->style.gap * (count - 1); else node->natural_height += node->style.gap * (count - 1); }
        if (count) {
            node->natural_width += node->style.padding[1] + node->style.padding[3] + 2 * node->style.border;
            node->natural_height += node->style.padding[0] + node->style.padding[2] + 2 * node->style.border;
        }
    }
}

static void lw_layout_node(lw_view_t *view, int index, ui_rect_t rect, ui_rect_t clip)
{
    lw_node_t *node = &view->nodes[index];
    int child, fixed = 0, weights = 0, cursor, axis, count = 0, gap, remaining;
    ui_rect_t inner = rect;
    node->rect = rect; node->clip = lw_intersect(rect, clip);
    if (!node->visible) { memset(&node->rect, 0, sizeof(node->rect)); return; }
    inner.x += node->style.padding[3] + node->style.border;
    inner.y += node->style.padding[0] + node->style.border;
    inner.width -= node->style.padding[1] + node->style.padding[3] + 2 * node->style.border;
    inner.height -= node->style.padding[0] + node->style.padding[2] + 2 * node->style.border;
    if (inner.width < 0) inner.width = 0; if (inner.height < 0) inner.height = 0;
    axis = node->style.row ? inner.width : inner.height;
    for (child = node->first_child; child >= 0; child = view->nodes[child].next_sibling) {
        lw_node_t *c = &view->nodes[child];
        if (!c->visible || c->style.absolute) continue;
        ++count;
        fixed += node->style.row ? c->style.margin[1] + c->style.margin[3] : c->style.margin[0] + c->style.margin[2];
        if (c->style.flex) weights += c->style.flex;
        else fixed += lw_size(c, axis, node->style.row);
    }
    gap = node->style.gap;
    if (count > 1) fixed += gap * (count - 1);
    node->content_height=node->style.row?node->natural_height-node->style.padding[0]-node->style.padding[2]-2*node->style.border:fixed;
    node->content_width=node->style.row?fixed:node->natural_width-node->style.padding[1]-node->style.padding[3]-2*node->style.border;
    node->bar_x=node->bar_y=0;
    for(int pass=0;pass<2;++pass){
        if(!node->bar_y&&node->style.scroll&&node->content_height>inner.height){node->bar_y=1;inner.width=inner.width>12?inner.width-12:0;}
        if(!node->bar_x&&node->style.scroll_x&&node->content_width>inner.width){node->bar_x=1;inner.height=inner.height>12?inner.height-12:0;}
    }
    node->viewport=inner;axis=node->style.row?inner.width:inner.height;
    remaining = axis > fixed ? axis - fixed : 0;
    if (!weights && node->style.justify == 3 && count > 1) gap += remaining / (count - 1);
    if(node->content_height<inner.height)node->content_height=inner.height;
    if(node->content_width<inner.width)node->content_width=inner.width;
    if (weights && node->content_height < inner.height) node->content_height = inner.height;
    if (node->scroll_y > node->content_height - inner.height)
        node->scroll_y = node->content_height - inner.height;
    if (node->scroll_y < 0) node->scroll_y = 0;
    if (node->scroll_x > node->content_width - inner.width) node->scroll_x = node->content_width - inner.width;
    if (node->scroll_x < 0) node->scroll_x = 0;
    cursor = node->style.row ? inner.x - node->scroll_x : inner.y - node->scroll_y;
    if (!weights && node->style.justify == 1) cursor += remaining / 2;
    if (!weights && node->style.justify == 2) cursor += remaining;
    for (child = node->first_child; child >= 0; child = view->nodes[child].next_sibling) {
        lw_node_t *c = &view->nodes[child];
        ui_rect_t child_rect;
        int size, cross;
        if (!c->visible) { memset(&c->rect, 0, sizeof(c->rect)); continue; }
        if (c->style.absolute) {
            child_rect.width = c->style.width < 0 && c->style.left >= 0 && c->style.right >= 0 ?
                inner.width - c->style.left - c->style.right : lw_size(c,inner.width,1);
            child_rect.height = c->style.height < 0 && c->style.top >= 0 && c->style.bottom >= 0 ?
                inner.height - c->style.top - c->style.bottom : lw_size(c,inner.height,0);
            child_rect.x = inner.x + (c->style.left >= 0 ? c->style.left : c->style.right >= 0 ? inner.width - child_rect.width - c->style.right : 0);
            child_rect.y = inner.y + (c->style.top >= 0 ? c->style.top : c->style.bottom >= 0 ? inner.height - child_rect.height - c->style.bottom : 0);
            if (child_rect.width < 0) child_rect.width = 0; if (child_rect.height < 0) child_rect.height = 0;
            lw_layout_node(view,child,child_rect,node->clip); continue;
        }
        size = c->style.flex ? (remaining * c->style.flex / weights)
                             : lw_size(c, axis, node->style.row);
        cross = node->style.row ? inner.height : inner.width;
        cross -= node->style.row ? c->style.margin[0] + c->style.margin[2] : c->style.margin[1] + c->style.margin[3];
        if (node->style.row && (c->style.height >= 0 || node->style.align == 1 || node->style.align == 2)) cross = lw_size(c,inner.height,0);
        if (!node->style.row && (c->style.width >= 0 || node->style.align == 1 || node->style.align == 2)) cross = lw_size(c,inner.width,1);
        cursor += node->style.row ? c->style.margin[3] : c->style.margin[0];
        child_rect.x = node->style.row ? cursor : inner.x + c->style.margin[3] - node->scroll_x;
        child_rect.y = node->style.row ? inner.y + c->style.margin[0] - node->scroll_y : cursor;
        if (node->style.align == 1) { if (node->style.row) child_rect.y += (inner.height - cross) / 2; else child_rect.x += (inner.width - cross) / 2; }
        if (node->style.align == 2) { if (node->style.row) child_rect.y += inner.height - cross; else child_rect.x += inner.width - cross; }
        child_rect.width = node->style.row ? size : cross;
        child_rect.height = node->style.row ? cross : size;
        lw_layout_node(view, child, child_rect, node->bar_x||node->bar_y?lw_intersect(node->clip,inner):node->clip);
        cursor += size + gap + (node->style.row ? c->style.margin[1] : c->style.margin[2]);
    }
}

static void lw_layout(lw_view_t *view)
{
    ui_rect_t viewport = {0, 0, view->width, view->height};
    if (view->layout_depth) return;
    ++view->layout_depth;
    if (view->root >= 0) { lw_compute_style(view,view->root); lw_layout_node(view, view->root, viewport, viewport); }
    lw_caret(view);
    if (view->hwnd) InvalidateRect(view->hwnd, NULL, FALSE);
    view->dirty = 0;
    --view->layout_depth;
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
        if (view->nodes[i].used && lw_input(&view->nodes[i]) && strcmp(view->nodes[i].id, id) == 0) {
            JS_FreeCString(ctx, id); return JS_NewString(ctx, view->nodes[i].value);
        }
    JS_FreeCString(ctx, id);
    return JS_ThrowReferenceError(ctx, "input id not found");
}

static JSValue lw_element(lw_view_t *view, int index);

static int lw_uid_index(const lw_view_t *view, unsigned uid)
{
    int i;
    for (i = 0; i < view->node_count; ++i)
        if (view->nodes[i].used && view->nodes[i].uid == uid) return i;
    return -1;
}

static int lw_js_index(JSContext *ctx, JSValueConst object)
{
    lw_view_t *view = (lw_view_t *)JS_GetContextOpaque(ctx);
    JSValue value = JS_GetPropertyStr(ctx,object,"_lw_uid");
    uint32_t uid = 0;
    (void)JS_ToUint32(ctx,&uid,value); JS_FreeValue(ctx,value);
    return lw_uid_index(view,uid);
}

static void lw_sync_value(lw_view_t *view, lw_node_t *node)
{
    wchar_t *wide = lw_wide(node->value);
    int length = wide ? (int)wcslen(wide) : 0;
    if (node->caret > length) node->caret = length;
    if (node->anchor > length) node->anchor = length;
    free(wide); lw_caret(view);
}

static void lw_unlink(lw_view_t *view, int index)
{
    lw_node_t *node = &view->nodes[index]; int *link;
    if (node->parent >= 0) {
        link = &view->nodes[node->parent].first_child;
        while (*link >= 0 && *link != index) link = &view->nodes[*link].next_sibling;
        if (*link == index) *link = node->next_sibling;
    }
    node->parent = node->next_sibling = -1;
}

static void lw_release_node(lw_view_t *view, int index)
{
    lw_node_t *node = &view->nodes[index]; int child, next, k;
    node->used = 0;
    for (child = node->first_child; child >= 0; child = next) {
        next = view->nodes[child].next_sibling; lw_release_node(view,child);
    }
    lw_text_free(node);
    if (node->font) DeleteObject(node->font);
    if (view->context) {
        JS_FreeValue(view->context,node->object);
        for (k = 0; k < LW_EVENTS; ++k) JS_FreeValue(view->context,node->listeners[k]);
    }
    node->used = node->attached = node->visible = 0;
    node->font = NULL;
    node->object = JS_UNDEFINED;
    for (k = 0; k < LW_EVENTS; ++k) node->listeners[k] = JS_UNDEFINED;
    if (view->focused == index) view->focused = -1;
    if (view->hovered == index) view->hovered = -1;
}

static int lw_event_kind(const char *name)
{
    static const char *names[] = {"click","input","keydown","focus","blur","mouseenter","mouseleave","mousedown","wheel","contextmenu","pointerdown","pointermove","pointerup","pointercancel","lostpointercapture"};
    int i;
    for (i = 0; i < LW_EVENTS; ++i) if (!strcmp(name,names[i])) return i;
    return -1;
}

enum { LW_PROP_ID, LW_PROP_TEXT, LW_PROP_CLASS, LW_PROP_VALUE, LW_PROP_DISABLED,
       LW_PROP_FIRST, LW_PROP_PARENT, LW_PROP_CHILDREN, LW_PROP_STYLE, LW_PROP_SRC, LW_PROP_READONLY };

static JSValue lw_dom_get(JSContext *ctx, JSValueConst object, int property)
{
    lw_view_t *view = (lw_view_t *)JS_GetContextOpaque(ctx);
    int index = lw_js_index(ctx,object), child;
    lw_node_t *node;
    JSValue array;
    uint32_t at = 0;
    if (index < 0) return JS_NULL;
    node = &view->nodes[index];
    switch (property) {
    case LW_PROP_ID: return JS_NewString(ctx,node->id);
    case LW_PROP_TEXT: return JS_NewString(ctx,node->text);
    case LW_PROP_CLASS: return JS_NewString(ctx,node->classes);
    case LW_PROP_VALUE: return JS_NewString(ctx,node->value);
    case LW_PROP_DISABLED: return JS_NewBool(ctx,node->disabled);
    case LW_PROP_READONLY: return JS_NewBool(ctx,node->readonly);
    case LW_PROP_SRC: { char id[32]; sprintf_s(id,sizeof(id),"%llu",(unsigned long long)node->image); return JS_NewString(ctx,id); }
    case LW_PROP_FIRST: return node->first_child >= 0 ? lw_element(view,node->first_child) : JS_NULL;
    case LW_PROP_PARENT: return node->parent >= 0 ? lw_element(view,node->parent) : JS_NULL;
    case LW_PROP_CHILDREN:
        array = JS_NewArray(ctx);
        for (child = node->first_child; child >= 0; child = view->nodes[child].next_sibling)
            JS_SetPropertyUint32(ctx,array,at++,lw_element(view,child));
        return array;
    default: return JS_UNDEFINED;
    }
}

static JSValue lw_dom_set(JSContext *ctx, JSValueConst object, JSValueConst value, int property)
{
    lw_view_t *view = (lw_view_t *)JS_GetContextOpaque(ctx);
    int index = lw_js_index(ctx,object), child, next, i;
    const char *text;
    char *destination;
    size_t capacity;
    lw_node_t *node;
    if (index < 0) return JS_ThrowReferenceError(ctx,"removed element");
    node = &view->nodes[index];
    if (property == LW_PROP_DISABLED || property == LW_PROP_READONLY) {
        if (property == LW_PROP_DISABLED) node->disabled = JS_ToBool(ctx,value);
        else node->readonly = JS_ToBool(ctx,value); view->dirty = 1; return JS_UNDEFINED;
    }
    text = JS_ToCString(ctx,value); if (!text) return JS_EXCEPTION;
    if (property == LW_PROP_SRC) { node->image = _strtoui64(text,NULL,10); JS_FreeCString(ctx,text); view->dirty = 1; return JS_UNDEFINED; }
    if (property == LW_PROP_VALUE && !strcmp(text,node->value)) { JS_FreeCString(ctx,text); return JS_UNDEFINED; }
    destination = property == LW_PROP_ID ? node->id : property == LW_PROP_TEXT ? node->text :
        property == LW_PROP_CLASS ? node->classes : node->value;
    capacity = property == LW_PROP_ID ? sizeof(node->id) : property == LW_PROP_CLASS ? sizeof(node->classes) : sizeof(node->text);
    if (strlen(text) >= capacity) { JS_FreeCString(ctx,text); return JS_ThrowRangeError(ctx,"element string exceeds limit"); }
    if (property == LW_PROP_ID && *text) {
        for (i = 0; i < view->node_count; ++i)
            if (i != index && view->nodes[i].used && !strcmp(view->nodes[i].id,text)) {
                JS_FreeCString(ctx,text); return JS_ThrowTypeError(ctx,"duplicate element id");
            }
    }
    if (property == LW_PROP_TEXT) {
        for (child = node->first_child; child >= 0; child = next) {
            next = view->nodes[child].next_sibling; lw_release_node(view,child);
        }
        node->first_child = -1;
    }
    strcpy(destination,text); JS_FreeCString(ctx,text);
    if (property == LW_PROP_VALUE) lw_sync_value(view,node);
    view->dirty = 1; return JS_UNDEFINED;
}

static const char *lw_style_names[] = {"cssText","display","width","height","left","top","right","bottom","position","background","background-color","color","padding","margin","gap","border","border-radius","flex","flex-direction","font-size","font-weight","min-width","max-width","min-height","max-height","overflow-y","align-items","justify-content","white-space","text-align","overflow-x"};

static JSValue lw_style_get(JSContext *ctx, JSValueConst object, int property)
{
    lw_view_t *view = (lw_view_t *)JS_GetContextOpaque(ctx);
    int index = lw_js_index(ctx,object);
    char copy[LW_ATTR_CAP], *at;
    if (index < 0) return JS_UNDEFINED;
    if (!property) return JS_NewString(ctx,view->nodes[index].inline_style);
    strcpy(copy,view->nodes[index].inline_style); at = copy;
    while (*at) {
        char *end = strchr(at,';'), *colon;
        if (end) *end = 0;
        colon = strchr(at,':');
        if (colon) { *colon = 0; if (!strcmp(lw_trim(at),lw_style_names[property])) return JS_NewString(ctx,lw_trim(colon + 1)); }
        if (!end) break; at = end + 1;
    }
    return JS_NewString(ctx,"");
}

static JSValue lw_style_set(JSContext *ctx, JSValueConst object, JSValueConst value, int property)
{
    lw_view_t *view = (lw_view_t *)JS_GetContextOpaque(ctx);
    int index = lw_js_index(ctx,object);
    const char *text;
    char output[LW_ATTR_CAP] = {0}, copy[LW_ATTR_CAP], *at;
    lw_style_t validate;
    if (index < 0) return JS_ThrowReferenceError(ctx,"removed element");
    text = JS_ToCString(ctx,value); if (!text) return JS_EXCEPTION;
    if (!property) {
        if (strlen(text) >= sizeof(output)) goto invalid;
        strcpy(output,text);
    } else {
        strcpy(copy,view->nodes[index].inline_style); at = copy;
        while (*at) {
            char *end = strchr(at,';'), *colon;
            if (end) *end = 0;
            colon = strchr(at,':');
            if (colon) {
                *colon = 0;
                if (strcmp(lw_trim(at),lw_style_names[property])) {
                    size_t needed = strlen(output) + strlen(lw_trim(at)) + strlen(lw_trim(colon + 1)) + 3;
                    if (needed >= sizeof(output)) goto invalid;
                    strcat(output,lw_trim(at)); strcat(output,":"); strcat(output,lw_trim(colon + 1)); strcat(output,";");
                }
            }
            if (!end) break; at = end + 1;
        }
        if (*text) {
            if (strlen(output) + strlen(text) + strlen(lw_style_names[property]) + 3 >= sizeof(output)) goto invalid;
            strcat(output,lw_style_names[property]); strcat(output,":"); strcat(output,text); strcat(output,";");
        }
    }
    strcpy(copy,output);
    if (lw_parse_style(&validate,copy) != UI_STATUS_OK) goto invalid;
    strcpy(view->nodes[index].inline_style,output); view->dirty = 1;
    JS_FreeCString(ctx,text); return JS_UNDEFINED;
invalid:
    JS_FreeCString(ctx,text); return JS_ThrowTypeError(ctx,"unsupported or oversized CSS declaration");
}

static JSValue lw_dom_method(JSContext *ctx, JSValueConst object, int argc,
                              JSValueConst *argv, int method)
{
    lw_view_t *view = (lw_view_t *)JS_GetContextOpaque(ctx);
    int index = lw_js_index(ctx,object), child, ancestor, kind;
    lw_node_t *node;
    const char *name;
    if (index < 0) return JS_ThrowReferenceError(ctx,"removed element");
    node = &view->nodes[index];
    if (method == 0 || method == 1) { /* appendChild/removeChild */
        if (argc < 1 || (child = lw_js_index(ctx,argv[0])) < 0) return JS_ThrowTypeError(ctx,"element required");
        if (method == 0) {
            if ((lw_input(node) || node->kind == 5)) return JS_ThrowTypeError(ctx,"input cannot contain children");
            for (ancestor = index; ancestor >= 0; ancestor = view->nodes[ancestor].parent)
                if (ancestor == child) return JS_ThrowTypeError(ctx,"DOM cycle");
            lw_unlink(view,child);
            view->nodes[child].parent = index; view->nodes[child].attached = 1;
            ancestor = node->first_child;
            if (ancestor < 0) node->first_child = child;
            else { while (view->nodes[ancestor].next_sibling >= 0) ancestor = view->nodes[ancestor].next_sibling; view->nodes[ancestor].next_sibling = child; }
        } else {
            if (view->nodes[child].parent != index) return JS_ThrowTypeError(ctx,"not a child");
            lw_unlink(view,child); lw_release_node(view,child);
        }
        view->dirty = 1; return JS_DupValue(ctx,argv[0]);
    }
    if (method == 2) { /* remove */
        if (index == view->root) return JS_ThrowTypeError(ctx,"cannot remove root");
        lw_unlink(view,index); lw_release_node(view,index); view->dirty = 1; return JS_UNDEFINED;
    }
    if (method == 3) { /* addEventListener */
        if (argc < 2 || !JS_IsFunction(ctx,argv[1])) return JS_ThrowTypeError(ctx,"listener function required");
        name = JS_ToCString(ctx,argv[0]); if (!name) return JS_EXCEPTION;
        kind = lw_event_kind(name); JS_FreeCString(ctx,name);
        if (kind < 0) return JS_ThrowTypeError(ctx,"unsupported event");
        if (JS_IsUndefined(node->listeners[kind])) node->listeners[kind] = JS_NewArray(ctx);
        {
            JSValue length = JS_GetPropertyStr(ctx,node->listeners[kind],"length"); uint32_t n;
            (void)JS_ToUint32(ctx,&n,length); JS_FreeValue(ctx,length);
            if (n >= 32) return JS_ThrowRangeError(ctx,"too many event listeners");
            JS_SetPropertyUint32(ctx,node->listeners[kind],n,JS_DupValue(ctx,argv[1]));
        }
        return JS_UNDEFINED;
    }
    if (method == 4 || method == 5 || method == 6) { /* attributes */
        if (!argc) return JS_ThrowTypeError(ctx,"attribute name required");
        name = JS_ToCString(ctx,argv[0]); if (!name) return JS_EXCEPTION;
        kind = !strcmp(name,"id") ? LW_PROP_ID : !strcmp(name,"class") ? LW_PROP_CLASS :
            !strcmp(name,"value") ? LW_PROP_VALUE : !strcmp(name,"disabled") ? LW_PROP_DISABLED : !strcmp(name,"src") ? LW_PROP_SRC : !strcmp(name,"readonly") ? LW_PROP_READONLY : -1;
        if (kind >= 0) {
            JSValue result;
            if (method == 5) result = lw_dom_get(ctx,object,kind);
            else if (method == 6) {
                JSValue empty = kind == LW_PROP_DISABLED ? JS_FALSE : JS_NewString(ctx,"");
                result = lw_dom_set(ctx,object,empty,kind); JS_FreeValue(ctx,empty);
            }
            else if (argc < 2) result = JS_ThrowTypeError(ctx,"attribute value required");
            else result = lw_dom_set(ctx,object,kind == LW_PROP_DISABLED ? JS_TRUE : argv[1],kind);
            JS_FreeCString(ctx,name); return result;
        }
        if (!strcmp(name,"style")) {
            JSValue result;
            if (method == 5) result = JS_NewString(ctx,node->inline_style);
            else if (method == 6) { JSValue empty = JS_NewString(ctx,""); result = lw_style_set(ctx,object,empty,0); JS_FreeValue(ctx,empty); }
            else result = argc > 1 ? lw_style_set(ctx,object,argv[1],0) : JS_ThrowTypeError(ctx,"style value required");
            JS_FreeCString(ctx,name); return result;
        }
        JS_FreeCString(ctx,name);
        return JS_ThrowTypeError(ctx,"unsupported attribute");
    }
    if (method == 8) {
        int parent;
        if(view->dirty)lw_layout(view);
        for(parent=node->parent;parent>=0;parent=view->nodes[parent].parent){lw_node_t *p=&view->nodes[parent];
            if(p->style.scroll){if(node->rect.y<p->clip.y)p->scroll_y-=p->clip.y-node->rect.y;
                else if(node->rect.y+node->rect.height>p->clip.y+p->clip.height)p->scroll_y+=node->rect.y+node->rect.height-p->clip.y-p->clip.height;
                if(p->scroll_y<0)p->scroll_y=0;}
            if(p->style.scroll_x){if(node->rect.x<p->clip.x)p->scroll_x-=p->clip.x-node->rect.x;
                else if(node->rect.x+node->rect.width>p->clip.x+p->clip.width)p->scroll_x+=node->rect.x+node->rect.width-p->clip.x-p->clip.width;
                if(p->scroll_x<0)p->scroll_x=0;}}
        view->dirty=1;return JS_UNDEFINED;
    }
    if(method==9){JSValue r=JS_NewObject(ctx);if(view->dirty)lw_layout(view);
        JS_SetPropertyStr(ctx,r,"x",JS_NewInt32(ctx,node->rect.x));JS_SetPropertyStr(ctx,r,"left",JS_NewInt32(ctx,node->rect.x));
        JS_SetPropertyStr(ctx,r,"y",JS_NewInt32(ctx,node->rect.y));JS_SetPropertyStr(ctx,r,"top",JS_NewInt32(ctx,node->rect.y));
        JS_SetPropertyStr(ctx,r,"width",JS_NewInt32(ctx,node->rect.width));JS_SetPropertyStr(ctx,r,"height",JS_NewInt32(ctx,node->rect.height));return r;}
    /* Re-acquiring the same HWND emits WM_CAPTURECHANGED synchronously.
     * Keep its existing capture when promoting a button press to DOM capture. */
    if(method==10){view->captured=node->uid;if(view->hwnd&&GetCapture()!=view->hwnd)SetCapture(view->hwnd);return JS_UNDEFINED;}
    if(method==11){if(view->captured==node->uid){view->captured=0;if(view->hwnd&&GetCapture()==view->hwnd)ReleaseCapture();}return JS_UNDEFINED;}
    if(method==12)return JS_NewBool(ctx,view->captured==node->uid);
    if (method == 7) {
        if(view->focused!=index)lw_cancel_composition(view);
        view->focused = index; if (view->hwnd&&IsWindowVisible(view->hwnd)&&IsWindowEnabled(view->hwnd)) SetFocus(view->hwnd); lw_caret(view); view->dirty = 1; return lw_event(view,index,3,0) == UI_STATUS_OK ? JS_UNDEFINED : JS_EXCEPTION;
    }
    return JS_UNDEFINED;
}

static void lw_js_property(JSContext *ctx, JSValue object, const char *name,
                            int magic, JSValue (*getter)(JSContext *,JSValueConst,int),
                            JSValue (*setter)(JSContext *,JSValueConst,JSValueConst,int))
{
    JSAtom atom = JS_NewAtom(ctx,name);
    JSValue get = JS_NewCFunction2(ctx,(JSCFunction *)getter,name,0,JS_CFUNC_getter_magic,magic);
    JSValue set = setter ? JS_NewCFunction2(ctx,(JSCFunction *)setter,name,1,JS_CFUNC_setter_magic,magic) : JS_UNDEFINED;
    JS_DefinePropertyGetSet(ctx,object,atom,get,set,JS_PROP_ENUMERABLE);
    JS_FreeAtom(ctx,atom);
}

static JSValue lw_element(lw_view_t *view, int index)
{
    JSContext *ctx = view->context; lw_node_t *node = &view->nodes[index];
    JSValue object, style, global, element_proto, style_proto;
    int i;
    static const char *properties[] = {"id","textContent","className","value","disabled","firstChild","parentNode","children"};
    static const char *methods[] = {"appendChild","removeChild","remove","addEventListener","setAttribute","getAttribute","removeAttribute","focus","scrollIntoView","getBoundingClientRect","setPointerCapture","releasePointerCapture","hasPointerCapture"};
    if (!JS_IsUndefined(node->object)) return JS_DupValue(ctx,node->object);
    global = JS_GetGlobalObject(ctx);
    element_proto = JS_GetPropertyStr(ctx,global,"__lwElementPrototype");
    style_proto = JS_GetPropertyStr(ctx,global,"__lwStylePrototype");
    if (JS_IsUndefined(element_proto)) {
        JS_FreeValue(ctx,element_proto); JS_FreeValue(ctx,style_proto);
        element_proto = JS_NewObject(ctx); style_proto = JS_NewObject(ctx);
        lw_js_property(ctx,element_proto,"src",LW_PROP_SRC,lw_dom_get,lw_dom_set);
        lw_js_property(ctx,element_proto,"readOnly",LW_PROP_READONLY,lw_dom_get,lw_dom_set);
        for (i = 0; i < 8; ++i) lw_js_property(ctx,element_proto,properties[i],i,lw_dom_get,i < 5 ? lw_dom_set : NULL);
        for (i = 0; i < (int)(sizeof(methods)/sizeof(methods[0])); ++i) JS_SetPropertyStr(ctx,element_proto,methods[i],JS_NewCFunctionMagic(ctx,lw_dom_method,methods[i],2,JS_CFUNC_generic_magic,i));
        for (i = 0; i < (int)(sizeof(lw_style_names) / sizeof(lw_style_names[0])); ++i) {
            char camel[64]; int j = 0, capital = 0; const char *s = lw_style_names[i];
            lw_js_property(ctx,style_proto,s,i,lw_style_get,lw_style_set);
            while (*s && j < (int)sizeof(camel) - 1) { if (*s == '-') capital = 1; else { camel[j++] = capital ? (char)toupper((unsigned char)*s) : *s; capital = 0; } ++s; }
            camel[j] = 0;
            if (strcmp(camel,lw_style_names[i])) lw_js_property(ctx,style_proto,camel,i,lw_style_get,lw_style_set);
        }
        JS_SetPropertyStr(ctx,global,"__lwElementPrototype",JS_DupValue(ctx,element_proto));
        JS_SetPropertyStr(ctx,global,"__lwStylePrototype",JS_DupValue(ctx,style_proto));
    }
    object = JS_NewObjectProto(ctx,element_proto); style = JS_NewObjectProto(ctx,style_proto);
    JS_FreeValue(ctx,element_proto); JS_FreeValue(ctx,style_proto); JS_FreeValue(ctx,global);
    JS_DefinePropertyValueStr(ctx,object,"_lw_uid",JS_NewUint32(ctx,node->uid),0);
    JS_DefinePropertyValueStr(ctx,style,"_lw_uid",JS_NewUint32(ctx,node->uid),0);
    JS_SetPropertyStr(ctx,object,"style",style);
    node->object = JS_DupValue(ctx,object); return object;
}

static JSValue lw_document_listener(JSContext *ctx, JSValueConst object,
                                      int argc, JSValueConst *argv)
{
    const char *name;
    JSValue listeners, length;
    uint32_t n = 0;
    if (argc < 2 || !JS_IsFunction(ctx,argv[1])) return JS_ThrowTypeError(ctx,"listener function required");
    name = JS_ToCString(ctx,argv[0]); if (!name) return JS_EXCEPTION;
    {int kind=lw_event_kind(name);if(kind<0){JS_FreeCString(ctx,name);return JS_ThrowTypeError(ctx,"unsupported document event");}}
    {char property[64];snprintf(property,sizeof(property),"__%s",name);
    listeners = JS_GetPropertyStr(ctx,object,property);
    if (JS_IsUndefined(listeners)) { JS_FreeValue(ctx,listeners); listeners = JS_NewArray(ctx); JS_SetPropertyStr(ctx,object,property,JS_DupValue(ctx,listeners)); }}
    JS_FreeCString(ctx,name);
    length = JS_GetPropertyStr(ctx,listeners,"length"); (void)JS_ToUint32(ctx,&n,length); JS_FreeValue(ctx,length);
    if (n >= 32) { JS_FreeValue(ctx,listeners); return JS_ThrowRangeError(ctx,"too many listeners"); }
    JS_SetPropertyUint32(ctx,listeners,n,JS_DupValue(ctx,argv[1])); JS_FreeValue(ctx,listeners); return JS_UNDEFINED;
}

static JSValue lw_js_document(JSContext *ctx, JSValueConst object, int argc,
                               JSValueConst *argv, int method)
{
    lw_view_t *view = (lw_view_t *)JS_GetContextOpaque(ctx);
    const char *name;
    int i, kind = 1, k;
    (void)object;
    if (!argc) return JS_ThrowTypeError(ctx,"name required");
    name = JS_ToCString(ctx,argv[0]); if (!name) return JS_EXCEPTION;
    if (method == 0) {
        for (i = 0; i < view->node_count; ++i)
            if (view->nodes[i].used && view->nodes[i].attached && !strcmp(view->nodes[i].id,name)) { JS_FreeCString(ctx,name); return lw_element(view,i); }
        JS_FreeCString(ctx,name); return JS_NULL;
    }
    if (!strcmp(name,"button")) kind = 2;
    else if (!strcmp(name,"input")) kind = 3;
    else if (!strcmp(name,"textarea")) kind = 4;
    else if (!strcmp(name,"img")) kind = 5;
    else if (strcmp(name,"div") && strcmp(name,"span") && strcmp(name,"p") && strcmp(name,"main") && strcmp(name,"section") && strcmp(name,"aside") && strcmp(name,"header") && strcmp(name,"footer") && strcmp(name,"nav")) {
        JS_FreeCString(ctx,name); return JS_ThrowTypeError(ctx,"unsupported element");
    }
    for (i = 0; i < view->node_count && view->nodes[i].used; ++i) {}
    if (i >= LW_MAX_NODES) { JS_FreeCString(ctx,name); return JS_ThrowRangeError(ctx,"document node limit"); }
    if (lw_reserve_nodes(view,i + 1) != UI_STATUS_OK) { JS_FreeCString(ctx,name); return JS_ThrowOutOfMemory(ctx); }
    if (i == view->node_count) ++view->node_count;
    memset(&view->nodes[i],0,sizeof(view->nodes[i]));
    view->nodes[i].object = JS_UNDEFINED;
    for (k = 0; k < LW_EVENTS; ++k) view->nodes[i].listeners[k] = JS_UNDEFINED;
    view->nodes[i].used = 1; view->nodes[i].uid = ++view->next_uid; view->nodes[i].kind = kind;
    view->nodes[i].parent = view->nodes[i].first_child = view->nodes[i].next_sibling = -1;
    strcpy(view->nodes[i].tag,name); JS_FreeCString(ctx,name); lw_style_default(&view->nodes[i].style);
    return lw_element(view,i);
}

static JSValue lw_document_get(JSContext *ctx, JSValueConst object, int property)
{
    lw_view_t *view=(lw_view_t *)JS_GetContextOpaque(ctx);int i,count=0;(void)object;
    if(property==0)return view->focused>=0?lw_element(view,view->focused):JS_NULL;
    for(i=0;i<view->node_count;++i)if(view->nodes[i].used)++count;
    return JS_NewInt32(ctx,count);
}

static JSValue lw_js_post(JSContext *ctx, JSValueConst object, int argc,
                           JSValueConst *argv)
{
    lw_view_t *view = (lw_view_t *)JS_GetContextOpaque(ctx);
    JSValue json, parsed;
    const char *text;
    ui_status_t status = UI_STATUS_OK;
    (void)object;
    if (!argc) return JS_ThrowTypeError(ctx,"message required");
    json = JS_IsString(argv[0]) ? JS_DupValue(ctx,argv[0]) : JS_JSONStringify(ctx,argv[0],JS_UNDEFINED,JS_UNDEFINED);
    text = JS_ToCString(ctx,json);
    if (!text) { JS_FreeValue(ctx,json); return JS_EXCEPTION; }
    if (strlen(text) > LW_HTML_LIMIT) status = UI_STATUS_VALIDATION_FAILED;
    parsed = status == UI_STATUS_OK ? JS_ParseJSON(ctx,text,strlen(text),"ui.postMessage") : JS_EXCEPTION;
    if (JS_IsException(parsed)) status = UI_STATUS_VALIDATION_FAILED;
    if (status == UI_STATUS_OK && view->message_handler) view->message_handler(text,view->message_user);
    JS_FreeValue(ctx,parsed); JS_FreeCString(ctx,text); JS_FreeValue(ctx,json);
    if (status != UI_STATUS_OK) return JS_ThrowTypeError(ctx,"message rejected");
    return JS_UNDEFINED;
}

static void lw_free_js(lw_view_t *view)
{
    if (view->context) JS_FreeContext(view->context);
    if (view->runtime) JS_FreeRuntime(view->runtime);
    view->context = NULL; view->runtime = NULL;
}

static ui_status_t lw_setup_js(lw_view_t *view)
{
    JSValue global, ui, document;
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
    JS_SetPropertyStr(view->context, ui, "postMessage", JS_NewCFunction(view->context,lw_js_post,"postMessage",1));
    JS_SetPropertyStr(view->context, global, "ui", ui);
    document = JS_NewObject(view->context);
    lw_js_property(view->context,document,"activeElement",0,lw_document_get,NULL);
    lw_js_property(view->context,document,"nodeCount",1,lw_document_get,NULL);
    JS_SetPropertyStr(view->context,document,"getElementById",JS_NewCFunctionMagic(view->context,lw_js_document,"getElementById",1,JS_CFUNC_generic_magic,0));
    JS_SetPropertyStr(view->context,document,"createElement",JS_NewCFunctionMagic(view->context,lw_js_document,"createElement",1,JS_CFUNC_generic_magic,1));
    JS_SetPropertyStr(view->context,document,"addEventListener",JS_NewCFunction(view->context,lw_document_listener,"addEventListener",2));
    JS_SetPropertyStr(view->context,global,"document",document);
    JS_SetPropertyStr(view->context,global,"window",JS_DupValue(view->context,global));
    JS_FreeValue(view->context, global);
    {
        const char *bootstrap = "var __lwMessages=[];window.addEventListener=function(name,fn){if(name!=='message'||typeof fn!=='function')throw new TypeError('unsupported window event');if(__lwMessages.length>=32)throw new RangeError('too many message listeners');__lwMessages.push(fn);};";
        JSValue result = JS_Eval(view->context,bootstrap,strlen(bootstrap),"light-web-bootstrap",JS_EVAL_TYPE_GLOBAL);
        if (JS_IsException(result)) { JS_FreeValue(view->context,result); return UI_STATUS_OUT_OF_MEMORY; }
        JS_FreeValue(view->context,result);
    }
    return UI_STATUS_OK;
}

static ui_status_t lw_eval(lw_view_t *view, const char *script)
{
    JSValue result;
    ui_status_t status = UI_STATUS_OK;
    if (script == NULL || *script == 0) return status;
    ++view->dispatch_depth;
    view->script_deadline = GetTickCount64() + LW_SCRIPT_MILLISECONDS;
    result = JS_Eval(view->context, script, strlen(script), "application-inline", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
        JSValue exception = JS_GetException(view->context);
        JS_FreeValue(view->context, exception);
        status = UI_STATUS_VALIDATION_FAILED;
    }
    JS_FreeValue(view->context, result);
    --view->dispatch_depth;
    return status;
}

static ui_status_t lw_handler(lw_view_t *view, JSValueConst event, const char *script)
{
    JSContext *ctx = view->context;
    JSValue global, previous;
    ui_status_t status;
    global = JS_GetGlobalObject(ctx); previous = JS_GetPropertyStr(ctx,global,"event");
    JS_SetPropertyStr(ctx,global,"event",JS_DupValue(ctx,event));
    status = lw_eval(view, script);
    JS_SetPropertyStr(ctx,global,"event",previous); JS_FreeValue(ctx,global);
    return status;
}

static JSValue lw_js_prevent(JSContext *ctx, JSValueConst object, int argc, JSValueConst *argv)
{
    (void)argc; (void)argv;
    JS_SetPropertyStr(ctx,object,"defaultPrevented",JS_TRUE); return JS_UNDEFINED;
}

static ui_status_t lw_event(lw_view_t *view, int index, int kind, uint32_t key)
{
    JSContext *ctx = view->context;
    JSValue event, target, result;
    ui_status_t status = UI_STATUS_OK;
    unsigned path[LW_MAX_NODES];
    int current = index, count = 0, at;
    if (!ctx || index < 0 || index >= view->node_count || !view->nodes[index].used) return UI_STATUS_OK;
    for (; current >= 0; current = view->nodes[current].parent) path[count++] = view->nodes[current].uid;
    event = JS_NewObject(ctx); target = lw_element(view,index);
    JS_SetPropertyStr(ctx,event,"target",target);
    JS_SetPropertyStr(ctx,event,"keyCode",JS_NewUint32(ctx,key));
    JS_SetPropertyStr(ctx,event,"ctrlKey",JS_NewBool(ctx,(view->event_modifiers & UI_INPUT_MODIFIER_CONTROL) != 0));
    JS_SetPropertyStr(ctx,event,"shiftKey",JS_NewBool(ctx,(view->event_modifiers & UI_INPUT_MODIFIER_SHIFT) != 0));
    JS_SetPropertyStr(ctx,event,"altKey",JS_NewBool(ctx,(view->event_modifiers & UI_INPUT_MODIFIER_ALT) != 0));
    JS_SetPropertyStr(ctx,event,"deltaY",JS_NewInt32(ctx,-view->wheel_delta));
    JS_SetPropertyStr(ctx,event,"clientX",JS_NewInt32(ctx,view->pointer_x));JS_SetPropertyStr(ctx,event,"clientY",JS_NewInt32(ctx,view->pointer_y));
    JS_SetPropertyStr(ctx,event,"pointerId",JS_NewInt32(ctx,1));JS_SetPropertyStr(ctx,event,"button",JS_NewInt32(ctx,0));
    JS_SetPropertyStr(ctx,event,"preventDefault",JS_NewCFunction(ctx,lw_js_prevent,"preventDefault",0));
    ++view->dispatch_depth;
    view->script_deadline = GetTickCount64() + LW_SCRIPT_MILLISECONDS;
    for (at = 0; at < count; ++at) {
        lw_node_t *node;
        JSValue object, listeners;
        const char *inline_script;
        char script[LW_ATTR_CAP];
        current = lw_uid_index(view,path[at]); if (current < 0) continue;
        node = &view->nodes[current];
        object = lw_element(view,current); listeners = JS_DupValue(ctx,node->listeners[kind]);
        inline_script = kind == 0 ? node->onclick : kind == 1 ? node->oninput : kind == 7 ? node->onmousedown : "";
        strcpy(script,inline_script);
        JS_SetPropertyStr(ctx,event,"currentTarget",JS_DupValue(ctx,object));
        if (*script && lw_handler(view,event,script) != UI_STATUS_OK) status = UI_STATUS_VALIDATION_FAILED;
        if (!JS_IsUndefined(listeners)) {
            JSValue length = JS_GetPropertyStr(ctx,listeners,"length"); uint32_t n = 0, i;
            (void)JS_ToUint32(ctx,&n,length); JS_FreeValue(ctx,length);
            for (i = 0; i < n; ++i) {
                JSValue callback = JS_GetPropertyUint32(ctx,listeners,i);
                result = JS_Call(ctx,callback,object,1,&event);
                if (JS_IsException(result)) { lw_report_exception(view);status = UI_STATUS_VALIDATION_FAILED; }
                JS_FreeValue(ctx,result); JS_FreeValue(ctx,callback);
                if (status != UI_STATUS_OK) break;
            }
        }
        JS_FreeValue(ctx,listeners); JS_FreeValue(ctx,object);
        if (kind == 3 || kind == 4 || kind == 5 || kind == 6) break;
    }
    if (kind == 2 || kind>=10) {
        JSValue global = JS_GetGlobalObject(ctx), document = JS_GetPropertyStr(ctx,global,"document");
        static const char *names[]={"__pointerdown","__pointermove","__pointerup","__pointercancel","__lostpointercapture"};
        JSValue listeners = JS_GetPropertyStr(ctx,document,kind==2?"__keydown":names[kind-10]);
        if (!JS_IsUndefined(listeners)) {
            JSValue length = JS_GetPropertyStr(ctx,listeners,"length"); uint32_t n = 0, i;
            (void)JS_ToUint32(ctx,&n,length); JS_FreeValue(ctx,length);
            for (i = 0; i < n; ++i) {
                JSValue callback = JS_GetPropertyUint32(ctx,listeners,i);
                result = JS_Call(ctx,callback,document,1,&event);
                if (JS_IsException(result)) { JSValue exception = JS_GetException(ctx); JS_FreeValue(ctx,exception); status = UI_STATUS_VALIDATION_FAILED; }
                JS_FreeValue(ctx,result); JS_FreeValue(ctx,callback);
            }
        }
        JS_FreeValue(ctx,listeners); JS_FreeValue(ctx,document); JS_FreeValue(ctx,global);
    }
    { JSValue prevented = JS_GetPropertyStr(ctx,event,"defaultPrevented"); view->default_prevented = JS_ToBool(ctx,prevented); JS_FreeValue(ctx,prevented); }
    JS_FreeValue(ctx,event); --view->dispatch_depth;
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
    for (i = 0; i < view->node_count; ++i) if (view->nodes[i].used) lw_release_node(view,i);
    if (view->document) lxb_html_document_destroy(view->document);
    view->document = NULL; view->node_count = 0; view->root = -1;
    free(view->nodes); view->nodes = NULL; view->node_capacity = 0;
    view->media_breakpoint = 0; view->media_hide_assistant = 0;
    view->rule_count = 0; view->hovered = -1;
    free(view->rules); view->rules = NULL; view->rule_capacity = 0;
    view->focused = view->pressed = -1;
    lw_free_js(view);
}

static RECT lw_pixel_rect(const lw_view_t *view, ui_rect_t rect)
{
    RECT r = {lw_pixel(rect.x, view->dpi), lw_pixel(rect.y, view->dpi),
              lw_pixel(rect.x + rect.width, view->dpi), lw_pixel(rect.y + rect.height, view->dpi)};
    return r;
}

static COLORREF lw_rgb(unsigned color)
{
    return RGB((color >> 16) & 255,(color >> 8) & 255,color & 255);
}

static int lw_next_node(const lw_view_t *view, int index)
{
    if (view->nodes[index].first_child >= 0) return view->nodes[index].first_child;
    while (index != view->root) {
        if (view->nodes[index].next_sibling >= 0) return view->nodes[index].next_sibling;
        index = view->nodes[index].parent;
        if (index < 0) return -1;
    }
    return -1;
}

#include "light_text.inc"
#include "light_scroll.inc"

static void lw_paint(lw_view_t *view, HDC dc, RECT client)
{
    int i;
    HBRUSH brush = CreateSolidBrush(RGB(32,34,38));
    FillRect(dc,&client,brush); DeleteObject(brush);
    SetBkMode(dc,TRANSPARENT);
    for (i = view->root; i >= 0; i = lw_next_node(view,i)) {
        lw_node_t *node = &view->nodes[i];
        RECT rect, clip;
        int saved;
        wchar_t *text;
        if (!node->used || !node->visible || !node->clip.width || !node->clip.height) continue;
        rect = lw_pixel_rect(view,node->rect); clip = lw_pixel_rect(view,node->clip);
        saved = SaveDC(dc); IntersectClipRect(dc,clip.left,clip.top,clip.right,clip.bottom);
        if (node->kind == 2 || node->style.background_set || node->style.border) {
            unsigned color = node->style.background_set ? node->style.background : node->kind == 2 ? 0x3968a8u : 0x202226u;
            HPEN pen = CreatePen(PS_SOLID,lw_pixel(node->style.border,view->dpi),lw_rgb(node->style.border_color));
            HGDIOBJ old_pen, old_brush;
            brush = node->style.background_set||node->kind==2?CreateSolidBrush(lw_rgb(color)):NULL;
            old_brush = SelectObject(dc,brush?brush:GetStockObject(NULL_BRUSH));
            old_pen = SelectObject(dc,node->style.border ? pen : GetStockObject(NULL_PEN));
            if (node->style.radius) RoundRect(dc,rect.left,rect.top,rect.right,rect.bottom,
                lw_pixel(node->style.radius * 2,view->dpi),lw_pixel(node->style.radius * 2,view->dpi));
            else Rectangle(dc,rect.left,rect.top,rect.right,rect.bottom);
            SelectObject(dc,old_pen); SelectObject(dc,old_brush); DeleteObject(pen); if(brush)DeleteObject(brush);
        }
        if (lw_input(node)) lw_text_paint(view,node,dc,rect);
        if (node->kind == 5 && node->image) ui_image_draw(view->host,node->image,dc,&rect,node->disabled);
        if (node->kind < 3 && *node->text) {
            UINT flags = DT_NOPREFIX | (node->style.text_align == 1 ? DT_CENTER : node->style.text_align == 2 ? DT_RIGHT : DT_LEFT);
            SetTextColor(dc,lw_rgb(node->disabled ? 0x8e969fu : node->style.color));
            if (node->font) SelectObject(dc,node->font);
            text = lw_wide(node->text);
            if (text) {
                int horizontal = node->style.padding[3] + node->style.border;
                if (!horizontal && node->kind == 2) horizontal = 8;
                rect.left += lw_pixel(horizontal,view->dpi);
                rect.right -= lw_pixel(node->style.padding[1] + node->style.border + (!node->style.padding[1] && node->kind == 2 ? 8 : 0),view->dpi);
                rect.top += lw_pixel(node->style.padding[0] + node->style.border,view->dpi);
                rect.bottom -= lw_pixel(node->style.padding[2] + node->style.border,view->dpi);
                if (node->kind == 2 || node->style.nowrap) flags |= DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
                else flags |= DT_WORDBREAK;
                DrawTextW(dc,text,-1,&rect,flags); free(text);
            }
        }
        if (view->focused == i && !lw_input(node)) {
            RECT focus = rect; InflateRect(&focus,-3,-3); DrawFocusRect(dc,&focus);
        }
        lw_scroll_paint(view,node,dc);
        RestoreDC(dc,saved);
    }
}

static LRESULT lw_wnd_inner(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    lw_view_t *view = (lw_view_t *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    if (message == WM_NCCREATE) {
        view = (lw_view_t *)((CREATESTRUCTW *)lp)->lpCreateParams;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)view); view->hwnd = hwnd; return TRUE;
    }
    if (view == NULL) return DefWindowProcW(hwnd, message, wp, lp);
    if(message==WM_CAPTURECHANGED||message==WM_CANCELMODE||message==WM_KILLFOCUS||(message==WM_SHOWWINDOW&&!wp)){
        if(view->pressed>=0){view->pressed=-1;lw_layout(view);}
    }
    if(message==WM_CAPTURECHANGED||message==WM_CANCELMODE||(message==WM_SHOWWINDOW&&!wp))lw_scroll_cancel(view,1);
    if((message==WM_CAPTURECHANGED||message==WM_CANCELMODE||message==WM_SHOWWINDOW&&!wp)&&view->captured){
        int index=lw_uid_index(view,view->captured);view->captured=0;(void)lw_event(view,index,14,0);if(view->dirty)lw_layout(view);}
    if ((message==WM_LBUTTONDOWN||message==WM_LBUTTONUP||message==WM_MOUSEWHEEL||
         message==WM_MOUSEMOVE||message==WM_RBUTTONUP||message==WM_KEYDOWN||
         message==WM_SYSKEYDOWN||message==WM_CHAR||message==WM_IME_STARTCOMPOSITION||
         message==WM_IME_COMPOSITION)&&!ui_components_input_allowed(view->host,view)) return 0;
    if (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_MOUSEWHEEL || message == WM_MOUSEMOVE || message == WM_RBUTTONUP) {
        ui_input_event_t event = {0};
        POINT point = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        if (message == WM_MOUSEWHEEL) ScreenToClient(hwnd, &point);
        event.size = sizeof(event); event.x = point.x * 96 / (int)view->dpi;
        event.y = point.y * 96 / (int)view->dpi; event.pointer_button = 1;
        event.kind = message == WM_MOUSEMOVE ? UI_INPUT_POINTER_MOVE : message == WM_LBUTTONDOWN ? UI_INPUT_POINTER_DOWN :
                     message == WM_LBUTTONUP ? UI_INPUT_POINTER_UP : UI_INPUT_WHEEL;
        event.wheel_delta = message == WM_MOUSEWHEEL ? GET_WHEEL_DELTA_WPARAM(wp) : 0;
        if (GetKeyState(VK_SHIFT) & 0x8000) event.modifiers |= UI_INPUT_MODIFIER_SHIFT;
        if (GetKeyState(VK_CONTROL) & 0x8000) event.modifiers |= UI_INPUT_MODIFIER_CONTROL;
        if (message == WM_RBUTTONUP) { event.pointer_button=2;event.kind=UI_INPUT_POINTER_UP; }
        if (message == WM_MOUSEMOVE) {
            TRACKMOUSEEVENT track = {sizeof(track),TME_LEAVE,hwnd,0}; TrackMouseEvent(&track);
        }
        if (message == WM_LBUTTONDOWN) SetFocus(hwnd);
        (void)lw_dispatch_input(NULL, view, &event); return 0;
    }
    if (message == WM_MOUSELEAVE) {
        if (view->hovered >= 0) (void)lw_event(view,view->hovered,6,0);
        view->hovered = -1; lw_layout(view); return 0;
    }
    if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN || message == WM_KEYUP || message == WM_SYSKEYUP) {
        ui_input_event_t event = {0};
        event.size = sizeof(event); event.kind = (message==WM_KEYUP||message==WM_SYSKEYUP)?UI_INPUT_KEY_UP:UI_INPUT_KEY_DOWN; event.key_code = (uint32_t)wp;
        if (GetKeyState(VK_SHIFT) & 0x8000) event.modifiers |= UI_INPUT_MODIFIER_SHIFT;
        if (GetKeyState(VK_CONTROL) & 0x8000) event.modifiers |= UI_INPUT_MODIFIER_CONTROL;
        if (GetKeyState(VK_MENU) & 0x8000) event.modifiers |= UI_INPUT_MODIFIER_ALT;
        if (lw_dispatch_input(NULL,view,&event) == UI_STATUS_OK) return 0;
    }
    if (lw_text_message(view,message,wp,lp)) return 0;
    if (message == WM_ERASEBKGND) return 1;
    if (message == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps), memory = CreateCompatibleDC(dc);
        RECT client; HBITMAP bitmap; HGDIOBJ previous;
        GetClientRect(hwnd,&client);
        bitmap = CreateCompatibleBitmap(dc,client.right > 0 ? client.right : 1,client.bottom > 0 ? client.bottom : 1);
        previous = bitmap && memory ? SelectObject(memory,bitmap) : NULL;
        lw_paint(view,previous ? memory : dc,client);
        if (previous) { BitBlt(dc,0,0,client.right,client.bottom,memory,0,0,SRCCOPY); SelectObject(memory,previous); }
        if (bitmap) DeleteObject(bitmap); if (memory) DeleteDC(memory);
        EndPaint(hwnd, &ps); return 0;
    }
    return DefWindowProcW(hwnd, message, wp, lp);
}

static LRESULT CALLBACK lw_wnd_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    lw_view_t *view = (lw_view_t *)GetWindowLongPtrW(hwnd,GWLP_USERDATA);
    ui_host_t *host;
    LRESULT result;
    if (message == WM_NCCREATE) view = (lw_view_t *)((CREATESTRUCTW *)lp)->lpCreateParams;
    if (!view) return DefWindowProcW(hwnd,message,wp,lp);
    host = view->host; ui_dispatch_enter(host);
    result = lw_wnd_inner(hwnd,message,wp,lp);
    ui_dispatch_leave(host); return result;
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
    view->root = view->focused = view->pressed = -1;view->scroll_focus=-1;
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
    lw_scroll_cancel(view,0);lw_free_document(view);
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
    if (view->dispatch_depth) return UI_STATUS_PLATFORM_ERROR;
    length = strlen(html);
    if (length > LW_HTML_LIMIT) return UI_STATUS_VALIDATION_FAILED;
    valid = lw_wide(html); if (!valid) return UI_STATUS_VALIDATION_FAILED; free(valid);
    lw_scroll_cancel(view,0);view->captured=0;view->scroll_focus=-1;if(view->hwnd&&GetCapture()==view->hwnd)ReleaseCapture();
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
    if (status == UI_STATUS_OK) status = lw_reserve_nodes(view,1);
    if (status == UI_STATUS_OK) {
        lw_node_t *root = &view->nodes[0]; int k;
        memset(root,0,sizeof(*root)); root->used = root->attached = 1;
        root->uid = ++view->next_uid; root->kind = 1;
        root->parent = root->first_child = root->next_sibling = -1;
        root->object = JS_UNDEFINED;
        for (k = 0; k < LW_EVENTS; ++k) root->listeners[k] = JS_UNDEFINED;
        strcpy(root->tag,"body");
        view->node_count = 1; view->root = 0;
        status = lw_attr(lxb_dom_interface_element(body),"id",root->id,sizeof(root->id));
        if (status == UI_STATUS_OK) status = lw_attr(lxb_dom_interface_element(body),"class",root->classes,sizeof(root->classes));
        if (status == UI_STATUS_OK) status = lw_attr(lxb_dom_interface_element(body),"style",root->inline_style,sizeof(root->inline_style));
    }
    if (status == UI_STATUS_OK) status = lw_collect(view, body, 0);
    if (status == UI_STATUS_OK) {
        JSValue global = JS_GetGlobalObject(view->context);
        JSValue document = JS_GetPropertyStr(view->context,global,"document");
        JS_SetPropertyStr(view->context,document,"body",lw_element(view,0));
        JS_FreeValue(view->context,document); JS_FreeValue(view->context,global);
    }
    if (status == UI_STATUS_OK) status = lw_collect_scripts(view, lxb_dom_interface_node(view->document), 1, 0);

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
    if(view->width!=rect->width||view->height!=rect->height||view->dpi!=dpi)lw_scroll_cancel(view,1);
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
    int i, hit = -1;
    for (i = view->root; i >= 0; i = lw_next_node(view,i)) {
        const lw_node_t *n = &view->nodes[i];
        if (n->used && n->visible && x >= n->clip.x && y >= n->clip.y &&
            x < n->clip.x + n->clip.width && y < n->clip.y + n->clip.height) hit = i;
    }
    return hit;
}
static int lw_disabled(const lw_view_t *view,int index)
{for(;index>=0;index=view->nodes[index].parent)if(view->nodes[index].disabled)return 1;return 0;}

static ui_status_t lw_dispatch_input(void *user, void *data, const ui_input_event_t *event)
{
    lw_view_t *view = (lw_view_t *)data;
    int hit;
    (void)user;
    if (!view || !event || event->size < sizeof(*event)) return UI_STATUS_INVALID_ARGUMENT;
    if (event->kind==UI_INPUT_CANCEL||!ui_components_input_allowed(view->host,view)) {
        view->pressed=-1;
        lw_scroll_cancel(view,1);
        if(view->captured){int index=lw_uid_index(view,view->captured);view->captured=0;(void)lw_event(view,index,14,0);if(view->hwnd&&GetCapture()==view->hwnd)ReleaseCapture();}
        lw_layout(view);return event->kind==UI_INPUT_CANCEL?UI_STATUS_OK:UI_STATUS_CANCELLED;
    }
    if (!view->context) return UI_STATUS_NOT_FOUND;
    if(ui_menus_route_input(view->host,view,event,view->composing)==UI_STATUS_OK)return UI_STATUS_OK;
    if(lw_scroll_input(view,event))return UI_STATUS_OK;
    view->event_modifiers = event->modifiers; view->wheel_delta = event->wheel_delta;
    view->pointer_x=event->x;view->pointer_y=event->y;
    hit = lw_hit(view, event->x, event->y);
    if(view->captured&&(event->kind==UI_INPUT_POINTER_MOVE||event->kind==UI_INPUT_POINTER_UP))hit=lw_uid_index(view,view->captured);
    if (event->kind == UI_INPUT_POINTER_DOWN || event->kind == UI_INPUT_POINTER_UP) {
        int pressed=view->pressed;
        if (event->pointer_button == 2) {
            if(event->kind==UI_INPUT_POINTER_UP&&hit>=0&&!lw_disabled(view,hit)) {
                ui_status_t status=lw_event(view,hit,9,0);lw_layout(view);return status;
            }
            return UI_STATUS_OK;
        }
        if (event->pointer_button > 1) return UI_STATUS_UNSUPPORTED;
        if (event->kind == UI_INPUT_POINTER_DOWN) {
            int previous = view->focused, focus_hit=hit;
            while(focus_hit>=0&&view->nodes[focus_hit].kind!=2&&!lw_input(&view->nodes[focus_hit]))focus_hit=view->nodes[focus_hit].parent;
            if(view->focused!=focus_hit)lw_cancel_composition(view);
            view->pressed = hit;
            view->focused = focus_hit>=0&&!lw_disabled(view,focus_hit)?focus_hit:-1;
            if (view->hwnd) SetFocus(view->hwnd);
            if(view->hwnd&&focus_hit>=0&&view->nodes[focus_hit].kind==2&&!lw_disabled(view,focus_hit)&&GetCapture()!=view->hwnd)SetCapture(view->hwnd);
            lw_text_pointer(view,hit,event->x,event->y,0);
            if (previous != view->focused) { (void)lw_event(view,previous,4,0); (void)lw_event(view,view->focused,3,0); }
            if (hit >= 0 && !lw_disabled(view,hit)) (void)lw_event(view,hit,7,0);
            if (hit >= 0 && !lw_disabled(view,hit)) (void)lw_event(view,hit,10,0);
            lw_layout(view);
            return UI_STATUS_OK;
        }
        if(hit>=0&&!lw_disabled(view,hit))(void)lw_event(view,hit,12,0);
        view->captured=0;view->dragging = 0; if (view->hwnd && GetCapture() == view->hwnd) ReleaseCapture();
        if (hit >= 0 && hit == pressed && !lw_disabled(view,hit)) {
            ui_status_t status;
            view->pressed = -1; status = lw_event(view,hit,0,0); lw_layout(view); return status;
        }
        view->pressed = -1; lw_layout(view);return UI_STATUS_OK;
    }
    if (event->kind == UI_INPUT_POINTER_MOVE) {
        (void)lw_event(view,hit,11,0);
        if (view->dragging) lw_text_pointer(view,view->focused,event->x,event->y,1);
        if (hit != view->hovered) {
            (void)lw_event(view,view->hovered,6,0); view->hovered = hit;
            (void)lw_event(view,hit,5,0); lw_layout(view);
        }
        if(view->dirty)lw_layout(view);return UI_STATUS_OK;
    }
    if (event->kind == UI_INPUT_WHEEL) {
        (void)lw_event(view,hit,8,0);
        if (view->default_prevented) return UI_STATUS_OK;
        while (hit >= 0 && !view->nodes[hit].style.scroll && !view->nodes[hit].style.scroll_x) hit = view->nodes[hit].parent;
        if (hit < 0) return UI_STATUS_UNSUPPORTED;
        {
            int horizontal = view->nodes[hit].style.scroll_x && (!view->nodes[hit].style.scroll || (event->modifiers & UI_INPUT_MODIFIER_SHIFT));
            int *offset = horizontal ? &view->nodes[hit].scroll_x : &view->nodes[hit].scroll_y;
            int64_t scroll = (int64_t)*offset - (int64_t)event->wheel_delta * 48 / 120;
            *offset = scroll < 0 ? 0 : scroll > LW_MAX_DIMENSION * LW_MAX_NODES
                ? LW_MAX_DIMENSION * LW_MAX_NODES : (int)scroll;
        }
        lw_layout(view); return UI_STATUS_OK;
    }
    if (event->kind == UI_INPUT_TEXT) return lw_text_insert(view,event->text_utf8);
    if (event->kind == UI_INPUT_KEY_DOWN) {
        if (view->composing && (event->key_code == VK_RETURN || event->key_code == VK_ESCAPE)) return UI_STATUS_OK;
        if (lw_text_key(view,event)) return UI_STATUS_OK;
        if (event->key_code == VK_TAB) {
            int direction = event->modifiers & UI_INPUT_MODIFIER_SHIFT ? -1 : 1;
            int i, start = view->focused;
            for (i = 0; i < view->node_count; ++i) {
                start = (start + direction + view->node_count) % view->node_count;
                if (view->nodes[start].used && view->nodes[start].visible && !view->nodes[start].disabled && (view->nodes[start].kind == 2 || lw_input(&view->nodes[start]))) {
                    lw_cancel_composition(view);(void)lw_event(view,view->focused,4,0); view->focused = start;
                    if (view->hwnd) SetFocus(view->hwnd); lw_caret(view);
                    (void)lw_event(view,start,3,0); lw_layout(view); return UI_STATUS_OK;
                }
            }
        }
        if (view->focused >= 0) {
            int focused = view->focused;
            ui_status_t status = lw_event(view,focused,2,event->key_code);
            if (!view->default_prevented && view->focused == focused && view->nodes[focused].used && view->nodes[focused].kind == 2 && (event->key_code == VK_RETURN || event->key_code == VK_SPACE)) status = lw_event(view,focused,0,0);
            if (!view->default_prevented) (void)ui_host_dispatch_shortcut(view->host,event->key_code,event->modifiers,lw_input(&view->nodes[focused]));
            if (view->dirty) lw_layout(view);
            return status;
        }
        { ui_status_t status = lw_event(view,view->root,2,event->key_code);
          if(!view->default_prevented)(void)ui_host_dispatch_shortcut(view->host,event->key_code,event->modifiers,0);
          if (view->dirty) lw_layout(view); return status; }
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
    for (i = 0; i < view->node_count; ++i) if (view->nodes[i].used && strcmp(view->nodes[i].id, id) == 0) {
        *rect = view->nodes[i].rect; return UI_STATUS_OK;
    }
    return UI_STATUS_NOT_FOUND;
}

static ui_status_t lw_set_message_handler(void *user, void *data,
    ui_web_backend_message_fn callback, void *callback_user)
{
    lw_view_t *view = (lw_view_t *)data;
    (void)user;
    if (!view) return UI_STATUS_INVALID_ARGUMENT;
    view->message_handler = callback; view->message_user = callback_user;
    return UI_STATUS_OK;
}

static ui_status_t lw_post_json(void *user, void *data, const char *json)
{
    lw_view_t *view = (lw_view_t *)data;
    JSContext *ctx;
    JSValue parsed, global, ui, callback, result, listeners, length, event;
    ui_status_t status = UI_STATUS_OK;
    wchar_t *valid;
    uint32_t count = 0, i;
    (void)user;
    if (!view || !json) return UI_STATUS_INVALID_ARGUMENT;
    if (!view->context) return UI_STATUS_NOT_FOUND;
    if (strlen(json) > LW_HTML_LIMIT) return UI_STATUS_VALIDATION_FAILED;
    valid = lw_wide(json); if (!valid) return UI_STATUS_VALIDATION_FAILED; free(valid);
    ctx = view->context; ++view->dispatch_depth;
    view->script_deadline = GetTickCount64() + LW_SCRIPT_MILLISECONDS;
    parsed = JS_ParseJSON(ctx,json,strlen(json),"host-message");
    if (JS_IsException(parsed)) {
        lw_report_exception(view);
        JS_FreeValue(ctx,parsed); --view->dispatch_depth; return UI_STATUS_VALIDATION_FAILED;
    }
    global = JS_GetGlobalObject(ctx); ui = JS_GetPropertyStr(ctx,global,"ui");
    callback = JS_GetPropertyStr(ctx,ui,"onmessage");
    if (JS_IsFunction(ctx,callback)) {
        result = JS_Call(ctx,callback,ui,1,&parsed);
        if (JS_IsException(result)) { JSValue exception = JS_GetException(ctx);const char *error=JS_ToCString(ctx,exception);fprintf(stderr,"Web update: %s\n",error?error:"unknown");JS_FreeCString(ctx,error);JS_FreeValue(ctx,exception);status = UI_STATUS_VALIDATION_FAILED; }
        JS_FreeValue(ctx,result);
    }
    JS_FreeValue(ctx,callback); JS_FreeValue(ctx,ui);
    listeners = JS_GetPropertyStr(ctx,global,"__lwMessages");
    length = JS_GetPropertyStr(ctx,listeners,"length");
    (void)JS_ToUint32(ctx,&count,length); JS_FreeValue(ctx,length);
    event = JS_NewObject(ctx); JS_SetPropertyStr(ctx,event,"data",JS_DupValue(ctx,parsed));
    for (i = 0; i < count && status == UI_STATUS_OK; ++i) {
        callback = JS_GetPropertyUint32(ctx,listeners,i);
        result = JS_Call(ctx,callback,global,1,&event);
        if (JS_IsException(result)) { JSValue exception=JS_GetException(ctx);const char *error=JS_ToCString(ctx,exception);fprintf(stderr,"Web listener: %s\n",error?error:"unknown");JS_FreeCString(ctx,error);JS_FreeValue(ctx,exception);status=UI_STATUS_VALIDATION_FAILED; }
        JS_FreeValue(ctx,result); JS_FreeValue(ctx,callback);
    }
    JS_FreeValue(ctx,event); JS_FreeValue(ctx,listeners); JS_FreeValue(ctx,global); JS_FreeValue(ctx,parsed);
    --view->dispatch_depth;
    if (!view->dispatch_depth && view->dirty) lw_layout(view);
    return status;
}

static ui_status_t lw_get_capabilities(void *user, void *data, uint64_t *capabilities)
{
    lw_view_t *view = (lw_view_t *)data;
    (void)user;
    if (!view || !capabilities) return UI_STATUS_INVALID_ARGUMENT;
    *capabilities = UI_WEB_CAP_JSON_MESSAGES | UI_WEB_CAP_DYNAMIC_DOM |
        UI_WEB_CAP_RESPONSIVE_LAYOUT | UI_WEB_CAP_TEXT_INPUT | UI_WEB_CAP_IMAGES | UI_WEB_CAP_WEB_TEXT_EDIT | UI_WEB_CAP_COMPONENTS | UI_WEB_CAP_OFFSCREEN_CAPTURE | UI_WEB_CAP_PRESENTATION_QUERY;
    if (view->hwnd) *capabilities |= UI_WEB_CAP_NATIVE_WINDOW;
    return UI_STATUS_OK;
}

static void *lw_native_handle(void *user, void *data)
{
    lw_view_t *view = (lw_view_t *)data; (void)user;
    return view ? view->hwnd : NULL;
}

static void lw_image_changed(void *user,void *data,uint64_t id)
{
    lw_view_t *view=(lw_view_t *)data;int i;(void)user;
    if(!view||!view->hwnd)return;
    for(i=0;i<view->node_count;++i)if(view->nodes[i].used&&view->nodes[i].image==id&&view->nodes[i].visible){
        RECT rect=lw_pixel_rect(view,view->nodes[i].clip);InvalidateRect(view->hwnd,&rect,FALSE);}
}
static ui_status_t lw_get_presentation(void *user,void *data,const char *id,ui_element_presentation_t *out)
{
    lw_view_t *v=(lw_view_t *)data;int i;(void)user;
    if(!v||!id||!out||out->size<sizeof(*out))return UI_STATUS_INVALID_ARGUMENT;
    if(v->dirty)lw_layout(v);
    for(i=0;i<v->node_count;++i)if(v->nodes[i].used&&!strcmp(v->nodes[i].id,id)){
        lw_node_t *n=&v->nodes[i];memset(out,0,sizeof(*out));out->size=sizeof(*out);out->rect=n->rect;out->clip=n->clip;
        out->visible=n->visible&&n->clip.width>0&&n->clip.height>0;out->enabled=!lw_disabled(v,i);out->focused=v->focused==i;
        strcpy_s(out->text_utf8,sizeof(out->text_utf8),lw_input(n)?n->value:n->text);
        out->text_overflow=n->natural_width>n->rect.width||n->natural_height>n->rect.height;
        return UI_STATUS_OK;
    }return UI_STATUS_NOT_FOUND;
}
static ui_status_t lw_capture(void *user,void *data,ui_pixel_buffer_t *out)
{
    lw_view_t *v=(lw_view_t *)data;uint64_t width,height,bytes;size_t stride;HDC dc;HBITMAP bitmap;HGDIOBJ old;void *bits;BITMAPINFO info={0};RECT r;uint32_t y,x;(void)user;
    if(!v||!out||out->size<sizeof(*out))return UI_STATUS_INVALID_ARGUMENT;
    width=((uint64_t)v->width*v->dpi+48)/96;height=((uint64_t)v->height*v->dpi+48)/96;
    if(!width||!height||width>32767||height>32767||width*height>32u*1024u*1024u/4)return UI_STATUS_LIMIT_EXCEEDED;
    out->width=(uint32_t)width;out->height=(uint32_t)height;stride=out->stride?out->stride:(size_t)width*4;
    if(stride<width*4||stride>SIZE_MAX/(size_t)height)return UI_STATUS_INVALID_ARGUMENT;
    bytes=(height-1)*stride+width*4;out->stride=stride;
    if(!out->pixels)return UI_STATUS_OK;if(bytes>out->capacity)return UI_STATUS_LIMIT_EXCEEDED;
    info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=(LONG)width;info.bmiHeader.biHeight=-(LONG)height;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    dc=CreateCompatibleDC(NULL);if(!dc)return UI_STATUS_PLATFORM_ERROR;
    bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);if(!bitmap){DeleteDC(dc);return UI_STATUS_OUT_OF_MEMORY;}
    old=SelectObject(dc,bitmap);r.left=r.top=0;r.right=(LONG)width;r.bottom=(LONG)height;
    if(v->dirty)lw_layout(v);lw_paint(v,dc,r);GdiFlush();
    for(y=0;y<height;++y)for(x=0;x<width;++x){uint8_t *src=(uint8_t *)bits+((size_t)y*(size_t)width+x)*4,*dst=out->pixels+(size_t)y*stride+(size_t)x*4;
        dst[0]=src[2];dst[1]=src[1];dst[2]=src[0];dst[3]=255;}
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);return UI_STATUS_OK;
}
static ui_status_t lw_flush(void *user,void *data,uint32_t budget)
{
    lw_view_t *v=(lw_view_t *)data;JSContext *ctx;uint32_t i;int result;(void)user;
    if(!v||!v->runtime)return UI_STATUS_NOT_FOUND;
    v->script_deadline=GetTickCount64()+LW_SCRIPT_MILLISECONDS;
    for(i=0;i<budget;++i){result=JS_ExecutePendingJob(v->runtime,&ctx);if(result<0){JSValue exception=JS_GetException(ctx);JS_FreeValue(ctx,exception);return UI_STATUS_PLATFORM_ERROR;}if(!result)break;}
    if(v->dirty)lw_layout(v);return JS_IsJobPending(v->runtime)?UI_STATUS_CANCELLED:UI_STATUS_OK;
}
static const ui_web_backend_ops_t lw_ops = {
    sizeof(ui_web_backend_ops_t), lw_create_view, lw_destroy_view, lw_load_html,
    lw_resize, lw_dispatch_input, lw_invalidate, lw_get_element_rect, lw_set_rect,
    lw_set_message_handler,lw_post_json,lw_get_capabilities,lw_native_handle,lw_image_changed,lw_get_presentation,lw_capture,lw_flush
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
    return "html:lexbor,1024-nodes,256KiB;js:quickjs-ng,50ms,8MiB;layout:flex,absolute,box-model;controls:button,input,textarea;dom:incremental;events:ui.invoke,ui.value,ui.postMessage,ui.onmessage;scroll:vertical;css:class,id,descendant,hover,active,focus,disabled,width-media;thread:ui";
}
