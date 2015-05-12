//
//  StateEstimate.c
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

#include <stdint.h>
#include <math.h>

#include "StateEstimate.h"
#include "UART.h"
#include "Config.h"
#include "MSP430_MPU6050.h"


// Holds the raw accelerometer data.
int16_t ax=0;
int16_t ay=0;
int16_t az=0;


// Holds the raw gyroscope data.
int16_t gx, gy, gz;

float ax_bias=0;
float az_bias=0;
float gy_bias=0;
float atan2_bias=0;

float accel_x, accel_z;

float angleA, angleG;

float angleEstimate = 0;

#define BURNIN 100
#define BURNIN_DATA 1.0/BURNIN

float getAngularAcceleration();

void burnin(){
	int i=BURNIN;
    P1DIR |= BIT0;
    P1OUT |= BIT0;
	while(i--){
		if (i%10==0) {
			P1OUT^=BIT0;
		}
		getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
		//ax_bias += (float)ax*BURNIN_DATA;
		//az_bias += (float)az*BURNIN_DATA;
		gy_bias += (float)gy*BURNIN_DATA;
		atan2_bias += atan2(ax, az) * BURNIN_DATA;
	}
	P1OUT &= ~BIT0;
}

/*
 * Return an estimate in radians of the angle being made with the ground
 */
float getAngleEstimate() {
    getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    // Get Gyro Value
    angleG = getAngularVelocity();
    angleA = getAngularAcceleration();
    // State estimation
    angleEstimate = HPF * (angleEstimate + angleG * DT) + LPF * angleA;
    return angleEstimate;
}

// Fixing Gyro values
float getAngularVelocity() {
    return ((float)(gy)-gy_bias)*GYRO_RANGE;
}



float getAngularAcceleration() {
    // Fixing Accel values
    accel_x = (((float)(ax) - ax_bias));
    accel_z = (((float)(az) - az_bias));
    return atan2(accel_x, accel_z) - atan2_bias;
    //return accel_x;
}
