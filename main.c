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


// SysTick Interrupt Handler
void SysTickIntHandler(void)
{
	static uint8_t mode = 0;
	uint32_t ADCValue;
	uint8_t i;
	static uint32_t count = 0;
	for(i = 0 ; i < ui32NumOfChannels ; i++)
	{
		muxChannelChange(0); // Change channel
		ROM_ADCProcessorTrigger(ADC0_BASE, 3);
		while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
		{
		}
		ROM_ADCIntClear(ADC0_BASE, 3);
		ROM_ADCSequenceDataGet(ADC0_BASE, 3, &ADCValue);
		if(ui32BufferMode == MODE_A)
		{
			bufferA[i][count] = count;//(ADCValue>>4); // Make 12-bit ADC value to 8-bit and store in buffer
		}
		else if (ui32BufferMode == MODE_B)
		{
			bufferB[i][count] = count;//(ADCValue>>4);
		}
	}
	count = count + 1;
	if(count == ui32WindowSize)
	{
		count = 0;
		// Switch channels
		if(ui32BufferMode == MODE_A)
		{
			ui32BufferMode = MODE_B;
			transmitOn = true; // start transmission
		}
		else if(ui32BufferMode == MODE_B)
		{
			ui32BufferMode = MODE_A;
			transmitOn = true; //  start transmission
		}
	}
	if(mode == 0)
	{
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);
		mode = 1;
	}
	else
	{
		GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0);
		mode = 0;
	}
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
	ROM_ADCReferenceSet(ADC0_BASE, ADC_REF_EXT_3V);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
	ROM_ADCSequenceEnable(ADC0_BASE, 3);
	ROM_ADCIntClear(ADC0_BASE, 3);
}

//// Configure ping pong buffers for ADC operations
//void BufferInit(uint32_t channels, uint32_t window)
//{
//	uint8_t i;
//	bufferA = (unsigned char **)malloc(channels*sizeof(unsigned char*));
//	for(i=0;i<channels;i++)
//	{
//		bufferA[i] = (unsigned char *)malloc(window*sizeof(unsigned char));
//	}
//	bufferB = (unsigned char **)malloc(channels*sizeof(unsigned char*));
//	for(i=0;i<channels;i++)
//	{
//		bufferB[i] = (unsigned char *)malloc(window*sizeof(unsigned char));
//	}
//	// Set buffer A as defualt buffer
//	ui32BufferMode = MODE_A;
//}

// Interrupt handler for RF TX failures
void IRQInterruptHandler(void)
{
	SPISetCELow(); // set CE low to cease all operation

	if(RFReadRegister(READ_REG + STATUSREG) & 0x20)
	{
		RFPacketSent = true;
		GPIOIntClear(IRQ_BASE, GPIO_INT_PIN_7);
		RFWriteRegister(WRITE_REG + STATUSREG, 0x20); // Clear TX_DS flag
	}
	else
	{
		GPIOIntClear(IRQ_BASE, GPIO_INT_PIN_7); // EVK, Launchpad: clear interrupt flag
		//GPIOIntClear(IRQ_BASE, GPIO_INT_PIN_4); // 32-channel: clear interrupt flag

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
	}
	SPISetCEHigh(); // set CE high again to start all operation
}

// Function to transfer passed buffer over RF
void RFTransmit(uint8_t buffer)
{

}

// Main application code
int main(void)
{
	int i, j;
	// Setup the system clock to run at 12 Mhz from PLL with internal oscillator and disable main oscillator
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_16 | SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_MAIN_OSC_DIS);

	// Enable and configure the GPIO port for the LED operation.
	// EVK Board
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, LED_0 | LED_1 | LED_2 | LED_3 );

    // Initial configuration parameters
    ui32NumOfChannels = 1;  // number of channels
    ui32WindowSize = 256;	// window length
    //BufferInit(ui32NumOfChannels, ui32WindowSize);	// initialise buffer
    transmitOn = false;

    // Disable all interrupts
    ROM_IntMasterDisable();

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
    GPIOIntEnable(IRQ_BASE, GPIO_INT_PIN_7); // EVK Board Launchpad Board
    //GPIOIntEnable(IRQ_BASE, GPIO_INT_PIN_4); // 32 channel Board

    // Enable SysTick operation
    ConfigureSysTick();

    // Enable ADC operation
    ConfigureADC();

    // Enable all interrupts
    ROM_IntMasterEnable();


    // --------------- TX operation  ------------- //
    // Generate packet to send
//    for(i = 1 ; i <= 32 ; ++i)
//    	ui32TxBuffer[i-1] = i;
    // --------------- TX operation  ------------- //

    // Loop Forever
    while(1)
    {
   	//UARTprintf("here3\n");
    	if(transmitOn == true)
    	{
    		if(ui32BufferMode == MODE_A) // Transfer contents from B
    		{
    			for(i = 0 ; i < ui32NumOfChannels ; i++)
    			{
    				ROM_GPIOPinWrite(GPIO_PORTB_BASE, LED_3, LED_3);
    				for(j = 0 ; j < ui32WindowSize ; j = j + 32)
    				{
    					RFPacketSent = false;
    					RFWriteSendBuffer((uint8_t *)(bufferB[i]+j), 32);
    					while(RFPacketSent != true)
    					{
    						ROM_SysCtlSleep();
    					}
    				}
    				ROM_GPIOPinWrite(GPIO_PORTB_BASE, LED_3, 0);
    			}
    			transmitOn = false;
    		}
    		else if(ui32BufferMode == MODE_B) // Transfer contents from  A
    		{
    			for(i = 0 ; i < ui32NumOfChannels ; i++)
    			{
    				for(j = 0 ; j < ui32WindowSize ; j = j + 32)
    				{
    					RFPacketSent = false;
    					RFWriteSendBuffer((uint8_t *)(bufferA[i]+j), 32);
    					while(RFPacketSent != true)
    					{
    						ROM_SysCtlSleep();
    					}
    				}
    			}
    			transmitOn = false;
    		}
    	}
    	else
    	{
    		ROM_SysCtlSleep();
    	}
    	// --------------- TX operation  ------------- //
        // Send packet every one second
//        RFWriteSendBuffer(ui32TxBuffer, 32);
//        ROM_SysCtlDelay(SysCtlClockGet()/3);

        // --------------- TX operation  ------------- //
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


