#include "src/input.h"
#include <conio.h>
#include <dos.h>
#include <memory.h>

static uint16_t keysDown[0x81];
static uint16_t keysPressed[0x81];

typedef void (__interrupt __far *kbdIntFuncPtr)();

static kbdIntFuncPtr oldKbdInterrupt; // original keyboard interrupt handler

// internal: interrupt handler for the keyboard - update key state tables
static void __interrupt __far keyboardHandler()
{
    (void)kbd_updateInput();
}

/* ***** */
void kbd_start()
{
    union REGS r;
    struct SREGS s;
    _disable();
    segread(&s);

    // save old interrupt vector (irq 09h)
    r.h.al = 0x09;
    r.h.ah = 0x35;
    int386x(0x21, &r, &r, &s);
    oldKbdInterrupt = (kbdIntFuncPtr)MK_FP(s.es, r.x.ebx);

    // install new interrupt handler
    r.h.al = 0x09;
    r.h.ah = 0x25;
    s.ds    = FP_SEG(keyboardHandler);
    r.x.edx = FP_OFF(keyboardHandler);
    int386x(0x21, &r, &r, &s);

    _enable();
}

/* ***** */
void kbd_finish()
{
    union REGS r;
    struct SREGS s;

    if(!oldKbdInterrupt) return;

    _disable();
    segread(&s);
 
    // restore original interrupt handler
    r.h.al = 0x09;
    r.h.ah = 0x25;
    s.ds    = FP_SEG(oldKbdInterrupt);
    r.x.edx = FP_OFF(oldKbdInterrupt);
    int386x(0x21, &r, &r, &s);

    _enable();
}

/* ***** */
const uint16_t *kbd_updateInput()
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

    if(key < 0x80)
        keysDown[key] = 1;
    else
    {
        keysDown[key - 0x80]    = 0;
        keysPressed[key - 0x80] = 0;
    }

    return &keysDown[0];
}

/* ***** */
int kbd_keyPressed(enum kbd_KeyCode key)
{
    if(keysDown[key] && keysPressed[key])
        return 0;

    if(keysDown[key] && !keysPressed[key])
    {
        keysPressed[key] = 1;
        return 1;
    }

    return 0;
}

/* ***** */
int kbd_keyDown(enum kbd_KeyCode key)
{
    return keysDown[key];
}

/* ***** */
void kbd_flush()
{
    memset(keysDown, 0, sizeof(uint16_t) * 0x81);
    memset(keysPressed, 0, sizeof(uint16_t) * 0x81);
}
