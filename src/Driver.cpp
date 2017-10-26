/*
 * Driver.cpp
 *
 *  Created on: 23.10.2017
 *      Author: micromikko
 */

#include "Driver.h"
//#include "PlotterData.h"
//#include "CommandPacket.h"
#include "DigitalIoPin.h"
#include "ITM_write.h"
#include <cstring>


#include "Parser.h"
#include "Handles.h"
#include "stdlib.h"
#include <cmath>
#include "event_groups.h"

xSemaphoreHandle sbRIT;
xSemaphoreHandle motorSemaphore;
volatile uint32_t RIT_count;
EventGroupHandle_t egrp = xEventGroupCreate();

extern "C" {
	void RIT_IRQHandler(void) {
		// This used to check if a context switch is required
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		// Tell timer that we have processed the interrupt.
		// Timer then removes the IRQ until next match occurs
		Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
		if(RIT_count > 0) {
//			char asd = RIT_count + '0';
//			Board_UARTPutSTR("OLLAA IRSSISSA!\r\n");
//			Board_UARTPutChar(asd);
//			Board_UARTPutSTR("\r\n");
			RIT_count--;
			// do something useful here...
			xSemaphoreGiveFromISR( motorSemaphore, &xHigherPriorityTaskWoken );
		} else {
			Chip_RIT_Disable(LPC_RITIMER); // disable timer
			// Give semaphore and set context switch flag if a higher priority task was woken up
			xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityTaskWoken);
		}
		// End the ISR and (possibly) do a context switch
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

void taskExecute(void *pvParameters) {
	Handles *commonHandles = (Handles*) pvParameters;
	PlotterData plotdat;
	Parser parsakaali;

//	BaseType_t status;

	// caribourate()
	// init()
	for(;;) {
		std::string *rawCommand;
		xQueueReceive(commonHandles->commandQueue_raw, &rawCommand, portMAX_DELAY);
		parsakaali.generalParse(plotdat, *rawCommand);
		parsakaali.debug(plotdat, *rawCommand, false);
		calculateDrive(plotdat);

//		parsakaali.debug(*rawCommand, false);		// set true or false to see all info in compack or given command

		delete rawCommand;

		xSemaphoreGive(commonHandles->readyToReceive);
	}
}

//void taskExecute(void *pvParameters) {
//	Handles *commonHandles = (Handles*) pvParameters;
//	PlotterData plotdat(plotdat.pen, 380, 310);
//	Parser parsakaali;
//	std::string *rawCommand = new std::string("G1 X30 Y50 A0");
////	BaseType_t status;
//
////	 caribourate(plotdat);
//	// init()
//	parsakaali.generalParse(plotdat, *rawCommand);
////	parsakaali.debug(plotdat, *rawCommand, false);
//	calculateDrive(plotdat);
//	delete rawCommand;
////	plotdat.resetCompack();
//
//	std::string *com2 = new std::string("G1 X0 Y-20 A0");
//	parsakaali.generalParse(plotdat, *com2);
////	parsakaali.debug(plotdat, *com2, false);
//	calculateDrive(plotdat);
////	plotdat.resetCompack();
//	delete com2;
//
//	std::string *com3 = new std::string("G1 X30 Y0 A0");
//	parsakaali.generalParse(plotdat, *com3);
////	parsakaali.debug(plotdat, *com3, false);
//	calculateDrive(plotdat);
//	delete com3;
////	plotdat.resetCompack();
////	parsakaali.debug(plotdat, *rawCommand, false);		// set true or false to see all info in compack or given command
////	plotdat.resetCompack(); // -.,-.,-.,
////	RIT_count = 0;
////	ITM_write("VALMIS\r\n");
//	for(;;) {
//
////		xQueueReceive(commonHandles->commandQueue_raw, &rawCommand, portMAX_DELAY);
////		parsakaali.generalParse(plotdat, *rawCommand);
////		calculateDrive(plotdat);
////		justDrive(plotdat);
//////		parsakaali.debug(*rawCommand, false);		// set true or false to see all info in compack or given command
////		plotdat.resetCompack(); // -.,-.,-.,
////		RIT_count = 0;
////		xSemaphoreGive(commonHandles->readyToReceive);
//	}
//}

/* Calibration function that is to be run _before_ GPIO interrupts are initialized! */
void caribourate(PlotterData &pd) {

	/* Initialize pins to use in calibration. Also they need to be in
	 * pullup mode for use in GPIO interrupts later, and the DigitalIoPin
	 * constructor also conveniently does that. */
	DigitalIoPin sw1(0, 8, DigitalIoPin::pullup, true); // no use in calibration but needs to be set for GPIO interrupts
	DigitalIoPin sw2(1, 6, DigitalIoPin::pullup, true); // no use in calibration but needs to be set for GPIO interrupts
	DigitalIoPin ls1(0, 29, DigitalIoPin::pullup, true);
	DigitalIoPin ls2(0, 9, DigitalIoPin::pullup, true);
	DigitalIoPin ls3(1, 3, DigitalIoPin::pullup, true);
	DigitalIoPin ls4(0, 0, DigitalIoPin::pullup, true);

	/* Pointers for limit switches */
	DigitalIoPin* x1;
	DigitalIoPin* x2;
	DigitalIoPin* y1;
	DigitalIoPin* y2;

	/* Amount of steps to back up from limit switches. This is calculated in the upcoming loop. */
	int backupSteps = 0;

	/* Do once per axis */
	for (int axisNr = 0; axisNr < 2; axisNr++) {
		/* Drive to zero on each axis */
		while (!ls1.read() && !ls2.read() && !ls3.read() && !ls4.read()) {
			if (axisNr = 0) {
				// X axis
				driveX(false);
			} else {
				// Y axis
				driveY(false);
			}
		}

		/* Assign correct switches for the axes */
		if (ls1.read()) {
			if (axisNr = 0) {
				x1 = ls1;
				x2 = ls2;
			} else {
				y1 = ls1;
				y2 = ls2;
			}
		} else if (ls2.read()) {
			if (axisNr = 0) {
				x1 = ls2;
				x2 = ls1;
			} else {
				y1 = ls2;
				y2 = ls1;
			}
		} else if (ls3.read()) {
			if (axisNr = 0) {
				x1 = ls3;
				x2 = ls4;
			} else {
				y1 = ls3;
				y2 = ls4;
			}
		} else if (ls4.read()) {
			if (axisNr = 0) {
				x1 = ls4;
				x2 = ls3;
			} else {
				y1 = ls4;
				y2 = ls3;
			}
		}

		/* Calculate the amount of steps to back up from the limit switch */
		if (backupSteps = 0) {
			int stepsTravelled = 0;
			while (x1.read() || x2.read()) {
				driveX(true);
				stepsTravelled++;
			}
			backupSteps = (int) ((double) stepsTravelled * (double) 1.5);
		}
		/* Use the calculated amount of backup steps to backup from the Y axis limit switch */
		else {
			for (int i; i < backupSteps; i++) {
				driveY(true);
			}
		}
	}

	/* Calculate amount of steps to the other end of the axes and travel back to origin. */
	while (!xFinished || !yFinished) {
		bool xFinished = false;
		bool yFinished = false;
		int xStepsTravelled = 0;
		int yStepsTravelled = 0;

		/* As long as limit switches aren't pressed, drive forward on the X axis */
		if (!x1.read() && !x2.read()) {
			driveX(true);
			xStepsTravelled++;
		} else {
			xFinished = true;
		}

		/* As long as limit switches aren't pressed, drive forward on the Y axis */
		if (!y1.read() && !y2.read()) {
			driveY(true);
			yStepsTravelled++;
		} else {
			yFinished = true;
		}

		/* Do this after we are at the positive end of both axes */
		if (xFinished && yFinished) {
			/*  */
			for (int i = 0; i < backupSteps; i++) {
				driveX(false);
				driveY(false);
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
					driveX(false);
					xStepsTravelled++;
				}
				if (yStepsTravelled < pd.axisStepCountY) {
					driveY(false);
					yStepsTravelled++;
				}
			}

			pd.absoluteCurrentX = 0.0;
			pd.absoluteCurrentY = 0.0;
		}
	}
}

