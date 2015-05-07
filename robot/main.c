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

void setMotorSpeedAndDirection(double);
void loop();
void PWM_init();
double getPIDOutput(double);

// PWM state variables
unsigned int duty = 0;
char isForward = 1;
unsigned int timer_counter = 0;
// PID state variables
double output=0;
double angle=0;
double error=0;
double preverror=0;
double integral=0;
double derivative=0;


int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW + WDTHOLD;
    // 1MHZ operation
    BCSCTL1 = CALBC1_12MHZ;
    DCOCTL = CALDCO_12MHZ;

    // Reset all pins
    P1OUT = 0x00;
    P1DIR = 0xFF;
    P2OUT = 0x00;
    P2DIR = 0xFF;


    // Initialize the MPU6050 IMU
	// Assign I2C pins to USCI_A0
    P1SEL |= BIT6 + BIT7;
    P1SEL2|= BIT6 + BIT7;
    initializeIMU();
    msDelay(100); // warm-up time
    burnin();

    // Set pins 1.1 and 1.2 to UART outputs
	P1SEL |= BIT1 + BIT2;
	P1SEL2 |= BIT1 + BIT2;
	UART_init();

	// Set pins 1.4 and 1.5 to PWM outputs
//    P1DIR |= BIT4 + BIT5;
//    P1SEL |= BIT4 + BIT5;
	PWM_init();

	// Init Bluetooth
	// Bluetooth_init();

	for (;;) {
		loop();
	}
}

/*
 * main loop
 */
void loop() {
    double angle = getAngleEstimate();
    double output = getPIDOutput(angle);
    setMotorSpeedAndDirection(output);
    UART_out_bytes(&output,8);
}

void setMotorSpeedAndDirection(double output) {
    if (output > 0) {
        isForward = 1;
    } else {
        isForward = 0;
        output*=-1;
    }

    duty = Kout * output;
    TA0CCR1 = MIN(MAX(duty,PWM_PERIOD),0); // saturate pulse width
}

/* Update PID state and return system output */
double getPIDOutput(double angle) {
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
    TA0CCR0 = PWM_PERIOD;
    TA0CCR1 = duty;
    TA0CCTL0 = OUTMOD_7;
    TA0CCTL0 |= CCIE; // enable the interrupt
    TA0CTL |= TASSEL_2 + MC_1; // set the timer to use SMCLK

    //_BIS_SR(LPM0_bits + GIE);
}

/*
 *  Interrupt for PWM output.
 *  Outputs on either pin 1.4 or pin 1.5, depending on intended direction
 *  H-bridge design causes short when 1.4 and 1.5 are both HIGH,
 *  so this condition should be avoided in all circumstances
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 () {
	   if(isForward){
		   P1OUT ^= BIT4;
		   P1OUT &= ~BIT5;
	   }else{
		   P1OUT ^= BIT5;
		   P1OUT &= ~BIT4;
	   }
}
