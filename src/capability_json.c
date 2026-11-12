#include "capability_internal.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#endif

void ui_capability_release(void *p) { free(p); }

static uint32_t utf8_next(const unsigned char **p)
{
    uint32_t c = *(*p)++;
    unsigned n = c < 0x80 ? 0 : c < 0xe0 ? 1 : c < 0xf0 ? 2 : 3;
    if (n) c &= (1u << (6u - n)) - 1;
    while (n--) c = (c << 6) | (*(*p)++ & 63u);
    return c;
}

int ui_cap_text(const char *s, size_t max, int empty)
{
    const unsigned char *p = (const unsigned char *)s;
    size_t n = 0;
    if (!s) return 0;
    while (n <= max && s[n]) ++n;
    if (n > max || (!n && !empty)) return 0;
    while (*p) {
        unsigned char a = *p++; uint32_t cp; unsigned rest, i;
        if (a < 0x80) continue;
        if (a >= 0xc2 && a <= 0xdf) { rest = 1; cp = a & 31u; }
        else if (a >= 0xe0 && a <= 0xef) { rest = 2; cp = a & 15u; }
        else if (a >= 0xf0 && a <= 0xf4) { rest = 3; cp = a & 7u; }
        else return 0;
        for (i = 0; i < rest; ++i) {
            if ((*p & 0xc0) != 0x80) return 0;
            cp = (cp << 6) | (*p++ & 63u);
        }
        if ((rest == 1 && cp < 0x80) || (rest == 2 && cp < 0x800) ||
            (rest == 3 && cp < 0x10000) || cp > 0x10ffff ||
            (cp >= 0xd800 && cp <= 0xdfff)) return 0;
    }
    return 1;
}

static int same_string(yyjson_val *a, yyjson_val *b)
{
    return yyjson_is_str(a) && yyjson_is_str(b) &&
        yyjson_get_len(a) == yyjson_get_len(b) &&
        !memcmp(yyjson_get_str(a), yyjson_get_str(b), yyjson_get_len(a));
}

static ui_status_t inspect(yyjson_val *v, unsigned depth, size_t *nodes)
{
    size_t i, max; yyjson_val *key, *value;
    if (++*nodes > 4096) return UI_STATUS_LIMIT_EXCEEDED;
    if (yyjson_is_ctn(v)) {
        if (++depth > 16) return UI_STATUS_LIMIT_EXCEEDED;
        max = yyjson_get_len(v);
        if (max > (yyjson_is_obj(v) ? 128u : 512u)) return UI_STATUS_LIMIT_EXCEEDED;
    }
    if (yyjson_is_obj(v)) {
        yyjson_obj_iter it = yyjson_obj_iter_with(v);
        while ((key = yyjson_obj_iter_next(&it)) != NULL) {
            yyjson_obj_iter prev = yyjson_obj_iter_with(v); yyjson_val *other;
            while ((other = yyjson_obj_iter_next(&prev)) != key)
                if (same_string(key, other)) return UI_STATUS_VALIDATION_FAILED;
            if (++*nodes > 4096) return UI_STATUS_LIMIT_EXCEEDED;
            value = yyjson_obj_iter_get_val(key);
            { ui_status_t s = inspect(value, depth, nodes); if (s) return s; }
        }
    } else if (yyjson_is_arr(v)) {
        yyjson_arr_foreach(v, i, max, value) {
            ui_status_t s = inspect(value, depth, nodes); if (s) return s;
        }
    }
    return UI_STATUS_OK;
}

