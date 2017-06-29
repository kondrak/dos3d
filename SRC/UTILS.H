#ifndef UTILS_H
#define UTILS_H

#include "src/graphics.h"
#include <assert.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASSERT(x) assert(x)
#define DBG(msg, ...) { fprintf(stdout, msg, __VA_ARGS__); fflush(stdout); }
#define LERP(start, end, r) ( start * (1.0 - r) + end * r )
#define MIN(a, b) ( a > b ? b : a )
#define MAX(a, b) ( a > b ? a : b )
#define ROUND(a)  ( a < 0 ? (int)(a - 0.5) : (int)(a + 0.5) )

    // debug text printing to buffer using standard mode 13h font glyphs
    void utl_printf(const char *str, const int x, const int y, const unsigned char fgCol, const unsigned char bgCol, gfx_drawBuffer *buffer);

    // draw numbered palette grid to buffer
    void utl_drawPalette(gfx_drawBuffer *buffer);

#ifdef __cplusplus
}
#endif
#endif
