#include "src/graphics.h"
#include "src/utils.h"

// internal: render a single character
static void drawChar(const int x, const int y, unsigned char c, const unsigned char fgCol, const unsigned char bgCol, gfx_drawBuffer *buffer);

/* ***** */
void utl_printf(const char *str, const int x, const int y, const unsigned char fgCol, const unsigned char bgCol, gfx_drawBuffer *buffer)
{
    int i=-1;
    while(str[++i])
        drawChar(x + (i << 3), y, str[i], fgCol, bgCol, buffer);
}

/* ***** */
static void drawChar(const int x, const int y, unsigned char c, const unsigned char fgCol, const unsigned char bgCol, gfx_drawBuffer *buffer)
{
    // start address of ROM character set storage
    static unsigned char *romCharSet = (unsigned char *)0xFFA6E;
    unsigned char bitMask;
    int xOffset, yOffset;
    int buffWidth  = buffer ? buffer->width : SCREEN_WIDTH;
    int buffHeight = buffer ? buffer->height : SCREEN_HEIGHT;

    // stored characters are 8x8 bitmaps
    unsigned char *currChar = &romCharSet[c * 8];

    for(yOffset = 0; yOffset < 8; ++yOffset) {
        if(y + yOffset < buffHeight) 
        {
            bitMask = 0x80;

            for(xOffset = 0; xOffset < 8; ++xOffset) 
            {
                if(x + xOffset < buffWidth) 
                {
                   if (*currChar & bitMask) 
                        gfx_drawPixel(x + xOffset, y + yOffset, fgCol, NULL);
                    else
                        gfx_drawPixel(x + xOffset, y + yOffset, bgCol, NULL);
                }
                bitMask >>= 1;
            }
        }

        ++currChar;
    }
}

