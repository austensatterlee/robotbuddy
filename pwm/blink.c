//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  J. Stevenson
//  Texas Instruments, Inc
//  July 2011
//  Built with Code Composer Studio v5
//***************************************************************************************

#include <msp430.h>				

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer
	TA0CCR0 |= 3000-1;
	TA0CCTL0 |= CCIE;
	TA0CTL |= TASSEL_1 + MC_1;

	P1DIR |= BIT0;
	_BIS_SR(LPM0_bits + GIE);
}
#pragma vector=TIMER0_A0_VECTOR
   __interrupt void Timer0_A0 (void) {

    P1OUT ^= BIT0;

}
