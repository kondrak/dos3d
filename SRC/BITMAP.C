#include "src/bitmap.h"
#include "src/graphics.h"
#include "src/utils.h"
#include <conio.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>

extern gfx_drawBuffer VGA_BUFFER;

// internal: skips file sections when loading a bitmap
static void fskip(FILE *fp, int num_bytes)
{
    int i;
    for (i = 0; i < num_bytes; ++i)
        fgetc(fp);
}

/* ***** */
gfx_Bitmap gfx_loadBitmap(const char *filename)
{
    gfx_Bitmap bmp;
    int32_t index;
    int x;
    uint16_t num_colors;
    FILE *fp = fopen(filename, "rb");

    ASSERT(fp, "Error opening file %s.\n", filename);

    if(fgetc(fp) != 'B' || fgetc(fp) != 'M')
    {
        fclose(fp);
        ASSERT(0, "%s is not a bitmap file.\n", filename);
    }

    /* read in the width and height of the image, and the
       number of colors used; ignore the rest */
    fskip(fp, 16);
    fread(&bmp.width, sizeof(uint16_t), 1, fp);
    fskip(fp, 2);
    fread(&bmp.height, sizeof(uint16_t), 1, fp);
    fskip(fp, 22);
    fread(&num_colors, sizeof(uint16_t), 1, fp);
    fskip(fp, 6);

    // assume we are working with an 8-bit file
    if(!num_colors) num_colors = 256;

    if((bmp.data = (uint8_t *)malloc(sizeof(uint8_t) * bmp.width * bmp.height)) == NULL)
    {
        fclose(fp);
        ASSERT(0, "Error allocating memory for file %s.\n", filename);
    }

    // read the palette information
    for(index = 0; index < num_colors; ++index)
    {
        bmp.palette[index*3 + 2] = fgetc(fp) >> 2;
        bmp.palette[index*3 + 1] = fgetc(fp) >> 2;
        bmp.palette[index*3 + 0] = fgetc(fp) >> 2;
        x = fgetc(fp);
    }

    // read the bitmap
    for(index = (bmp.height - 1) * bmp.width; index >= 0; index -= bmp.width)
        for(x = 0; x < bmp.width; ++x)
            bmp.data[index + x] = (uint8_t)fgetc(fp);

    fclose(fp);
    return bmp;
}

/* ***** */
gfx_Bitmap gfx_bitmapFromAtlas(const gfx_Bitmap *atlas, int x, int y, int w, int h)
{
    int cy, p;
    gfx_Bitmap subImage;
    subImage.width  = w;
    subImage.height = h;
    // retain the palette of the atlas
    memcpy(subImage.palette, atlas->palette, sizeof(uint8_t)*256*3);

    subImage.data = (uint8_t *)malloc(sizeof(uint8_t) * w * h);
    ASSERT(subImage.data, "Error allocating memory for atlas sub image bitmap!\n");

    for(p = 0, cy = y; cy < y + h && p < w * (h - 1); ++cy, p += w)
        memcpy(subImage.data + p, atlas->data + x + cy * atlas->width, sizeof(uint8_t) * (x + w));

    return subImage;
}

/* ***** */
gfx_Bitmap gfx_resizeBitmap(gfx_Bitmap *bmp, int w, int h)
{
    int cx,cy;
    float scaleX = (float)bmp->width / w;
    float scaleY = (float)bmp->height / h;
    gfx_Bitmap resized;
    resized.width  = w;
    resized.height = h;
    memcpy(resized.palette, bmp->palette, sizeof(uint8_t)*256*3);

    resized.data = (uint8_t *)malloc(sizeof(uint8_t) * w * h);
    ASSERT(resized.data, "Error allocating memory for resized bitmap!\n");

    // rescale bitmap using nearest neighbour algorithm
    for(cy = 0; cy < h; ++cy)
    {
        for(cx = 0; cx < w; ++cx)
        {
            int p = cy * w + cx;
            int nn = (int)(cy * scaleY) * bmp->width + (int)cx * scaleX;

            resized.data[p]   = bmp->data[nn];
            resized.data[p+1] = bmp->data[nn+1];
            resized.data[p+2] = bmp->data[nn+2];
        }
    }

    // release the original image data
    gfx_freeBitmap(bmp);
    return resized;
}

