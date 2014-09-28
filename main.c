#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
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
#include "adg731.h"
#include "main.h"

#define SIZE 20

// SysTick Interrupt Handler
void SysTickIntHandler(void)
{
	uint8_t ADCValue[1];
	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValue);
	UARTprintf("%d\n", ADCValue[0]);
}

// UART configuration for uartstdio library
void ConfigureUART(void)
{
    // Enable the GPIO Peripheral used by the UART.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable UART0
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins for UART mode.
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Initialize the UART for console I/O.
    UARTStdioConfig(0, 115200, 16000000);
}

// Configure SysTick
void ConfigureSysTick(void)
{
    // Enable SysTick to interrupt at a rate of SAMPLE_FREQ
	ROM_SysTickPeriodSet(ROM_SysCtlClockGet()/SAMPLE_FREQ);
	ROM_SysTickIntEnable();
	ROM_SysTickEnable();
}

// Configure ADC
void ConfigureADC(void)
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	ROM_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3); // EVK Board
	ROM_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
	ROM_ADCSequenceEnable(ADC0_BASE, 3);
	ROM_ADCIntClear(ADC0_BASE, 3);
}

// Configure ping pong buffers for ADC operations
void BufferInit(uint32_t channels, uint32_t window)
{
	uint32_t i;
	bufferA = (unsigned char **)malloc(channels*sizeof(unsigned char*));
	 for(i=0;i<channels;i++)
		 bufferA[i] = (unsigned char *)malloc(window*sizeof(unsigned char));

	 bufferB = (unsigned char **)malloc(channels*sizeof(unsigned char*));
	 	 for(i=0;i<channels;i++)
	    	bufferB[i] = (unsigned char *)malloc(window*sizeof(unsigned char));
}

// Interrupt handler for RF TX failures
void IRQInterruptHandler(void)
{
	GPIOIntClear(IRQ_BASE, GPIO_INT_PIN_7); // EVK, Launchpad: clear interrupt flag
	//GPIOIntClear(IRQ_BASE, GPIO_INT_PIN_4); // 32-channel: clear interrupt flag

	SPISetCELow(); // set CE low to cease all operation

	// --------------- TX operation  ------------- //
	//Flush TX buffer
	SPISetCSNLow();
	SPIDataWrite(FLUSH_TX);
	SPIDataRead();
	SPISetCSNHigh();

	RFWriteRegister(WRITE_REG + STATUSREG, 0x10); // Clear MAX_RT flag

	// Do something
	// EVK Board
	GPIOPinWrite(GPIO_PORTB_BASE, LED_0, 0);
	SysCtlDelay(SysCtlClockGet()/12);
	GPIOPinWrite(GPIO_PORTB_BASE, LED_0, LED_0);
	SysCtlDelay(SysCtlClockGet()/12);
	UARTprintf("fail\n");

	// 32-channel board
	/*GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);
	SysCtlDelay(SysCtlClockGet()/12);
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
	SysCtlDelay(SysCtlClockGet()/12);*/

	// --------------- TX operation  ------------- //

	SPISetCEHigh(); // set CE high again to start all operation
}

// Function to transfer passed buffer over RF
void RFTransmit(uint8_t buffer)
{

}

// Main application code
int main(void)
{
	int i;
	// Setup the system clock to run at 80 Mhz from PLL with internal oscillator and disable main oscillator
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_MAIN_OSC_DIS);

	// Enable and configure the GPIO port for the LED operation.
	// EVK Board
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, LED_0 | LED_1 | LED_2 | LED_3 );

    // Enable UART operation
    ConfigureUART();

    // Initialise the MUX channel. At initialization all channels are off.
    muxInit();

    // Initialise the RF channel for TX
    RFInit(1);

    // Set up IRQ for handling interrupts
    ROM_GPIOPinTypeGPIOInput(IRQ_BASE, IRQ);
    ROM_GPIOPadConfigSet(IRQ_BASE, IRQ, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntRegister(IRQ_BASE, IRQInterruptHandler);
    ROM_GPIOIntTypeSet(IRQ_BASE, IRQ, GPIO_FALLING_EDGE);
    GPIOIntEnable(IRQ_BASE, GPIO_INT_PIN_7); // EVK, Launchpad Board
    //GPIOIntEnable(IRQ_BASE, GPIO_INT_PIN_4); // 32 channel Board

    ui32NumOfChannels = 1;
    ui32WindowSize = 256;
    BufferInit(ui32NumOfChannels, ui32WindowSize);

    // Enable SysTick operation
    ConfigureSysTick();

    // Enable ADC operation
    ConfigureADC();

//    // --------------- TX operation  ------------- //
//    // Generate packet to send
//    for(i = 1 ; i <= 32 ; ++i)
//    	ui32TxBuffer[i-1] = i;
//    // --------------- TX operation  ------------- //

    // Loop Forever
    while(1)
    {
//    	// --------------- TX operation  ------------- //
//        // Send packet every one second
//        RFWriteSendBuffer(ui32TxBuffer, 32);
//        ROM_SysCtlDelay(SysCtlClockGet()/3);
//        // --------------- TX operation  ------------- //
    }
//    // Wait till initial configuration data is received
//    while(isConfigured==false)
//    {
//
//    }
//
//
////    UARTgetc();

////
////
////    UARTprintf("Number of ticks: %d\n", bufferA[0][0]);//ROM_SysCtlClockGet()/SAMPLE_FREQ);
////    UARTprintf("Number of ticks: %d\n", bufferB[0][1]);
////
//    // Enable SysTick operation
//    ConfigureSysTick();
//
//    // Enable ADC operation
//    ConfigureADC();
////    // Loop Forever
//    while(1)
//    {
//    	//ROM_SysCtlSleep();
//    	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
//    	SysCtlDelay(SysCtlClockGet()/3);
//    	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);
//    	SysCtlDelay(SysCtlClockGet()/3);
//    }
}


