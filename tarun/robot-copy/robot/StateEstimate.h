//
//  StateEstimate.h
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

#ifndef __robot__StateEstimate__
#define __robot__StateEstimate__

#include <msp430.h>

double getAngleEstimate();

// Sensor biases when still
// (accelx, accely, accelz)
// -7.37593331e+02,   1.99838676e+03,   1.58679221e+04

// (gyrox, gyroy, gyroz)
// -2.12991066e+02,   2.04664451e+02,   1.03180331e+01

#define GYRO_BIAS -213.0
#define GYRO_RANGE 131.0

#define ACCEL_X_BIAS -737.593331
#define ACCEL_X_RANGE 16384.0

#define ACCEL_Y_BIAS 1998.38676
#define ACCEL_Y_RANGE 16384.0

#define ACCEL_Z_BIAS 15876.9221
#define ACCEL_Z_RANGE 16384.0

#define HPF 0.98
#define LPF (1-HPF)

#define PI 3.14159265
#define DEGREES 180

#endif /* defined(__robot__StateEstimate__) */
