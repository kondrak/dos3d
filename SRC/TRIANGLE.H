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

    enum TextureMappingMode
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

    typedef struct
    {
        enum TextureMappingMode texMapMode;
        short colorKey;
    } gfx_drawOptions;

    // default draw options initialization since the compiler can't handle struct constructors
    #define DRAWOPTS_DEFAULT(o) \
            o.texMapMode = PERSPECTIVE; \
            o.colorKey   = -1;

    /* *** Interface *** */

    // draw triangle to buffer (no color keying)
    void gfx_drawTriangle(const gfx_Triangle *t, unsigned char *buffer);
    void gfx_drawTriangleOpts(const gfx_Triangle *t, const gfx_drawOptions drawOpts, unsigned char *buffer);

#ifdef __cplusplus
}
#endif
#endif
