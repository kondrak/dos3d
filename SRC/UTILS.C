#include "src/graphics.h"
#include "src/utils.h"
#include <stdarg.h>
#include <string.h>

// maximum lenght of a string processed by utl_printf()
#define PRINTF_LEN 128

// internal: render a single character
static void drawChar(const int x, const int y, char c, const uint8_t fgCol, const uint8_t bgCol, gfx_drawBuffer *buffer);

/* ***** */
void utl_printf(gfx_drawBuffer *buffer, const int x, const int y, const uint8_t fgCol, const uint8_t bgCol, const char *format, ...)
{
    int i=-1;
    char strBuffer[PRINTF_LEN];
    va_list args;
    va_start(args, format);
    vsnprintf(strBuffer, PRINTF_LEN, format, args);
    va_end(args);

    while(strBuffer[++i])
        drawChar(x + (i << 3), y, strBuffer[i], fgCol, bgCol, buffer);
}

/* ***** */
void utl_drawPalette(gfx_drawBuffer *buffer)
{
    const int tileW = 20;
    const int tileH = 12;
    int x, y, xOffset, yOffset, c = 0;
    int bufferWidth  = buffer ? buffer->width : SCREEN_WIDTH;
    int bufferHeight = buffer ? buffer->height : SCREEN_HEIGHT;
    gfx_clrBufferColor(buffer, c);

    for(c = 0; c < 256; ++c)
    {
        xOffset = c % 16;
        yOffset = c / 16;

        for(y = yOffset * tileH + 4; y < (yOffset + 1) * tileH + 4 && y < bufferHeight; ++y)
        {
            for(x = xOffset * tileW; x < (xOffset + 1) * tileW && x < bufferWidth; ++x)
                gfx_drawPixel(x, y, c, buffer);
        }

        utl_printf(buffer, xOffset * tileW, yOffset * tileH + 4, 0, c, "%x", c);
    }
}

/* ***** */
static void drawChar(const int x, const int y, char c, const uint8_t fgCol, const uint8_t bgCol, gfx_drawBuffer *buffer)
{
    // start address of ROM character set storage
    static uint8_t *romCharSet = (uint8_t *)0xFFA6E;
    uint8_t bitMask;
    int xOffset, yOffset;
    int buffWidth  = buffer ? buffer->width : SCREEN_WIDTH;
    int buffHeight = buffer ? buffer->height : SCREEN_HEIGHT;

    // stored characters are 8x8 bitmaps
    uint8_t *currChar = &romCharSet[c * 8];

    for(yOffset = 0; yOffset < 8; ++yOffset) {
        if(y + yOffset < buffHeight) 
        {
            bitMask = 0x80;

            for(xOffset = 0; xOffset < 8; ++xOffset) 
            {
                if(x + xOffset < buffWidth) 
                {
                   if(*currChar & bitMask) 
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

