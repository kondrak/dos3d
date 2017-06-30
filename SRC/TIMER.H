#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/*
 *   IRQ0 (interrupt 8) timer with 1ms granularity
 */

#ifdef __cplusplus
extern "C" {
#endif

    // initialize timer (sets proper interrupt handlers in motion)
    void tmr_start();

    // restores interrupt handler to it's original state
    void tmr_finish();

    // fetch current millisecond count
    uint32_t tmr_getMs();

#ifdef __cplusplus
}
#endif
#endif