ui_status_t ui_cap_parse(const char *text, size_t length, size_t limit, ui_cap_json_doc_t *out)
{
    size_t pool_size, nodes = 0; yyjson_alc alc; yyjson_read_err err;
    ui_status_t status;
    memset(out, 0, sizeof(*out));
    if (!text || !length) return UI_STATUS_INVALID_ARGUMENT;
    if (length > limit) return UI_STATUS_LIMIT_EXCEEDED;
    /* A fixed pool bounds allocation before parsing, not only after traversal. */
    pool_size = yyjson_read_max_memory_usage(length, 0);
    if (!pool_size || pool_size > 16u * 1024u * 1024u) return UI_STATUS_LIMIT_EXCEEDED;
    out->pool = malloc(pool_size);
    if (!out->pool) return UI_STATUS_OUT_OF_MEMORY;
    if (!yyjson_alc_pool_init(&alc, out->pool, pool_size)) {
        free(out->pool); out->pool = NULL; return UI_STATUS_OUT_OF_MEMORY;
    }
    out->doc = yyjson_read_opts((char *)text, length, 0, &alc, &err);
    if (!out->doc) {
        free(out->pool); out->pool = NULL;
        return err.code == YYJSON_READ_ERROR_DEPTH ? UI_STATUS_LIMIT_EXCEEDED : UI_STATUS_VALIDATION_FAILED;
    }
    status = inspect(yyjson_doc_get_root(out->doc), 0, &nodes);
    if (status) ui_cap_doc_free(out);
    return status;
}

void ui_cap_doc_free(ui_cap_json_doc_t *p)
{
    if (p->doc) yyjson_doc_free(p->doc);
    free(p->pool); memset(p, 0, sizeof(*p));
}

typedef struct writer { char *data; size_t count; ui_status_t status; } writer_t;
static void put(writer_t *w, const char *s, size_t n)
{
    if (w->status) return;
    if (n > UI_CAPABILITY_OUTPUT_BYTES - w->count) { w->status = UI_STATUS_LIMIT_EXCEEDED; return; }
    memcpy(w->data + w->count, s, n); w->count += n;
}
static void string(writer_t *w, const char *s, size_t n)
{
    size_t i; static const char hex[] = "0123456789abcdef";
    put(w, "\"", 1);
    for (i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c == '"' || c == '\\') { char b[2] = {'\\', (char)c}; put(w, b, 2); }
        else if (c < 32) {
            const char *escape = c == 8 ? "\\b" : c == 9 ? "\\t" : c == 10 ? "\\n" : c == 12 ? "\\f" : c == 13 ? "\\r" : NULL;
            if (escape) put(w, escape, 2);
            else { char b[6] = {'\\','u','0','0',hex[c >> 4],hex[c & 15]}; put(w, b, 6); }
        } else put(w, s + i, 1);
    }
    put(w, "\"", 1);
}

/* UTF-16 ordering differs from UTF-8 ordering for non-BMP keys. */
typedef struct key_cursor { const unsigned char *p, *end; uint32_t low; } key_cursor_t;
static uint32_t unit(key_cursor_t *c)
{
    uint32_t v;
    if (c->low) { v = c->low; c->low = 0; return v; }
    if (c->p == c->end) return UINT32_MAX;
    v = utf8_next(&c->p);
    if (v > 0xffff) { v -= 0x10000; c->low = 0xdc00 + (v & 1023); return 0xd800 + (v >> 10); }
    return v;
}
static int key_compare(const void *aa, const void *bb)
{
    yyjson_val *a = *(yyjson_val *const *)aa, *b = *(yyjson_val *const *)bb;
    key_cursor_t ac = {(const unsigned char *)yyjson_get_str(a), NULL, 0};
    key_cursor_t bc = {(const unsigned char *)yyjson_get_str(b), NULL, 0};
    uint32_t x, y; ac.end = ac.p + yyjson_get_len(a); bc.end = bc.p + yyjson_get_len(b);
    do { x = unit(&ac); y = unit(&bc); if (x != y) return x == UINT32_MAX ? -1 : y == UINT32_MAX ? 1 : x < y ? -1 : 1; } while (x != UINT32_MAX);
    return 0;
}

