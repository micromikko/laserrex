/*
 * Driver.cpp
 *
 *  Created on: 23.10.2017
 *      Author: micromikko
 */

#include "Driver.h"

#include "DigitalIoPin.h"
#include "ITM_write.h"
#include <cstring>
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "sw_btn_interrupts.h"

#include "Parser.h"
#include "Handles.h"
#include "stdlib.h"
#include <cmath>

xSemaphoreHandle sbRIT;
xSemaphoreHandle motorSemaphore;
volatile uint32_t RIT_count;
volatile uint8_t kumpi;

extern "C" {
	void RIT_IRQHandler(void) {
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

		Chip_RIT_ClearIntStatus(LPC_RITIMER);
		if(RIT_count > 0) {
			RIT_count--;
			xSemaphoreGiveFromISR(motorSemaphore, &xHigherPriorityTaskWoken);
		} else {
			Chip_RIT_Disable(LPC_RITIMER);
			xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityTaskWoken);
		}
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	}
}

/*
 * starts repetitive interrupt timer
 */
void RIT_start(int count, int us) {
	uint64_t cmp_value;
	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;

	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);
	RIT_count = count;
	// enable automatic clear on when compare value==timer value
	// this makes interrupts trigger periodically
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	// reset the counter
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	// start counting
	Chip_RIT_Enable(LPC_RITIMER);
	// Enable the interrupt signal in NVIC (the interrupt controller)
	NVIC_EnableIRQ(RITIMER_IRQn);
	// wait for ISR to tell that we're done
	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
		NVIC_DisableIRQ(RITIMER_IRQn);
//		Board_UARTPutSTR("SEMAFOORI OTETTU\r\n");
	} else {
		// unexpected error
	}
}

/*
 * initializes the repetitive interrupt timer
 */
void RIT_init() {
	Chip_RIT_Init(LPC_RITIMER);
	NVIC_SetPriority(RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3 );
	RIT_count = 0;
	sbRIT = xSemaphoreCreateBinary();
	motorSemaphore = xSemaphoreCreateBinary();
}


/* Drive X for one step in the given direction. */
//void driveX (bool dir, DigitalIoPin &dirPin, PlotterData &pd) {
//	if (dir != pd.dirX) {
//		if (dir) {
//			pd.dirX = true;
//		} else {
//			pd.dirX = false;
//		}
//		dirPin.write(pd.dirX);
//	}
//	kumpi = 1;
//	RIT_start(1, 250);
//}

/* Drive Y for one step in the given direction. Y DRIVE VALUES FLIPPED! */
//void driveY(bool dir, DigitalIoPin &dirPin, PlotterData &pd) {
//	bool flipDir = !dir;
//	if (flipDir != pd.dirY) {
//		if (flipDir) {
//			pd.dirY = true;
//		} else {
//			pd.dirY = false;
//		}
//		dirPin.write(pd.dirY);
//	}
//	kumpi = 2;
//	RIT_start(1, 250);
//}

void disablePinInterrupts() {
	NVIC_DisableIRQ(PIN_INT0_IRQn);
	NVIC_DisableIRQ(PIN_INT1_IRQn);
	NVIC_DisableIRQ(PIN_INT2_IRQn);
	NVIC_DisableIRQ(PIN_INT3_IRQn);
	NVIC_DisableIRQ(PIN_INT4_IRQn);
	NVIC_DisableIRQ(PIN_INT5_IRQn);
}

