#include "ui_framework/package.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

static int failures;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); ++failures; } } while (0)

static const char manifest[] =
    "[application]\napp_id=test.eda\nname=EDA = example\nversion=1.0\n"
    "architecture=x64\nabi_version=1\nframework_api_version=1\n"
    "module=main.dll\nmultiple_instances=true\n";

/* Independent encoder for the documented v1 wire format; no packer helpers. */
static void le32(unsigned char *p, uint32_t value)
{
    unsigned i;
    for (i = 0; i < 4; ++i) { p[i] = (unsigned char)(value & 255); value >>= 8; }
}
static void le64(unsigned char *p, uint64_t value)
{
    unsigned i;
    for (i = 0; i < 8; ++i) { p[i] = (unsigned char)(value & 255); value >>= 8; }
}

static size_t fixture_named(unsigned char *bytes, const char *description,
                             const char *first_name, const char *second_name,
                             size_t *first_index)
{
    size_t m = strlen(description), a = strlen(first_name), b = strlen(second_name);
    size_t index = 64 + m, data = index + 48 + a + b;
    size_t total = data + 10;
    memset(bytes, 0, total);
    memcpy(bytes, "UAPP\r\n\032\n", 8);
    le32(bytes + 8, 1); le32(bytes + 12, 64); le64(bytes + 16, total);
    le64(bytes + 24, 64); le64(bytes + 32, m); le64(bytes + 40, index);
    le32(bytes + 48, 2); le64(bytes + 56, data);
    memcpy(bytes + 64, description, m);
    le32(bytes + index, (uint32_t)a); le64(bytes + index + 8, data); le64(bytes + index + 16, 3);
    memcpy(bytes + index + 24, first_name, a);
    le32(bytes + index + 24 + a, (uint32_t)b);
    le64(bytes + index + 32 + a, data + 3); le64(bytes + index + 40 + a, 7);
    memcpy(bytes + index + 48 + a, second_name, b);
    memcpy(bytes + data, "MZ\0abc\0xyz", 10);
    *first_index = index;
    return total;
}

static size_t fixture(unsigned char *bytes, const char *description,
                       const char *second_name, size_t *first_index)
{
    return fixture_named(bytes, description, "main.dll", second_name, first_index);
}

#ifdef _WIN32
static FILE *test_fopen(const char *path, const wchar_t *mode)
{
    wchar_t wide[2048];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 2048)) return NULL;
    return _wfopen(wide, mode);
}
static void test_remove(const char *path)
{
    wchar_t wide[2048];
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 2048)) _wremove(wide);
}
static int test_mkdir(const char *path)
{
    wchar_t wide[2048];
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 2048)) return -1;
    return _wmkdir(wide);
}
static void test_rmdir(const char *path)
{
    wchar_t wide[2048];
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wide, 2048)) _wrmdir(wide);
}
#else
static FILE *test_fopen(const char *path, const char *mode) { return fopen(path, mode); }
static void test_remove(const char *path) { remove(path); }
static int test_mkdir(const char *path) { return mkdir(path, 0700); }
static void test_rmdir(const char *path) { rmdir(path); }
#endif

static int save_bytes(const char *path, const void *bytes, size_t length)
{
#ifdef _WIN32
    FILE *file = test_fopen(path, L"wb");
#else
    FILE *file = test_fopen(path, "wb");
#endif
    int success;
    if (!file) return 0;
    success = fwrite(bytes, 1, length, file) == length;
    if (fclose(file)) success = 0;
    return success;
}

static void expect_open(const char *path, const unsigned char *bytes,
                         size_t length, ui_status_t expected)
{
    ui_package_t *package = (ui_package_t *)(uintptr_t)1;
    char error[512];
    ui_status_t status;
    CHECK(save_bytes(path, bytes, length));
    status = ui_package_open(path, &package, error, sizeof(error));
    if (status != expected) fprintf(stderr, "unexpected package status %d, expected %d: %s\n", (int)status, (int)expected, error);
    CHECK(status == expected);
    if (expected == UI_STATUS_OK) {
        CHECK(package != NULL); CHECK(error[0] == 0); ui_package_destroy(package);
    } else { CHECK(package == NULL); CHECK(error[0] != 0); }
}

