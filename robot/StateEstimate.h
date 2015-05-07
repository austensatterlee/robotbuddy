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

#define GYRO_BIAS 1.2
#define GYRO_RANGE 131.0

#define ACCEL_X_BIAS -7684.57407407
#define ACCEL_X_RANGE 16384.0

#define ACCEL_Y_BIAS -862.14814815
#define ACCEL_Y_RANGE 16384.0

#define ACCEL_Z_BIAS -661.60185185
#define ACCEL_Z_RANGE 16384.0

#define HPF 0.98
#define LPF (1-HPF)

#define PI 3.14159265
#define DEGREES 180

#endif /* defined(__robot__StateEstimate__) */
