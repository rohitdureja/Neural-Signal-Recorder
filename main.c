#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "utils/uartstdio.h"
#include "main.h"


// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

// Initialise UART for debug information
void ConfigureUART(void)
{
    // Enable the GPIO Peripheral used by the UART.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable UART0
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins for UART mode.
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Initialize the UART for console I/O.
    UARTStdioConfig(0, 115200, 16000000);
}

// Configure pin for recieving IRQ interrupts
void IRQInitialize(void)
{
	// Set up IRQ for handling interrupts
	ROM_GPIOPinTypeGPIOInput(IRQ_BASE, IRQ);
	ROM_GPIOPadConfigSet(IRQ_BASE, IRQ, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOIntRegister(IRQ_BASE, IRQInterruptHandler);
	ROM_GPIOIntTypeSet(IRQ_BASE, IRQ, GPIO_FALLING_EDGE);
	GPIOIntClear(IRQ_BASE, GPIO_INT_PIN_7);
	GPIOIntEnable(IRQ_BASE, GPIO_INT_PIN_7); // EVK, Launchpad Board
	//GPIOIntEnable(IRQ_BASE, GPIO_INT_PIN_4); // 32 channel Boaes
}

// Configure timer to interrupt at a rate of SAMPLE_FREQ
void ConfigureTimer()
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerIntRegister(TIMER0_BASE, TIMER_A, TimerIntHandler);
	ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet()/ui32SamplingFrequency);
	ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // clear timer interrupt
	ROM_IntEnable(INT_TIMER0A);
	ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	ROM_TimerEnable(TIMER0_BASE, TIMER_A);
	UARTprintf("Timer configured\n");
}

// Configure ADC
void ConfigureADC()
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); // EVK Board
	ROM_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3); // EVK Board
	//ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); // 32 channel
	//ROM_GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5); // 32 channel
	ROM_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	ROM_ADCReferenceSet(ADC0_BASE, ADC_REF_EXT_3V);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END); // EVK Board
	//ROM_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH11 | ADC_CTL_IE | ADC_CTL_END); // 32 channel
	ROM_ADCSequenceEnable(ADC0_BASE, 3);
	ROM_ADCIntClear(ADC0_BASE, 3);
}

// Initialise buffer
void BufferInit()
{
	int i;
	bufferA = (unsigned char **)malloc(ui32NumOfChannels * sizeof(unsigned char *));
	for(i = 0 ; i < ui32NumOfChannels ; ++i)
		bufferA[i] = (unsigned char *)malloc(ui32WindowSize*sizeof(unsigned char));
	bufferB = (unsigned char **)malloc(ui32NumOfChannels * sizeof(unsigned char *));
	for(i = 0 ; i < ui32NumOfChannels ; ++i)
		bufferB[i] = (unsigned char *)malloc(ui32WindowSize*sizeof(unsigned char));
}

int main(void)
{
    int i, j;
	// Setup the system clock to run at 20MHz Mhz from PLL with internal oscillator and disable main oscillator
	//ROM_SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_MAIN_OSC_DIS);
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Disable all interrupts
    ROM_IntMasterDisable();

    // Enable and configure the GPIO port for the LED operation.
    // EVK Board
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, LED_0 | LED_1 | LED_2 | LED_3 );

    // 32-channel Boars
    //ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    //ROM_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4);

