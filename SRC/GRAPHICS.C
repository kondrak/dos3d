#include "src/graphics.h"

#include <math.h>
#include <memory.h>
#include <stdlib.h>

// pointer to VGA memory
static unsigned char *VGA = (unsigned char *)0xA0000;

// graphics mode setter
void setMode(unsigned char mode)
{
    _asm {
        mov ah, 0x00
        mov al, mode
        int 10h
    }
}

void drawPixel(int x, int y, unsigned char color, unsigned char *buffer)
{
    // naive "clipping"
    if(x > SCREEN_WIDTH || x < 0 || y > SCREEN_HEIGHT || y < 0) return;

    if(!buffer)
        VGA[(y << 8) + (y << 6) + x] = color;
    else
        buffer[(y << 8) + (y << 6) + x] = color;
}

// Bresenham line drawing
void drawLine(int x0, int y0, int x1, int y1, unsigned char color, unsigned char *buffer)
{
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
        drawPixel(x0, y0, color, buffer);
        n  += ay;
        x0 += n >= ax ? dx1 : dx2;
        y0 += n >= ax ? dy1 : dy2;
        n  -= n >= ax ? ax  : 0;
    }
}

void drawLineVec(const Vector4f *from, const Vector4f *to, unsigned char color, unsigned char *buffer)
{
    drawLine(from->x, from->y, to->x, to->y, color, buffer);
}

// only flat bottom triangle supported for now
void drawTriangle(const Triangle *t, unsigned char *buffer)
{
    float invDy  = 1.f / (t->vertices[1].position.y - t->vertices[0].position.y);
    float dxLeft  = (t->vertices[2].position.x - t->vertices[0].position.x) * invDy;
    float dxRight = (t->vertices[1].position.x - t->vertices[0].position.x) * invDy;
    float xLeft  = t->vertices[0].position.x;
    float xRight = xLeft;
    int x, y;

    if(!t->texture)
    {
        for(y = t->vertices[0].position.y; y <= t->vertices[1].position.y; ++y)
        {
            drawLine(xLeft, y, xRight, y, t->color, buffer);
            xLeft += dxLeft;
            xRight += dxRight;
        }
    }
    else
    {
        float texW = t->texture->width - 1;
        float texH = t->texture->height - 1;
        float duLeft = texW * (t->vertices[2].uv.u - t->vertices[0].uv.u) * invDy;
        float dvLeft = texH * (t->vertices[2].uv.v - t->vertices[0].uv.v) * invDy;
        float duRight = texW * (t->vertices[1].uv.u - t->vertices[0].uv.u) * invDy;
        float dvRight = texH * (t->vertices[1].uv.v - t->vertices[0].uv.v) * invDy;

        float uLeft = texW * t->vertices[0].uv.u;
        float uRight = uLeft;
        float vLeft = texH * t->vertices[0].uv.v;
        float vRight = vLeft;
        int startY = t->vertices[0].position.y;
        int endY = t->vertices[1].position.y;

        for(y = startY; y <= endY; ++y)
        {
            int startX = xLeft;
            int endX  = xRight;
            float u = uLeft;
            float v = vLeft;
            float du, dv;
            float dx = endX - startX;

            if(dx > 0)
            {
                du = (uRight - uLeft) / dx;
                dv = (vRight - vLeft) / dx;
            }
            else
            {
                du = uRight - uLeft;
                dv = vRight - vLeft;
            }

            for(x = startX; x <= endX; ++x)
            {
                byte pixel = t->texture->data[(int)u + ((int)v << 6)];
                drawPixel(x, y, pixel, buffer);
                u += du;
                v += dv;
            }

            xLeft += dxLeft;
            xRight += dxRight;
            uLeft += duLeft;
            uRight += duRight;
            vLeft += dvLeft;
            vRight += dvRight;
        }
    }
}

void clrScrBuffer(unsigned char *buffer)
{
    if(!buffer)
        memset(VGA, 0, SCREEN_WIDTH*SCREEN_HEIGHT);
    else
        memset(buffer, 0, SCREEN_WIDTH*SCREEN_HEIGHT);
}

void updateScreen(unsigned char *buffer)
{
    memcpy(VGA, buffer, SCREEN_WIDTH*SCREEN_HEIGHT);
}
