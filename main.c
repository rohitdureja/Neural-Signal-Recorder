#include "main.h"
#define SIZE 20

// SysTick Interrupt Handler
void SysTickIntHandler(void)
{
	static uint8_t mode = 0;
	static uint32_t ADCValueA[1];
	static uint32_t ADCValueB[1];
	static uint32_t ADCValueC[1];
	static uint32_t ADCValueD[1], i , j, k, temp;
	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}

	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueA);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}

	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueB);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}

	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueC);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}

	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	ROM_ADCProcessorTrigger(ADC0_BASE, 3);
	while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
	{
	}
	ROM_ADCIntClear(ADC0_BASE, 3);
	ROM_ADCSequenceDataGet(ADC0_BASE, 3, ADCValueD);

	for(i = 0 ; i < SIZE ; i++)
	{
		for(j = 0 ; j < SIZE ; j++)
		{
			k = i + j;
			temp = k + ADCValueA[0] + ADCValueB[0] + ADCValueC[0] + ADCValueD[0];
			ADCValueA[0] = k;
			ADCValueB[0] = ADCValueA[0];

		}
	}
	if(mode==0)
	{
		ROM_GPIOPinWrite(GPIO_PORTB_BASE, PIN_0, PIN_0);
		mode = 1;
	}
	else
	{
		ROM_GPIOPinWrite(GPIO_PORTB_BASE, PIN_0, 0);
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
	ROM_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_7);
	ROM_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
	ROM_ADCSequenceEnable(ADC0_BASE, 3);
	ROM_ADCIntClear(ADC0_BASE, 3);
}

void ConfigureTimer()
{

}

int main(void)
{
    // Setup the system clock to run at 12.5 Mhz from PLL with internal oscillator and disable main oscillator
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_MAIN_OSC_DIS);

   // Enable and configure the GPIO port for pin operation
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, PIN_0 );

    // Enable and configure the GPIO port for the LED operation.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED );

    // Enable UART operation
    ConfigureUART();

    UARTprintf("Number of ticks: %d\n", ROM_SysCtlClockGet()/SAMPLE_FREQ);

    // Enable SysTick operation
    ConfigureSysTick();

    // Enable ADC operation
    ConfigureADC();
    // Loop Forever
    while(1)
    {
    	ROM_SysCtlSleep();
    }
}
