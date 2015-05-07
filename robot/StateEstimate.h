//
//  StateEstimate.h
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

#ifndef __robot__StateEstimate__
#define __robot__StateEstimate__

#include <msp430g2553.h>

double getAngleEstimate();
double getAngularVelocity();
double getAngularAcceleration();
void burnin();

#define GYRO_BIAS 13
#define GYRO_RANGE 1/131.0

#define ACCEL_X_BIAS 0
#define ACCEL_X_RANGE 1/16384.0

#define ACCEL_Y_BIAS 314
#define ACCEL_Y_RANGE 1/16384.0

#define ACCEL_Z_BIAS 0
#define ACCEL_Z_RANGE 1/16384.0

#define HPF 0.98
#define LPF (1-HPF)

#define PI 3.14159265
#define DEGREES 180

#endif /* defined(__robot__StateEstimate__) */
