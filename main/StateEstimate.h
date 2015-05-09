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

#define GYRO_RANGE 1

#define HPF 0.98
#define LPF (1-HPF)

#define PI 3.14159265
#define DEGREES 180

#endif /* defined(__robot__StateEstimate__) */
