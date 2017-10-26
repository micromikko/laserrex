/*
 * Servo.h
 *
 *  Created on: Oct 23, 2017
 *      Author: Tuomas-laptop
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#ifndef SRC_SERVO_H_
#define SRC_SERVO_H_

#include "DigitalIoPin.h"

class Servo {

private:
	DigitalIoPin *pin;
	int portNum;
	int pinNum;
	const uint16_t penUp_cycle_length = 1200;
	const uint16_t penDown_cycle_length = 1450;
	void init();
public:
	Servo(int portNum, int pinNum);
	virtual ~Servo();
	void penDown();
	void penUp();

};

#endif /* SRC_SERVO_H_ */