void enablePinInterrupts() {

	uint8_t x1_register = 0;
	uint8_t x2_register = 1;
	uint8_t y1_register = 2;
	uint8_t y2_register = 3;
	uint8_t sw1_register = 4;
	uint8_t sw2_register = 5;

	uint8_t all_registers = PININTCH(x1_register) | PININTCH(x2_register) | PININTCH(y1_register)
			| PININTCH(y2_register) | PININTCH(sw1_register) | PININTCH(sw2_register);

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, all_registers); // clear status


	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT2_IRQn);
	NVIC_EnableIRQ(PIN_INT3_IRQn);
	NVIC_EnableIRQ(PIN_INT4_IRQn);
	NVIC_EnableIRQ(PIN_INT5_IRQn);
}
/* Calibration function that is to be run _before_ GPIO interrupts are initialized! */
void caribourate(PlotterData &pd) {
//	disablePinInterrupts();
	/* Initialize pins to use in calibration. Also they need to be in
	 * pullup mode for use in GPIO interrupts later, and the DigitalIoPin
	 * constructor also conveniently does that. */

	DigitalIoPin ls1(0, 29, DigitalIoPin::pullup, true);
	DigitalIoPin ls2(0, 9, DigitalIoPin::pullup, true);
	DigitalIoPin ls3(1, 3, DigitalIoPin::pullup, true);
	DigitalIoPin ls4(0, 0, DigitalIoPin::pullup, true);

	DigitalIoPin dirPinX(0, 28, DigitalIoPin::output, false);
	DigitalIoPin dirPinY(1, 0, DigitalIoPin::output, false);

	/* Pointers for limit switches */
	DigitalIoPin* x1;
	DigitalIoPin* x2;
	DigitalIoPin* y1;
	DigitalIoPin* y2;

	pd.axisStepCountX = 0;
	pd.axisStepCountY = 0;

	/* Amount of steps to back up from limit switches. This is calculated in the upcoming loop. */
	int backupSteps = 1000;


	while (!ls1.read() && !ls2.read() && !ls3.read() && !ls4.read()) {
//		driveX(false, dirPinX, pd);
		dirPinX.write(false);
		kumpi = 1;
		RIT_start(1, 250);
	}

	/* Assign correct switches for the axis */
	if (ls1.read()) {
		x1 = &ls1;
	} else if (ls2.read()) {
		x1 = &ls2;
	} else if (ls3.read()) {
		x1 = &ls3;
	} else if (ls4.read()) {
		x1 = &ls4;
	}

	/*
	 * drive back backupSteps
	 */
	for (int i = 0; i < backupSteps; i++) {
//		driveX(true, dirPinX, pd);
		dirPinX.write(true);
		kumpi = 1;
		RIT_start(1, 250);
	}

	while (!ls1.read() && !ls2.read() && !ls3.read() && !ls4.read()) {
//		driveX(true, dirPinX, pd);
		dirPinX.write(true);
		kumpi = 1;
		RIT_start(1, 250);
		pd.axisStepCountX++;
	}

	if (ls1.read()) {
		x2 = &ls1;
	} else if (ls2.read()) {
		x2 = &ls2;
	} else if (ls3.read()) {
		x2 = &ls3;
	} else if (ls4.read()) {
		x2 = &ls4;
	}

	for (int i = 0; i < backupSteps; i++) {
//		driveX(false, dirPinX, pd);
		dirPinX.write(false);
		kumpi = 1;
		RIT_start(1, 250);
	}

	pd.axisStepCountX -= backupSteps;
	/*
	 * checking y bottom
	 */
	while (!ls1.read() && !ls2.read() && !ls3.read() && !ls4.read()) {
//		driveY(true, dirPinY, pd);
		dirPinY.write(true);
		kumpi = 2;
		RIT_start(1, 250);
	}

	if (ls1.read()) {
		y1 = &ls1;
	} else if (ls2.read()) {
		y1 = &ls2;
	} else if (ls3.read()) {
		y1 = &ls3;
	} else if (ls4.read()) {
		y1 = &ls4;
	}

	for (int i = 0; i < backupSteps; i++) {
//		driveY(false, dirPinY, pd);
		dirPinY.write(false);
		kumpi = 2;
		RIT_start(1, 250);
	}

	/*
	 *
	 */
	while (!ls1.read() && !ls2.read() && !ls3.read() && !ls4.read()) {
//		driveY(false, dirPinY, pd);
		dirPinY.write(false);
		kumpi = 2;
		RIT_start(1, 250);
		pd.axisStepCountY++;
	}

	if (ls1.read()) {
		y2 = &ls1;
	} else if (ls2.read()) {
		y2 = &ls2;
	} else if (ls3.read()) {
		y2 = &ls3;
	} else if (ls4.read()) {
		y2 = &ls4;
	}

	for (int i = 0; i < backupSteps; i++) {
//		driveY(true, dirPinY, pd);
		dirPinY.write(true);
		kumpi = 2;
		RIT_start(1, 250);
	}

	pd.axisStepCountY -= backupSteps;

	for(int i = 0; i < pd.axisStepCountX; i++) {
//		driveX(false, dirPinX, pd);
		dirPinX.write(false);
		kumpi = 1;
		RIT_start(1, 250);
	}

	for(int i = 0; i < pd.axisStepCountY; i++) {
//		driveY(true, dirPinY, pd);
		dirPinY.write(true);
		kumpi = 2;
		RIT_start(1, 250);
	}


	pd.absoluteCurrentX = 0.0;
	pd.absoluteCurrentY = 0.0;

	pd.calculateStepsPerMM();
//	enablePinInterrupts();

		/* Calculate the amount of steps to back up from the limit switch */
//		if (backupSteps = 0) {
//			int stepsTravelled = 0;
//			while (x1.read() || x2.read()) {
//				driveX(true, pd.dirX, dirPinX);
//				stepsTravelled++;
//			}
//			backupSteps = (int) ((double) stepsTravelled * (double) 1.5);
//		}
		/* Use the calculated amount of backup steps to backup from the Y axis limit switch */

		/* Back up from limit switches */
//		for (int i; i < backupSteps; i++) {
//			if (axisNr == 0) {
//				driveX(true, dirPinX, pd);
//			} else {
//				driveY(true, dirPinY, pd);
//			}
//		}


	/*
	 * HIH HII! KUTITTAA!
	 */
//	bool xFinished = false;
//	bool yFinished = false;
//	/* Calculate amount of steps to the other end of the axes and travel back to origin. */
//	while (!xFinished || !yFinished) {
//
//		int xStepsTravelled = 0;
//		int yStepsTravelled = 0;
//
//		/* As long as limit switches aren't pressed, drive forward on the X axis */
//		if (!x2->read()) {
//			driveX(true, dirPinX, pd);
//			xStepsTravelled++;
//		} else {
//			xFinished = true;
//		}
//
//		/* As long as limit switches aren't pressed, drive forward on the Y axis */
//		if (!y2->read()) {
//			driveY(true, dirPinY, pd);
//			yStepsTravelled++;
//		} else {
//			yFinished = true;
//		}
//
//		/* Do this after we are at the positive end of both axes */
//		if (xFinished && yFinished) {
//			/*  */
//			for (int i = 0; i < backupSteps; i++) {
//				driveX(false, dirPinX, pd);
//				driveY(false, dirPinY, pd);
//			}
//
//			pd.axisStepCountX = xStepsTravelled - backupSteps;
//			pd.axisStepCountY = yStepsTravelled - backupSteps;
//			// TODO: steps are calculated from X axis info, but the length constants don't match up
//			// to actual measurements (ratio is off) so on the Y axis actual print length will be off
//			pd.stepsPerMM = (double) pd.axisStepCountX / (double) pd.axisLengthX;
//			xStepsTravelled = 0;
//			yStepsTravelled = 0;
//			bool xFinished2 = false;
//			bool yFinished2 = false;
//
//			while(!xFinished2 || !yFinished2) {
//				if (xStepsTravelled < pd.axisStepCountX) {
//					driveX(false, dirPinX, pd);
//					xStepsTravelled++;
//				}
//				if (yStepsTravelled < pd.axisStepCountY) {
//					driveY(false, dirPinY, pd);
//					yStepsTravelled++;
//				}
//			}
//
//			pd.absoluteCurrentX = 0.0;
//			pd.absoluteCurrentY = 0.0;
//		}
//	}
//	enablePinInterrupts();
}

