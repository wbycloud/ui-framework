#if !defined(_WIN32) && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 700
#endif

#include "ui_framework/package.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define PACKAGE_HEADER_SIZE 64u
#define PACKAGE_INDEX_SIZE 24u
#define PACKAGE_MANIFEST_LIMIT (64u * 1024u)
#define PACKAGE_NAME_LIMIT 255u

static const unsigned char package_magic[8] = {'U','A','P','P','\r','\n',26,'\n'};

typedef struct package_entry_internal {
    char *name;
    uint64_t offset;
    uint64_t length;
} package_entry_internal_t;

struct ui_package {
    unsigned char *bytes;
    size_t length;
    ui_package_metadata_t metadata;
    package_entry_internal_t *entries;
    size_t entry_count;
};

typedef struct pack_file {
    char *name;
    char *path;
    unsigned char *bytes;
    size_t length;
} pack_file_t;

typedef struct pack_files {
    pack_file_t *files;
    size_t count;
    size_t capacity;
} pack_files_t;

static ui_status_t package_error(ui_status_t status, char *error,
                                 size_t capacity, const char *message)
{
    if (error && capacity) {
        size_t n = strlen(message);
        if (n >= capacity) n = capacity - 1;
        memcpy(error, message, n);
        error[n] = 0;
    }
    return status;
}

static char *package_copy_string(const char *text)
{
    size_t n = strlen(text) + 1;
    char *copy = (char *)malloc(n);
    if (copy) memcpy(copy, text, n);
    return copy;
}

static uint32_t read_u32(const unsigned char *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint64_t read_u64(const unsigned char *p)
{
    return (uint64_t)read_u32(p) | ((uint64_t)read_u32(p + 4) << 32);
}

static void write_u32(unsigned char *p, uint32_t value)
{
    unsigned i;
    for (i = 0; i < 4; ++i) p[i] = (unsigned char)(value >> (i * 8));
}

static void write_u64(unsigned char *p, uint64_t value)
{
    write_u32(p, (uint32_t)value);
    write_u32(p + 4, (uint32_t)(value >> 32));
}

/* Reject NUL, overlong encodings, surrogates and code points above U+10FFFF. */
static int valid_utf8(const unsigned char *p, size_t length)
{
    size_t i = 0;
    while (i < length) {
        uint32_t c = p[i++];
        unsigned count, j;
        uint32_t minimum;
        if (c == 0) return 0;
        if (c < 128) continue;
        if (c >= 0xc2 && c <= 0xdf) { count = 1; c &= 31; minimum = 128; }
        else if (c >= 0xe0 && c <= 0xef) { count = 2; c &= 15; minimum = 2048; }
        else if (c >= 0xf0 && c <= 0xf4) { count = 3; c &= 7; minimum = 65536; }
        else return 0;
        if (length - i < count) return 0;
        for (j = 0; j < count; ++j) {
            uint32_t tail = p[i++];
            if ((tail & 0xc0) != 0x80) return 0;
            c = (c << 6) | (tail & 63);
        }
        if (c < minimum || c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff)) return 0;
    }
    return 1;
}

#ifdef _WIN32
static wchar_t *utf8_to_wide(const char *text)
{
    int count;
    wchar_t *wide;
    if (!text) return NULL;
    count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, NULL, 0);
    if (!count) return NULL;
    wide = (wchar_t *)malloc((size_t)count * sizeof(wchar_t));
    if (!wide) return NULL;
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, wide, count)) {
        free(wide);
        return NULL;
    }
    return wide;
}

static char *wide_to_utf8(const wchar_t *text)
{
    int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text, -1,
                                    NULL, 0, NULL, NULL);
    char *result;
    if (!count) return NULL;
    result = (char *)malloc((size_t)count);
    if (result && !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text, -1,
                                      result, count, NULL, NULL)) {
        free(result);
        return NULL;
    }
    return result;
}

static FILE *utf8_fopen(const char *path, const wchar_t *mode)
{
    wchar_t *wide = utf8_to_wide(path);
    FILE *file = wide ? _wfopen(wide, mode) : NULL;
    free(wide);
    return file;
}

static int names_equal(const char *a, const char *b)
{
    wchar_t wa[PACKAGE_NAME_LIMIT + 1], wb[PACKAGE_NAME_LIMIT + 1];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, a, -1,
                            wa, PACKAGE_NAME_LIMIT + 1) ||
        !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, b, -1,
                            wb, PACKAGE_NAME_LIMIT + 1)) return 0;
    return CompareStringOrdinal(wa, -1, wb, -1, TRUE) == CSTR_EQUAL;
}

