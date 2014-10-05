/*
 * main.h
 *
 *  Created on: Sep 20, 2014
 *      Author: Lab
 */

#ifndef MAIN_H_
#define MAIN_H_
#include "nrf24l01.h"
#include "spi.h"

#define SAMPLE_FREQ 5000UL

// Buffer modes
#define MODE_A 0
#define MODE_B 1

#define NUM_CHANNEL 1
#define WINDOW_SIZE 256

// Function prototypes
void SysTickHandler(void);
void ConfigureUART();
void ConfigureSysTick();
void ConfigureADC();
void IRQInterruptHandler(void);
void BufferInit(uint32_t, uint32_t);
void RFTransmit(uint8_t);

// Define pin to LED color mapping.
#define LED_0	GPIO_PIN_0
#define LED_1	GPIO_PIN_1
#define LED_2	GPIO_PIN_2
#define LED_3	GPIO_PIN_3

// Global variables
// Ping Pong buffers for ADC
//unsigned char **bufferA;
//unsigned char **bufferB;
unsigned char bufferA[NUM_CHANNEL][WINDOW_SIZE];
unsigned char bufferB[NUM_CHANNEL][WINDOW_SIZE];

// Buffers for RF RX and TX
uint32_t ui32TxBuffer[MAX_PLOAD];
uint8_t ui32RxBuffer[MAX_PLOAD];

// configuration parameters
uint8_t ui32NumOfChannels;
uint32_t ui32WindowSize;
uint8_t ui32BufferMode;

// Flag for initial configuration status
bool isConfigured;

// Flag indicating transfer window
bool transmitOn;

// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif



#endif /* MAIN_H_ */
