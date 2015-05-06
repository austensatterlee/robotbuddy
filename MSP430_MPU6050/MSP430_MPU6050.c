// MSP430 MPU6050/6000 I2C Code
// Based on InvenSense MPU-6050 register map document rev. 3.2, 5/19/2011 (RM-MPU-6000A-00)
// 07/05/2012 by Mushfiq Sarker <mushfiqsarker@inventige.com>

// Changelog:
// ... - ongoing debug release

// NOTE: THIS IS ONLY A PARIAL RELEASE. THIS DEVICE CLASS IS CURRENTLY UNDERGOING ACTIVE
// DEVELOPMENT AND IS STILL MISSING SOME IMPORTANT FEATURES. PLEASE KEEP THIS IN MIND IF
// YOU DECIDE TO USE THIS PARTICULAR CODE FOR ANYTHING.

/* ============================================
MSP430 MPU6050 code is placed under the MIT license
Copyright (c) 2012 Mushfiq Sarker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#include "MPU6050.h"
#include "MSP430_MPU6050.h"
#include "I2C_MSP430.h"

uint8_t initMPU(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t source);	//<! Internal function that handles the initialization of the sensor.
void readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *b );	//<! Internal function that handles the reading of single byte.
void readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);	//<! Internal function that handles the reading of multiple bytes.

static uint8_t buffer[14];	//<! Stores the motion data temporarily before being manipulated.

/******************************** MPU6050 Functions ********************************/
void initializeIMU()
{
	// Delays do not need to be this long. Delays are present to keep I2C line clear.
	// This functions can be altered by the user. Check MPU6050.h file by Jeff Rowberg to determine the sensitivty variables.
	setClockSource(MPU6050_CLOCK_PLL_XGYRO);	// Set MPU6050 clock
	msDelay(10);
	setFullScaleGyroRange(MPU6050_GYRO_FS_2000);	// Gyroscope sensitivity set to 2000 degrees/sec
	msDelay(10);
	setFullScaleAccelRange(MPU6050_ACCEL_FS_4);	// Accelerometer sensitivity set to 4g
	msDelay(10);
	setSleepEnabled(0);	// Wake up device.
	msDelay(10);
}

uint8_t getDeviceID()
{
	uint8_t b = 0;
	readByte(MPU6050Addr,  MPU6050_RA_WHO_AM_I, &b );
	return b;
}

void readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *b )
{
	I2CbeginTransmission(devAddr);
	I2Cwrite(regAddr);
	I2CendTransmission();
	msDelay(2);

	I2CrequestFrom(devAddr, 1);	// read a single byte
	*b = (uint8_t)I2Cread();
}

void readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data)
{
	int i;
	I2CbeginTransmission(devAddr);
	I2Cwrite(regAddr);
	msDelay(2);
	I2CendTransmission();
	msDelay(2);
	I2CrequestFrom(devAddr, 14);
	msDelay(5);

	for (i = 0; i < length; i++)
		{
			data[i] = I2Cread();
		}
}


void writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
	I2CbeginTransmission(devAddr);
	I2Cwrite(regAddr);
	I2Cwrite(data);
	I2CendTransmission();
}

void writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data)
{
	uint8_t b = 0;
	readByte(devAddr, regAddr, &b);
	msDelay(2);
	b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
	writeByte(devAddr, regAddr, b);
}

uint8_t setFullScaleAccelRange(uint8_t range)
{
	return initMPU(MPU6050Addr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range);
}

void setFullScaleGyroRange(uint8_t range)
{
	initMPU(MPU6050Addr, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range);
}

void setClockSource(uint8_t source)
{
	initMPU(MPU6050Addr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source);
}

void setSleepEnabled(uint8_t enabled)
{
	writeBit(MPU6050Addr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled);
}

uint8_t initMPU(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t source)
{
	uint8_t b = 0;

	readByte(devAddr, regAddr, &b);
	msDelay(2);
	uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
	source <<= (bitStart - length + 1);	// shift data into correct position
	source &= mask;	// zero all non-important bits in data

	b &= ~(mask); // zero all important bits in existing byte
	b |= source; // combine data with existing byte

	writeByte(devAddr, regAddr, b);
	return b;
}

void getMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz)
{
	readBytes(MPU6050Addr, MPU6050_RA_ACCEL_XOUT_H, 14, buffer);
	*ax = (((int16_t)buffer[0]) << 8) | buffer[1];
	*ay = (((int16_t)buffer[2]) << 8) | buffer[3];
	*az = (((int16_t)buffer[4]) << 8) | buffer[5];
	*gx = (((int16_t)buffer[8]) << 8) | buffer[9];
	*gy = (((int16_t)buffer[10]) << 8) | buffer[11];
	*gz = (((int16_t)buffer[12]) << 8) | buffer[13];
}


void msDelay(unsigned int msTime)
{
	// Internal MCLK is running at 16 Mhz (1/16Mhz = 62.5nS per cycle of code).
	// MCLK = 16000000UL/16000 = 1 ms of real time. Can run a for LOOP that does delays for amount of msTime
	long counter;
	for ( counter = 0; counter <= msTime; counter++)
	__delay_cycles( 16000000/16000  );				// 1 millisecond delay
}
