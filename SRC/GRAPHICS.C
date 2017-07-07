#include "src/graphics.h"
#include "src/utils.h"

#include <conio.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>

// pointer to VGA memory
static uint8_t *VGA = (uint8_t *)0xA0000;

/* ***** */
void gfx_setMode(const uint8_t mode)
{
    _asm {
        mov ah, 0x00
        mov al, mode
        int 10h
    }
}

/* ***** */
void gfx_drawPixel(int x, int y, const uint8_t color, gfx_drawBuffer *buffer)
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
void gfx_drawPixelWithDepth(int x, int y, float invZ, const uint8_t color, gfx_drawBuffer *buffer)
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
void gfx_drawLine(int x0, int y0, int z0, int x1, int y1, int z1, const uint8_t color, gfx_drawBuffer *buffer)
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
void gfx_drawLineVec(const mth_Vector4 *from, const mth_Vector4 *to, const uint8_t color, gfx_drawBuffer *buffer)
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
    if(bType & DB_DEPTH && buffer && buffer->depthBuffer)
        memset(buffer->depthBuffer, 0, sizeof(float) * buffer->width * buffer->height);
}

/* ***** */
void gfx_clrBufferColor(gfx_drawBuffer *buffer, const uint8_t color)
{
    if(!buffer)
        memset(VGA, color, sizeof(uint8_t) * SCREEN_WIDTH * SCREEN_HEIGHT);
    else
        memset(buffer->colorBuffer, color, sizeof(uint8_t) * buffer->width * buffer->height);
}

/* ***** */
void gfx_blitBuffer(int x, int y, const gfx_drawBuffer *src, gfx_drawBuffer *target)
{
    int i;
    // adjust for offscreen positioning
    int startX = x < 0 ? -x : 0;
    int startY = y < 0 ? -y : 0;
    int targetWidth  = target ? target->width : SCREEN_WIDTH;
    int targetHeight = target ? target->height : SCREEN_HEIGHT;
    int width  = MIN(src->width - startX, targetWidth - (x < 0 ? startX : x));
    int height = MIN(src->height, targetHeight - y);
    uint8_t *dstBuff = target != NULL ? target->colorBuffer : VGA;
    uint8_t *srcBuff = src->colorBuffer;

    if(width < 0 || x > targetWidth) return;

    for(i = 0; i < height - startY; ++i)
    {
        memcpy(&dstBuff[startX + x + (startY + i + y) * targetWidth], 
               &srcBuff[startX     + (startY + i    ) * src->width], width);
    }
}

/* ***** */
void gfx_updateScreen(gfx_drawBuffer *buffer)
{
    memcpy(VGA, buffer->colorBuffer, SCREEN_WIDTH * SCREEN_HEIGHT);
}

/* ***** */
void gfx_setPalette(const uint8_t *palette)
{
    int i;
    outp(0x03c8, 0);

    for(i = 0; i < 256*3; ++i)
    {
        outp(0x03c9, palette[i]);
    }
}

/* ***** */
void gfx_setPalette8(const uint8_t *palette)
{
    int i;
    outp(0x03c8, 0);

    for(i = 0; i < 256*3; ++i)
    {
        // convert to 6 bits per channel value
        outp(0x03c9, palette[i] >> 2);
    }
}

/* ***** */
void gfx_getPalette(uint8_t *outPalette)
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