static void number(writer_t *w, yyjson_val *v)
{
    char raw[64], digits[32], expbuf[16], *end, *p;
    int n = 0, point = -1, exponent = 0, k, start = 0, negative;
    double d = yyjson_get_num(v); yyjson_val real = *v;
    /* JCS numbers have binary64 semantics, including integer-looking tokens.
     * Exact IDs use strings; the integer Schema enforces the safe range. */
    if (!isfinite(d)) { w->status = UI_STATUS_VALIDATION_FAILED; return; }
    if (d == 0) { put(w, "0", 1); return; }
    yyjson_set_real(&real, d); end = yyjson_write_number(&real, raw);
    if (!end || end - raw >= (ptrdiff_t)sizeof(raw)) { w->status = UI_STATUS_VALIDATION_FAILED; return; }
    *end = 0; p = raw; negative = *p == '-'; if (negative) ++p;
    while (*p && *p != 'e' && *p != 'E') {
        if (*p == '.') point = n; else digits[n++] = *p; ++p;
    }
    if (point < 0) point = n;
    if (*p) exponent = atoi(p + 1);
    while (start < n && digits[start] == '0') ++start;
    k = point + exponent - start; n -= start; memmove(digits, digits + start, (size_t)n);
    while (n > 1 && digits[n - 1] == '0') --n;
    if (negative) put(w, "-", 1);
    if (k > 0 && k <= 21) {
        int i;
        for (i = 0; i < k; ++i) put(w, i < n ? digits + i : "0", 1);
        if (n > k) { put(w, ".", 1); put(w, digits + k, (size_t)(n - k)); }
    } else if (k <= 0 && k > -6) {
        put(w, "0.", 2); while (k++ < 0) put(w, "0", 1); put(w, digits, (size_t)n);
    } else {
        int len; put(w, digits, 1);
        if (n > 1) { put(w, ".", 1); put(w, digits + 1, (size_t)(n - 1)); }
        len = snprintf(expbuf, sizeof(expbuf), "e%+d", k - 1); put(w, expbuf, (size_t)len);
    }
}

static void value(writer_t *w, yyjson_val *v)
{
    size_t i, max; yyjson_val *item;
    if (yyjson_is_obj(v)) {
        yyjson_val *keys[128]; size_t n = 0;
        yyjson_obj_iter it = yyjson_obj_iter_with(v);
        while ((item = yyjson_obj_iter_next(&it)) != NULL) keys[n++] = item;
        qsort(keys, n, sizeof(*keys), key_compare); put(w, "{", 1);
        for (i = 0; i < n; ++i) {
            if (i) put(w, ",", 1);
            string(w, yyjson_get_str(keys[i]), yyjson_get_len(keys[i])); put(w, ":", 1);
            value(w, yyjson_obj_iter_get_val(keys[i]));
        }
        put(w, "}", 1);
    } else if (yyjson_is_arr(v)) {
        put(w, "[", 1);
        yyjson_arr_foreach(v, i, max, item) { if (i) put(w, ",", 1); value(w, item); }
        put(w, "]", 1);
    } else if (yyjson_is_str(v)) string(w, yyjson_get_str(v), yyjson_get_len(v));
    else if (yyjson_is_num(v)) number(w, v);
    else if (yyjson_is_null(v)) put(w, "null", 4);
    else if (yyjson_get_bool(v)) put(w, "true", 4);
    else put(w, "false", 5);
}

ui_status_t ui_cap_jcs_value(yyjson_val *v, char **out, size_t *length)
{
    writer_t w; char *small;
    w.data = malloc(UI_CAPABILITY_OUTPUT_BYTES + 1u); w.count = 0; w.status = UI_STATUS_OK;
    if (!w.data) return UI_STATUS_OUT_OF_MEMORY;
    value(&w, v);
    if (w.status) { free(w.data); return w.status; }
    w.data[w.count] = 0; small = malloc(w.count + 1);
    if (!small) { free(w.data); return UI_STATUS_OUT_OF_MEMORY; }
    memcpy(small,w.data,w.count+1);free(w.data);*out=small;
    if (length) *length = w.count;
    return UI_STATUS_OK;
}

