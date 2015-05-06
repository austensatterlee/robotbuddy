// MSP430 I2C Library
// 07/05/2012 by Mushfiq Sarker <mushfiqsarker@inventige.com>
// Library compatible with MSP430F22x2, MSP430F22x4 MCU from Texas Instruments.


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
// LIBRARY COMPATIBLE WITH MSP430F22X2 AND MSP430F22X4 MCU FROM TEXAS INSTRUMENTS.
#include "I2C_MSP430.h"

/******************************** I2C Functions ********************************/
//-------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move received data from the I2C slave
// to the MSP430 memory. It is structured such that it can be used to receive
// any 2+ number of bytes by pre-loading RXByteCtr with the byte count.
//-------------------------------------------------------------------------------
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{

	if(RX == 1)
	{
		RXByteCtr--;	// Decrement RX byte counter
		if (RXByteCtr)
			{
				bufferedDataRX[Bytecount] = UCB0RXBUF;
				Bytecount++;
			}
		else
			{
				bufferedDataRX[Bytecount] = UCB0RXBUF;
				Bytecount++;

				if(RPT_Flag == 0)
					UCB0CTL1 |= UCTXSTP;	// No Repeated Start: stop condition
				if(RPT_Flag == 1)	// if Repeated Start: do nothing
					RPT_Flag = 0;

				throwAway = UCB0RXBUF;

				__bic_SR_register_on_exit(CPUOFF);	// Exit LPM0
			}
	}
	else
	{	// Master Transmit
		if (TXByteCtr)	// Check TX byte counter
  			{
    	  		UCB0TXBUF = bufferedDataOutput[Bytecount];
    	  		Bytecount++;
    			TXByteCtr--;	// Decrement TX byte counter
  			}
  		else
  			{
    			if(RPT_Flag == 1)
    				{
    					RPT_Flag = 0;
						TXByteCtr = NUM_BYTES_TX;	// Load TX byte counter
   	 					__bic_SR_register_on_exit(CPUOFF);
    				}
    			else
    				{
						UCB0CTL1 |= UCTXSTP;	// I2C stop condition
						IFG2 &= ~UCB0TXIFG;	// Clear USCI_B0 TX intterupt flag
						__bic_SR_register_on_exit(CPUOFF);	// Exit LPM0
					}
  			}
 	}

}



void I2CbeginTransmission(char address)
{
	SLV_Addr = address;	// bit shift due to MSP430 requiring a Write bit of 1.
	NUM_BYTES_TX = 0;
	Bytecount = 0;
}


void I2Cwrite(char info)
{
	bufferedDataOutput[NUM_BYTES_TX] = info;	// store the info byte into the buffered array
	NUM_BYTES_TX++;	// increment the number of bytes that are held in array
}


void I2CendTransmission(void)
{

    TXByteCtr = NUM_BYTES_TX;	// Load TX byte counter
    _DINT();
		RX = 0;
		IE2 &= ~UCB0RXIE;
		while (UCB0CTL1 & UCTXSTP);	// Ensure stop condition got sent// Disable RX interrupt
		UCB0CTL1 |= UCSWRST;	// Enable SW reset
		UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;	// I2C Master, synchronous mode
		UCB0CTL1 = UCSSEL_2 + UCSWRST;	// Use SMCLK, keep SW reset
		UCB0BR0 = 0x0A;	// fSCL = SMCLK/12 = ~100kHz
		UCB0BR1 = 0;
		UCB0I2CSA = SLV_Addr;	// Slave Address is set in I2CBeginTransmission() functions
		UCB0CTL1 &= ~UCSWRST;	// Clear SW reset, resume operation
		IE2 |= UCB0TXIE;	// Enable TX interrupt

    while (UCB0CTL1 & UCTXSTP);	// Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;	// I2C TX, start condition
    __bis_SR_register(CPUOFF + GIE);	// Enter LPM0 w/ interrupts
}



void I2CrequestFrom(char address, int numOfBytes)
{
	Bytecount = 0;
	RXByteCtr = numOfBytes;
	numRXbytesSent = 0;	// Use as counter in @I2CRead()


	// Setup RX
	  _DINT();
	  RX = 1;
	  IE2 &= ~UCB0TXIE;
	  UCB0CTL1 |= UCSWRST;	// Enable SW reset
	  UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;	// I2C Master, synchronous mode
	  UCB0CTL1 = UCSSEL_2 + UCSWRST;	// Use SMCLK, keep SW reset
	  UCB0BR0 = 0x0A;	// fSCL = SMCLK/12 = ~100kHz
	  UCB0BR1 = 0;
	  UCB0I2CSA = SLV_Addr;	// Slave Address is set with I2CBeginTransmission
	  UCB0CTL1 &= ~UCSWRST;	// Clear SW reset, resume operation
	  IE2 |= UCB0RXIE;	// Enable RX interrupt
	// End Setup RX

    while (UCB0CTL1 & UCTXSTP);	// Ensure stop condition got sent
    UCB0CTL1 |= UCTXSTT;	// I2C start condition
    __bis_SR_register(CPUOFF + GIE);	// Enter LPM0 w/ interrupts


}

char I2Cread(void)
{
	uint8_t out;

	if (numRXbytesSent < Bytecount)
		{
			 out = bufferedDataRX[numRXbytesSent];
			numRXbytesSent++;
		}

	return out;

}

/***********************************************************************************/
