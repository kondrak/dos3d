#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "src/bitmap.h"
#include "src/graphics.h"
#include "src/math.h"

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

    // draw triangle to buffer
    void gfx_drawTriangle(const gfx_Triangle *t, const mth_Matrix4 *matrix, gfx_drawBuffer *buffer);

#ifdef __cplusplus
}
#endif
#endif