/*
 * chooses which command to execute (whether to move motors or servo)
 */
void executeCommand(PlotterData *pd, Servo &penServo) {
	if(pd->gorm == 'G') {
		switch(pd->gormNum) {
			case 1:
				calculateDrive(pd);
				break;
			case 28:
				// home
				break;
		}
	}
	else if(pd->gorm == 'M' && pd->gormNum == 1) {
		switch(pd->targetPen) {
			case 90:
				penServo.penDown();
				break;
			case 130:
				penServo.penUp();
				break;
		}
	}
}




/*
 * "main task" of the plotter.
 */
void taskExecute(void *pvParameters) {
	Handles *commonHandles = (Handles*) pvParameters;
	PlotterData *plotdat = new PlotterData(plotdat->pen, 380, 310);
	Servo penServo(0, 10);
	Parser parsakaali;
//	caribourate(*plotdat);		// rikki
//	GPIO_interrupt_init();
	for(;;){
		std::string *rawCommand;
		xQueueReceive(commonHandles->commandQueue_raw, &rawCommand, portMAX_DELAY);
		parsakaali.generalParse(plotdat, *rawCommand);
		executeCommand(plotdat, penServo);
		delete rawCommand;
		xSemaphoreGive(commonHandles->readyToReceive);
	}
}

/*
 * Calculates necessary values for justDrive()
 */
