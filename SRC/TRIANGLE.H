#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "src/bitmap.h"
#include "src/graphics.h"

/*
 * Triangle definition and rendering.
 */

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct
    {
        int color;
        gfx_Vertex  vertices[3];
        gfx_Bitmap *texture;
    } gfx_Triangle;

    /* *** Interface *** */

    // draw triangle to buffer (no color keying)
    void gfx_drawTriangle(const gfx_Triangle *t, unsigned char *buffer);

    // draw triangle to buffer with colorKey as the tranpsarent color
    void gfx_drawTriangleColorKey(const gfx_Triangle *t, unsigned char *buffer, const short colorKey);

#ifdef __cplusplus
}
#endif
#endif
