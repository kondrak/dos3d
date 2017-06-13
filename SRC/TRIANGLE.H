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

    enum TextureMappingMode
    {
        TM_AFFINE,
        TM_PERSPECTIVE
    };

    enum FaceCullingMode
    {
        FC_BACK,
        FC_FRONT,
        FC_NONE
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
        enum FaceCullingMode cullMode;
        short colorKey;
    } gfx_drawOptions;

    // default draw options initialization since the compiler can't handle struct constructors
    #define DRAWOPTS_DEFAULT(o) {\
            o.texMapMode = TM_PERSPECTIVE; \
            o.cullMode = FC_NONE; \
            o.colorKey   = -1; \
            }

    /* *** Interface *** */

    // draw triangle to buffer
    void gfx_drawTriangle(const gfx_Triangle *t, const mth_Matrix4 *matrix, gfx_drawBuffer *buffer);
    void gfx_drawTriangleOpts(const gfx_Triangle *t, const mth_Matrix4 *matrix, const gfx_drawOptions *drawOpts, gfx_drawBuffer *buffer);

#ifdef __cplusplus
}
#endif
#endif
