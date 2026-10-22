#ifndef BROWSER_SETTINGS_H
#define BROWSER_SETTINGS_H
#include "browser_url.h"
#include <windows.h>
#define BROWSER_SETTINGS_CAP 4096
typedef struct browser_settings {
    wchar_t path[32768];
    char defaults[BROWSER_SETTINGS_CAP],text[BROWSER_SETTINGS_CAP],message[256];
    browser_policy_t policy;
    int valid,custom,initialized;
} browser_settings_t;
int browser_settings_init(browser_settings_t *,const char *defaults);
int browser_settings_reload(browser_settings_t *); /* Changed =>1; invalid disk =>deny all. */
int browser_settings_save(browser_settings_t *,const char *text);
int browser_settings_reset(browser_settings_t *);
#endif