ui_status_t ui_cap_json_canonicalize(const char *json, size_t length, char **out, size_t *out_length)
{
    ui_cap_json_doc_t doc; ui_status_t status;
    if (!out || !out_length) return UI_STATUS_INVALID_ARGUMENT;
    *out = NULL; *out_length = 0;
    status = ui_cap_parse(json, length, UI_CAPABILITY_OUTPUT_BYTES, &doc);
    if (!status) { status = ui_cap_jcs_value(yyjson_doc_get_root(doc.doc), out, out_length); ui_cap_doc_free(&doc); }
    return status;
}

char *ui_cap_quote(const char *text)
{
    writer_t w = {NULL, 0, UI_STATUS_OK}; w.data = malloc(UI_CAPABILITY_OUTPUT_BYTES + 1u);
    if (!w.data) return NULL;
    string(&w, text, strlen(text));
    if (w.status) { free(w.data); return NULL; }
    w.data[w.count] = 0; return w.data;
}

ui_status_t ui_cap_hash_bytes(const char *data, size_t n, char hash[65])
{
#ifdef _WIN32
    BCRYPT_ALG_HANDLE alg = NULL; BCRYPT_HASH_HANDLE h = NULL;
    unsigned char digest[32]; size_t i; NTSTATUS status;
    status = BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, NULL, 0);
    if (status >= 0) status = BCryptCreateHash(alg, &h, NULL, 0, NULL, 0, 0);
    if (status >= 0) status = BCryptHashData(h, (PUCHAR)data, (ULONG)n, 0);
    if (status >= 0) status = BCryptFinishHash(h, digest, sizeof(digest), 0);
    if (h) BCryptDestroyHash(h);
    if (alg) BCryptCloseAlgorithmProvider(alg, 0);
    if (status < 0) return UI_STATUS_PLATFORM_ERROR;
    for (i = 0; i < 32; ++i) { hash[i * 2] = "0123456789abcdef"[digest[i] >> 4]; hash[i * 2 + 1] = "0123456789abcdef"[digest[i] & 15]; }
    hash[64] = 0; return UI_STATUS_OK;
#else
    (void)data; (void)n; (void)hash; return UI_STATUS_UNSUPPORTED;
#endif
}

ui_status_t ui_cap_json_fingerprint(const char *json, size_t n, char hash[65])
{
    char *canonical; size_t len; ui_status_t status;
    if (!hash) return UI_STATUS_INVALID_ARGUMENT;
    hash[0] = 0; status = ui_cap_json_canonicalize(json, n, &canonical, &len);
    if (!status) { status = ui_cap_hash_bytes(canonical, len, hash); free(canonical); }
    return status;
}

static int equal(yyjson_val *a, yyjson_val *b)
{
    if (yyjson_is_num(a) && yyjson_is_num(b)) return yyjson_get_num(a) == yyjson_get_num(b);
    if (yyjson_is_str(a) && yyjson_is_str(b)) return same_string(a, b);
    return (yyjson_is_null(a) && yyjson_is_null(b)) ||
        (yyjson_is_bool(a) && yyjson_is_bool(b) && yyjson_get_bool(a) == yyjson_get_bool(b));
}
static int type_is(yyjson_val *s, const char *type) { return yyjson_equals_str(yyjson_obj_get(s, "type"), type); }
static int nonnegative(yyjson_val *v) { return yyjson_is_num(v) && yyjson_get_num(v) >= 0 && yyjson_get_num(v) <= 9007199254740991.0 && floor(yyjson_get_num(v)) == yyjson_get_num(v); }

