#ifndef LOW_VOLTAGE_H
#define LOW_VOLTAGE_H

#include <stdbool.h>  

#ifdef __cplusplus
extern "C" {
#endif
    void adc_voltage_setup(void);
    bool is_voltage_low(void);
#ifdef __cplusplus
}
#endif
#endif

