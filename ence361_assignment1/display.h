#ifndef DISPLAY_H_
#define DISPLAY_H_

/**********************************************************
 *
 * display.h
 *
 * A Module which read stepCount data and displays the
 * resulting data on the Orbit OLED display. This data is
 * formatted as per global variables which can be accessed
 * and changed through module functions.
 *
 * Mikael Ewans
 * 21 May 2020
 *
 **********************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/*******************************************************
 * Constants
 *******************************************************/
enum units {KILOMETRES = 0, MILES};
enum dMode {NO_OF_STEPS = 0, TOTAL_DISTANCE};

/**********************************************************
 * Initialises the OLED Display on the Orbit Booster Pack.
 **********************************************************/
void initDisplay(void);

/**********************************************************
 * Toggles the current units being used for display of the
 * total distance. Only toggles the units when in total
 * distance display mode. Returns true if the units were
 * toggled. False if not
 **********************************************************/
bool displayUnitsToggle(void);

/**********************************************************
 * Toggles the current display mode.
 **********************************************************/
void displayModeToggle(void);

/**********************************************************
 * Function to display the different formats of data.
 * Format of display depends upon dispMode and dispUnits.
 **********************************************************/
void displayUpdate(int32_t stepCount);

#endif /* DISPLAY_H_ */