static int names_conflict(const char *a, const char *b)
{
    wchar_t wa[PACKAGE_NAME_LIMIT + 1], wb[PACKAGE_NAME_LIMIT + 1];
    size_t na, nb, n;
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, a, -1, wa, PACKAGE_NAME_LIMIT + 1) ||
        !MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, b, -1, wb, PACKAGE_NAME_LIMIT + 1)) return 0;
    na = wcslen(wa); nb = wcslen(wb); n = na < nb ? na : nb;
    if (CompareStringOrdinal(wa, (int)n, wb, (int)n, TRUE) != CSTR_EQUAL) return 0;
    return na == nb || (na < nb ? wb[n] == L'/' : wa[n] == L'/');
}
#else
static int names_equal(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a++) != tolower((unsigned char)*b++)) return 0;
    }
    return *a == *b;
}

static int names_conflict(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a++) != tolower((unsigned char)*b++)) return 0;
    }
    return (*a == 0 && (*b == 0 || *b == '/')) || (*b == 0 && *a == '/');
}
#endif

static int reserved_component(const char *start, size_t length)
{
    char base[16];
    size_t i = 0;
    while (i < length && start[i] != '.') {
        if (i >= sizeof(base) - 1) return 0;
        base[i] = (char)toupper((unsigned char)start[i]);
        ++i;
    }
    base[i] = 0;
    if (!strcmp(base, "CON") || !strcmp(base, "PRN") ||
        !strcmp(base, "AUX") || !strcmp(base, "NUL") ||
        !strcmp(base, "CLOCK$") || !strcmp(base, "CONIN$") ||
        !strcmp(base, "CONOUT$")) return 1;
    if (i == 4 && (!memcmp(base, "COM", 3) || !memcmp(base, "LPT", 3)) &&
        base[3] >= '1' && base[3] <= '9') return 1;
    return i == 5 && (!memcmp(base, "COM", 3) || !memcmp(base, "LPT", 3)) &&
           (unsigned char)base[3] == 0xc2 &&
           ((unsigned char)base[4] == 0xb9 || (unsigned char)base[4] == 0xb2 ||
            (unsigned char)base[4] == 0xb3);
}

static int valid_entry_name(const char *name)
{
    size_t n = strlen(name), start = 0, i;
    if (!n || n > PACKAGE_NAME_LIMIT || !valid_utf8((const unsigned char *)name, n)) return 0;
    for (i = 0; i <= n; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (i == n || c == '/') {
            size_t length = i - start;
            if (!length || (length == 1 && name[start] == '.') ||
                (length == 2 && name[start] == '.' && name[start + 1] == '.') ||
                name[i - 1] == '.' || name[i - 1] == ' ' ||
                reserved_component(name + start, length)) return 0;
            start = i + 1;
        } else if (c < 32 || c == 127 || strchr("\\:<>\"|?*", c)) return 0;
    }
    return 1;
}

static ui_status_t read_file(const char *path, size_t maximum,
                              unsigned char **bytes, size_t *length,
                              char *error, size_t capacity)
{
    FILE *file;
    unsigned char *buffer;
    int seek_result;
#ifdef _WIN32
    __int64 file_length;
    file = utf8_fopen(path, L"rb");
    if (!file) return package_error(UI_STATUS_PLATFORM_ERROR, error, capacity, "Cannot open input file.");
    seek_result = _fseeki64(file, 0, SEEK_END);
    file_length = seek_result ? -1 : _ftelli64(file);
    seek_result = _fseeki64(file, 0, SEEK_SET);
#else
    long file_length;
    file = fopen(path, "rb");
    if (!file) return package_error(UI_STATUS_PLATFORM_ERROR, error, capacity, "Cannot open input file.");
    seek_result = fseek(file, 0, SEEK_END);
    file_length = seek_result ? -1 : ftell(file);
    seek_result = fseek(file, 0, SEEK_SET);
#endif
    if (file_length < 0 || seek_result || (uint64_t)file_length > maximum) {
        fclose(file);
        return package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Input file is too large or unreadable.");
    }
    buffer = (unsigned char *)malloc((size_t)file_length + 1);
    if (!buffer) { fclose(file); return package_error(UI_STATUS_OUT_OF_MEMORY, error, capacity, "Out of memory reading input."); }
    if (fread(buffer, 1, (size_t)file_length, file) != (size_t)file_length || ferror(file)) {
        free(buffer);
        fclose(file);
        return package_error(UI_STATUS_PLATFORM_ERROR, error, capacity, "Failed to read complete input file.");
    }
    if (fgetc(file) != EOF) {
        free(buffer);
        fclose(file);
        return package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Input file changed while being read.");
    }
    fclose(file);
    buffer[file_length] = 0;
    *bytes = buffer;
    *length = (size_t)file_length;
    return UI_STATUS_OK;
}

