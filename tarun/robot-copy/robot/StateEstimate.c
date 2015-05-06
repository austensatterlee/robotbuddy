//
//  StateEstimate.c
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

#include <stdint.h>

#include "StateEstimate.h"
#include "Timer.h"
#include "UART.h"


// Holds the raw accelerometer data.
int16_t ax, ay, az;

// Holds the raw gyroscope data.
int16_t gx, gy, gz;

double accel_x, accel_y, accel_z;

double gyro, pitch;

double angleEstimate = 0;

double getAngleEstimate() {
    getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Get Gyro Value
    gyro = getGyroValue();

    // Fixing Accel values
    // accel = ((double)(ax) - ACCEL_BIAS) / ACCEL_RANGE;

    pitch = calculatePitchFromAccel();

    // State estimation
    angleEstimate = HPF * (angleEstimate + gyro * DT) + LPF * pitch;

    UART_out_double(pitch);
    UART_out_double(gyro);

    return angleEstimate;
}

// Fixing Gyro values
double getGyroValue() {
    return ((double)(gx) - GYRO_BIAS) / GYRO_RANGE;
}

double calculatePitchFromAccel() {
    // Fixing Accel values
    accel_x = ((double)(ax) - ACCEL_X_BIAS) / ACCEL_X_RANGE;
    accel_y = ((double)(ay) - ACCEL_Y_BIAS) / ACCEL_Y_RANGE;
    accel_z = ((double)(az) - ACCEL_Z_BIAS) / ACCEL_Z_RANGE;
    return (double)(atan2(accel_y, sqrt(sq(accel_z) + sq(accel_x)))*DEGREES/PI);
}
