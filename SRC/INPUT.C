#include "src/input.h"
#include <conio.h>
#include <memory.h>

static uint16_t keys[0x81];

// faster keyhit detector than kbhit()
const uint16_t *kbd_getInput()
{
    uint8_t key = '\0';

    // get last key 
    _asm {
        in al, 60h
        mov key, al
        in al, 61h
        or al, 128
        out 61h, al
        xor al, 128
        out 61h, al
    }

    // clear keyboard buffer
    outpd(0x20, 0x20);
    outpd(0x20, 0x20);

    if (key >= 0x80) keys[key - 0x80] = 0;
    if (key < 0x80)  keys[key] = 1;

    return &keys[0];
}

void kbd_flush()
{
    memset(keys, 0, sizeof(uint16_t) * 0x81);
}
