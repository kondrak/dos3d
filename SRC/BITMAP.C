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

BITMAP loadBitmap(const char* filename)
{
   BITMAP bmp;
   BITMAP *b;
   FILE *fp;
   long index;
   int x;
   word num_colors;
   
   b = &bmp;
   
   /* open the file */
   if ((fp = fopen(filename, "rb")) == NULL)
   {
     printf("Error opening file %s.\n",filename);
     exit(1);
   }

   /* check to see if it is a valid bitmap file */
   if (fgetc(fp) != 'B' || fgetc(fp) != 'M')
   {
      fclose(fp);
      printf("%s is not a bitmap file.\n", filename);
      exit(1);
   }

   /* read in the width and height of the image, and the
     number of colors used; ignore the rest */
   fskip(fp, 16);
   fread(&b->width, sizeof(word), 1, fp);
   fskip(fp, 2);
   fread(&b->height, sizeof(word), 1, fp);
   fskip(fp, 22);
   fread(&num_colors, sizeof(word), 1, fp);
   fskip(fp, 6);

   /* assume we are working with an 8-bit file */
   if (num_colors==0) num_colors=256;


   /* try to allocate memory */
   if ((b->data = (byte *) malloc((word)(b->width*b->height))) == NULL)
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
         b->data[ (word)index + x ] = (byte)fgetc(fp);

   fclose(fp);
   return bmp;
}

void drawBitmap(BITMAP* bmp, int x, int y, unsigned char *buffer)
{
   int j;
   word screen_offset = (y<<8)+(y<<6)+x;
   word bitmap_offset = 0;

   for(j = 0; j < bmp->height; j++)
   {
       memcpy(&buffer[screen_offset], &bmp->data[bitmap_offset], bmp->width);
       bitmap_offset += bmp->width;
       screen_offset += SCREEN_WIDTH;
   }
}

void drawTransparentBitmap(BITMAP *bmp, int x, int y, unsigned char *buffer)
{
   int i,j;
   word screen_offset = (y<<8)+(y<<6);
   word bitmap_offset = 0;
   byte data;

   for(j = 0; j < bmp->height; j++)
   {
      for(i = 0; i < bmp->width; i++, bitmap_offset++)
      {
         data = bmp->data[bitmap_offset];
         if (data) buffer[screen_offset+x+i] = data;
      }
	  
      screen_offset += SCREEN_WIDTH;
   }
}

void setPalette(byte *palette)
{
   int i;
   outp(0x03c8, 0);

   for(i = 0; i < 256*3; ++i)
   {
      outp(0x03c9, palette[i]);
   }
}

void rotatePalette(byte *palette)
{
   int i,red,green,blue;

   red  = palette[3];
   green= palette[4];
   blue = palette[5];

   for(i = 3; i < 256*3-3; i++)
      palette[i] = palette[i+3];

   palette[256*3-3] = red;
   palette[256*3-2] = green;
   palette[256*3-1] = blue;

   setPalette(palette);
}
