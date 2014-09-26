/*
 * adg731.c
 *
 *  Created on: Sep 25, 2014
 *      Author: Rohit Dureja
 */
#include "adg731.h"
#include "spi.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"

void muxInit(void)
{
	// enable SSI3 and GPIOD peripherals
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Configure GPIO pins for special functions
	GPIOPinConfigure(GPIO_PF2_SSI1CLK);
	GPIOPinConfigure(GPIO_PF1_SSI1TX);
	GPIOPinConfigure(GPIO_PF3_SSI1FSS);
	ROM_GPIOPinTypeSSI(MUX_BASE, DIN_PIN | SCLK_PIN | SYNC_PIN);

	//Configure and enable SSI port
	// Use internal 16Mhz RC oscillator as SSI clock source
	ROM_SSIClockSourceSet(SSI1_BASE, SSI_CLOCK_PIOSC);
	ROM_SSIConfigSetExpClk(SSI1_BASE, 16000000, SSI_FRF_MOTO_MODE_0,
			SSI_MODE_MASTER, 8000000, 8);
	ROM_SSIEnable(SSI1_BASE);

	// Load default configuration parameters
	ROM_SSIDataPut(SSI1_BASE, ALL_OFF);
}

void muxChannelChange(unsigned char channelNum)
{
	ROM_SSIDataPut(SSI1_BASE, BASE_CHANNEL + channelNum);
}

