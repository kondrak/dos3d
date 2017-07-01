#ifndef UTILS_H
#define UTILS_H

#include "src/graphics.h"
#include <assert.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NDEBUG
#   define ASSERT(x, ...) if(!(x)) { gfx_setMode(0x03); fprintf(stderr, __VA_ARGS__); assert(x); }
#else
#   define ASSERT(x, ...)
#endif

#define DBG(b, ...) utl_printf((b), 1, 1, 15, 0, __VA_ARGS__)
#define LERP(start, end, r) ( (start) * (1.0 - (r)) + (end) * (r) )
#define MIN(a, b) ( (a) > (b) ? (b) : (a) )
#define MAX(a, b) ( (a) > (b) ? (a) : (b) )
#define ROUND(a)  ( (a) < 0 ? (int)((a) - 0.5) : (int)((a) + 0.5) )

    // debug text printing to buffer using standard mode 13h font glyphs
    void utl_printf(gfx_drawBuffer *buffer, const int x, const int y, const uint8_t fgCol, const uint8_t bgCol, const uint8_t *format, ...);

    // draw numbered palette grid to buffer
    void utl_drawPalette(gfx_drawBuffer *buffer);

#ifdef __cplusplus
}
#endif
#endif
