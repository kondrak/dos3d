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
void utl_drawPalette(gfx_drawBuffer *buffer)
{
    const int tileW = 20;
    const int tileH = 12;
    int x, y, xOffset, yOffset, c = 0;
    char colorNum[3];
    int bufferWidth  = buffer ? buffer->width : SCREEN_WIDTH;
    int bufferHeight = buffer ? buffer->height : SCREEN_HEIGHT;
    gfx_clrBufferColor(buffer, c);

    for(c = 0; c < 256; ++c)
    {
        xOffset = c % 16;
        yOffset = c / 16;

        for(y = yOffset * tileH; y < (yOffset + 1) * tileH && y < bufferHeight; ++y)
        {
            for(x = xOffset * tileW; x < (xOffset + 1) * tileW && x < bufferWidth; ++x)
                gfx_drawPixel(x, y, c, buffer);
        }

        sprintf(colorNum, "%d", c);
        utl_printf(colorNum, xOffset * tileW, yOffset * tileH, 15, c, buffer);
    }
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
                        gfx_drawPixel(x + xOffset, y + yOffset, fgCol, buffer);
                    else
                        gfx_drawPixel(x + xOffset, y + yOffset, bgCol, buffer);
                }
                bitMask >>= 1;
            }
        }

        ++currChar;
    }
}

