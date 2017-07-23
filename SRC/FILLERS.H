#ifndef FILLERS_H
#define FILLERS_H

#include "src/graphics.h"
#include "src/math.h"
#include "src/triangle.h"

/*
 * Triangle color fillers.
 * These functions are meant to be called by triangle rasterizer only.
 */

#ifdef __cplusplus
extern "C" {
#endif

    // wireframe
    void gfx_wireFrame(const gfx_Triangle *t, gfx_drawBuffer *target);

    // flat color fill
    void gfx_flatFill(const gfx_Triangle *t, gfx_drawBuffer *target, enum TriangleType type);

    // texture mapping
    void gfx_perspectiveTextureMap(const gfx_Triangle *t, gfx_drawBuffer *target, enum TriangleType type);
    void gfx_affineTextureMap(const gfx_Triangle *t, gfx_drawBuffer *target, enum TriangleType type);

#ifdef __cplusplus
}
#endif
#endif
