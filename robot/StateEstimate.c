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
int16_t ax, ay, az;

// Holds the raw gyroscope data.
int16_t gx, gy, gz;

double ax_bias=0;
double az_bias=0;
double gy_bias=0;

double accel_x, accel_z;

double angleA, angleG;

double angleEstimate = 0;

double getAngularAcceleration();

void burnin(){
	int i=10000;
	while(i--){
		getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
		ax_bias += ax*0.0001;
		az_bias += az*0.0001;
		gy_bias += gy*0.0001;
	}
}
double getAngleEstimate() {
    getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
//    UART_out_bytes((char*)&ax,2);
//    UART_out_bytes((char*)&ay,2);
//    UART_out_bytes((char*)&az,2);
//    UART_out_bytes((char*)&gy,2);
    // Get Gyro Value
    angleG = getAngularVelocity();
//    UART_out_bytes((char*)&angleG,8);

    angleA = getAngularAcceleration();
//    UART_out_bytes((char*)&angleA,8);

    // State estimation
    angleEstimate = HPF * (angleEstimate + angleG * DT) + LPF * angleA;
//    UART_out_bytes((char*)&angleEstimate,8);

//    UART_out_double(accel_x);
//    UART_out_double(accel_y);
//    UART_out_double(accel_z);
//    UART_out_double(pitch);
//    UART_out_double(gyro);

    return angleEstimate;
}

// Fixing Gyro values
double getAngularVelocity() {
    return ((double)(gy - gy_bias)) * GYRO_RANGE;
}



double getAngularAcceleration() {
    // Fixing Accel values
    accel_x = ((ax - ax_bias));
    accel_z = ((az - az_bias));
    return atan2(accel_x, accel_z);
    //return accel_x;
}
