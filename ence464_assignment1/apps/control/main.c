#include "control.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DELTA_T 10
#define SPEED 1

int main()
{
    srand(time(NULL));

    int8_t position = 0;
    int8_t command = 0;
    int8_t desired = 75;

    for (int i = 0; i < 1000; i++)
    {
        double noise = rand() % 100 / 1000.0 + 0.95;
        // printf("Noise is %.2lf\n", noise);

        position += SPEED * DELTA_T / 1000.0 * command;

        position = position * noise;
        position = position < 0 ? 0 : (position > 100 ? 100 : position);

        command = control_height_poll(position, desired);
        printf("%i,%i\n", command, position);
    }

}