#include "src/timer.h"
#include <conio.h>
#include <dos.h>

#define ASM_TIMER

typedef void (__interrupt __far *intFuncPtr)();

static intFuncPtr oldTimerInterrupt; // original timer interrupt handler

volatile uint32_t milliseconds = 0; // elapsed time in milliseconds

// internal: 1ms timer interrupt handler function
static void __interrupt __far timerHandler()
{
    static uint32_t count = 0; // to keep track of original timer ticks
    
    ++milliseconds;
    count += 1103;
    if(count >= 65536) // it is now time to call the original handler
    {
        count -= 65536;
        _chain_intr(oldTimerInterrupt);
    }
    else
    {
        // acknowledge interrupt
#ifdef ASM_TIMER
        __asm
        {
            mov al, 20h
            out 20h, al
        }
#else
    outp(0x20, 0x20);
#endif
    }
}

/* ***** */
void tmr_start()
{
    union REGS r;
    struct SREGS s;
    _disable();
    segread(&s);

    // save old interrupt vector (irq 08h)
    r.h.al = 0x08;
    r.h.ah = 0x35;
    int386x(0x21, &r, &r, &s);
    oldTimerInterrupt = (intFuncPtr)MK_FP(s.es, r.x.ebx);

    // install new interrupt handler
    milliseconds = 0;
    r.h.al = 0x08;
    r.h.ah = 0x25;
    s.ds    = FP_SEG(timerHandler);
    r.x.edx = FP_OFF(timerHandler);
    int386x(0x21, &r, &r, &s);

    // Set resolution of timer chip to 1ms
#ifdef ASM_TIMER
    __asm
    {
        mov al, 36h
        out 43h, al
        mov al, 4Fh
        out 40h, al
        mov al, 04h
        out 40h, al
    }
#else
    outp(0x43, 0x36);
    outp(0x40, (uint8_t)(1103 & 0xff));
    outp(0x40, (uint8_t)((1103 >> 8) & 0xff));
#endif
    _enable();
}

/* ***** */
void tmr_finish()
{
    union REGS r;
    struct SREGS s;

    if(!oldTimerInterrupt) return;

    _disable();
    segread(&s);
 
    // restore original interrupt handler
    r.h.al = 0x08;
    r.h.ah = 0x25;
    s.ds    = FP_SEG(oldTimerInterrupt);
    r.x.edx = FP_OFF(oldTimerInterrupt);
    int386x(0x21, &r, &r, &s);
 
    // reset timer chip resolution to 18.2...ms
#ifdef ASM_TIMER
    __asm
    {
        mov al, 36h
        out 43h, al
        mov al, 00h
        out 40h, al
        mov al, 00h
        out 40h, al
    }
#else
    outp(0x43, 0x36);
    outp(0x40, 0x00);
    outp(0x40, 0x00);
#endif
    _enable();
}

/* ***** */
uint32_t tmr_getMs()
{
    return milliseconds;
}