//    ROM_SysCtlPeripheralClockGating(true);
//    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
//    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOD);
//    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_ADC0);
//    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_SSI3);
//    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER0);


    // configure UART for console operation
    ConfigureUART();

    // Initialise global configuration
    ui32NumOfChannels = 4;
    ui32WindowSize = 256;
    ui32SamplingFrequency = 20000;
    ui32BufferMode = MODE_A;
    transmitOn = false;
    BufferAEmpty = true;
    BufferBEmpty = true;
    isConfigured = false;
    BufferInit();

    // Delay for a bit
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3);

    // Initialize RF module port for RX to recieve configuration parameters
    RFInit(0);
    IRQInitialize();

    // Delay for a bit
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3);

    ROM_IntMasterEnable();

    while(isConfigured == false)
    {

    }

    // Disable all interrupts
    ROM_IntMasterDisable();

    // Now that configuration is received, change to TX
    RFInit(1);
    IRQInitialize();


    // Delay for a bit
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3);

    // Initialise MUX
    muxInit();

    // Configure ADC
    ConfigureADC();

    // Configure TimerA to generate interrupts
    ConfigureTimer();

    // --------------- TX operation  ------------- //
    // Generate packet to send
    for(i = 1 ; i <= 32 ; ++i)
    	ui8TxBuffer[i-1] = i;
    // --------------- TX operation  ------------- //


    // Set priorities for interrupts
    ROM_IntPrioritySet(INT_GPIOB, 0x00); // high priority
    ROM_IntPrioritySet(INT_TIMER0A, 0x10); // low priority

    // Enable all interrupts
    ROM_IntMasterEnable();

    ROM_GPIOPinWrite(GPIO_PORTB_BASE, LED_0, LED_0);
    ROM_GPIOPinWrite(GPIO_PORTB_BASE, LED_1, LED_1);


    // Loop Forever
    while(1)
    {
    	if(transmitOn == true)
    	{
    		if(ui32BufferMode == MODE_A) // Transfer contents from B
    	    {
    			//ROM_GPIOPinWrite(GPIO_PORTB_BASE, LED_0, 0);
    	   		for(i = 0 ; i < ui32NumOfChannels ; i++)
    	    	{

    	    		for(j = 0 ; j < ui32WindowSize ; j = j + 32)
    	    		{
    	    			RFPacketSent = false;
    	    			bufferB[i][j] = channelIndex[i];
    	    			RFWriteSendBuffer(bufferB[i]+j, 32);
    	    			while(RFPacketSent != true)
    	    			{
    	    				//ROM_SysCtlSleep();
    	    			}
    	    		}

    	    	}
    	   		//ROM_GPIOPinWrite(GPIO_PORTB_BASE, LED_0, LED_0);
    	    	BufferBEmpty = true;
    	    	transmitOn = false;
    	    }
    	    else if(ui32BufferMode == MODE_B) // Transfer contents from  A
    	    {
    	    	//ROM_GPIOPinWrite(GPIO_PORTB_BASE, LED_1, 0);
    	    	for(i = 0 ; i < ui32NumOfChannels ; i++)
    	    	{
    	    		for(j = 0 ; j < ui32WindowSize ; j = j + 32)
    	    		{

    	    			RFPacketSent = false;
    	    			bufferA[i][j] = channelIndex[i];
    	    			RFWriteSendBuffer(bufferA[i]+j, 32);
    	    			while(RFPacketSent != true)
    	    			{
    	    				//ROM_SysCtlSleep();
    	    			}
    	    		}
    	    	}
    	   		//ROM_GPIOPinWrite(GPIO_PORTB_BASE, LED_1, LED_1);
    	    	BufferAEmpty = true;
    	    	transmitOn = false;
    	    }
    	}
    	else
    	{
    		//ROM_SysCtlSleep();
    	}
//    	// --------------- TX operation  ------------- //
//    	// Send packet every one second
//    	RFWriteSendBuffer(ui32TxBuffer, 32);
//       ROM_SysCtlDelay(SysCtlClockGet()/3);
//        // --------------- TX operation  ------------- //
    }

}

