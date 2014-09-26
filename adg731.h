/*
 * adg731.h
 *
 *  Created on: Sep 25, 2014
 *      Author: Rohit Dureja
 */

#ifndef ADG731_H_
#define ADG731_H_

/* The ADG731 control pins are connected to SPI1 on
 * DIN - PF1
 * SCLK - PF2
 * SYNC - PF3
 */
#define DIN_PIN GPIO_PIN_1
#define SCLK_PIN GPIO_PIN_2
#define SYNC_PIN GPIO_PIN_3
#define MUX_BASE GPIO_PORTF_BASE

// These are the default channel configuration parameters
#define BASE_CHANNEL 0x0
#define ALL_OFF 0x80
#define NO_CHANGE 0x40

// Function prototypes
// Initialises the MUX and the SPI peripheral to which it is connected
void muxInit(void);

// Changes the current multiplexer channel
void muxChannelChange(unsigned char);

#endif /* ADG731_H_ */