static void test_manifest_variants(const char *path)
{
    static const char *invalid[] = {
        "[application]\napp_id=x\n",
        "[application]\napp_id=x\napp_id=y\n",
        "[application]\nunknown=x\n",
        "[application]\n[application]\n",
        "[different]\n",
        "app_id=x\n[application]\n",
        "[application]\napp_id\n"
    };
    unsigned char bytes[8192];
    char variant[2048];
    size_t i, index, length;
    for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        length = fixture(bytes, invalid[i], "ui.html", &index);
        expect_open(path, bytes, length, UI_STATUS_VALIDATION_FAILED);
    }
    strcpy(variant, manifest); strcat(variant, "name=again\n");
    length = fixture(bytes, variant, "ui.html", &index);
    expect_open(path, bytes, length, UI_STATUS_VALIDATION_FAILED);
    strcpy(variant, manifest); strcat(variant, "extra=yes\n");
    length = fixture(bytes, variant, "ui.html", &index);
    expect_open(path, bytes, length, UI_STATUS_VALIDATION_FAILED);
    {
        static const struct { const char *from; const char *to; ui_status_t status; } cases[] = {
            {"abi_version=1", "abi_version=0", UI_STATUS_UNSUPPORTED},
            {"abi_version=1", "abi_version=4294967296", UI_STATUS_VALIDATION_FAILED},
            {"abi_version=1", "abi_version=-1", UI_STATUS_VALIDATION_FAILED},
            {"abi_version=1", "abi_version=+1", UI_STATUS_VALIDATION_FAILED},
            {"abi_version=1", "abi_version=1x", UI_STATUS_VALIDATION_FAILED},
            {"framework_api_version=1", "framework_api_version=5", UI_STATUS_UNSUPPORTED},
            {"architecture=x64", "architecture=x86", UI_STATUS_UNSUPPORTED},
            {"name=EDA = example", "name=", UI_STATUS_VALIDATION_FAILED},
            {"app_id=test.eda", "app_id=../eda", UI_STATUS_VALIDATION_FAILED},
            {"module=main.dll", "module=missing.dll", UI_STATUS_VALIDATION_FAILED},
            {"module=main.dll", "module=MAIN.dll", UI_STATUS_VALIDATION_FAILED},
            {"module=main.dll", "module=main.exe", UI_STATUS_VALIDATION_FAILED},
            {"multiple_instances=true", "multiple_instances=yes", UI_STATUS_VALIDATION_FAILED},
            {"multiple_instances=true", "multiple_instances=false", UI_STATUS_OK}
        };
        for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
            const char *start = strstr(manifest, cases[i].from);
            size_t prefix = (size_t)(start - manifest);
            memcpy(variant, manifest, prefix); variant[prefix] = 0;
            strcat(variant, cases[i].to); strcat(variant, start + strlen(cases[i].from));
            length = fixture(bytes, variant, "ui.html", &index);
            expect_open(path, bytes, length, cases[i].status);
        }
    }
    /* UTF-8 BOM, CRLF and comments are a compatible grammar variant. */
    strcpy(variant, "\357\273\277# comment\r\n ; comment\r\n");
    {
        char *target = variant + strlen(variant);
        const char *p = manifest;
        while (*p) { if (*p == '\n') *target++ = '\r'; *target++ = *p++; }
        *target = 0;
    }
    length = fixture(bytes, variant, "ui.html", &index);
    expect_open(path, bytes, length, UI_STATUS_OK);
    strcpy(variant, manifest); ((unsigned char *)variant)[30] = 0xc0;
    length = fixture(bytes, variant, "ui.html", &index);
    expect_open(path, bytes, length, UI_STATUS_VALIDATION_FAILED);
}

