#include "src/bitmap.h"
#include "src/graphics.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <mem.h>

void fskip(FILE *fp, int num_bytes)
{
    int i;
    for (i = 0; i < num_bytes; i++)
        fgetc(fp);
}

Bitmap loadBitmap(const char* filename)
{
    Bitmap bmp;
    Bitmap *b;
    FILE *fp;
    long index;
    int x;
    unsigned short num_colors;
   
    b = &bmp;
   
    if((fp = fopen(filename, "rb")) == NULL)
    {
        printf("Error opening file %s.\n",filename);
        exit(1);
    }

    /* check to see if it is a valid bitmap file */
    if(fgetc(fp) != 'B' || fgetc(fp) != 'M')
    {
        fclose(fp);
        printf("%s is not a bitmap file.\n", filename);
        exit(1);
    }

    /* read in the width and height of the image, and the
        number of colors used; ignore the rest */
    fskip(fp, 16);
    fread(&b->width, sizeof(unsigned short), 1, fp);
    fskip(fp, 2);
    fread(&b->height, sizeof(unsigned short), 1, fp);
    fskip(fp, 22);
    fread(&num_colors, sizeof(unsigned short), 1, fp);
    fskip(fp, 6);

    /* assume we are working with an 8-bit file */
    if(num_colors==0) num_colors=256;

    if((b->data = (unsigned char *)malloc(b->width*b->height)) == NULL)
    {
        fclose(fp);
        printf("Error allocating memory for file %s.\n",filename);
        exit(1);
    }

    /* read the palette information */
    for(index = 0; index < num_colors; index++)
    {
        b->palette[(int)(index*3+2)] = fgetc(fp) >> 2;
        b->palette[(int)(index*3+1)] = fgetc(fp) >> 2;
        b->palette[(int)(index*3+0)] = fgetc(fp) >> 2;
        x = fgetc(fp);
    }

    /* read the bitmap */
    for(index = (b->height-1) * b->width; index >= 0; index -= b->width)
        for(x = 0; x < b->width; x++)
            b->data[ index + x ] = (unsigned char)fgetc(fp);

    fclose(fp);
    return bmp;
}

void drawBitmap(Bitmap* bmp, int x, int y, unsigned char *buffer)
{
    int j;
    int screen_offset = (y<<8)+(y<<6)+x;
    int bitmap_offset = 0;

    for(j = 0; j < bmp->height; j++)
    {
        memcpy(&buffer[screen_offset], &bmp->data[bitmap_offset], bmp->width);
        bitmap_offset += bmp->width;
        screen_offset += SCREEN_WIDTH;
    }
}

void drawTransparentBitmap(Bitmap *bmp, int x, int y, unsigned char *buffer)
{
    int i,j;
    int screen_offset = (y<<8)+(y<<6);
    int bitmap_offset = 0;
    unsigned char data;

    for(j = 0; j < bmp->height; j++)
    {
        for(i = 0; i < bmp->width; i++, bitmap_offset++)
        {
            data = bmp->data[bitmap_offset];
            if(data) 
                buffer[screen_offset+x+i] = data;
        }
      screen_offset += SCREEN_WIDTH;
    }
}

Bitmap bitmapFromAtlas(Bitmap *atlas, int x, int y, int w, int h)
{
    int cx, cy, p;
    Bitmap subImage;
    subImage.width = w;
    subImage.height = h;
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

Bitmap resizeBitmap(Bitmap* bmp, int w, int h)
{
    int cx,cy;
    float scaleX = (float)w / bmp->width;
    float scaleY = (float)h / bmp->height;
    Bitmap resized;
    resized.width = w;
    resized.height = h;
    memcpy(resized.palette, bmp->palette, sizeof(unsigned char)*256*3);

    if ((resized.data = (unsigned char *) malloc(w*h)) == NULL)
    {
        printf("Error allocating memory for resized bitmap!\n");
        exit(1);
    }

    // rescale bitmap using nearest neighbour
    for(cy = 0; cy < h; cy++)
    {
        for(cx = 0; cx < w; cx++)
        {
            int p = cy * w + cx;
            int nearest = (((int)(cy / scaleY) * bmp->width) + ((int)(cx / scaleX)));

            resized.data[p] = bmp->data[nearest];
            resized.data[p+1] = bmp->data[nearest+1];
            resized.data[p+2] = bmp->data[nearest+2];
        }
    }

    freeBitmap(bmp);
    return resized;
}

void freeBitmap(Bitmap *bmp)
{
    free(bmp->data);
}

void setPalette(unsigned char *palette)
{
    int i;
    outp(0x03c8, 0);

    for(i = 0; i < 256*3; ++i)
    {
        outp(0x03c9, palette[i]);
    }
}
