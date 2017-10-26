/*
 * Driver.cpp tri trii tri trii tri trii
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

void RIT_init() {
	Chip_RIT_Init(LPC_RITIMER);
	NVIC_SetPriority(RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3 );
	RIT_count = 0;
	sbRIT = xSemaphoreCreateBinary();
	motorSemaphore = xSemaphoreCreateBinary();
}

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

void driveX (bool dir, bool prevDir, DigitalIoPin &dirPin, PlotterData &pd) {
	if (dir != pd.dirX) {
		if (dir) {
			pd.dirX = true;
		} else {
			pd.dirX = false;
		}
		dirPin.write(pd.dirX);
	}
	kumpi = 1;
	RIT_start(1, 750);
}

/* Drive Y for one step in the negative direction. Y DRIVE VALUES FLIPPED! */
void driveY(bool dir, bool prevDir, DigitalIoPin &dirPin, PlotterData &pd) {
	if (dir != pd.dirY) {
		if (dir) {
			pd.dirY = false;
		} else {
			pd.dirY = true;
		}
		dirPin.write(pd.dirY);
	}
	kumpi = 2;
	RIT_start(1, 750);
}



/* Calibration function that is to be run _before_ GPIO interrupts are initialized! */
void caribourate(PlotterData &pd) {

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

	/* Amount of steps to back up from limit switches. This is calculated in the upcoming loop. */
	int backupSteps = 500;

	/* Do once per axis */
	for (int axisNr = 0; axisNr < 2; axisNr++) {
		/* Drive to zero on each axis */
		while (!ls1.read() && !ls2.read() && !ls3.read() && !ls4.read()) {
			if (axisNr == 0) {
				// X axis
				driveX(false, pd.dirX, dirPinX, pd);
			} else {
				// Y axis
				driveY(false, pd.dirY, dirPinY, pd);
			}
		}

		/* Assign correct switches for the axes */
		if (ls1.read()) {
			if (axisNr == 0) {
				x1 = &ls1;
				x2 = &ls2;
			} else {
				y1 = &ls1;
				y2 = &ls2;
			}
		} else if (ls2.read()) {
			if (axisNr == 0) {
				x1 = &ls2;
				x2 = &ls1;
			} else {
				y1 = &ls2;
				y2 = &ls1;
			}
		} else if (ls3.read()) {
			if (axisNr == 0) {
				x1 = &ls3;
				x2 = &ls4;
			} else {
				y1 = &ls3;
				y2 = &ls4;
			}
		} else if (ls4.read()) {
			if (axisNr == 0) {
				x1 = &ls4;
				x2 = &ls3;
			} else {
				y1 = &ls4;
				y2 = &ls3;
			}
		}

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
		for (int i; i < backupSteps; i++) {
			if (axisNr == 0) {
				driveX(false, pd.dirX, dirPinX, pd);
			} else {
				driveY(true, pd.dirY, dirPinY, pd);
			}
		}
	}
	bool xFinished = false;
	bool yFinished = false;
	/* Calculate amount of steps to the other end of the axes and travel back to origin. */
	while (!xFinished || !yFinished) {

		int xStepsTravelled = 0;
		int yStepsTravelled = 0;

		/* As long as limit switches aren't pressed, drive forward on the X axis */
		if (!x1->read() && !x2->read()) {
			driveX(true, pd.dirX, dirPinX, pd);
			xStepsTravelled++;
		} else {
			xFinished = true;
		}

		/* As long as limit switches aren't pressed, drive forward on the Y axis */
		if (!y1->read() && !y2->read()) {
			driveY(true, pd.dirY, dirPinY, pd);
			yStepsTravelled++;
		} else {
			yFinished = true;
		}

		/* Do this after we are at the positive end of both axes */
		if (xFinished && yFinished) {
			/*  */
			for (int i = 0; i < backupSteps; i++) {
				driveX(false, pd.dirX, dirPinX, pd);
				driveY(false, pd.dirY, dirPinY, pd);
			}

			pd.axisStepCountX = xStepsTravelled - backupSteps;
			pd.axisStepCountY = yStepsTravelled - backupSteps;
			// TODO: steps are calculated from X axis info, but the length constants don't match up
			// to actual measurements (ratio is off) so on the Y axis actual print length will be off
			pd.stepsPerMM = (double) pd.axisStepCountX / (double) pd.axisLengthX;
			xStepsTravelled = 0;
			yStepsTravelled = 0;
			bool xFinished2 = false;
			bool yFinished2 = false;

			while(!xFinished2 || !yFinished2) {
				if (xStepsTravelled < pd.axisStepCountX) {
					driveX(false, pd.dirX, dirPinX, pd);
					xStepsTravelled++;
				}
				if (yStepsTravelled < pd.axisStepCountY) {
					driveY(false, pd.dirY, dirPinY, pd);
					yStepsTravelled++;
				}
			}

			pd.absoluteCurrentX = 0.0;
			pd.absoluteCurrentY = 0.0;
		}
	}
}


void taskExecute(void *pvParameters) {
	Handles *commonHandles = (Handles*) pvParameters;
	PlotterData *plotdat = new PlotterData(plotdat->pen, 380, 310);
	Servo penServo(0, 10);
	Parser parsakaali;



	NVIC_DisableIRQ(PIN_INT0_IRQn);
	NVIC_DisableIRQ(PIN_INT1_IRQn);
	NVIC_DisableIRQ(PIN_INT2_IRQn);
	NVIC_DisableIRQ(PIN_INT3_IRQn);
	NVIC_DisableIRQ(PIN_INT4_IRQn);
	NVIC_DisableIRQ(PIN_INT5_IRQn);

	caribourate(*plotdat);

	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT2_IRQn);
	NVIC_EnableIRQ(PIN_INT3_IRQn);
	NVIC_EnableIRQ(PIN_INT4_IRQn);
	NVIC_EnableIRQ(PIN_INT5_IRQn);
	// caribourate()

	for(;;) {
		std::string *rawCommand;
		xQueueReceive(commonHandles->commandQueue_raw, &rawCommand, portMAX_DELAY);
		parsakaali.generalParse(plotdat, *rawCommand);
		executeCommand(plotdat, penServo);
//		vTaskDelay(5);
//		parsakaali.debug(plotdat, *rawCommand, false);
		delete rawCommand;
		xSemaphoreGive(commonHandles->readyToReceive);

	}
}

