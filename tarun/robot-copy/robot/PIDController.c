//
//  PIDController.c
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

#include "PIDController.h"
#include "Timer.h"

double getOutput(double angle) {
    /* PID Control */
    error = -1 * angle;
    integral += error * DT;
    derivative = (error - preverror) * DT;
    output = Kp * error + Ki * integral + Kd * derivative;
    preverror = error;

    return output;
}
