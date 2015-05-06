/*
 * hwuart.h
 *
 *  Created on: Apr 27, 2015
 *      Author: Austen
 */

#ifndef HWUART_H_
#define HWUART_H_

#include "msp430g2553.h"

void initializeUART(void);
void uartSendBytes( char*,unsigned int numBytes);
void uartSendByte( char*);
void uartSendString( char*);


#endif
