/*
 * hwuart.h
 *
 *  Created on: Apr 27, 2015
 *      Author: Austen
 */
#include "hwuart.h"

void initializeUART(void){
	// initialize UART to USCI_A0
	UCA0CTL1 |= UCSSEL_2;                     // Use SMCLK for UART
	UCA0BR0 = 104;
	UCA0BR1 = 0;
	UCA0MCTL = UCBRS_0;						  // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     // Clear USCI for operation
	IE2 |= UCA0RXIE;						  // enable USCI_A0 rx interrupt
}


void uartSendByte(unsigned char *txByte){
	while(!(IFG2 & UCA0TXIFG)); // wait until usci is ready
	UCA0TXBUF = *txByte;
}

/* Send numBytes bytes of data starting from given memory address*/
void uartSendBytes(unsigned char *txBytes, unsigned char numBytes){
	while(numBytes--){
		uartSendByte(txBytes);
		txBytes++;
	}
}

/* Send data starting from given memory address until a null terminator is reached */
void uartSendString(unsigned char *txString){
	while(*txString!=0x0){
		uartSendByte(txString);
		txString++;
	}
}
