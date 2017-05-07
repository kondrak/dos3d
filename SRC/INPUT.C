#include "src/input.h"
#include <conio.h>

static unsigned short __keys[0x81];

// faster keyhit detector than kbhit()
const unsigned short *kbd_getInput()
{
    unsigned char c, al, ah;

    // get last key 
    c = inpd(0x60);
    if (c >= 0x80) __keys[c - 0x80] = 0;
    if (c < 0x80)  __keys[c] = 1;
    al = ah = inpd(0x61);
    al |= 0x80;
    outpd(0x61, al);
    outpd(0x61, ah);
    outpd(0x20, 0x20);
    outpd(0x20, 0x20);

    return &__keys[0];
}
