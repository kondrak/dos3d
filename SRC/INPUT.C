#include "src/input.h"
#include <conio.h>
#include <memory.h>

static uint16_t keys[0x81];

// faster keyhit detector than kbhit()
const uint16_t *kbd_getInput()
{
    uint8_t key = 0;

    // get last key and flush keyboard buffer
    _asm {
        in al, 60h
        mov key, al
        in al, 61h
        mov ah, al
        or al, 80h
        out 61h, al
        xchg ah, al
        out 61h, al
        mov al, 20h
        out 20h, al
        mov ah, 0Ch
        mov al, 0h
        int 21h
    }

    if (key >= 0x80) keys[key - 0x80] = 0;
    if (key < 0x80)  keys[key] = 1;

    return &keys[0];
}

void kbd_flush()
{
    memset(keys, 0, sizeof(uint16_t) * 0x81);
}
