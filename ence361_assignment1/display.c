/**********************************************************
 *
 * display.c
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

#include "display.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../OrbitOLED/OrbitOLEDInterface.h"
#include "utils/ustdlib.h"

/*******************************************************
 * Globals to module
 *******************************************************/
static uint8_t dispUnits = 0;
static uint8_t dispMode = 0;
static char units_arr[2][6] = {"KM", "MILES"};
static int32_t intPart = 0;
static int32_t decPart = 0;
static int32_t distance = 0;

/**********************************************************
 * Local (Private) Prototypes
 *********************************************************/
static void displayUpdateLineInt(char *str, int32_t num, uint8_t charLine);
static void displayUpdateLineFloat(char *str, int32_t num1, int32_t num2, uint8_t charLine);
static void displayFormatSteps(int32_t stepCount, uint8_t dispUnits);

/**********************************************************
 * Local (Private) Declarations
 *********************************************************/

/**********************************************************
 * Function to display a changing integer on a line of the
 * display. The display has 4 rows of 16 characters, with
 * 0, 0 at top left.
 **********************************************************/
static void displayUpdateLineInt(char *str, int32_t num, uint8_t charLine)
{
    char text_buffer[17]; //Display fits 16 characters wide.

    // "Undraw" the previous contents of the line to be updated.
    OLEDStringDraw ("                ", 0, charLine);
    // Form a new string for the line.  The maximum width specified for the
    // number field ensures it is displayed right justified.
    usnprintf(text_buffer, sizeof(text_buffer), "%10d %s", num, str);
    // Update line on display.
    OLEDStringDraw(text_buffer, 0, charLine);
}

/**********************************************************
 * Function to display a changing float described as two
 * integers on the display. The display has 4 rows of 16
 * characters, with 0, 0 at top left.
 **********************************************************/
static void displayUpdateLineFloat(char *str, int32_t num1, int32_t num2, uint8_t charLine)
{
    char text_buffer[17]; //Display fits 16 characters wide.

    // "Undraw" the previous contents of the line to be updated.
    OLEDStringDraw ("                ", 0, charLine);
    // Form a new string for the line.  The maximum width specified for the
    // number field ensures it is displayed right justified.
    usnprintf(text_buffer, sizeof(text_buffer), "%5d.%04d %s", num1, num2, str); //first num is after decimal, second num is before decimal.
    // Update line on display.
    OLEDStringDraw(text_buffer, 0, charLine);
}

/**********************************************************
 * Calculates the distance with regard to the current units
 * being displayed. Then adjusts the static global distance
 * accordingly.
 **********************************************************/
static void displayFormatSteps(int32_t stepCount, uint8_t dispUnits)
{
    if (dispUnits == KILOMETRES)
    {
        distance = stepCount*9;
    }
    else //dispMode == MILES
    {
        distance = (2*(stepCount*9*0.621371) + 1)/2; //
    }
}

/**********************************************************
 * Public Declarations (defined in .h file)
 *********************************************************/

/**********************************************************
 * Initialises the OLED Display on the Orbit Booster Pack.
 **********************************************************/
void initDisplay(void)
{
    // Initialise the Orbit OLED display
    OLEDInitialise();
}

/**********************************************************
 * Toggles the current units being used for display of the
 * total distance. Only toggles the units when in total
 * distance display mode. Returns true if the units were
 * toggled. False if not
 **********************************************************/
bool displayUnitsToggle(void)
{
    if (dispMode == TOTAL_DISTANCE) //Only toggle units whilst in total distance display.
    {
        if (dispUnits == KILOMETRES)
        {
            dispUnits = MILES;
        }
        else //dispUnits == MILES
        {
            dispUnits = KILOMETRES;
        }
        return true;
    }
    return false;
}

/**********************************************************
 * Toggles the current display mode.
 **********************************************************/
void displayModeToggle(void)
{
    if (dispMode == NO_OF_STEPS)
    {
        dispMode = TOTAL_DISTANCE;
    }
    else //dispMode == TOTAL_DISTANCE
    {
        dispMode = NO_OF_STEPS;
    }
}

/**********************************************************
 * Function to display the different formats of data.
 * Format of display depends upon dispMode and dispUnits.
 **********************************************************/
void displayUpdate(int32_t stepCount)
{
    if (dispMode == NO_OF_STEPS)
    {
        OLEDStringDraw ("NUMBER OF STEPS ", 0, 0);
        displayUpdateLineInt("STEPS", stepCount, 2);
    }
    else //dispMode == TOTAL_DISTANCE
    {
        displayFormatSteps(stepCount, dispUnits);

        intPart = distance/10000; //Grab the numbers to be placed after the decimal place <--.0000
        decPart = distance%10000; //Grab the numbers to be placed before the decimal place 0.-->

        OLEDStringDraw ("TOTAL DISTANCE  ", 0, 0);
        displayUpdateLineFloat(units_arr[dispUnits], intPart, decPart, 2);
    }
}