void calculateDrive(PlotterData *pd) {

	int stepAbsoluteCurrentX = pd->convertToSteps(pd->absoluteCurrentX);
	int stepAbsoluteCurrentY = pd->convertToSteps(pd->absoluteCurrentY);

	int stepAbsoluteTargetX = pd->convertToSteps(pd->absoluteTargetX);
	int stepAbsoluteTargetY = pd->convertToSteps(pd->absoluteTargetY);

	int stepDeltaX = stepAbsoluteTargetX - stepAbsoluteCurrentX;
	int stepDeltaY = stepAbsoluteTargetY - stepAbsoluteCurrentY;

	double stepDeltaMax = std::max(abs(stepDeltaX), abs(stepDeltaY));

	double ratioX = (double) abs(stepDeltaX) / stepDeltaMax;
	double ratioY = (double) abs(stepDeltaY) / stepDeltaMax;

	justDrive(pd, stepAbsoluteCurrentX, stepAbsoluteCurrentY, stepAbsoluteTargetX, stepAbsoluteTargetY, stepDeltaX, stepDeltaY, ratioX, ratioY);
}

void justDrive(PlotterData *pd,
		int stepAbsoluteCurrentX, int stepAbsoluteCurrentY,
		int stepAbsoluteTargetX, int stepAbsoluteTargetY,
		int stepDeltaX, int stepDeltaY,
		double ratioX, double ratioY) {

	DigitalIoPin dirPinX(0, 28, DigitalIoPin::output, false);
	DigitalIoPin dirPinY(1, 0, DigitalIoPin::output, false);

	int poro = 400;
	double countX = 0;
	double countY = 0;

	bool dirx = true;
	bool diry = true;
//	dirPinX.write(dirx);
//	dirPinY.write(diry);
	dirPinX.write(pd->dirX);
	dirPinY.write(pd->dirY);

	while((stepAbsoluteCurrentX != stepAbsoluteTargetX) || (stepAbsoluteCurrentY != stepAbsoluteTargetY)) {

		if(stepAbsoluteCurrentX != stepAbsoluteTargetX){
			countX += ratioX;
			if(countX >= 1) {
				if(stepDeltaX > 0) {
//					xuunta = true;
//					dirPinX.write(true);
					if(pd->dirX != true) {
						dirPinX.write(true);
						pd->dirX = true;
					}
//					if(dirx != true) {
//						dirPinX.write(true);
//					}
					stepAbsoluteCurrentX += 1;
				} else {
//					xuunta = false;
					if(pd->dirX != false) {
						dirPinX.write(false);
						pd->dirX = false;
					}
//					if(dirx != false) {
//						dirPinX.write(false);
//					}
					stepAbsoluteCurrentX -= 1;
				}

				kumpi = 1;
				RIT_start(1, poro);
				countX -= 1;
			}
		}

		if(stepAbsoluteCurrentY != stepAbsoluteTargetY){
			countY += ratioY;
			if(countY >= 1) {
				if(stepDeltaY > 0) {
//					yuunta = true;
					if(pd->dirY != false) {
						dirPinY.write(false);
						pd->dirY = false;
					}
//					if(diry != false) {
//						dirPinY.write(false);
//					}
					stepAbsoluteCurrentY += 1;
				} else {
//					yuunta = false;
					if(pd->dirY != true) {
						dirPinY.write(true);
						pd->dirY = true;
					}
//					if(diry != true) {
//						dirPinY.write(true);
//					}
					stepAbsoluteCurrentY -= 1;
				}

				kumpi = 2;
				RIT_start(1, poro);
				countY -= 1;
			}
		}
		if(poro > 200) {
			poro -= 3;
		}
	}

//	pd->dirX = dirx;
//	pd->dirY = diry;
	pd->absoluteCurrentX = pd->absoluteTargetX;
	pd->absoluteCurrentY = pd->absoluteTargetY;

	pd->resetCompack();
}


void dtaskMotor(void *pvParameters) {

	DigitalIoPin stepPinX(0, 27, DigitalIoPin::output, false);
//	DigitalIoPin dirPinX(0, 28, DigitalIoPin::output, false);

	DigitalIoPin stepPinY(0, 24, DigitalIoPin::output, false);
//	DigitalIoPin dirPinY(1, 0, DigitalIoPin::output, false);

	while(1) {
		xSemaphoreTake(motorSemaphore, (TickType_t) portMAX_DELAY );

		switch(kumpi) {
		case 1:
//			dirPinX.write(xuunta);
			stepPinX.write(true);
			stepPinX.write(false);
			break;
		case 2:
//			dirPinY.write(!yuunta);
			stepPinY.write(true);
			stepPinY.write(false);
			break;
		}

//		switch(kumpi) {
//		case 1:
////			dirPinX.write(xuunta);
//			stepPinX.write(true);
//			stepPinX.write(false);
//			break;
//		case 2:
////			dirPinY.write(!yuunta);
//			stepPinY.write(true);
//			stepPinY.write(false);
//			break;
//		}



	}
}