static void test_pack_roundtrip(const char *root)
{
    char source[1024], module[1024], resource[1024], description[1024], output[1024], output2[1024];
    char error[512];
    ui_package_t *package = NULL;
    void *data = NULL;
    size_t length = 0;
    FILE *a, *b;
    int ca, cb;
    snprintf(source, sizeof(source), "%s/source", root);
    snprintf(module, sizeof(module), "%s/main.dll", source);
    snprintf(resource, sizeof(resource), "%s/\xe7\x94\xbb\xe5\xb8\x83.txt", source);
    snprintf(description, sizeof(description), "%s/manifest.ini", root);
    snprintf(output, sizeof(output), "%s/\xe5\xba\x94\xe7\x94\xa8.uapp", root);
    snprintf(output2, sizeof(output2), "%s/copy.uapp", root);
    CHECK(test_mkdir(source) == 0);
    CHECK(save_bytes(module, "MZ", 2)); CHECK(save_bytes(resource, "", 0));
    CHECK(save_bytes(description, manifest, strlen(manifest)));
    CHECK(ui_package_pack(description, source, output, error, sizeof(error)) == UI_STATUS_OK);
    CHECK(ui_package_pack(description, source, output2, error, sizeof(error)) == UI_STATUS_OK);
    CHECK(ui_package_open(output, &package, error, sizeof(error)) == UI_STATUS_OK);
    if (package) {
        ui_package_entry_t entry;
        CHECK(ui_package_entry_count(package) == 2);
        CHECK(ui_package_get_entry(package, 0, &entry) == UI_STATUS_OK && !strcmp(entry.name, "main.dll"));
        CHECK(ui_package_read(package, "\xe7\x94\xbb\xe5\xb8\x83.txt", &data, &length) == UI_STATUS_OK);
        CHECK(length == 0 && data && ((unsigned char *)data)[0] == 0);
        ui_package_release(data); ui_package_destroy(package);
    }
#ifdef _WIN32
    a = test_fopen(output, L"rb"); b = test_fopen(output2, L"rb");
#else
    a = test_fopen(output, "rb"); b = test_fopen(output2, "rb");
#endif
    CHECK(a && b);
    if (a && b) { do { ca = fgetc(a); cb = fgetc(b); CHECK(ca == cb); } while (ca != EOF && cb != EOF); }
    if (a) fclose(a); if (b) fclose(b);
    CHECK(ui_package_pack(description, source, module, error, sizeof(error)) == UI_STATUS_INVALID_ARGUMENT);
    CHECK(ui_package_pack(description, source, description, error, sizeof(error)) == UI_STATUS_INVALID_ARGUMENT);
    test_remove(module); test_remove(resource); test_remove(description); test_remove(output); test_remove(output2);
    test_rmdir(source);
}

