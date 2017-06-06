#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

    // 1ms resolution timer based on IRQ0 (interrupt 8)
    void tmr_start();
    void tmr_finish();
    unsigned long int tmr_getMs();

#ifdef __cplusplus
}
#endif

#endif