static char *trim_ascii(char *text)
{
    char *end;
    while (*text == ' ' || *text == '\t') ++text;
    end = text + strlen(text);
    while (end > text && (end[-1] == ' ' || end[-1] == '\t')) --end;
    *end = 0;
    return text;
}

static int copy_value(char *target, size_t capacity, const char *value)
{
    size_t length = strlen(value), i;
    if (!length || length >= capacity) return 0;
    for (i = 0; i < length; ++i)
        if ((unsigned char)value[i] < 32 || (unsigned char)value[i] == 127) return 0;
    memcpy(target, value, length + 1);
    return 1;
}

static int parse_u32(const char *value, uint32_t *result)
{
    uint32_t number = 0;
    const unsigned char *p = (const unsigned char *)value;
    if (!*p) return 0;
    while (*p) {
        unsigned digit;
        if (*p < '0' || *p > '9') return 0;
        digit = *p++ - '0';
        if (number > (UINT32_MAX - digit) / 10) return 0;
        number = number * 10 + digit;
    }
    *result = number;
    return 1;
}

static ui_status_t parse_manifest(const unsigned char *bytes, size_t length,
                                   ui_package_metadata_t *metadata,
                                   char *error, size_t capacity)
{
    static const char *keys[] = {"app_id", "name", "version", "architecture",
        "abi_version", "framework_api_version", "module", "multiple_instances"};
    char *copy, *line;
    unsigned found = 0;
    int section = 0;
    ui_status_t status = UI_STATUS_VALIDATION_FAILED;
    if (!length || length > PACKAGE_MANIFEST_LIMIT || !valid_utf8(bytes, length))
        return package_error(status, error, capacity, "Manifest is empty, too large, or invalid UTF-8.");
    copy = (char *)malloc(length + 1);
    if (!copy) return package_error(UI_STATUS_OUT_OF_MEMORY, error, capacity, "Out of memory reading manifest.");
    memcpy(copy, bytes, length);
    copy[length] = 0;
    memset(metadata, 0, sizeof(*metadata));
    metadata->size = sizeof(*metadata);
    line = copy;
    if (length >= 3 && bytes[0] == 0xef && bytes[1] == 0xbb && bytes[2] == 0xbf) line += 3;
    while (*line) {
        char *next = strchr(line, '\n');
        char *text, *equals, *key, *value;
        size_t n;
        unsigned index;
        int valid = 1;
        if (next) *next++ = 0;
        n = strlen(line);
        if (n && line[n - 1] == '\r') line[n - 1] = 0;
        text = trim_ascii(line);
        if (!*text || *text == '#' || *text == ';') goto next_line;
        if (*text == '[') {
            if (section || strcmp(text, "[application]")) goto invalid;
            section = 1;
            goto next_line;
        }
        if (!section) goto invalid;
        equals = strchr(text, '=');
        if (!equals) goto invalid;
        *equals = 0;
        key = trim_ascii(text);
        value = trim_ascii(equals + 1);
        for (index = 0; index < 8; ++index) if (!strcmp(key, keys[index])) break;
        if (index == 8 || (found & (1u << index))) goto invalid;
        found |= 1u << index;
        switch (index) {
        case 0: valid = copy_value(metadata->app_id, sizeof(metadata->app_id), value); break;
        case 1: valid = copy_value(metadata->name, sizeof(metadata->name), value); break;
        case 2: valid = copy_value(metadata->version, sizeof(metadata->version), value); break;
        case 3: valid = copy_value(metadata->architecture, sizeof(metadata->architecture), value); break;
        case 4: valid = parse_u32(value, &metadata->abi_version); break;
        case 5: valid = parse_u32(value, &metadata->framework_api_version); break;
        case 6: valid = copy_value(metadata->module, sizeof(metadata->module), value); break;
        case 7:
            if (!strcmp(value, "true")) metadata->multiple_instances = 1;
            else if (!strcmp(value, "false")) metadata->multiple_instances = 0;
            else valid = 0;
            break;
        }
        if (!valid) goto invalid;
next_line:
        if (!next) break;
        line = next;
    }
    if (found != 255 || !section) goto invalid;
    {
        const unsigned char *id = (const unsigned char *)metadata->app_id;
        size_t module_length = strlen(metadata->module);
        if (!((*id >= 'A' && *id <= 'Z') || (*id >= 'a' && *id <= 'z') || (*id >= '0' && *id <= '9'))) goto invalid;
        while (*id) {
            if (!((*id >= 'A' && *id <= 'Z') || (*id >= 'a' && *id <= 'z') ||
                  (*id >= '0' && *id <= '9') || *id == '.' || *id == '_' || *id == '-')) goto invalid;
            ++id;
        }
        if (!valid_entry_name(metadata->module) || module_length < 5 ||
            !names_equal(metadata->module + module_length - 4, ".dll")) goto invalid;
    }
    if (strcmp(metadata->architecture, "x64") ||
        metadata->abi_version != UI_PACKAGE_APPLICATION_ABI_VERSION ||
        metadata->framework_api_version != UI_FRAMEWORK_API_VERSION) {
        status = package_error(UI_STATUS_UNSUPPORTED, error, capacity, "Application architecture, ABI, or framework API is unsupported.");
        free(copy);
        return status;
    }
    free(copy);
    return UI_STATUS_OK;
invalid:
    free(copy);
    return package_error(status, error, capacity, "Manifest has missing, duplicate, unknown, or invalid fields.");
}

