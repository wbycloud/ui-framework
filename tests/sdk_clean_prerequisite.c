#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <wchar.h>

int main(void)
{
    wchar_t actual[MAX_PATH];
    wchar_t expected[MAX_PATH];
    RECT client = {0};
    DWORD session = 0;
    HMODULE runtime = LoadLibraryExW(L"vcruntime140.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (sizeof(void *) != 8 || !runtime || !GetSystemDirectoryW(expected, MAX_PATH) ||
        wcscat_s(expected, MAX_PATH, L"\\vcruntime140.dll") ||
        !GetModuleFileNameW(runtime, actual, MAX_PATH) || _wcsicmp(actual, expected)) {
        fwprintf(stderr, L"x64 System32 runtime unavailable, error %lu\n", GetLastError());
        if (runtime) FreeLibrary(runtime);
        return 2;
    }

    HWND window = CreateWindowExW(0, L"STATIC", L"Synthetic Windows SDK prerequisite",
        WS_OVERLAPPEDWINDOW, 10, 10, 480, 320, NULL, NULL, GetModuleHandleW(NULL), NULL);
    if (!window) {
        fwprintf(stderr, L"Win32 window unavailable, error %lu\n", GetLastError());
        FreeLibrary(runtime);
        return 3;
    }
    ShowWindow(window, SW_SHOWNOACTIVATE);
    UpdateWindow(window);
    BOOL shown = IsWindowVisible(window) && GetClientRect(window, &client);
    BOOL finished = DestroyWindow(window);
    BOOL has_session = ProcessIdToSessionId(GetCurrentProcessId(), &session);
    FreeLibrary(runtime);
    if (!shown || !finished || !has_session || client.right <= 0 || client.bottom <= 0) return 4;
    wprintf(L"x64 runtime=%ls; client=%ldx%ld; session=%lu; window released\n",
        actual, client.right, client.bottom, session);
    return 0;
}
