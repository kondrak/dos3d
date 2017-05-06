#ifndef BITMAP_H
#define BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    unsigned short width;
    unsigned short height;
    unsigned char palette[256*3];
    unsigned char *data;
} Bitmap;

void setPalette(unsigned char *palette);

Bitmap loadBitmap(const char* filename);
void drawBitmap(Bitmap* bmp, int x, int y, unsigned char *buffer);
void drawTransparentBitmap(Bitmap *bmp,int x, int y, unsigned char *buffer);
Bitmap bitmapFromAtlas(Bitmap *atlas, int x, int y, int w, int h);
Bitmap resizeBitmap(Bitmap *bmp, int w, int h);
void freeBitmap(Bitmap *bmp);

#ifdef __cplusplus
}
#endif
#endif
