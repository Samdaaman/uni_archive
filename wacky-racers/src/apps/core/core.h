#ifndef CORE_H
#define CORE_H


#include <stdbool.h>  

#ifdef __cplusplus
extern "C" {
#endif
    void core_printf(const char *format, ...);
    void core_panic(void);
    void core_initialise(void);
    int core_read_dip_value(void);
    void core_initialise_usb_serial(void);
    int limit_int(int min, int max, int value);
    void core_go_to_sleep(void);
    bool core_sleep_poll(void);
#ifdef __cplusplus
}
#endif    
#endif

