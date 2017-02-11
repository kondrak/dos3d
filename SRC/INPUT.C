#include "src/input.h"
#include <conio.h>

unsigned short keys[0x81];

// faster keyhit detector than kbhit()
unsigned short *translateInput()
{
    unsigned char c, al, ah;

    // get last key 
    c = inportb(0x60);
    if (c >= 0x80) keys[c - 0x80] = 0;
    if (c < 0x80)  keys[c] = 1;
    al = ah = inportb(0x61);
    al |= 0x80;
    outportb(0x61, al);
    outportb(0x61, ah);
    outportb(0x20, 0x20);
    outport(0x20, 0x20);

    return &keys[0];
}
