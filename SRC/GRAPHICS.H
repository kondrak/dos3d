#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "src/bitmap.h"
#include "src/math.h"

#ifdef __cplusplus
extern "C" {
#endif
    #define SCREEN_WIDTH  320
    #define SCREEN_HEIGHT 200
    
    typedef struct
    {
        Vector4f position;
        Vector2f uv;
    } Vertex;
    
    typedef struct
    {
        Vertex vertices[3];
        int color;
        BITMAP *texture;
    } Triangle;
    
    void setMode(unsigned char mode);
    void drawPixel(int x, int y, unsigned char color, unsigned char *buffer);
    void drawLine(int x0, int y0, int x1, int y1, unsigned char color, unsigned char *buffer);
    void drawLineVec(const Vector4f *from, const Vector4f *to, unsigned char color, unsigned char *buffer);
    void drawTriangle(const Triangle *t, unsigned char *buffer);
    void clrScrBuffer(unsigned char *buffer);
    void updateScreen(unsigned char *buffer);

#ifdef __cplusplus
}
#endif
#endif
