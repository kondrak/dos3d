#include "src/timer.h"
#include <conio.h>
#include <dos.h>

#define ASM_TIMER

typedef void (__interrupt __far *intFuncPtr)();

intFuncPtr oldTimerInterrupt; // Original interrupt handler

volatile unsigned long int milliseconds = 0; // Elapsed time in milliseconds

// internal: 1ms timer interrupt handler function
void __interrupt __far timerHandler()
{
    static unsigned long count = 0; // To keep track of original timer ticks
    
    ++milliseconds;
    count += 1103;
    if(count >= 65536) // It is now time to call the original handler
    {
        count -= 65536;
        _chain_intr(oldTimerInterrupt);
    }
    else
    {
        // Acknowledge interrupt
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

    /* Save old interrupt vector: */
    r.h.al = 0x08;
    r.h.ah = 0x35;
    int386x(0x21, &r, &r, &s);
    oldTimerInterrupt = (intFuncPtr)MK_FP(s.es, r.x.ebx);

    /* Install new interrupt handler: */
    milliseconds = 0;
    r.h.al = 0x08;
    r.h.ah = 0x25;
    s.ds = FP_SEG(timerHandler);
    r.x.edx = FP_OFF(timerHandler);
    int386x(0x21, &r, &r, &s);

    /* Set resolution of timer chip to 1ms: */
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
    outp(0x40, (unsigned char)(1103 & 0xff));
    outp(0x40, (unsigned char)((1103 >> 8) & 0xff));
#endif
    _enable();
}

/* ***** */
void tmr_finish()
{
    union REGS r;
    struct SREGS s;
    _disable();
    segread(&s);
 
 /* Re-install original interrupt handler: */
    r.h.al = 0x08;
    r.h.ah = 0x25;
    s.ds = FP_SEG(oldTimerInterrupt);
    r.x.edx = FP_OFF(oldTimerInterrupt);
    int386x(0x21, &r, &r, &s);
 
 /* Reset timer chip resolution to 18.2...ms: */
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
unsigned long int tmr_getMs()
{
    return milliseconds;
}