UI_API ui_status_t ui_package_open(const char *path_utf8, ui_package_t **out,
                                    char *error, size_t error_capacity)
{
    ui_package_t *package;
    ui_status_t status;
    uint64_t manifest_offset, manifest_length, index_offset, data_offset, cursor, data_cursor;
    uint32_t count;
    size_t i, j;
    int module_found = 0;
    if (out) *out = NULL;
    if (error && error_capacity) error[0] = 0;
    if (!path_utf8 || !*path_utf8 || !out)
        return package_error(UI_STATUS_INVALID_ARGUMENT, error, error_capacity, "Package path and output are required.");
    package = (ui_package_t *)calloc(1, sizeof(*package));
    if (!package) return package_error(UI_STATUS_OUT_OF_MEMORY, error, error_capacity, "Out of memory opening package.");
    status = read_file(path_utf8, UI_PACKAGE_MAX_BYTES, &package->bytes, &package->length, error, error_capacity);
    if (status != UI_STATUS_OK) goto failed;
    status = UI_STATUS_VALIDATION_FAILED;
    if (package->length < PACKAGE_HEADER_SIZE || memcmp(package->bytes, package_magic, 8)) goto invalid;
    if (read_u32(package->bytes + 8) != UI_PACKAGE_FORMAT_VERSION) {
        status = package_error(UI_STATUS_UNSUPPORTED, error, error_capacity, "Unsupported application package format version.");
        goto failed;
    }
    manifest_offset = read_u64(package->bytes + 24);
    manifest_length = read_u64(package->bytes + 32);
    index_offset = read_u64(package->bytes + 40);
    count = read_u32(package->bytes + 48);
    data_offset = read_u64(package->bytes + 56);
    if (read_u32(package->bytes + 12) != PACKAGE_HEADER_SIZE ||
        read_u64(package->bytes + 16) != package->length ||
        read_u32(package->bytes + 52) || manifest_offset != PACKAGE_HEADER_SIZE ||
        !manifest_length || manifest_length > PACKAGE_MANIFEST_LIMIT ||
        manifest_length > package->length - PACKAGE_HEADER_SIZE ||
        index_offset != manifest_offset + manifest_length ||
        data_offset < index_offset || data_offset > package->length ||
        !count || count > UI_PACKAGE_MAX_ENTRIES ||
        (uint64_t)count * (PACKAGE_INDEX_SIZE + 1) > data_offset - index_offset) goto invalid;
    status = parse_manifest(package->bytes + (size_t)manifest_offset, (size_t)manifest_length,
                            &package->metadata, error, error_capacity);
    if (status != UI_STATUS_OK) goto failed;
    if (sizeof(void *) != 8) {
        status = package_error(UI_STATUS_UNSUPPORTED, error, error_capacity, "The application requires a 64-bit host.");
        goto failed;
    }
    package->entries = (package_entry_internal_t *)calloc(count, sizeof(*package->entries));
    if (!package->entries) {
        status = package_error(UI_STATUS_OUT_OF_MEMORY, error, error_capacity, "Out of memory reading package index.");
        goto failed;
    }
    package->entry_count = count;
    cursor = index_offset;
    data_cursor = data_offset;
    for (i = 0; i < count; ++i) {
        uint32_t name_length;
        package_entry_internal_t *entry = package->entries + i;
        const unsigned char *record;
        if (cursor > data_offset || data_offset - cursor < PACKAGE_INDEX_SIZE) goto invalid;
        record = package->bytes + (size_t)cursor;
        name_length = read_u32(record);
        if (!name_length || name_length > PACKAGE_NAME_LIMIT || read_u32(record + 4) ||
            name_length > data_offset - cursor - PACKAGE_INDEX_SIZE) goto invalid;
        entry->offset = read_u64(record + 8);
        entry->length = read_u64(record + 16);
        cursor += PACKAGE_INDEX_SIZE;
        if (entry->offset != data_cursor || entry->length > package->length - data_cursor ||
            !valid_utf8(package->bytes + (size_t)cursor, name_length)) goto invalid;
        entry->name = (char *)malloc((size_t)name_length + 1);
        if (!entry->name) {
            status = package_error(UI_STATUS_OUT_OF_MEMORY, error, error_capacity, "Out of memory reading resource name.");
            goto failed;
        }
        memcpy(entry->name, package->bytes + (size_t)cursor, name_length);
        entry->name[name_length] = 0;
        if (!valid_entry_name(entry->name)) goto invalid;
        for (j = 0; j < i; ++j) if (names_conflict(entry->name, package->entries[j].name)) goto invalid;
        if (!strcmp(entry->name, package->metadata.module)) module_found = 1;
        cursor += name_length;
        data_cursor += entry->length;
    }
    if (cursor != data_offset || data_cursor != package->length || !module_found) goto invalid;
    *out = package;
    return UI_STATUS_OK;
invalid:
    status = package_error(UI_STATUS_VALIDATION_FAILED, error, error_capacity, "Malformed package header, index, resource path, or module entry.");
failed:
    ui_package_destroy(package);
    return status;
}