// Interrupt handler for ADC operation
void TimerIntHandler()
{
	ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // clear timer interrupt
	static uint8_t mode = 0;
	uint32_t ADCValue;
	uint8_t i;
	static uint32_t count = 0;
	if(BufferAEmpty || BufferBEmpty)
	{
		for(i = 0 ; i < ui32NumOfChannels ; ++i)
		{
			muxChannelChange(channelIndex[i]); // Change channel
			ROM_ADCProcessorTrigger(ADC0_BASE, 3);
			while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
			{
			}
			ROM_ADCIntClear(ADC0_BASE, 3);
			ROM_ADCSequenceDataGet(ADC0_BASE, 3, &ADCValue);
			ADCValue = ADCValue>>4; // Make 12-bit ADC value to 8-bit
			if(ADCValue <= 0xFF - ui32NumOfChannels)
				ADCValue += ui32NumOfChannels;

			if(ui32BufferMode == MODE_A)
			{
				bufferA[i][count] = ADCValue <= 250 ? ADCValue + ui32NumOfChannels : ADCValue; // Make 12-bit ADC value to 8-bit and store in buffer
			}
			else if (ui32BufferMode == MODE_B)
			{
				bufferB[i][count] = ADCValue <= 250 ? ADCValue + ui32NumOfChannels : ADCValue;
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
				BufferAEmpty = false;
				transmitOn = true; // start transmission
			}
			else if(ui32BufferMode == MODE_B)
			{
				ui32BufferMode = MODE_A;
				BufferBEmpty = false;
				transmitOn = true; //  start transmission
			}
		}
			if(mode == 0)
			{

				//ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4); // 32 channel
				ROM_GPIOPinWrite(GPIO_PORTB_BASE, LED_3, 0); // EVK
				mode = 1;
			}
			else if(mode == 1)
			{

				//ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0); // 32 channel
				ROM_GPIOPinWrite(GPIO_PORTB_BASE, LED_3, LED_3); // EVK
				mode = 0;
			}
	}
}

// Interrupt handler called RF max retries reached or transmission complete
void IRQInterruptHandler(void)
{
	uint32_t ui32Bytes, i;
	GPIOIntClear(IRQ_BASE, GPIO_INT_PIN_7); // EVK, Launchpad: clear interrupt flag
	//GPIOIntClear(IRQ_BASE, GPIO_INT_PIN_4); // 32-channel: clear interrupt flag
	SPISetCELow(); // set CE low to cease all operation

	if(RFReadRegister(READ_REG + STATUSREG) & 0x20) // successful transmission
	{
		RFWriteRegister(WRITE_REG + STATUSREG, 0x20); // Clear TX_DS flag
		RFPacketSent = true;
	}
	else if(RFReadRegister(READ_REG + STATUSREG) & 0x10) // max retries
	{
		//Flush TX buffer
		SPISetCSNLow();
		SPIDataWrite(FLUSH_TX);
		SPIDataRead();
		SPISetCSNHigh();
		RFWriteRegister(WRITE_REG + STATUSREG, 0x10); // Clear MAX_RT flag
		RFPacketSent = true;

	}
	else if(RFReadRegister(READ_REG + STATUSREG) & 0x40)// configuration parameters recieved
	{
		ui32Bytes = RFReadRecieveBuffer(ui8RxBuffer);
		UARTprintf("Bytes received : %d \n", ui32Bytes);
		if(ui8RxBuffer[0] == 1) // channels
		{
			ui32NumOfChannels = ((ui8RxBuffer[3] << 8) | ui8RxBuffer[2]);
			UARTprintf("Number of channels: %d\n", ui32NumOfChannels);
			channelIndex = (uint8_t *)malloc(ui32NumOfChannels*sizeof(uint8_t));
			for(i = 0 ; i < ui32NumOfChannels ; ++i)
			{
				channelIndex[i] = i;//ui8RxBuffer[4+i];
				UARTprintf("%d\n", channelIndex[i]);
			}
		}
		else if(ui8RxBuffer[0] == 2) // buffersize
		{
			ui32WindowSize = (ui8RxBuffer[3] << 8) | ui8RxBuffer[2];
			UARTprintf("Buffer size: %d\n", ui32WindowSize);
		}
		else if(ui8RxBuffer[0] == 3) // sampling frequency
		{
			ui32SamplingFrequency = (ui8RxBuffer[3] << 8) | ui8RxBuffer[2];
			UARTprintf("Sampling Frequency: %d\n", ui32SamplingFrequency);
		}
		else if(ui8RxBuffer[0] == 10) // configuration complete
		{
			UARTprintf("configureation complete\n");
			isConfigured = true;
		}
		RFWriteRegister(WRITE_REG + STATUSREG, 0x40); // Clear RX_DR flag
	}
	SPISetCEHigh(); // set CE high again to start all operation
}
