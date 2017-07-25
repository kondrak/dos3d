#ifndef UTILS_H
#define UTILS_H

#include "src/graphics.h"
#include "src/input.h"
#include "src/timer.h"
#include <assert.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NDEBUG
#   define ASSERT(x, ...) if(!(x)) \
                          { \
                              gfx_setMode(0x03); \
                              /* Clean up interrupt handlers */ \
                              kbd_finish(); \
                              tmr_finish(); \
                              fprintf(stderr, __VA_ARGS__); \
                              assert(x); \
                          }
#else
#   define ASSERT(x, ...)
#endif

#define DBG(b, ...) utl_printf((b), 1, 1, 15, 0, __VA_ARGS__)
#define LERP(start, end, r) ( (start) * (1.0 - (r)) + (end) * (r) )
#define MIN(a, b)  ( (a) > (b) ? (b) : (a) )
#define MAX(a, b)  ( (a) > (b) ? (a) : (b) )
#define ROUND(a)   ( (a) < 0 ? (int)((a) - 0.5) : (int)((a) + 0.5) )
#define SWAP(a, b) { (a) ^= (b); (b) ^= (a); (a) ^= (b); }

    // debug text printing to target buffer using standard mode 13h font glyphs
    void utl_printf(gfx_drawBuffer *target, const int x, const int y, const uint8_t fgCol, const uint8_t bgCol, const char *format, ...);

    // draw numbered palette grid to target buffer
    void utl_drawPalette(gfx_drawBuffer *target);

#ifdef __cplusplus
}
#endif
#endif
