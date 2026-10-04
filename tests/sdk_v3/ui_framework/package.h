#ifndef UI_FRAMEWORK_PACKAGE_H
#define UI_FRAMEWORK_PACKAGE_H

#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UI_PACKAGE_FORMAT_VERSION 1u
#define UI_PACKAGE_APPLICATION_ABI_VERSION 1u
#define UI_PACKAGE_MAX_BYTES (256u * 1024u * 1024u)
#define UI_PACKAGE_MAX_ENTRIES 4096u

typedef struct ui_package ui_package_t;

typedef struct ui_package_metadata {
    uint32_t size;
    char app_id[128];
    char name[256];
    char version[64];
    char architecture[16];
    char module[256];
    uint32_t abi_version;
    uint32_t framework_api_version;
    int multiple_instances;
} ui_package_metadata_t;

/* Borrowed name remains valid until ui_package_destroy. */
typedef struct ui_package_entry {
    const char *name;
    uint64_t length;
} ui_package_entry_t;

/* Paths are UTF-8. Open fully validates the immutable package before success.
 * Error text is UTF-8, terminated when error_capacity > 0; it may be NULL.
 * Version/architecture mismatches return UI_STATUS_UNSUPPORTED. */
UI_API ui_status_t ui_package_open(const char *path_utf8,
                                    ui_package_t **package,
                                    char *error, size_t error_capacity);
UI_API void ui_package_destroy(ui_package_t *package);
UI_API const ui_package_metadata_t *ui_package_get_metadata(
    const ui_package_t *package);
UI_API size_t ui_package_entry_count(const ui_package_t *package);
UI_API ui_status_t ui_package_get_entry(const ui_package_t *package,
                                         size_t index,
                                         ui_package_entry_t *entry);
/* Allocates length + 1 bytes, including a trailing zero even for binary data.
 * Release only through ui_package_release, including across DLL boundaries. */
UI_API ui_status_t ui_package_read(const ui_package_t *package,
                                    const char *name, void **data,
                                    size_t *length);
UI_API void ui_package_release(void *data);

/* Recursively packs ordinary source-directory files in name order. Symlinks,
 * junctions and other reparse points are rejected. Does not follow them.
 * Output must be outside source_directory. */
UI_API ui_status_t ui_package_pack(const char *manifest_path_utf8,
                                    const char *source_directory_utf8,
                                    const char *output_path_utf8,
                                    char *error, size_t error_capacity);

#ifdef __cplusplus
}
#endif
#endif
