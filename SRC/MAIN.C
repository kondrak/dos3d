#include "src/input.h"
#include "src/math.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const int SCREEN_WIDTH  = 320;
static const int SCREEN_HEIGHT = 200;

// pointer to VGA memory
unsigned char *VGA = (unsigned char *)0xA0000000L;

// graphics mode setter
void setMode(unsigned char mode)
{
    _asm {
	    mov ah, 0x00
	    mov al, mode
	    int 10h
    }
}

// good bye
void Shutdown(int exitCode)
{
    setMode(0x03);
    exit(exitCode);
}

void draw(int x, int y, unsigned char color)
{
    VGA[(y << 8) + (y << 6) + x] = color;
}

void Bresenham(int x0, int y0, int x1, int y1, unsigned char color)
{
  int x = x1 - x0;
  int y = y1 - y0;
  int ax = abs(x), ay = abs(y);
  int dx1 = x < 0 ? -1 : x > 0 ? 1 : 0;
  int dy1 = y < 0 ? -1 : y > 0 ? 1 : 0;
  int dx2 = dx1, dy2 = 0;
  int n, i = 0;

  if(ax <= ay)
  {
      // swap ax <=> ay
      ax ^= ay; ay ^= ax; ax ^= ay;
      dx2 = 0; dy2 = dy1;
  }

  n = ax >> 1;

  for(; i <= ax; ++i)
  {
      draw(x0, y0, color);
      n += ay;

      if (n >= ax)
      { x0 += dx1; y0 += dy1; n -= ax; }
      else
      { x0 += dx2; y0 += dy2;           }
  }
}

unsigned short *keysPressed;

void drawCircle(int ox, int oy, int r)
{
    float l = 0.0f;
    for(l; l < 2.0f*M_PI; l += 0.001f)
    {
      int color = (int)((l * 10.f) + 1) % 256;
      Bresenham(ox, oy, ox + r * cos(l), oy + r * sin(l), color);
      draw(ox + r * cos(l), oy + r * sin(l), color);
      keysPressed = translateInput();
      if(keysPressed[KEY_ESC]) return;
    }
}


/* ***** */

int main(int argc, char **argv)
{
    float invWidth, invHeight, aspectRatio, angle;
    int x, y, renderFinished = 0;
    int fov = 45;

    keysPressed = translateInput();
    
    for (x = 0; x < argc; x++)
    {
        // custom fov flag
        if (!strcmp(argv[x], "-f") && x + 1 < argc)
        {
            if (isdigit(argv[x + 1][0]))
                fov = atof(argv[x + 1]);
        }
    }

    setMode(0x13);

    // calculate the view
    invWidth = 1.0f / SCREEN_WIDTH;
    invHeight = 1.0f / SCREEN_HEIGHT;
    aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    angle = tan(M_PI * 0.5 * fov / 180.);

    while (!keysPressed[KEY_ESC])
    {
        // perform draw after vsync
        if (!renderFinished)
        {
            for (x = 0; x < SCREEN_WIDTH; x++)
            {
                for (y = 0; y < SCREEN_HEIGHT; y++)
                {
                    // draw the pixel
                    VGA[(y << 8) + (y << 6) + x] = 0;
                }
            }

            drawCircle(160, 100, 95);
            
            renderFinished = 1;
            keysPressed = translateInput();
        }
        else
        {
            keysPressed = translateInput();
        }
    }
    
    Shutdown(0);
    return 0;
}
