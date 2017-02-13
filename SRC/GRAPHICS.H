#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef __cplusplus
extern "C" {
#endif

    void setMode(unsigned char mode);
    void drawPixel(int x, int y, unsigned char color);
    void drawLine(int x0, int y0, int x1, int y1, unsigned char color);

#ifdef __cplusplus
}
#endif
#endif