static int schema(yyjson_val *s)
{
    yyjson_obj_iter it; yyjson_val *key, *v, *props, *required; size_t i, n, j; int valid_type;
    if (!yyjson_is_obj(s)) return 0;
    valid_type = type_is(s,"object") || type_is(s,"array") || type_is(s,"string") || type_is(s,"integer") || type_is(s,"number") || type_is(s,"boolean") || type_is(s,"null");
    if (!valid_type) return 0;
    it = yyjson_obj_iter_with(s);
    while ((key = yyjson_obj_iter_next(&it)) != NULL) {
        v = yyjson_obj_iter_get_val(key);
        if (yyjson_equals_str(key,"type")) continue;
        if (yyjson_equals_str(key,"title") || yyjson_equals_str(key,"description")) { if (!yyjson_is_str(v)) return 0; }
        else if (yyjson_equals_str(key,"examples")) { if (!yyjson_is_arr(v)) return 0; }
        else if (yyjson_equals_str(key,"properties")) {
            yyjson_obj_iter pit; yyjson_val *pk;
            if (!type_is(s,"object") || !yyjson_is_obj(v)) return 0;
            pit = yyjson_obj_iter_with(v);
            while ((pk = yyjson_obj_iter_next(&pit)) != NULL) if (!schema(yyjson_obj_iter_get_val(pk))) return 0;
        } else if (yyjson_equals_str(key,"required")) { if (!type_is(s,"object") || !yyjson_is_arr(v)) return 0; }
        else if (yyjson_equals_str(key,"additionalProperties")) { if (!type_is(s,"object") || !yyjson_is_false(v)) return 0; }
        else if (yyjson_equals_str(key,"items")) { if (!type_is(s,"array") || !schema(v)) return 0; }
        else if (yyjson_equals_str(key,"minItems") || yyjson_equals_str(key,"maxItems")) { if (!type_is(s,"array") || !nonnegative(v)) return 0; }
        else if (yyjson_equals_str(key,"minLength") || yyjson_equals_str(key,"maxLength")) { if (!type_is(s,"string") || !nonnegative(v)) return 0; }
        else if (yyjson_equals_str(key,"minimum") || yyjson_equals_str(key,"maximum")) { if (!(type_is(s,"number") || type_is(s,"integer")) || !yyjson_is_num(v)) return 0; }
        else if (yyjson_equals_str(key,"enum")) {
            yyjson_val *e; if (!yyjson_is_arr(v) || !yyjson_arr_size(v)) return 0;
            yyjson_arr_foreach(v,i,n,e) { if (yyjson_is_ctn(e)) return 0; for(j=0;j<i;++j) if(equal(e,yyjson_arr_get(v,j))) return 0; }
        } else if (yyjson_equals_str(key,"const")) { if (yyjson_is_ctn(v)) return 0; }
        else return 0;
    }
    props = yyjson_obj_get(s,"properties"); required = yyjson_obj_get(s,"required");
    if (type_is(s,"object") && !yyjson_is_false(yyjson_obj_get(s,"additionalProperties"))) return 0;
    if (type_is(s,"array") && !yyjson_obj_get(s,"items")) return 0;
    if (required) {
        yyjson_val *r;
        yyjson_arr_foreach(required,i,n,r) {
            if (!yyjson_is_str(r) || !yyjson_obj_getn(props,yyjson_get_str(r),yyjson_get_len(r))) return 0;
            for(j=0;j<i;++j) if(same_string(r,yyjson_arr_get(required,j))) return 0;
        }
    }
    { const char *lo[] = {"minimum","minLength","minItems"}, *hi[] = {"maximum","maxLength","maxItems"};
      for(i=0;i<3;++i) { yyjson_val *a=yyjson_obj_get(s,lo[i]),*b=yyjson_obj_get(s,hi[i]); if(a&&b&&yyjson_get_num(a)>yyjson_get_num(b))return 0; } }
    return 1;
}