UI_API void ui_package_destroy(ui_package_t *package)
{
    size_t i;
    if (!package) return;
    for (i = 0; i < package->entry_count; ++i) free(package->entries[i].name);
    free(package->entries);
    free(package->bytes);
    free(package);
}

UI_API const ui_package_metadata_t *ui_package_get_metadata(const ui_package_t *package)
{
    return package ? &package->metadata : NULL;
}

UI_API size_t ui_package_entry_count(const ui_package_t *package)
{
    return package ? package->entry_count : 0;
}

UI_API ui_status_t ui_package_get_entry(const ui_package_t *package, size_t index,
                                         ui_package_entry_t *entry)
{
    if (!package || !entry) return UI_STATUS_INVALID_ARGUMENT;
    if (index >= package->entry_count) return UI_STATUS_NOT_FOUND;
    entry->name = package->entries[index].name;
    entry->length = package->entries[index].length;
    return UI_STATUS_OK;
}

UI_API ui_status_t ui_package_read(const ui_package_t *package, const char *name,
                                    void **data, size_t *length)
{
    size_t i;
    if (data) *data = NULL;
    if (length) *length = 0;
    if (!package || !name || !data || !length) return UI_STATUS_INVALID_ARGUMENT;
    for (i = 0; i < package->entry_count; ++i) {
        const package_entry_internal_t *entry = package->entries + i;
        if (!strcmp(name, entry->name)) {
            unsigned char *copy = (unsigned char *)malloc((size_t)entry->length + 1);
            if (!copy) return UI_STATUS_OUT_OF_MEMORY;
            memcpy(copy, package->bytes + (size_t)entry->offset, (size_t)entry->length);
            copy[entry->length] = 0;
            *data = copy;
            *length = (size_t)entry->length;
            return UI_STATUS_OK;
        }
    }
    return UI_STATUS_NOT_FOUND;
}

UI_API void ui_package_release(void *data) { free(data); }

static char *join_path(const char *root, const char *suffix)
{
    size_t a = strlen(root), b = strlen(suffix);
    char *path;
    if (a > SIZE_MAX - b - 2) return NULL;
    path = (char *)malloc(a + b + 2);
    if (path) { memcpy(path, root, a); path[a] = '/'; memcpy(path + a + 1, suffix, b + 1); }
    return path;
}

static void destroy_pack_files(pack_files_t *list)
{
    size_t i;
    for (i = 0; i < list->count; ++i) {
        free(list->files[i].name);
        free(list->files[i].path);
        free(list->files[i].bytes);
    }
    free(list->files);
}

static ui_status_t add_pack_file(pack_files_t *list, const char *name,
                                  const char *path, char *error, size_t capacity)
{
    pack_file_t *file;
    size_t i;
    if (!valid_entry_name(name)) return package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Source file has an unsafe or unsupported resource name.");
    if (list->count >= UI_PACKAGE_MAX_ENTRIES) return package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Too many source files.");
    for (i = 0; i < list->count; ++i) if (names_conflict(name, list->files[i].name))
        return package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Source has duplicate or conflicting resource names ignoring case.");
    if (list->count == list->capacity) {
        size_t next = list->capacity ? list->capacity * 2 : 16;
        pack_file_t *files = (pack_file_t *)realloc(list->files, next * sizeof(*files));
        if (!files) return package_error(UI_STATUS_OUT_OF_MEMORY, error, capacity, "Out of memory enumerating source files.");
        list->files = files;
        list->capacity = next;
    }
    file = list->files + list->count;
    memset(file, 0, sizeof(*file));
    file->name = package_copy_string(name);
    file->path = package_copy_string(path);
    if (!file->name || !file->path) {
        free(file->name); free(file->path);
        return package_error(UI_STATUS_OUT_OF_MEMORY, error, capacity, "Out of memory storing source file.");
    }
    ++list->count;
    return UI_STATUS_OK;
}