int main(void)
{
    unsigned char bytes[8192], good[8192];
    size_t index, length, i;
    char root[256], path[512], error[512];
    ui_package_t *package = NULL;
    void *data = NULL;
    size_t data_length = 0;
    ui_package_entry_t entry;
    const ui_package_metadata_t *metadata;
    static const char *unsafe_names[] = {
        "../ui.html", "/ui.html", "a//ui.html", "a/./ui.html", "a/../ui.html",
        "a\\ui.html", "C:ui.html", "a/file:stream", "a/file.", "a/file ",
        "CON", "nul.txt", "AUX/a.txt", "a/LPT1.txt", "a/COM9", "a/CONIN$", "a/CONOUT$",
        "a/COM\302\271.txt", "a/LPT\302\262", "a/COM\302\263",
        "a/*.txt", "a/?.txt", "a/<x", "a/>x", "a/\"x", "a/|x", "", "a/",
        "MAIN.DLL", "main.dll/child", "MAIN.DLL/child", "a/\300\257.txt", "a/\355\240\200.txt", "a/\364\220\200\200.txt"
    };
#ifdef _WIN32
    snprintf(root, sizeof(root), "uapp-package-fixtures-%lu", (unsigned long)GetCurrentProcessId());
#else
    snprintf(root, sizeof(root), "uapp-package-fixtures-%lu", (unsigned long)getpid());
#endif
    CHECK(test_mkdir(root) == 0);
    snprintf(path, sizeof(path), "%s/fixture.uapp", root);
    length = fixture(good, manifest, "ui.html", &index);
    expect_open(path, good, length, UI_STATUS_OK);
    CHECK(ui_package_open(path, &package, error, sizeof(error)) == UI_STATUS_OK);
    if (package) {
        metadata = ui_package_get_metadata(package);
        CHECK(metadata && !strcmp(metadata->app_id, "test.eda") && !strcmp(metadata->name, "EDA = example"));
        CHECK(metadata && metadata->size == sizeof(*metadata) && metadata->multiple_instances == 1);
        CHECK(ui_package_entry_count(package) == 2);
        CHECK(ui_package_get_entry(package, 1, &entry) == UI_STATUS_OK && !strcmp(entry.name, "ui.html") && entry.length == 7);
        CHECK(ui_package_get_entry(package, 2, &entry) == UI_STATUS_NOT_FOUND);
        CHECK(ui_package_read(package, "ui.html", &data, &data_length) == UI_STATUS_OK);
        CHECK(data_length == 7 && data && !memcmp(data, "abc\0xyz\0", 8)); ui_package_release(data);
        CHECK(ui_package_read(package, "UI.HTML", &data, &data_length) == UI_STATUS_NOT_FOUND && data == NULL && data_length == 0);
        ui_package_destroy(package);
    }
    expect_open(path, good, 0, UI_STATUS_VALIDATION_FAILED);
    expect_open(path, good, 63, UI_STATUS_VALIDATION_FAILED);
    expect_open(path, good, length - 1, UI_STATUS_VALIDATION_FAILED);
#define BAD32(offset,value,expected) do { memcpy(bytes, good, length); le32(bytes + (offset), (value)); expect_open(path, bytes, length, (expected)); } while (0)
#define BAD64(offset,value) do { memcpy(bytes, good, length); le64(bytes + (offset), (value)); expect_open(path, bytes, length, UI_STATUS_VALIDATION_FAILED); } while (0)
    memcpy(bytes, good, length); bytes[0] = 'X'; expect_open(path, bytes, length, UI_STATUS_VALIDATION_FAILED);
    BAD32(8, 2, UI_STATUS_UNSUPPORTED); BAD32(12, 0, UI_STATUS_VALIDATION_FAILED);
    BAD64(16, UINT64_MAX); BAD64(24, UINT64_MAX); BAD64(32, UINT64_MAX);
    BAD64(32, 0); BAD64(40, UINT64_MAX); BAD64(56, UINT64_MAX); BAD64(56, 0);
    BAD32(48, 0, UI_STATUS_VALIDATION_FAILED); BAD32(48, 4097, UI_STATUS_VALIDATION_FAILED);
    BAD32(52, 1, UI_STATUS_VALIDATION_FAILED); BAD32(index, UINT32_MAX, UI_STATUS_VALIDATION_FAILED);
    BAD32(index + 4, 1, UI_STATUS_VALIDATION_FAILED); BAD64(index + 8, 0); BAD64(index + 16, UINT64_MAX);
    BAD64(index + 32 + strlen("main.dll"), 1); BAD64(index + 40 + strlen("main.dll"), UINT64_MAX);
    memcpy(bytes, good, length); bytes[length] = 0; le64(bytes + 16, length + 1);
    expect_open(path, bytes, length + 1, UI_STATUS_VALIDATION_FAILED);
    for (i = 0; i < sizeof(unsafe_names) / sizeof(unsafe_names[0]); ++i) {
        size_t n = fixture(bytes, manifest, unsafe_names[i], &index);
        expect_open(path, bytes, n, UI_STATUS_VALIDATION_FAILED);
    }
#ifdef _WIN32
    /* Windows names are compared with Unicode ordinal case folding. */
    {
        char unicode_manifest[2048];
        const char *part = strstr(manifest, "module=main.dll");
        size_t prefix = (size_t)(part - manifest);
        memcpy(unicode_manifest, manifest, prefix); unicode_manifest[prefix] = 0;
        strcat(unicode_manifest, "module=\303\204.dll"); strcat(unicode_manifest, part + strlen("module=main.dll"));
        length = fixture_named(bytes, unicode_manifest, "\303\204.dll", "\303\244.dll", &index);
        expect_open(path, bytes, length, UI_STATUS_VALIDATION_FAILED);
    }
#endif
    test_manifest_variants(path);
    test_pack_roundtrip(root);
    CHECK(ui_package_open(NULL, &package, error, sizeof(error)) == UI_STATUS_INVALID_ARGUMENT && package == NULL);
    CHECK(ui_package_open(path, NULL, NULL, 0) == UI_STATUS_INVALID_ARGUMENT);
    CHECK(ui_package_get_metadata(NULL) == NULL && ui_package_entry_count(NULL) == 0);
    CHECK(ui_package_read(NULL, "x", &data, &data_length) == UI_STATUS_INVALID_ARGUMENT);
    ui_package_release(NULL); ui_package_destroy(NULL);
    test_remove(path); test_rmdir(root);
    printf("package format and deterministic packing: %d failures\n", failures);
    return failures != 0;
}
