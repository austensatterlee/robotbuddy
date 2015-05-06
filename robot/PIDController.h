//
//  PIDController.h
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

#ifndef __robot__PIDController__
#define __robot__PIDController__

//#include <stdio.h>

#define Kp 1
#define Ki 1
#define Kd 1

double output;

double preverror;
double angle;
double error;
double integral;
double derivative;

double getOutput(double);

#endif /* defined(__robot__PIDController__) */
