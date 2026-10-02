#include "ui_framework/package.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static int pack_main(int argc, char **argv)
{
    char error[512];
    ui_status_t status;
    if (argc != 4) {
        fprintf(stderr, "Usage: uapp_pack manifest.ini source-directory output.uapp\n");
        return 2;
    }
    status = ui_package_pack(argv[1], argv[2], argv[3], error, sizeof(error));
    if (status != UI_STATUS_OK) {
        fprintf(stderr, "Cannot pack application: %s (status %d)\n", error, (int)status);
        return 1;
    }
    printf("Created application package: %s\n", argv[3]);
    return 0;
}

#ifdef _WIN32
int wmain(int argc, wchar_t **wide_argv)
{
    char **argv = (char **)calloc((size_t)argc, sizeof(*argv));
    int i, result = 1;
    if (!argv) return 1;
    for (i = 0; i < argc; ++i) {
        int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide_argv[i],
                                         -1, NULL, 0, NULL, NULL);
        if (!length) goto done;
        argv[i] = (char *)malloc((size_t)length);
        if (!argv[i] || !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                             wide_argv[i], -1, argv[i], length, NULL, NULL)) goto done;
    }
    result = pack_main(argc, argv);
done:
    for (i = 0; i < argc; ++i) free(argv[i]);
    free(argv);
    return result;
}
#else
int main(int argc, char **argv) { return pack_main(argc, argv); }
#endif
