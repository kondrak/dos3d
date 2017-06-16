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

    enum BufferType
    {
        DB_COLOR = 1 << 0,
        DB_DEPTH = 1 << 1
    };

    enum TextureMappingMode
    {
        TM_AFFINE = 1 << 0,
        TM_PERSPECTIVE = 1 << 1
    };

    enum FaceCullingMode
    {
        FC_BACK  = 1 << 0,
        FC_FRONT = 1 << 1,
        FC_NONE  = 1 << 2
    };

    enum DepthFunc
    {
        DF_ALWAYS = 1 << 0,
        DF_LESS   = 1 << 1
    };

    // basic vertex: position and UV mapping for textures
    typedef struct
    {
        mth_Vector4 position;
        mth_Vector2 uv;
    } gfx_Vertex;

    typedef struct
    {
        enum TextureMappingMode texMapMode;
        enum FaceCullingMode cullMode;
        enum DepthFunc depthFunc;
        short colorKey;
    } gfx_drawOptions;

    typedef struct
    {
        int width;
        int height;
        gfx_drawOptions drawOpts;
        unsigned char *colorBuffer;
        float *depthBuffer; // depth buffer based on 1/z values per pixel
    } gfx_drawBuffer;

    // default draw options initialization since the compiler can't handle struct constructors
    #define DRAWOPTS_DEFAULT(o) {\
            o.texMapMode = TM_PERSPECTIVE; \
            o.cullMode = FC_NONE; \
            o.depthFunc = DF_ALWAYS; \
            o.colorKey   = -1; \
            }

    #define ALLOC_DRAWBUFFER(b, w, h, f) {\
            b.width  = w; \
            b.height = h; \
            DRAWOPTS_DEFAULT(b.drawOpts); \
            b.colorBuffer = f & DB_COLOR ? (unsigned char *)malloc(sizeof(unsigned char) * w * h) : NULL; \
            b.depthBuffer = f & DB_DEPTH ? (float *)malloc(sizeof(float) * w * h) : NULL; \
            }

    #define FREE_DRAWBUFFER(b) {\
            free(b.colorBuffer); \
            free(b.depthBuffer); \
            }

    // set graphics mode
    void gfx_setMode(const unsigned char mode);

    // draw pixel to buffer (ignore depth)
    void gfx_drawPixel(int x, int y, const unsigned char color, gfx_drawBuffer *buffer);

    // draw pixel and take depth into account
    void gfx_drawPixelDepth(int x, int y, float invZ, const unsigned char color, gfx_drawBuffer *buffer);

    // Bresenham line drawing
    void gfx_drawLine(int x0, int y0, int z0, int x1, int y1, int z1, const unsigned char color, gfx_drawBuffer *buffer);

    // Bresenham line drawing - using mth_Vector4 structs
    void gfx_drawLineVec(const mth_Vector4 *from, const mth_Vector4 *to, const unsigned char color, gfx_drawBuffer *buffer);

    // clears buffer to 0
    void gfx_clrBuffer(gfx_drawBuffer *buffer, const enum BufferType bType);

    // clears buffer to specific color
    void gfx_clrBufferColor(gfx_drawBuffer *buffer, const unsigned char color);

    // copy contents of one buffer to another (at x,y offset)
    void gfx_blitBuffer(int x, int y, const gfx_drawBuffer *src, gfx_drawBuffer *target);

    // copy buffer contents to VGA memory
    void gfx_updateScreen(gfx_drawBuffer *buffer);

    // set current VGA palette
    void gfx_setPalette(const unsigned char *palette);

    // read current VGA palette
    void gfx_getPalette(unsigned char *outPalette);

    // wait for retrace
    void gfx_vSync();

#ifdef __cplusplus
}
#endif
#endif