static int validate(yyjson_val *s, yyjson_val *v)
{
    yyjson_val *x; size_t i,n; double d; int found;
    if (type_is(s,"object")) {
        yyjson_val *props=yyjson_obj_get(s,"properties"),*required=yyjson_obj_get(s,"required"),*key;
        yyjson_obj_iter it;
        if(!yyjson_is_obj(v))return 0;
        if(required)yyjson_arr_foreach(required,i,n,x)if(!yyjson_obj_getn(v,yyjson_get_str(x),yyjson_get_len(x)))return 0;
        it=yyjson_obj_iter_with(v);
        while((key=yyjson_obj_iter_next(&it))!=NULL) {
            x=yyjson_obj_getn(props,yyjson_get_str(key),yyjson_get_len(key));
            if(!x||!validate(x,yyjson_obj_iter_get_val(key)))return 0;
        }
    } else if(type_is(s,"array")) {
        if(!yyjson_is_arr(v))return 0;
        n=yyjson_arr_size(v); x=yyjson_obj_get(s,"minItems"); if(x&&n<yyjson_get_num(x))return 0;
        x=yyjson_obj_get(s,"maxItems");if(x&&n>yyjson_get_num(x))return 0;
        yyjson_arr_foreach(v,i,n,x)if(!validate(yyjson_obj_get(s,"items"),x))return 0;
    } else if(type_is(s,"string")) {
        const unsigned char *p,*end; size_t count=0;
        if(!yyjson_is_str(v))return 0;
        p=(const unsigned char *)yyjson_get_str(v);end=p+yyjson_get_len(v);
        while(p<end){(void)utf8_next(&p);++count;}
        x=yyjson_obj_get(s,"minLength");if(x&&count<yyjson_get_num(x))return 0;
        x=yyjson_obj_get(s,"maxLength");if(x&&count>yyjson_get_num(x))return 0;
    } else if(type_is(s,"integer")||type_is(s,"number")) {
        if(!yyjson_is_num(v))return 0;d=yyjson_get_num(v);
        if(!isfinite(d))return 0;
        if(type_is(s,"integer")&&(fabs(d)>9007199254740991.0||floor(d)!=d))return 0;
        x=yyjson_obj_get(s,"minimum");if(x&&d<yyjson_get_num(x))return 0;
        x=yyjson_obj_get(s,"maximum");if(x&&d>yyjson_get_num(x))return 0;
    } else if(type_is(s,"boolean")){if(!yyjson_is_bool(v))return 0;}
    else if(!yyjson_is_null(v))return 0;
    x=yyjson_obj_get(s,"const");if(x&&!equal(x,v))return 0;
    x=yyjson_obj_get(s,"enum");if(x){yyjson_val *e;found=0;yyjson_arr_foreach(x,i,n,e)if(equal(e,v))found=1;if(!found)return 0;}
    return 1;
}

ui_status_t ui_cap_schema_canonical(const char *text, char **out)
{
    ui_cap_json_doc_t doc; ui_status_t status; size_t length=0,canonical_length=0;
    if(!text||!out)return UI_STATUS_INVALID_ARGUMENT;
    *out=NULL;while(length<=UI_CAPABILITY_INPUT_BYTES&&text[length])++length;
    status=ui_cap_parse(text,length,UI_CAPABILITY_INPUT_BYTES,&doc);
    if(status)return status;
    if(!schema(yyjson_doc_get_root(doc.doc)))status=UI_STATUS_VALIDATION_FAILED;
    else {
        status=ui_cap_jcs_value(yyjson_doc_get_root(doc.doc),out,&canonical_length);
        if(!status&&canonical_length>UI_CAPABILITY_INPUT_BYTES){free(*out);*out=NULL;status=UI_STATUS_LIMIT_EXCEEDED;}
    }
    ui_cap_doc_free(&doc);return status;
}
ui_status_t ui_cap_json_validate(const char *text,const char *json,size_t length)
{
    ui_cap_json_doc_t s,v;ui_status_t status;size_t n=0;
    if(!text)return UI_STATUS_INVALID_ARGUMENT;
    while(n<=UI_CAPABILITY_INPUT_BYTES&&text[n])++n;
    status=ui_cap_parse(text,n,UI_CAPABILITY_INPUT_BYTES,&s);if(status)return status;
    if(!schema(yyjson_doc_get_root(s.doc))){ui_cap_doc_free(&s);return UI_STATUS_VALIDATION_FAILED;}
    status=ui_cap_parse(json,length,UI_CAPABILITY_OUTPUT_BYTES,&v);
    if(!status){if(!validate(yyjson_doc_get_root(s.doc),yyjson_doc_get_root(v.doc)))status=UI_STATUS_VALIDATION_FAILED;ui_cap_doc_free(&v);}
    ui_cap_doc_free(&s);return status;
}
