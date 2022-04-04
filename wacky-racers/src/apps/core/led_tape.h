#ifndef LED_TAPE_H
#define LED_TAPE_H

#ifdef __cplusplus
extern "C" {
#endif
    void led_tape_initialise(void);
    void led_tape_poll(void);
    void led_tape_stop(void);
#ifdef __cplusplus
}
#endif
#endif

