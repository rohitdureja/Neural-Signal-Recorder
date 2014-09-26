/*
 * main.h
 *
 *  Created on: Sep 20, 2014
 *      Author: Lab
 */

#ifndef MAIN_H_
#define MAIN_H_

#define SAMPLE_FREQ 20000UL

// Function prototypes
void SysTickHandler(void);
void ConfigureUART();
void ConfigureSysTick();
void ConfigureADC();
void ConfigureTimer();

// Define pin to LED color mapping.
#define PIN_0 GPIO_PIN_0
#define LED_RED GPIO_PIN_1


// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif



#endif /* MAIN_H_ */
