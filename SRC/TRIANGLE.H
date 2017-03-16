#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "src/bitmap.h"
#include "src/graphics.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct
    {
        Vertex vertices[3];
        int color;
        BITMAP *texture;
    } Triangle;

    void drawTriangle(const Triangle *t, unsigned char *buffer);

#ifdef __cplusplus
}
#endif
#endif
