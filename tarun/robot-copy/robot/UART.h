//
//  UART.h
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

#ifndef robot_UART_h
#define robot_UART_h

void UART_init();
void UART_out_bytes(char*, unsigned int numBytes);
void UART_out_byte(char*);
void UART_out_str(char*);
void UART_out_double(double);
void UART_out_long(long data, unsigned char ndigits);

#define NEW_LINE '\n'
#define SPACE ' '

#endif
