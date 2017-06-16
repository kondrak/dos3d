#ifndef TIMER_H
#define TIMER_H

/*
 *   1 ms resolution timer based on IRQ0 (interrupt 8)
 */

#ifdef __cplusplus
extern "C" {
#endif

    // initialize timer (sets proper interrupt handlers in motion)
    void tmr_start();

    // restores interrupt handler to it's original state
    void tmr_finish();

    // fetch current millisecond count
    unsigned long int tmr_getMs();

#ifdef __cplusplus
}
#endif
#endif
