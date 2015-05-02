#include "msp430g2553.h"
#include <stdint.h>
#include <stdio.h>

// Include I2C MSP430 and MPU6050 libraries.
#include "I2C_MSP430.h"						// Use the correct I2C library particular to your MCU.
#include "MSP430_MPU6050.h"
#include "MPU6050.h"

#include "hwuart.h"
static const uint8_t UARTOUT=0;
//void serial_setup(unsigned out_mask, unsigned in_mask, unsigned duration);

//void printf(char *, ...);

void initializeTimer(void);

int main(void)
{
	static int16_t ax, ay, az;					//<! Holds the raw accelerometer data.
	static int16_t gx, gy, gz;					//<! Holds the raw gyroscope data.
	  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
	  BCSCTL1 = CALBC1_1MHZ; 				// 1MHZ operation
	  DCOCTL = CALDCO_1MHZ;

	  P1DIR |= BIT0;
	  P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	  P1SEL2|= BIT6 + BIT7;
      initializeIMU();						// Initialize IMU
      if(UARTOUT){
      		 P1SEL |= BIT1 + BIT2;						// P1.1,1.2 are UART outputs
      		 P1SEL2 |= BIT1 + BIT2;
      		 initializeUART();
      }
		P1DIR |= BIT2;						// set P1.2 as timer output
		P1OUT |= BIT2;
		P1SEL |= BIT2;
		initializeTimer();

	  msDelay(10);							// Temporary wait. Can be shortened

	  for (;;)
	  	{
			getMotion6( &ax, &ay, &az, &gx, &gy, &gz);

			// ax, ay, az, gx, gy, gz have RAW values from accelerometer and gyroscope
		//{
		if(UARTOUT){
				uartSendBytes(ax,4);
				uartSendByte(0x20);
				uartSendBytes(ay,4);
				uartSendByte(0x20);
				uartSendBytes(az,4);
				uartSendByte(0x20);
				uartSendBytes(gx,4);
				uartSendByte(0x20);
				uartSendBytes(gy,4);
				uartSendByte(0x20);
				uartSendBytes(gz,4);
				uartSendByte(0x0A);
		}

}
}

/**
 * Initializes the timer to call the interrupt every 1 second
 */
void initializeTimer(void){
	TACCTL1 = OUTMOD_7; // Set on rising edge, reset on falling edge
	//TACCTL0 |= CCIE; // enable interrupt on edges
	CCR0 = 5001; // Period (number of timer ticks per period);
	CCR1 = 2500; // Duty cycle (number of timer ticks per HI part of period);
	TACTL = TASSEL_2 + MC_1;
	__bis_SR_register(LPM0_bits + GIE);
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void){
	P1OUT ^= BIT0;
}

