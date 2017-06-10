#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "src/math.h"

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

    // set graphics mode
    void gfx_setMode(const unsigned char mode);

    // draw pixel to buffer
    void gfx_drawPixel(int x, int y, const unsigned char color, unsigned char *buffer);

    // Bresenham line drawing
    void gfx_drawLine(int x0, int y0, int x1, int y1, const unsigned char color, unsigned char *buffer);

    // Bresenham line drawing - using mth_Vector4 structs
    void gfx_drawLineVec(const mth_Vector4 *from, const mth_Vector4 *to, const unsigned char color, unsigned char *buffer);

    // clears buffer to 0
    void gfx_clrBuffer(unsigned char *buffer);

    // clears buffer to specific color
    void gfx_clrBufferColor(unsigned char *buffer, const unsigned char color);

    // copy buffer contents to VGA memory
    void gfx_updateScreen(unsigned char *buffer);

    // set current VGA palette
    void gfx_setPalette(const unsigned char *palette);

    // wait for retrace
    void gfx_vSync();

#ifdef __cplusplus
}
#endif
#endif
