#include "src/bitmap.h"
#include "src/graphics.h"
#include <conio.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>

// internal: skips file sections when loading a bitmap
static void __fskip(FILE *fp, int num_bytes)
{
    int i;
    for (i = 0; i < num_bytes; i++)
        fgetc(fp);
}

/* ***** */
gfx_Bitmap gfx_loadBitmap(const char* filename)
{
    gfx_Bitmap bmp;
    FILE *fp;
    long index;
    int x;
    unsigned short num_colors;

    if((fp = fopen(filename, "rb")) == NULL)
    {
        printf("Error opening file %s.\n",filename);
        exit(1);
    }

    if(fgetc(fp) != 'B' || fgetc(fp) != 'M')
    {
        fclose(fp);
        printf("%s is not a bitmap file.\n", filename);
        exit(1);
    }

    /* read in the width and height of the image, and the
       number of colors used; ignore the rest */
    __fskip(fp, 16);
    fread(&bmp.width, sizeof(unsigned short), 1, fp);
    __fskip(fp, 2);
    fread(&bmp.height, sizeof(unsigned short), 1, fp);
    __fskip(fp, 22);
    fread(&num_colors, sizeof(unsigned short), 1, fp);
    __fskip(fp, 6);

    // assume we are working with an 8-bit file
    if(!num_colors) num_colors = 256;

    if((bmp.data = (unsigned char *)malloc(bmp.width*bmp.height)) == NULL)
    {
        fclose(fp);
        printf("Error allocating memory for file %s.\n",filename);
        exit(1);
    }

    // read the palette information
    for(index = 0; index < num_colors; index++)
    {
        bmp.palette[(int)(index*3+2)] = fgetc(fp) >> 2;
        bmp.palette[(int)(index*3+1)] = fgetc(fp) >> 2;
        bmp.palette[(int)(index*3+0)] = fgetc(fp) >> 2;
        x = fgetc(fp);
    }

    // read the bitmap
    for(index = (bmp.height-1) * bmp.width; index >= 0; index -= bmp.width)
        for(x = 0; x < bmp.width; x++)
            bmp.data[ index + x ] = (unsigned char)fgetc(fp);

    fclose(fp);
    return bmp;
}

/* ***** */
gfx_Bitmap gfx_bitmapFromAtlas(const gfx_Bitmap *atlas, int x, int y, int w, int h)
{
    int cx, cy, p;
    gfx_Bitmap subImage;
    subImage.width  = w;
    subImage.height = h;
    // retain the palette of the atlas
    memcpy(subImage.palette, atlas->palette, sizeof(unsigned char)*256*3);
    
    if ((subImage.data = (unsigned char *) malloc(w*h)) == NULL)
    {
        printf("Error allocating memory for atlas sub image bitmap!\n");
        exit(1);
    }
    
    for(p = 0, cy = y; cy < y+h; cy++)
    {
        for(cx = x; cx < x+w; cx++, p++)
        {
            subImage.data[p] = atlas->data[cy * atlas->width + cx];
        }
    }
    
    return subImage;
}

/* ***** */
gfx_Bitmap gfx_resizeBitmap(gfx_Bitmap *bmp, int w, int h)
{
    int cx,cy;
    float scaleX = (float)w / bmp->width;
    float scaleY = (float)h / bmp->height;
    gfx_Bitmap resized;
    resized.width  = w;
    resized.height = h;
    memcpy(resized.palette, bmp->palette, sizeof(unsigned char)*256*3);

    if ((resized.data = (unsigned char *) malloc(w*h)) == NULL)
    {
        printf("Error allocating memory for resized bitmap!\n");
        exit(1);
    }

    // rescale bitmap using nearest neighbour algorithm
    for(cy = 0; cy < h; cy++)
    {
        for(cx = 0; cx < w; cx++)
        {
            int p = cy * w + cx;
            int nn = (((int)(cy / scaleY) * bmp->width) + ((int)(cx / scaleX)));

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
void gfx_drawBitmap(const gfx_Bitmap *bmp, int x, int y, unsigned char *buffer)
{
    int j;
    int screenOffset = (y<<8)+(y<<6)+x;
    int bmpOffset = 0;

    for(j = 0; j < bmp->height; j++)
    {
        memcpy(&buffer[screenOffset], &bmp->data[bmpOffset], bmp->width);
        bmpOffset    += bmp->width;
        screenOffset += SCREEN_WIDTH;
    }
}

/* ***** */
void gfx_drawBitmapColorKey(const gfx_Bitmap *bmp, int x, int y, unsigned char *buffer, const short colorKey)
{
    int i,j;
    int screenOffset = (y<<8)+(y<<6);
    int bmpOffset = 0;
    unsigned char data;

    for(j = 0; j < bmp->height; j++)
    {
        for(i = 0; i < bmp->width; i++, bmpOffset++)
        {
            data = bmp->data[bmpOffset];
            // skip a pixel if it's the same color as colorKey
            if(data != (unsigned char)colorKey)
                buffer[screenOffset+x+i] = data;
        }
      screenOffset += SCREEN_WIDTH;
    }
}

void gfx_freeBitmap(gfx_Bitmap *bmp)
{
    free(bmp->data);
}