static ui_status_t enumerate_files(pack_files_t *list, const char *root,
                                    const char *relative, unsigned depth,
                                    char *error, size_t capacity)
{
    char *directory = *relative ? join_path(root, relative) : package_copy_string(root);
    ui_status_t status = UI_STATUS_OK;
    if (depth > PACKAGE_NAME_LIMIT || !directory) {
        free(directory);
        return package_error(depth > PACKAGE_NAME_LIMIT ? UI_STATUS_VALIDATION_FAILED : UI_STATUS_OUT_OF_MEMORY,
                              error, capacity, "Source directory is too deep or allocation failed.");
    }
#ifdef _WIN32
    {
        wchar_t *wide = utf8_to_wide(directory), *pattern;
        WIN32_FIND_DATAW data;
        HANDLE search;
        DWORD attributes;
        size_t n;
        if (!wide) { status = UI_STATUS_INVALID_ARGUMENT; goto windows_done; }
        attributes = GetFileAttributesW(wide);
        if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
            status = package_error(UI_STATUS_PLATFORM_ERROR, error, capacity, "Source directory is unavailable.");
            free(wide); goto windows_done;
        }
        if (attributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            status = package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Source must not contain symlinks, junctions, or reparse points.");
            free(wide); goto windows_done;
        }
        n = wcslen(wide);
        pattern = (wchar_t *)malloc((n + 3) * sizeof(wchar_t));
        if (!pattern) { free(wide); status = UI_STATUS_OUT_OF_MEMORY; goto windows_done; }
        memcpy(pattern, wide, n * sizeof(wchar_t));
        pattern[n] = L'/'; pattern[n + 1] = L'*'; pattern[n + 2] = 0;
        free(wide);
        search = FindFirstFileW(pattern, &data);
        free(pattern);
        if (search == INVALID_HANDLE_VALUE) {
            status = package_error(UI_STATUS_PLATFORM_ERROR, error, capacity, "Cannot enumerate source directory.");
            goto windows_done;
        }
        do {
            char *name, *resource, *path;
            if (!wcscmp(data.cFileName, L".") || !wcscmp(data.cFileName, L"..")) continue;
            if (data.dwFileAttributes & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DEVICE)) {
                status = package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Source contains a symlink, reparse point, or device.");
                break;
            }
            name = wide_to_utf8(data.cFileName);
            resource = name ? (*relative ? join_path(relative, name) : package_copy_string(name)) : NULL;
            path = name ? join_path(directory, name) : NULL;
            if (!resource || !path) status = UI_STATUS_OUT_OF_MEMORY;
            else if (!valid_entry_name(resource)) status = package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Source contains an unsafe resource path.");
            else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                status = enumerate_files(list, root, resource, depth + 1, error, capacity);
            else status = add_pack_file(list, resource, path, error, capacity);
            free(name); free(resource); free(path);
            if (status != UI_STATUS_OK) break;
        } while (FindNextFileW(search, &data));
        if (status == UI_STATUS_OK && GetLastError() != ERROR_NO_MORE_FILES)
            status = package_error(UI_STATUS_PLATFORM_ERROR, error, capacity, "Source enumeration failed.");
        FindClose(search);
windows_done:;
    }
#else
    {
        struct stat st;
        DIR *search;
        struct dirent *entry;
        if (lstat(directory, &st) || !S_ISDIR(st.st_mode)) {
            status = package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Source directory is unavailable or a symlink.");
            goto posix_done;
        }
        search = opendir(directory);
        if (!search) { status = UI_STATUS_PLATFORM_ERROR; goto posix_done; }
        for (;;) {
            char *resource, *path;
            errno = 0;
            entry = readdir(search);
            if (!entry) {
                if (errno) status = UI_STATUS_PLATFORM_ERROR;
                break;
            }
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
            resource = *relative ? join_path(relative, entry->d_name) : package_copy_string(entry->d_name);
            path = join_path(directory, entry->d_name);
            if (!resource || !path) status = UI_STATUS_OUT_OF_MEMORY;
            else if (!valid_entry_name(resource) || lstat(path, &st)) status = UI_STATUS_VALIDATION_FAILED;
            else if (S_ISDIR(st.st_mode)) status = enumerate_files(list, root, resource, depth + 1, error, capacity);
            else if (S_ISREG(st.st_mode)) status = add_pack_file(list, resource, path, error, capacity);
            else status = package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Source contains a symlink or non-regular file.");
            free(resource); free(path);
            if (status != UI_STATUS_OK) break;
        }
        closedir(search);
posix_done:;
    }
