// MSP430 I2C Library
// 07/05/2012 by Mushfiq Sarker <mushfiqsarker@inventige.com>

// Easy to use functions for communication with any I2C enabled device.
// This is usable only with MSP430F series of microcontrollers.

// Changelog:
// ... - ongoing debug release

// NOTE: THIS IS ONLY A PARIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY UNDERGOING ACTIVE
// DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES. PLEASE KEEP THIS IN MIND IF
// YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.

/* ============================================
MSP430 I2C Library code is placed under the MIT license
Copyright (c) 2012 Mushfiq Sarker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/



#ifndef I2C_MSP430F_H_
#define I2C_MSP430F_H_

#include "msp430g2553.h"
//#include "msp430x22x4.h"
#include <stdint.h>

/********* Internal Variables ***********/
static int	numRXbytesSent = 0,							//<! Stores the number of bytes received already.
		NUM_BYTES_TX = 0,							//<! Stores the total number of bytes to be sent to slave.
		NUM_BYTES_RX = 0;							//<! Stores the total number of bytes to be received from slave.

static char bufferedDataOutput[5],						//<! Stores the bytes that need to be sent to the slave. Buffers all the bytes and sent at once.
		bufferedDataRX[14];						//<! Stores the bytes that need to be received from the slave.

static int 	Bytecount;
static char SLV_Addr = 0;								//<! Stores the slave address. Do not set slave address here. Set slaved address with I2CBeginTransmission().

static int 	RPT_Flag = 0,               				//<! Stores the repeated start enable. If 1 it is enabled.
		Bytecount;

static char throwAway = 0;								//<! Stores a temporary byte. Needed for proper communications.

static unsigned char 	TXByteCtr, 						//<! Stores the counter of which TX byte it is on.
		RXByteCtr,						//<! Stores the counter of which RX byte it is on.
		RX = 0;							//<! Stores if the device is receiving or not.
/**********************************/


/********* Function Declarations ***********/
void msDelay(unsigned int ms);							//<! Function that conducts delays for I2C to operate properly.
void I2Cwrite(char info);								//<! Function to store bytes in an array to be sent to slave device..
void I2CendTransmission(void);							//<! Function to end transmission and send all bytes to slave device.
void I2CbeginTransmission(char address);				//<! Function to begin the transmission of I2C with slave address.
void I2CrequestFrom(char address, int numOfBytes);		//<! Function to request data back from the slave device.
char I2Cread(void);										//<! Function to read data returned from slave device.
/******************************************/


#endif /* I2C_MSP430F_H_ */
