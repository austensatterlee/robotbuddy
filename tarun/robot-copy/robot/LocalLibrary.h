//
// File			LocalLibrary.h
// Brief		Library header
//
// Project	 	robot
// Developed with [embedXcode](http://embedXcode.weebly.com)
// 
// Author		Tarun Chaudhry
// 				Tarun Chaudhry
// Date			5/6/15 12:00 AM
// Version		<#version#>
// 
// Copyright	© Tarun Chaudhry, 2015
// Licence		<#license#>
//
// See			ReadMe.txt for references
//


// Core library for code-sense - IDE-based
#if defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#else // error
#error Platform not defined
#endif // end IDE


#ifndef robot_LocalLibrary_h
#define robot_LocalLibrary_h

//
// Brief	Blink a LED
// Details	LED attached to pin is light on then light off
// Total cycle duration = ms
// Parameters:
//      pin pin to which the LED is attached
//      times number of times
//      ms cycle duration in ms
//
void blink(uint8_t pin, uint8_t times, uint16_t ms);

#endif
