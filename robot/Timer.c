//
//  Timer.c
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

#include <msp430.h>
#include "Timer.h"

unsigned int duty = 50;
char isForward = 1;
unsigned int timer_counter = 0;

/**
 * Initializes the timer to call the interrupt every 1 second
 */
void TIMER_init(void) {
    // Stop watchdog timer
    WDTCTL = WDTPW + WDTHOLD;
    // 1MHZ operation
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    TA0CCR0 |= 100;
    TA0CCTL0 |= CCIE;
    TA0CTL |= TASSEL_2 + MC_1;
    
    P1DIR |= BIT0;
    //_BIS_SR(LPM0_bits + GIE);
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void) {
	   timer_counter++;

	   if (timer_counter == duty) {
           P1OUT &= ~BIT4;
           P1OUT &= ~BIT5;
       }

	   if (timer_counter >= PWM_PERIOD) {
           if (isForward && duty>0) {
               P1OUT |= BIT4;
               P1OUT &= ~BIT5;
           } else if (duty>0) {
               P1OUT |= BIT5;
               P1OUT &= ~BIT4;
           }

           timer_counter = 0;
       }
}
