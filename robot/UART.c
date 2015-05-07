//
//  UART.c
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

// #include <stdio.h>
#include "UART.h"
#include <msp430.h>

void UART_init() {
    // initialize UART to USCI_A0
	UCA0CTL1 |= UCSWRST; 	// temporarily disable the USCI while we tinker with it
    UCA0CTL1 |= UCSSEL_2;   // Use SMCLK for UART
    UCA0BR0 = 226; // 9600 baud rate @ 12MHz
    UCA0BR1 = 4;
    UCA0MCTL = UCBRS_0 + UCBRF_0;
    UCA0CTL1 &= ~UCSWRST;   // Clear USCI for operation
    //IE2 |= UCA0RXIE;		// enable USCI_A0 rx interrupt
}


void UART_out_byte(char *txByte) {
    while(!(IFG2 & UCA0TXIFG)); // wait until usci is ready
    UCA0TXBUF = *txByte;
}

/* Send numBytes bytes of data starting from given memory address*/
void UART_out_bytes(char *txBytes, unsigned int numBytes) {
    while(numBytes--){
        UART_out_byte(txBytes);
        txBytes++;
    }
}

/* Send data starting from given memory address until a null terminator is reached */
void UART_out_str(char *txString) {
    unsigned int len = strlen(txString);
    while(len--){
        UART_out_byte(txString);
        txString++;
    }
}

void UART_out_double(double data) {
    UART_out_bytes((char*)&data, 8);
}

void UART_out_long(long data, unsigned char ndigits) {
    char sign = ' ', s[6];
    unsigned int i = 0;
    
    if(data < 0) {
        sign='-';
        data = -data;
    }
    
    do {
        s[i++] = data % 10 + '0';
        if(i == ndigits) {
            s[i++]='.';
        }
    } while( (data /= 10) > 0);
    
    s[i] = sign;
    
    UART_out_str(s);
}