void calculateDrive(PlotterData *pd) {

	int stepAbsoluteCurrentX = pd->convertToSteps(pd->absoluteCurrentX);
	int stepAbsoluteCurrentY = pd->convertToSteps(pd->absoluteCurrentY);

	int stepAbsoluteTargetX = pd->convertToSteps(pd->absoluteTargetX);
	int stepAbsoluteTargetY = pd->convertToSteps(pd->absoluteTargetY);

	int stepDeltaX = stepAbsoluteTargetX - stepAbsoluteCurrentX;
	int stepDeltaY = stepAbsoluteTargetY - stepAbsoluteCurrentY;

//	double stepsDistance = sqrt((stepDeltaX * stepDeltaX) + (stepDeltaY * stepDeltaY));

	double stepDeltaMax = std::max(abs(stepDeltaX), abs(stepDeltaY));

	double ratioX = (double) abs(stepDeltaX) / stepDeltaMax;
	double ratioY = (double) abs(stepDeltaY) / stepDeltaMax;

	justDrive(pd, stepAbsoluteCurrentX, stepAbsoluteCurrentY, stepAbsoluteTargetX, stepAbsoluteTargetY, stepDeltaX, stepDeltaY, ratioX, ratioY);
}

/*
 * Handles which motor drives, in which direction and how fast.
 */
void justDrive(PlotterData *pd,
		int stepAbsoluteCurrentX, int stepAbsoluteCurrentY,
		int stepAbsoluteTargetX, int stepAbsoluteTargetY,
		int stepDeltaX, int stepDeltaY,
		double ratioX, double ratioY) {

	DigitalIoPin dirPinX(0, 28, DigitalIoPin::output, false);
	DigitalIoPin dirPinY(1, 0, DigitalIoPin::output, false);

	int pulseInterval = 350;
	double countX = 0;
	double countY = 0;

	dirPinX.write(pd->dirX);
	dirPinY.write(pd->dirY);

	while((stepAbsoluteCurrentX != stepAbsoluteTargetX) || (stepAbsoluteCurrentY != stepAbsoluteTargetY)) {

		if(stepAbsoluteCurrentX != stepAbsoluteTargetX){
			countX += ratioX;
			if(countX >= 1) {
				if(stepDeltaX > 0) {
					if(pd->dirX != true) {
						dirPinX.write(true);
						pd->dirX = true;
					}
//					dirPinX.write(true);

					stepAbsoluteCurrentX += 1;
				} else {
					if(pd->dirX != false) {
						dirPinX.write(false);
						pd->dirX = false;
					}
//					dirPinX.write(false);

					stepAbsoluteCurrentX -= 1;
				}

				kumpi = 1;
				RIT_start(1, pulseInterval);
				countX -= 1;
			}
		}

		if(stepAbsoluteCurrentY != stepAbsoluteTargetY){
			countY += ratioY;
			if(countY >= 1) {
				if(stepDeltaY > 0) {
					if(pd->dirY != false) {
						dirPinY.write(false);
						pd->dirY = false;
					}
//					dirPinY.write(false);

					stepAbsoluteCurrentY += 1;
				} else {
					if(pd->dirY != true) {
						dirPinY.write(true);
						pd->dirY = true;
					}
//					dirPinY.write(true);


					stepAbsoluteCurrentY -= 1;
				}

				kumpi = 2;
				RIT_start(1, pulseInterval);
				countY -= 1;
			}
		}
		if(pulseInterval > 100) {
			pulseInterval -= 2;
		}
	}

//	if((stepAbsoluteCurrentX != stepAbsoluteTargetX) && (stepAbsoluteCurrentY != stepAbsoluteTargetY)) {
//		pd->absoluteCurrentX = pd->absoluteTargetX;
//		pd->absoluteCurrentY = pd->absoluteTargetY;
//	} else {
//		while(1);
//	}

	pd->absoluteCurrentX = pd->absoluteTargetX;
	pd->absoluteCurrentY = pd->absoluteTargetY;
	pd->resetCompack();
}

/*
 * Deferred task from RIT_IRQHandler to drive motors
 */
void dtaskMotor(void *pvParameters) {

	DigitalIoPin stepPinX(0, 27, DigitalIoPin::output, false);
	DigitalIoPin stepPinY(0, 24, DigitalIoPin::output, false);

	while(1) {
		xSemaphoreTake(motorSemaphore, (TickType_t) portMAX_DELAY );

		switch(kumpi) {
		case 1:
			stepPinX.write(true);
			stepPinX.write(false);
			break;
		case 2:
			stepPinY.write(true);
			stepPinY.write(false);
			break;
		}
	}
}

