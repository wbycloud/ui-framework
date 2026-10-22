#ifndef BROWSER_IMAGE_H
#define BROWSER_IMAGE_H
#include "browser_net.h"
/* Worker-only bounded WIC decoding. Caller owns returned RGBA bytes and checks
 * navigation cancellation before/after. No host/view or app-object access. */
int browser_image_decode(const unsigned char *,size_t,size_t remaining,
    browser_reply_t *,unsigned char **pixels);
#endif
