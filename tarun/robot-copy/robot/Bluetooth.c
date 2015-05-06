//
//  Bluetooth.c
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

#include "Bluetooth.h"
#include <msp430.h>

unsigned char bluetooth_data = 0;
unsigned char last_bluetooth_data = 0;

void Bluetooth_init(void) {
    /*** Set-up GPIO ***/
    P1SEL = BIT1 + BIT2;					// P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;					// P1.1 = RXD, P1.2=TXD
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
    // Assign received byte to bluetooth_data
    bluetooth_data = UCA0RXBUF;
}

void Bluetooth_command(void) {


    switch (bluetooth_data) {
	  					case 0x41:							// ON Command
            break;

	  					case 0x42:							// OFF Command
            break;

	  					case 0x46:							// FLASH Command
            break;

	  					case 0x3E:							// INCREASE Command
            break;

	  					case 0x3C:							// DECREASE Command
            break;

	  					case 0x31:							// 1 Command
            break;

	  					case 0x32:							// 2 Command
            break;

	  					case 0x33:							// 3 Command
            break;

	  					case 0x34:							// 4 Command
            break;
            
	  					case 0x35:							// 5 Command
            break;
            
	  					default: break;
    }
}