#endif
    free(directory);
    return status;
}

static int pack_file_compare(const void *a, const void *b)
{
    return strcmp(((const pack_file_t *)a)->name, ((const pack_file_t *)b)->name);
}

#ifdef _WIN32
static wchar_t *absolute_wide_path(const char *path)
{
    wchar_t *wide = utf8_to_wide(path), *absolute;
    DWORD count;
    if (!wide) return NULL;
    count = GetFullPathNameW(wide, 0, NULL, NULL);
    if (!count) { free(wide); return NULL; }
    absolute = (wchar_t *)malloc((size_t)count * sizeof(wchar_t));
    if (absolute && !GetFullPathNameW(wide, count, absolute, NULL)) {
        free(absolute); absolute = NULL;
    }
    free(wide);
    return absolute;
}

static int output_is_safe(const char *source, const char *manifest, const char *output)
{
    wchar_t *s = absolute_wide_path(source), *m = absolute_wide_path(manifest), *o = absolute_wide_path(output);
    int safe = 0;
    size_t n;
    if (!s || !m || !o) goto done;
    n = wcslen(s);
    while (n > 0 && (s[n - 1] == L'\\' || s[n - 1] == L'/')) --n;
    if (wcslen(o) >= n && CompareStringOrdinal(s, (int)n, o, (int)n, TRUE) == CSTR_EQUAL &&
        (o[n] == 0 || o[n] == L'\\' || o[n] == L'/')) goto done;
    if (CompareStringOrdinal(m, -1, o, -1, TRUE) == CSTR_EQUAL) goto done;
    safe = 1;
done:
    free(s); free(m); free(o);
    return safe;
}

static char *absolute_utf8_path(const char *path)
{
    wchar_t *wide = absolute_wide_path(path);
    char *result = wide ? wide_to_utf8(wide) : NULL;
    free(wide);
    return result;
}
#else
static int output_is_safe(const char *source, const char *manifest, const char *output)
{
    char *s = realpath(source, NULL), *m = realpath(manifest, NULL);
    char *copy = package_copy_string(output), *slash, *parent, *o = NULL;
    int safe = 0;
    size_t n;
    if (!s || !m || !copy) goto done;
    slash = strrchr(copy, '/');
    if (slash) { *slash = 0; parent = realpath(*copy ? copy : "/", NULL); }
    else parent = realpath(".", NULL);
    if (parent) { o = join_path(parent, slash ? slash + 1 : copy); free(parent); }
    if (!o) goto done;
    n = strlen(s);
    if (!strncmp(s, o, n) && (o[n] == 0 || o[n] == '/')) goto done;
    if (!strcmp(m, o)) goto done;
    safe = 1;
done:
    free(s); free(m); free(copy); free(o);
    return safe;
}
#endif

