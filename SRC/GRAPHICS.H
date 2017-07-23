#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "src/math.h"
#include <stdint.h>
#include <stdlib.h>

/*
 * Common graphics functions and data structures.
 */

#ifdef __cplusplus
extern "C" {
#endif
    #define SCREEN_WIDTH  320
    #define SCREEN_HEIGHT 200

    // draw buffer type
    enum BufferType
    {
        DB_COLOR = 1 << 0, // color buffer
        DB_DEPTH = 1 << 1  // depth buffer
    };

    // current draw mode for the buffer
    enum DrawMode
    {
        DM_AFFINE      = 1 << 0, // affine texture mapping
        DM_PERSPECTIVE = 1 << 1, // default: perspective correct texture mapping
        DM_FLAT        = 1 << 2, // flat colored rendering
        DM_WIREFRAME   = 1 << 3  // wireframe polygon
    };

    // face culling mode
    enum FaceCullingMode
    {
        FC_BACK  = 1 << 0,
        FC_FRONT = 1 << 1,
        FC_NONE  = 1 << 2  // default - no culling
    };

    // depth function type (OpenGL style)
    enum DepthFunc
    {
        DF_ALWAYS   = 1 << 0, // default - no depth testing
        DF_LESS     = 1 << 1,
        DF_LEQUAL   = 1 << 2,
        DF_GEQUAL   = 1 << 3,
        DF_GREATER  = 1 << 4,
        DF_NOTEQUAL = 1 << 5,
        DF_NEVER    = 1 << 6
    };

    // basic vertex: position and UV mapping for textures
    typedef struct
    {
        mth_Vector4 position;
        mth_Vector2 uv;
    } gfx_Vertex;

    // common draw options for a draw buffer
    typedef struct
    {
        enum DrawMode drawMode;
        enum FaceCullingMode cullMode;
        enum DepthFunc depthFunc;
        int16_t colorKey; // 16 bits - negatives disable keying and int8 is not enough for 0-255 range
    } gfx_drawOptions;

    // draw buffer/render target
    typedef struct
    {
        int width;
        int height;
        gfx_drawOptions drawOpts;
        uint8_t *colorBuffer;
        float *depthBuffer; // depth buffer based on 1/z values per pixel
    } gfx_drawBuffer;

    // default draw options initialization since the compiler can't handle struct constructors
    #define DRAWOPTS_DEFAULT(o) {\
                o.drawMode  = DM_PERSPECTIVE; \
                o.cullMode  = FC_NONE; \
                o.depthFunc = DF_ALWAYS; \
                o.colorKey  = -1; \
            }

    // draw buffer allocation and default initialization
    #define ALLOC_DRAWBUFFER(b, w, h, f) {\
                b.width  = (w); \
                b.height = (h); \
                DRAWOPTS_DEFAULT(b.drawOpts); \
                b.colorBuffer = (f) & DB_COLOR ? (uint8_t *)malloc(sizeof(uint8_t) * (w) * (h)) : NULL; \
                b.depthBuffer = (f) & DB_DEPTH ? (float *)malloc(sizeof(float) * (w) * (h)) : NULL; \
            }

    // draw buffer pointing directly to VGA memory - no depth information
    #define VGA_DRAWBUFFER(b) {\
                b.width  = SCREEN_WIDTH; \
                b.height = SCREEN_HEIGHT; \
                DRAWOPTS_DEFAULT(b.drawOpts); \
                b.colorBuffer = (uint8_t *)0xA0000; /* pointer to VGA memory */ \
                b.depthBuffer = NULL; \
            }

    // check pre-allocated buffer validity
    #define DRAWBUFFER_VALID(b, f) ( ((f) & DB_COLOR ? b.colorBuffer != NULL : 1) && \
                                     ((f) & DB_DEPTH ? b.depthBuffer != NULL : 1) )
    // draw buffer release
    #define FREE_DRAWBUFFER(b) {\
                free(b.colorBuffer); \
                free(b.depthBuffer); \
            }

    // set graphics mode
    void gfx_setMode(const uint8_t mode);

    // draw pixel to target buffer (ignore depth)
    void gfx_drawPixel(int x, int y, const uint8_t color, gfx_drawBuffer *target);

    // draw pixel and take depth into account
    void gfx_drawPixelWithDepth(int x, int y, float invZ, const uint8_t color, gfx_drawBuffer *target);

    // Bresenham line drawing
    void gfx_drawLine(int x0, int y0, int z0, int x1, int y1, int z1, const uint8_t color, gfx_drawBuffer *target);

    // Bresenham line drawing - using mth_Vector4 structs
    void gfx_drawLineVec(const mth_Vector4 *from, const mth_Vector4 *to, const uint8_t color, gfx_drawBuffer *target);

    // clears draw buffer to 0
    void gfx_clrBuffer(gfx_drawBuffer *target, const enum BufferType bType);

    // fills target color buffer to specific color (depth buffer is untouched)
    void gfx_clrBufferColor(gfx_drawBuffer *target, const uint8_t color);

    // copy contents of one buffer to another (at x,y offset), if target is NULL - blits to VGA
    void gfx_blitBuffer(int x, int y, const gfx_drawBuffer *src, gfx_drawBuffer *target);

    // fast copy buffer contents straight to VGA memory - source buffer must be the same size as VGA screen buffer
    void gfx_updateScreen(gfx_drawBuffer *src);

    // set current VGA palette (standard VGA 6 bits per channel)
    void gfx_setPalette(const uint8_t *palette);

    // set current VGA palette (8 bits per channel)
    void gfx_setPalette8(const uint8_t *palette);

    // read current VGA palette
    void gfx_getPalette(uint8_t *outPalette);

    // wait for retrace
    void gfx_vSync();

#ifdef __cplusplus
}
#endif
#endif
