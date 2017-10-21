/*
 * Motor.h
 *
 *  Created on: 12.9.2017
 *      Author: micromikko
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "DigitalIoPin.h"
#include "FreeRTOS.h"
#include "task.h"

class Motor {
public:
	Motor();
	virtual ~Motor();

	void drive();
	void driveX();
	void driveY();
	void RIT_start(int count, int us);
	int drive(const bool direction, const int speed);
	int drive(const bool direction, const int speed, const int distance);
//	int stop();
//	bool changeDir();
//	bool setDir(bool dir);



private:
	DigitalIoPin *dirPinX;
	DigitalIoPin *stepPinX;
	DigitalIoPin *dirPinY;
	DigitalIoPin *stepPinY;

	// us ("pps") reverse 1/pps
	int ritCountX;
	int ritCountY;
	bool dirX;
	bool dirY;
};

#endif /* MOTOR_H_ */

/*
 * 	DigitalIoPin(int port, int pin, pinMode mode, bool invert = false);
 *
 * 	DigitalIoPin motorDir(1, 0, DigitalIoPin::output, false); // CW, ACW
	DigitalIoPin motorStep(0, 24, DigitalIoPin::output, false); //??
 */
