/*
 * main.h
 *
 *  Created on: Oct 5, 2014
 *      Author: Lab
 */

#ifndef MAIN_H_
#define MAIN_H_
#include "nrf24l01.h"
#include "spi.h"
#include "adg731.h"

// ADC sampling frequency
#define SAMPLE_FREQ 20000UL

// Buffer modes
#define MODE_A 0
#define MODE_B 1

#define NUM_CHANNEL 4
#define WINDOW_SIZE 256

// Define pin to LED color mapping.
#define LED_0	GPIO_PIN_0
#define LED_1	GPIO_PIN_1
#define LED_2	GPIO_PIN_2
#define LED_3	GPIO_PIN_3

// Function prototypes
void ConfigureUART(void);
void IRQInterruptHandler(void);
void TimerIntHandler(void);
void IRQInitialize(void);
void ConfigureTimer(void);
void BufferInit(void);

// Global variables
// Ping Pong buffers for ADC
unsigned char **bufferA;
unsigned char **bufferB;
//unsigned char bufferA[NUM_CHANNEL][WINDOW_SIZE];
//unsigned char bufferB[NUM_CHANNEL][WINDOW_SIZE];

// Buffers for RF RX and TX
uint8_t ui32TxBuffer[MAX_PLOAD];
uint8_t ui32RxBuffer[MAX_PLOAD];

// configuration parameters
uint8_t ui32NumOfChannels;
uint32_t ui32WindowSize;
uint8_t ui32BufferMode;

// Flag indicating transfer window
bool transmitOn;

// Flag indicating current packet has been transferred
bool RFPacketSent;
bool BufferAEmpty;
bool BufferBEmpty;

#endif /* MAIN_H_ */
