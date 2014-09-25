/*
 * main.h
 *
 *  Created on: Sep 20, 2014
 *      Author: Lab
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "utils/uartstdio.h"

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