UI_API ui_status_t ui_package_pack(const char *manifest_path, const char *source,
                                    const char *output, char *error, size_t capacity)
{
    unsigned char *manifest = NULL, *buffer = NULL;
    size_t manifest_length = 0, i, cursor, data_cursor;
    uint64_t index_length = 0, data_length = 0, total;
    ui_package_metadata_t metadata;
    pack_files_t list = {0};
    ui_status_t status;
    FILE *file;
    int module_found = 0;
#ifdef _WIN32
    char *absolute_manifest = NULL, *absolute_source = NULL, *absolute_output = NULL;
#endif
    if (error && capacity) error[0] = 0;
    if (!manifest_path || !*manifest_path || !source || !*source || !output || !*output)
        return package_error(UI_STATUS_INVALID_ARGUMENT, error, capacity, "Manifest, source directory, and output paths are required.");
#ifdef _WIN32
    /* Resolve each input once: worker activity changing the process CWD must
     * not redirect a later file read or the final output write. */
    absolute_manifest = absolute_utf8_path(manifest_path);
    absolute_source = absolute_utf8_path(source);
    absolute_output = absolute_utf8_path(output);
    if (!absolute_manifest || !absolute_source || !absolute_output) {
        status = package_error(UI_STATUS_INVALID_ARGUMENT, error, capacity, "Cannot resolve package paths as UTF-8 absolute paths.");
        goto done;
    }
    manifest_path = absolute_manifest;
    source = absolute_source;
    output = absolute_output;
#endif
    if (!output_is_safe(source, manifest_path, output))
    {
        status = package_error(UI_STATUS_INVALID_ARGUMENT, error, capacity, "Output must be outside the source directory and must not replace the manifest.");
        goto done;
    }
    status = read_file(manifest_path, PACKAGE_MANIFEST_LIMIT, &manifest, &manifest_length, error, capacity);
    if (status != UI_STATUS_OK) goto done;
    status = parse_manifest(manifest, manifest_length, &metadata, error, capacity);
    if (status != UI_STATUS_OK) goto done;
    status = enumerate_files(&list, source, "", 0, error, capacity);
    if (status != UI_STATUS_OK) goto done;
    if (!list.count) { status = package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Source directory has no application files."); goto done; }
    qsort(list.files, list.count, sizeof(*list.files), pack_file_compare);
    for (i = 0; i < list.count; ++i) {
        pack_file_t *entry = list.files + i;
        index_length += PACKAGE_INDEX_SIZE + strlen(entry->name);
        if (!strcmp(entry->name, metadata.module)) module_found = 1;
        status = read_file(entry->path, UI_PACKAGE_MAX_BYTES, &entry->bytes, &entry->length, error, capacity);
        if (status != UI_STATUS_OK) goto done;
        data_length += entry->length;
        if (data_length > UI_PACKAGE_MAX_BYTES) { status = package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Application package exceeds 256 MiB."); goto done; }
    }
    total = PACKAGE_HEADER_SIZE + manifest_length + index_length + data_length;
    if (!module_found || total > UI_PACKAGE_MAX_BYTES) {
        status = package_error(UI_STATUS_VALIDATION_FAILED, error, capacity, "Module entry is missing or package exceeds 256 MiB.");
        goto done;
    }
    buffer = (unsigned char *)calloc(1, (size_t)total);
    if (!buffer) { status = UI_STATUS_OUT_OF_MEMORY; goto done; }
    memcpy(buffer, package_magic, 8);
    write_u32(buffer + 8, UI_PACKAGE_FORMAT_VERSION);
    write_u32(buffer + 12, PACKAGE_HEADER_SIZE);
    write_u64(buffer + 16, total);
    write_u64(buffer + 24, PACKAGE_HEADER_SIZE);
    write_u64(buffer + 32, manifest_length);
    write_u64(buffer + 40, PACKAGE_HEADER_SIZE + manifest_length);
    write_u32(buffer + 48, (uint32_t)list.count);
    write_u64(buffer + 56, PACKAGE_HEADER_SIZE + manifest_length + index_length);
    memcpy(buffer + PACKAGE_HEADER_SIZE, manifest, manifest_length);
    cursor = PACKAGE_HEADER_SIZE + manifest_length;
    data_cursor = cursor + (size_t)index_length;
    for (i = 0; i < list.count; ++i) {
        pack_file_t *entry = list.files + i;
        size_t n = strlen(entry->name);
        write_u32(buffer + cursor, (uint32_t)n);
        write_u64(buffer + cursor + 8, data_cursor);
        write_u64(buffer + cursor + 16, entry->length);
        memcpy(buffer + cursor + PACKAGE_INDEX_SIZE, entry->name, n);
        memcpy(buffer + data_cursor, entry->bytes, entry->length);
        cursor += PACKAGE_INDEX_SIZE + n;
        data_cursor += entry->length;
    }
#ifdef _WIN32
    file = utf8_fopen(output, L"wb");
#else
    file = fopen(output, "wb");
#endif
    if (!file) { status = package_error(UI_STATUS_PLATFORM_ERROR, error, capacity, "Cannot create output package."); goto done; }
    {
        int failed = fwrite(buffer, 1, (size_t)total, file) != (size_t)total;
        if (fclose(file)) failed = 1;
        if (failed) {
            status = package_error(UI_STATUS_PLATFORM_ERROR, error, capacity, "Failed to write complete output package.");
#ifdef _WIN32
            wchar_t *wide = utf8_to_wide(output);
            if (wide) { _wremove(wide); free(wide); }
#else
            remove(output);
#endif
            goto done;
        }
    }
    status = UI_STATUS_OK;
done:
    if (status != UI_STATUS_OK && error && capacity && !*error)
        package_error(status, error, capacity, "Unable to build application package.");
    free(manifest); free(buffer); destroy_pack_files(&list);
#ifdef _WIN32
    free(absolute_manifest); free(absolute_source); free(absolute_output);
#endif
    return status;
}