/* Drive X for one step in the given direction */
void driveX (bool dir) {
	if (dir) {
		xEventGroupSetBits(egrp, BIT_2 | BIT_0);
	} else {
		xEventGroupSetBits(egrp, BIT_2);
	}
	RIT_start(1, 750);
}

/* Drive Y for one step in the negative direction */
void driveY(bool dir) {
	if (dir) {
		xEventGroupSetBits(egrp, BIT_3 | BIT_1);
	} else {
		xEventGroupSetBits(egrp, BIT_3);
	}
	RIT_start(1, 750);
}

void calculateDrive(PlotterData &pd) {

	int stepAbsoluteCurrentX = pd.convertToSteps(pd.absoluteCurrentX);
	int stepAbsoluteCurrentY = pd.convertToSteps(pd.absoluteCurrentY);

	int stepAbsoluteTargetX = pd.convertToSteps(pd.absoluteTargetX);
	int stepAbsoluteTargetY = pd.convertToSteps(pd.absoluteTargetY);

	int stepDeltaX = stepAbsoluteTargetX - stepAbsoluteCurrentX;
	int stepDeltaY = stepAbsoluteTargetY - stepAbsoluteCurrentY;

	double stepDeltaMax = std::max(abs(stepDeltaX), abs(stepDeltaY));

	double ratioX = (double) abs(stepDeltaX) / stepDeltaMax;
	double ratioY = (double) abs(stepDeltaY) / stepDeltaMax;

//	char commandBuffer[400];
//	memset(commandBuffer, 0, sizeof(commandBuffer));
//	const char *format =
//			"curX: %.2f\r\n"
//			"curY: %.2f\r\n"
//			"tarX: %.2f\r\n"
//			"tarY: %.2f\r\n"
//
//			"dA: %d\r\n"
//			"dB: %d\r\n"
//			"dMax: %.2f\r\n"
//			"stepA: %.2f\r\n"
//			"stepB: %.2f\r\n";
//	memset(commandBuffer, 0, sizeof(commandBuffer));
//	snprintf(commandBuffer, sizeof(commandBuffer), format,
//			pd.absoluteCurrentX,
//			pd.absoluteCurrentY,
//			pd.absoluteTargetX,
//			pd.absoluteTargetY,
//
//			stepDeltaX,
//			stepDeltaY,
//			stepDeltaMax,
//			ratioX,
//			ratioX);
//
//	ITM_write(commandBuffer);
//	pd.currentX = pd.targetX;
//	pd.currentY = pd.targetY;

//	pd.absoluteCurrentX = pd.absoluteTargetX;
//	pd.absoluteCurrentY = pd.absoluteTargetY;
//	pd.resetCompack();
	justDrive(pd, stepDeltaX, stepDeltaY, ratioX, ratioY);
}

