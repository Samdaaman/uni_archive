/*
 * OrbitOLEDInterface.h
 *
 *  Created on: 23/11/2016
 *      Author: mdp46
 *	Modified on: 20/03/2017
 */

#ifndef ORBITOLEDINTERFACE_H_
#define ORBITOLEDINTERFACE_H_


/*
 * OLEDStringDraw
 * 		return:		void
 * 		input:		*pcStr	zero terminated character string
 * 					ulColumn	Character column in x axis
 * 					ulRow		Character row in y axis
 *
 * 		purpose:	Prints string in character row and column specified
 *
 * 		Note: 8x8 pixel character rows and columns are used.
 * 			  Row and column 0,0 is at the top left of the display
 *
 */
void OLEDStringDraw(const char *pcStr, uint32_t ulColumn, uint32_t ulRow);

/*
 * OLEDInitialise
 *   	return: 	void
 *   	input: 		void
 *
 *   	purpose:	Runs the initialise routines for the OLED display
 */
void OLEDInitialise (void);


#endif /* ORBITOLEDINTERFACE_H_ */
