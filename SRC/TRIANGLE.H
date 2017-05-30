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

    enum TextureMapping
    {
        AFFINE,
        PERSPECTIVE
    };

    typedef struct
    {
        int color;
        gfx_Vertex  vertices[3];
        gfx_Bitmap *texture;
    } gfx_Triangle;

    /* *** Interface *** */

    // draw triangle to buffer (no color keying)
    void gfx_drawTriangle(const gfx_Triangle *t, unsigned char *buffer);

    void gfx_drawTriangleTexMap(const gfx_Triangle *t, unsigned char *buffer, enum TextureMapping tm);

    // draw triangle to buffer with colorKey as the tranpsarent color
    void gfx_drawTriangleColorKey(const gfx_Triangle *t, unsigned char *buffer, const short colorKey);

    void gfx_drawTriangleColorKeyTexMap(const gfx_Triangle *t, unsigned char *buffer, const short colorKey, enum TextureMapping tm);

#ifdef __cplusplus
}
#endif
#endif
