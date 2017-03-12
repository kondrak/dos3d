#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef __cplusplus
extern "C" {
#endif
    #define SCREEN_WIDTH  320
    #define SCREEN_HEIGHT 200

    void setMode(unsigned char mode);
    void drawPixel(int x, int y, unsigned char color, unsigned char *buffer);
    void drawLine(int x0, int y0, int x1, int y1, unsigned char color, unsigned char *buffer);
    void clrScrBuffer(unsigned char *buffer);
    void updateScreen(unsigned char *buffer);

#ifdef __cplusplus
}
#endif
#endif
