#include "src/graphics.h"

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
void gfx_drawPixel(int x, int y, const unsigned char color, unsigned char *buffer)
{
    // naive "clipping"
    if(x >= SCREEN_WIDTH || x < 0 || y >= SCREEN_HEIGHT || y < 0) return;

    if(!buffer)
        VGA[(y << 8) + (y << 6) + x] = color;
    else
        buffer[(y << 8) + (y << 6) + x] = color;
}

/* ***** */
void gfx_drawLine(int x0, int y0, int x1, int y1, const unsigned char color, unsigned char *buffer)
{
    // Bresenham line drawing
    int x = x1 - x0;
    int y = y1 - y0;
    int ax = abs(x), ay = abs(y);
    int dx1 = x < 0 ? -1 : x > 0 ? 1 : 0;
    int dy1 = y < 0 ? -1 : y > 0 ? 1 : 0;
    int dx2 = dx1, dy2 = 0;
    int n, i = 0;

    if(ax <= ay)
    {
        // swap ax <=> ay
        ax ^= ay; ay ^= ax; ax ^= ay;
        dx2 = 0; dy2 = dy1;
    }

    n = ax >> 1;

    while(i++ <= ax)
    {
        gfx_drawPixel(x0, y0, color, buffer);
        n  += ay;
        x0 += n >= ax ? dx1 : dx2;
        y0 += n >= ax ? dy1 : dy2;
        n  -= n >= ax ? ax  : 0;
    }
}

/* ***** */
void gfx_drawLineVec(const mth_Vector4 *from, const mth_Vector4 *to, const unsigned char color, unsigned char *buffer)
{
    gfx_drawLine(from->x, from->y, to->x, to->y, color, buffer);
}

/* ***** */
void gfx_clrBuffer(unsigned char *buffer)
{
    gfx_clrBufferColor(buffer, 0);
}

/* ***** */
void gfx_clrBufferColor(unsigned char *buffer, const unsigned char color)
{
    if(!buffer)
        memset(VGA, color, SCREEN_WIDTH*SCREEN_HEIGHT);
    else
        memset(buffer, color, SCREEN_WIDTH*SCREEN_HEIGHT);
}

/* ***** */
void gfx_updateScreen(unsigned char *buffer)
{
    memcpy(VGA, buffer, SCREEN_WIDTH*SCREEN_HEIGHT);
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
void gfx_vSync()
{
    while((inp(0x03da) & 8));
    while(!(inp(0x03da) & 8));
}
