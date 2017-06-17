#include "src/graphics.h"
#include "src/utils.h"

#include <conio.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>

// pointer to VGA memory
static unsigned char *VGA = (unsigned char *)0xA0000;

/* ***** */
void gfx_setMode(const unsigned char mode)
{
    _asm {
        mov ah, 0x00
        mov al, mode
        int 10h
    }
}

/* ***** */
void gfx_drawPixel(int x, int y, const unsigned char color, gfx_drawBuffer *buffer)
{
    int bufferW = buffer ? buffer->width : SCREEN_WIDTH;
    int bufferH = buffer ? buffer->height : SCREEN_HEIGHT;

    // DF_NEVER - don't draw anything, abort
    if(buffer->drawOpts.depthFunc == DF_NEVER)
        return;

    // naive "clipping"
    if(x >= bufferW || x < 0 || y >= bufferH || y < 0) return;

    if(!buffer)
        VGA[(y << 8) + (y << 6) + x] = color;
    else
        buffer->colorBuffer[x + y * buffer->width] = color;
}

/* ***** */
void gfx_drawPixelWithDepth(int x, int y, float invZ, const unsigned char color, gfx_drawBuffer *buffer)
{
    int bufferW = buffer ? buffer->width : SCREEN_WIDTH;
    int bufferH = buffer ? buffer->height : SCREEN_HEIGHT;

    // DF_NEVER - don't draw anything, abort
    if(buffer->drawOpts.depthFunc == DF_NEVER)
        return;

    // naive "clipping"
    if(x >= bufferW || x < 0 || y >= bufferH || y < 0) return;

    // no depth info for VGA array, so ignore invZ
    if(!buffer)
        VGA[(y << 8) + (y << 6) + x] = color;
    else
    {
        size_t idx = x + y * buffer->width;
        int drawPixel = 1;

        // check condition for 1/z and determine whether the pixel should be drawn
        // note that this is *opposite* to how modern APIs make checks (since we store 1/z)
        switch(buffer->drawOpts.depthFunc)
        {
            case DF_LESS:     drawPixel = buffer->depthBuffer[idx] < invZ; break;
            case DF_LEQUAL:   drawPixel = buffer->depthBuffer[idx] <= invZ; break;
            case DF_GEQUAL:   drawPixel = buffer->depthBuffer[idx] >= invZ; break;
            case DF_GREATER:  drawPixel = buffer->depthBuffer[idx] > invZ; break;
            case DF_NOTEQUAL: drawPixel = buffer->depthBuffer[idx] != invZ; break;
            default:
            break;
        }

        if(drawPixel)
        {
            buffer->colorBuffer[idx] = color;
            buffer->depthBuffer[idx] = invZ;
        }
    }
}

/* ***** */
void gfx_drawLine(int x0, int y0, int z0, int x1, int y1, int z1, const unsigned char color, gfx_drawBuffer *buffer)
{
    // Bresenham line drawing
    int startX = x0;
    float invLineLength = x1 - x0 ? 1.f / (x1 - x0) : 1.f;
    int x = x1 - x0;
    int y = y1 - y0;
    int ax = abs(x), ay = abs(y);
    int dx1 = x < 0 ? -1 : x > 0 ? 1 : 0;
    int dy1 = y < 0 ? -1 : y > 0 ? 1 : 0;
    int dx2 = dx1, dy2 = 0;
    int n, i = 0;
    float startInvZ = z0 ? 1.f / z0 : 1.f;
    float endInvZ   = z1 ? 1.f / z1 : 1.f;

    // DF_NEVER - don't draw anything, abort
    if(buffer->drawOpts.depthFunc == DF_NEVER)
        return;

    if(ax <= ay)
    {
        // swap ax <=> ay
        ax ^= ay; ay ^= ax; ax ^= ay;
        dx2 = 0; dy2 = dy1;
    }

    n = ax >> 1;

    while(i++ <= ax)
    {
        // interpolate and store 1/z for pixel if depth testing is enabled
        if(buffer->drawOpts.depthFunc != DF_ALWAYS)
        {
            float r = (x0 - startX) * invLineLength;
            float lerpInvZ = LERP(startInvZ, endInvZ, r);
            gfx_drawPixelWithDepth(x0, y0, lerpInvZ, color, buffer);
        }
        else
            gfx_drawPixel(x0, y0, color, buffer);

        n  += ay;
        x0 += n >= ax ? dx1 : dx2;
        y0 += n >= ax ? dy1 : dy2;
        n  -= n >= ax ? ax  : 0;
    }
}

/* ***** */
void gfx_drawLineVec(const mth_Vector4 *from, const mth_Vector4 *to, const unsigned char color, gfx_drawBuffer *buffer)
{
    gfx_drawLine(from->x, from->y, from->z, to->x, to->y, to->z, color, buffer);
}

/* ***** */
void gfx_clrBuffer(gfx_drawBuffer *buffer, const enum BufferType bType)
{
    if(bType & DB_COLOR)
        gfx_clrBufferColor(buffer, 0);

    // CAUTION: C-standard does not guarantee that memsetting() float array to 0 will produce desired results,
    // this is platform dependent and may not work on architecture where 0.f is not represented by all 0 bits!
    if(bType & DB_DEPTH && buffer->depthBuffer)
        memset(buffer->depthBuffer, 0, sizeof(float) * buffer->width * buffer->height);
}

/* ***** */
void gfx_clrBufferColor(gfx_drawBuffer *buffer, const unsigned char color)
{
    if(!buffer)
        memset(VGA, color, SCREEN_WIDTH * SCREEN_HEIGHT);
    else
        memset(buffer->colorBuffer, color, buffer->width * buffer->height);
}

/* ***** */
void gfx_blitBuffer(int x, int y, const gfx_drawBuffer *src, gfx_drawBuffer *target)
{
    int i;
    if(target)
    {
        int width  = MIN(src->width, target->width - x);
        int height = MIN(src->height, target->height - y);

        for(i = 0; i < height; ++i)
            memcpy(&target->colorBuffer[x + (i + y) * target->width], &src->colorBuffer[i * src->width], width);
    }
    else
    {
        int width  = MIN(src->width, SCREEN_WIDTH - x);
        int height = MIN(src->height, SCREEN_HEIGHT - y);

        for(i = 0; i < height; ++i)
            memcpy(&VGA[(y << 8) + (y << 6) + x + i * SCREEN_WIDTH], &src->colorBuffer[i * src->width], width);
    }
}

/* ***** */
void gfx_updateScreen(gfx_drawBuffer *buffer)
{
    memcpy(VGA, buffer->colorBuffer, SCREEN_WIDTH * SCREEN_HEIGHT);
}

/* ***** */
void gfx_setPalette(const unsigned char *palette)
{
    int i;
    outp(0x03c8, 0);

    for(i = 0; i < 256*3; ++i)
    {
        outp(0x03c9, palette[i]);
    }
}

/* ***** */
void gfx_getPalette(unsigned char *outPalette)
{
    int i;
    outp(0x03c7, 0);

    for(i = 0; i < 256*3; ++i)
    {
        outPalette[i] = inp(0x03c9);
    }
}

/* ***** */
void gfx_vSync()
{
    while((inp(0x03da) & 8));
    while(!(inp(0x03da) & 8));
}
