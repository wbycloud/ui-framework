#ifndef BROWSER_BODY_H
#define BROWSER_BODY_H
#include "ui_framework/ui.h"
#define BROWSER_BODY_BYTES 8192
/* DOM body text, not CSS-visible/rendered text. No form values or script. */
ui_status_t browser_body_extract(const char *,size_t,char [BROWSER_BODY_BYTES+1]);
#endif
