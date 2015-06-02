//
// main.cpp
// Main file
// ---------
// Configuration constants
#include "Config.h"
// IMU communication dependencies
#include "I2C_MSP430.h"
#include "MSP430_MPU6050.h"
#include "MPU6050.h"
// ==================
// Include Serial Communication
#include "UART.h"
// ==================
// Include State Estimation
#include "StateEstimate.h"
// ==================
// Include Bluetooth
//#include "Bluetooth.h"
// ==================
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
// Define variables and constants

void setMotorSpeedAndDirection(float);
void loop();
void PWM_init();
float getPIDOutput(float);

// PWM state variables
unsigned int duty = 0;
unsigned int counter = 0;
char isForward = 1;
unsigned int timer_counter = 0;
// PID state variables
float output=0;
float angle=0;
float error=0;
float preverror=0;
float integral=0;
float derivative=0;


int main(void)
{
    // Stop watchdog timer

	WDTCTL = WDTPW + WDTHOLD;
    // 1MHZ operation
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // Reset all pins
    P1OUT = 0x00;
    P1DIR = 0xFF;
    P2OUT = 0x00;
    P2DIR = 0xFF;
    P3OUT = 0x00;
    P3DIR = 0xFF;

    // Initialize the MPU6050 IMU
	// Assign I2C pins to USCI_A0
    P1SEL |= BIT6 + BIT7;
    P1SEL2|= BIT6 + BIT7;
    initializeIMU();
    msDelay(1000); // warm-up time
    burnin();

    // Set pins 1.1 and 1.2 to UART outputs
    if(UART_ENABLE){
		P1SEL |= BIT1 + BIT2;
		P1SEL2 |= BIT1 + BIT2;
		UART_init();
    }

	// Set  PWM outputs
    P1DIR |= MOTORFWD_PIN + MOTORREV_PIN + PWD_PIN;
    P1SEL |= PWD_PIN; // set PWD_PIN to timer output
	PWM_init();

	for (;;) {
		loop();
	}
}

/*
 * main loop
 */
void loop() {
    float angle = getAngleEstimate();
    float output = getPIDOutput(angle);
    setMotorSpeedAndDirection(output);
    if(UART_ENABLE){
		UART_out_bytes(&angle,4);
		UART_out_bytes(&output,4);
    }
}

void setMotorSpeedAndDirection(float output) {
    if (output >= 0) {
        isForward = 1;
    } else if(output < 0) {
        isForward = 0;
        output*=-1;
    }

    duty = MAX(1,MIN(Kout * (int)output, PWM_PERIOD));
}

/* Update PID state and return system output */
float getPIDOutput(float angle) {
    error = angle;
    integral += error;
    derivative = getAngularVelocity();
    output = Kp * error + Ki * integral + Kd * derivative;
    preverror = error;

    return output;
}

/**
 * Initializes the timer so that the PWM interrupt is triggered
 * with a period determined by PWM_PERIOD (measured in clock cycles)
 */
void PWM_init() {
    TA0CCR0 = PWM_PERIOD-1;
    TA0CCR1 = duty;
    TA0CCTL1 = OUTMOD_7;
    TA0CCTL0 |= CCIE; // enable the interrupt
    TA0CTL |= TASSEL_2 + MC_1 + ID_3; // set the timer to use SMCLK
}

/*
 *  Interrupt for PWM output. Applies changes made in setMotorSpeedAndDirection method.
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 () {
   if(isForward){
	   P1OUT |= MOTORFWD_PIN;
	   P1OUT &= ~MOTORREV_PIN;
   }else{
	   P1OUT |= MOTORREV_PIN;
	   P1OUT &= ~MOTORFWD_PIN;
   }
   TA0CCR1 = duty;
}
