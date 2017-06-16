#ifndef BITMAP_H
#define BITMAP_H

#include "src/graphics.h"

/*
 * Bitmap definition and rendering (8bpp).
 */

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct
    {
        unsigned short width;
        unsigned short height;
        unsigned char  palette[256*3];
        unsigned char *data;
    } gfx_Bitmap;

    /* *** Interface *** */

    // load bitmap from file
    gfx_Bitmap gfx_loadBitmap(const char* filename);

    // create a bitmap from a larger *atlas at image position x,y and of size w,h
    gfx_Bitmap gfx_bitmapFromAtlas(const gfx_Bitmap *atlas, int x, int y, int w, int h);

    // return a resized bitmap, the original image data will be freed
    gfx_Bitmap gfx_resizeBitmap(gfx_Bitmap *bmp, int w, int h);

    // draw bitmap to buffer (no color keying)
    void gfx_drawBitmap(const gfx_Bitmap* bmp, int x, int y, gfx_drawBuffer *buffer);

    // draw bitmap to buffer with offsets (no color keying)
    void gfx_drawBitmapOffset(const gfx_Bitmap* bmp, int x, int y, int xOffset, int yOffset, gfx_drawBuffer *buffer);

    // draw bitmap to buffer with colorKey as the tranpsarent color
    void gfx_drawBitmapColorKey(const gfx_Bitmap *bmp, int x, int y, gfx_drawBuffer *buffer, const short colorKey);

    // release bitmap image data
    void gfx_freeBitmap(gfx_Bitmap *bmp);

#ifdef __cplusplus
}
#endif
#endif
