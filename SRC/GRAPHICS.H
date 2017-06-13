#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "src/math.h"
#include <stdlib.h>

/*
 * Common graphics functionality.
 */

#ifdef __cplusplus
extern "C" {
#endif
    #define SCREEN_WIDTH  320
    #define SCREEN_HEIGHT 200

    // basic vertex: position and UV mapping for textures
    typedef struct
    {
        mth_Vector4 position;
        mth_Vector2 uv;
    } gfx_Vertex;

    typedef struct
    {
        unsigned char *colorBuffer;
        int width;
        int height;
    } gfx_drawBuffer;

    enum BufferType
    {
        DB_COLOR = 1 << 0,
        DB_DEPTH = 1 << 1
    };

    #define ALLOC_DRAWBUFFER(b, w, h, f) \
            b.width  = w; \
            b.height = h; \
            b.colorBuffer = (unsigned char *)malloc(w * h);

    #define FREE_DRAWBUFFER(b) \
            free(b.colorBuffer);

    // set graphics mode
    void gfx_setMode(const unsigned char mode);

    // draw pixel to buffer
    void gfx_drawPixel(int x, int y, const unsigned char color, gfx_drawBuffer *buffer);

    // Bresenham line drawing
    void gfx_drawLine(int x0, int y0, int x1, int y1, const unsigned char color, gfx_drawBuffer *buffer);

    // Bresenham line drawing - using mth_Vector4 structs
    void gfx_drawLineVec(const mth_Vector4 *from, const mth_Vector4 *to, const unsigned char color, gfx_drawBuffer *buffer);

    // clears buffer to 0
    void gfx_clrBuffer(gfx_drawBuffer *buffer);

    // clears buffer to specific color
    void gfx_clrBufferColor(gfx_drawBuffer *buffer, const unsigned char color);

    // copy buffer contents to VGA memory
    void gfx_updateScreen(gfx_drawBuffer *buffer);

    // set current VGA palette
    void gfx_setPalette(const unsigned char *palette);

    // wait for retrace
    void gfx_vSync();

#ifdef __cplusplus
}
#endif
#endif
