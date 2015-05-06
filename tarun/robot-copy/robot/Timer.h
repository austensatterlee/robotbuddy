//
//  Timer.h
//  robot
//
//  Created by Tarun Chaudhry on 5/6/15.
//  Copyright (c) 2015 Tarun Chaudhry. All rights reserved.
//

#ifndef __robot__Timer__
#define __robot__Timer__

// #include <stdio.h>
#include <msp430.h>

void TIMER_init(void);

extern unsigned int duty;
extern char isForward;

#define PWM_PERIOD 1000

#define DT 0.001

#endif /* defined(__robot__Timer__) */
