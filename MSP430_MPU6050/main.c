#include "msp430g2553.h"
#include <stdint.h>
#include <stdio.h>

// Include I2C MSP430 and MPU6050 libraries.
#include "I2C_MSP430.h"						// Use the correct I2C library particular to your MCU.
#include "MSP430_MPU6050.h"
#include "MPU6050.h"

#include "hwuart.h"
#define ASCIIOUT 0
//void serial_setup(unsigned out_mask, unsigned in_mask, unsigned duration);

//void printf(char *, ...);

void initializeTimer(void);

int main(void)
{
	char message[64];
	static int16_t ax, ay, az;					//<! Holds the raw accelerometer data.
	static int16_t gx, gy, gz;					//<! Holds the raw gyroscope data.
	  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
	  BCSCTL1 = CALBC1_1MHZ; 				// 1MHZ operation
	  DCOCTL = CALDCO_1MHZ;

	  //Placed 10k or 4.7k ohm pullup resistors on SDA and SCL lines
      //P3DIR |= 0x0F;
	  //P3SEL |= 0x06;                        // Assign I2C pins to USCI_B0

	  P1DIR |= BIT0;
	  P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	  P1SEL2|= BIT6 + BIT7;

      initializeIMU();						// Initialize IMU

      P1SEL |= BIT1 + BIT2;						// P1.1,1.2 are UART outputs
      P1SEL2 |= BIT1 + BIT2;
      initializeUART();

	  msDelay(10);							// Temporary wait. Can be shortened

	  for (;;)
	  	{
			getMotion6( &ax, &ay, &az, &gx, &gy, &gz);

			// ax, ay, az, gx, gy, gz have RAW values from accelerometer and gyroscope
			if (ASCIIOUT){
				sprintf(message,"%d %d %d %d %d %d\r\n", ax, ay, az, gx, gy, gz);
				uartSendString(message);
			}else{
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
void initializeTimer(void) {
	P1DIR |= BIT0;						// set P1.0 (LED1) as output
	P1OUT |= BIT0;						// P1.0 low
	CCTL0 = CCIE;						// CCR0 interrupt enabled
	CCR0 = 4096;						// 32kHz/8/4096 -> 1 sec
	TACTL = TASSEL_1 + ID_3 + MC_1;		// ACLK, /8, upmode
}
