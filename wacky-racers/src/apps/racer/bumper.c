#include "bumper.h"
#include "pio.h"
#include "button.h"
#include "delay.h"
#include "motors.h"

#define BUMMPER_POLL_RATE 100
button_t bumper;

//Bumper release var
bool bumperReleased = true;

/*Set-up Bumper*/
static const button_cfg_t bumper_cfg =
{
    .pio = BUMPER_PIO
};



void bumper_initialise(void){
    /* Initialise button (bumper)*/
    
    bumper = button_init (&bumper_cfg);
    button_poll_count_set (BUTTON_POLL_COUNT (BUMMPER_POLL_RATE));
}


void bumper_poll(void){
    button_poll (bumper);
        if ((button_down_p (bumper)) && (bumperReleased)){
            pio_output_set(LED1_PIO, 0);
            pio_output_set(LED2_PIO, 0);
            
            //Stop
            motors_stop();
            delay_ms(5000);
            bumperReleased = false;
            motors_start();
            
        }else if(button_up_p (bumper) && (!bumperReleased)) {
                pio_output_set(LED1_PIO, 1);
                pio_output_set(LED2_PIO, 1);
                bumperReleased = true;
        }
}
        