/* ***** */
void gfx_drawBitmap(const gfx_Bitmap *bmp, int x, int y, gfx_drawBuffer *target)
{
    int j;
    gfx_drawBuffer *buffer = target ? target : &VGA_BUFFER;
    // adjust for offscreen positioning
    int offscreenX = x < 0 ? -x : 0;
    int offscreenY = y < 0 ? -y : 0;
    int screenOffset = x + offscreenX + (y + offscreenY) * buffer->width;
    int width  = MIN(bmp->width - offscreenX, buffer->width - (x < 0 ? offscreenX : x));
    int height = MIN(bmp->height, buffer->height - y);
    uint8_t *dstBuff = buffer->colorBuffer;
    uint8_t *bmpBuff = bmp->data;

    // attempting to write offscreen
    if(width < 0 || x > buffer->width) return;

    for(j = 0; j < height - offscreenY; ++j)
    {
        memcpy(&dstBuff[screenOffset + j * buffer->width], 
               &bmpBuff[offscreenX + (j + offscreenY) * bmp->width], sizeof(uint8_t) * width);
    }
}

/* ***** */
void gfx_drawBitmapOffset(const gfx_Bitmap *bmp, int x, int y, int xOffset, int yOffset, gfx_drawBuffer *target)
{
    int j, scanlineLength;
    gfx_drawBuffer *buffer = target ? target : &VGA_BUFFER;
    // adjust for offscreen positioning
    int offscreenX = x < 0 ? -x : 0;
    int offscreenY = y < 0 ? -y : 0;
    int screenOffset = x + offscreenX + (y + offscreenY) * buffer->width;
    int texArea = bmp->width * bmp->height;
    int height  = MIN(bmp->height, buffer->height - y);
    int drawWidth = buffer->width - (x < 0 ? offscreenX : x);
    uint8_t *dstBuff = buffer->colorBuffer;
    uint8_t *bmpBuff = bmp->data;

    // attemtping to write offscreen
    if(drawWidth < 0 || x > buffer->width) return;

    xOffset += xOffset > 0 ? -offscreenX : offscreenX;
    if(xOffset < 0) xOffset += bmp->width;
    if(yOffset < 0) yOffset += bmp->height;

    for(j = 0; j < height - offscreenY; ++j)
    {
        scanlineLength = bmp->width - xOffset;
        if(scanlineLength > bmp->width) scanlineLength -= bmp->width;
        if(scanlineLength > drawWidth)  scanlineLength = drawWidth;

        memcpy(&dstBuff[screenOffset + j * buffer->width], 
               &bmpBuff[(xOffset + (j + yOffset + offscreenY) * bmp->width) % texArea], sizeof(uint8_t) * scanlineLength);

        if(drawWidth > scanlineLength)
            memcpy(&dstBuff[screenOffset + j * buffer->width + scanlineLength], 
                   &bmpBuff[(j + yOffset + offscreenY) * bmp->width % texArea], sizeof(uint8_t) * (drawWidth - scanlineLength));
    }
}

/* ***** */
void gfx_drawBitmapColorKey(const gfx_Bitmap *bmp, int x, int y, gfx_drawBuffer *target, const uint8_t colorKey)
{
    int i,j;
    gfx_drawBuffer *buffer = target ? target : &VGA_BUFFER;
    // adjust for offscreen positioning
    int offscreenX = x < 0 ? -x : 0;
    int offscreenY = y < 0 ? -y : 0;
    int screenOffset = (y + offscreenY) * buffer->width;
    int width  = MIN(bmp->width - offscreenX, buffer->width - (x < 0 ? offscreenX : x));
    int height = MIN(bmp->height, buffer->height - y);

    // attemtping to write offscreen
    if(width < 0 || x > buffer->width) return;

    for(j = 0; j < height - offscreenY; ++j)
    {
        for(i = 0; i < width; ++i)
        {
            uint8_t data = bmp->data[i + offscreenX + (j + offscreenY) * bmp->height];
            // skip a pixel if it's the same color as colorKey
            if(data != colorKey)
                buffer->colorBuffer[screenOffset + x + i + j * buffer->width] = data;
        }
    }
}

/* ***** */
void gfx_freeBitmap(gfx_Bitmap *bmp)
{
    free(bmp->data);
}
