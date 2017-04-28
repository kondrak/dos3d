#ifndef BITMAP_H
#define BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char byte;
typedef unsigned short word;

typedef struct
{
   word width;
   word height;
   byte palette[256*3];
   byte *data;
} BITMAP;

void setPalette(byte *palette);
void rotatePalette(byte *palette);

BITMAP loadBitmap(const char* filename);
void drawBitmap(BITMAP* bmp, int x, int y, unsigned char *buffer);
void drawTransparentBitmap(BITMAP *bmp,int x,int y, unsigned char *buffer);
void freeBitmap(BITMAP *bmp);

#ifdef __cplusplus
}
#endif
#endif