void justDrive(PlotterData &pd, int stepDeltaX, int stepDeltaY, double ratioX, double ratioY) {

	double countX = 0;
	double countY = 0;

	while((pd.absoluteCurrentX != pd.absoluteTargetX) || (pd.absoluteCurrentY != pd.absoluteTargetY)) {

		if(pd.absoluteCurrentX != pd.absoluteTargetX){
			countX += ratioX;

			if(countX >= 1) {
				if(stepDeltaX > 0) {
//					pd.dirX = true;
					xEventGroupSetBits(egrp, BIT_0 | BIT_2);
					pd.absoluteCurrentX += 1;
				} else {
//					pd.dirX = false;
					xEventGroupSetBits(egrp, BIT_2);
					pd.absoluteCurrentX -= 1;
				}
				RIT_start(10, 1000);
				countX -= 1;
			}
		}

		if(pd.absoluteCurrentY != pd.absoluteTargetY){
			countY += ratioY;

			if(countY >= 1) {
				if(stepDeltaY > 0) {
//					pd.dirX = true;
					xEventGroupSetBits(egrp, BIT_1 | BIT_3);
					pd.absoluteCurrentY += 1;
				} else {
//					pd.dirX = false;
					xEventGroupSetBits(egrp, BIT_3);
					pd.absoluteCurrentY -= 1;
				}
				RIT_start(10, 1000);
				countY -= 1;
			}
		}
	}


//	if(!((pd.absoluteCurrentX == pd.absoluteTargetX) && (pd.absoluteCurrentY = pd.absoluteTargetY))) {
//		/*
//		 * IT'S A TRAP!
//		 */
//		while(1);
//	}

	pd.absoluteCurrentX = pd.absoluteTargetX;
	pd.absoluteCurrentY = pd.absoluteTargetY;

	pd.resetCompack();
}


void dtaskMotor(void *pvParameters) {

	DigitalIoPin stepPinX(0, 27, DigitalIoPin::output, false);
	DigitalIoPin dirPinX(0, 28, DigitalIoPin::output, false);

	DigitalIoPin stepPinY(0, 24, DigitalIoPin::output, false);
	DigitalIoPin dirPinY(1, 0, DigitalIoPin::output, false);

	while(1) {
		xSemaphoreTake(motorSemaphore, (TickType_t) portMAX_DELAY );

		ebits = xEventGroupWaitBits(egrp, allBits, pdTRUE, pdFALSE, configTICK_RATE_HZ / 100);
		if ((ebits & (BIT_2)) == BIT_2) {
			bool dir = ((ebits & BIT_0) == BIT_0);
			dirPinX.write(dir);
			stepPinX.write(true);
			stepPinX.write(false);
		} else  if ((ebits & (BIT_3)) == BIT_3) {
			bool dir = ((ebits & BIT_1) == BIT_1);
			dirPinY.write(dir);
			stepPinY.write(true);
			stepPinY.write(false);
		}



	}
}

