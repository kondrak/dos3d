#include "src/timer.h"
#include <conio.h>
#include <dos.h>

typedef void (__interrupt* INTFUNCPTR)();

INTFUNCPTR oldTimerInterrupt; // Original interrupt handler

volatile unsigned long int milliseconds = 0; // Elapsed time in milliseconds

unsigned long int tmr_getMs()
{
    return milliseconds;
}

void __interrupt timerHandler()
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
        outp(0x20, 0x20); // Acknowledge interrupt
    }
}

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
    oldTimerInterrupt = (INTFUNCPTR)MK_FP(s.es, r.x.ebx);

    /* Install new interrupt handler: */
    milliseconds = 0;
    r.h.al = 0x08;
    r.h.ah = 0x25;
    s.ds = FP_SEG(timerHandler);
    r.x.edx = FP_OFF(timerHandler);
    int386x(0x21, &r, &r, &s);

    /* Set resolution of timer chip to 1ms: */
    outp(0x43, 0x36);
    outp(0x40, (unsigned char)(1103 & 0xff));
    outp(0x40, (unsigned char)((1103 >> 8) & 0xff));
    _enable();
}

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
    outp(0x43, 0x36);
    outp(0x40, 0x00);
    outp(0x40, 0x00);
    _enable();
}
