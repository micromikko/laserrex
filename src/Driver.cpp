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
volatile uint8_t motorSelector;

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
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;

	Chip_RIT_Disable(LPC_RITIMER);
	RIT_count = count;
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	Chip_RIT_Enable(LPC_RITIMER);
	NVIC_EnableIRQ(RITIMER_IRQn);
	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		NVIC_DisableIRQ(RITIMER_IRQn);
	} else {
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
		dirPinX.write(false);
		motorSelector = 1;
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
	dirPinX.write(true);
	motorSelector = 1;
	RIT_start(backupSteps, 250);


	while (!ls1.read() && !ls2.read() && !ls3.read() && !ls4.read()) {
		dirPinX.write(true);
		motorSelector = 1;
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

	dirPinX.write(false);
	motorSelector = 1;
	RIT_start(backupSteps, 250);


	pd.axisStepCountX -= backupSteps;

	/*
	 * checking y bottom
	 */
	while (!ls1.read() && !ls2.read() && !ls3.read() && !ls4.read()) {
		dirPinY.write(true);
		motorSelector = 2;
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

	dirPinY.write(false);
	motorSelector = 2;
	RIT_start(backupSteps, 250);


	/*
	 *
	 */
	while (!ls1.read() && !ls2.read() && !ls3.read() && !ls4.read()) {
		dirPinY.write(false);
		motorSelector = 2;
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

	dirPinY.write(true);
	motorSelector = 2;
	RIT_start(backupSteps, 250);


	pd.axisStepCountY -= backupSteps;


	dirPinX.write(false);
	motorSelector = 1;
	RIT_start(pd.axisStepCountX, 250);


	dirPinY.write(true);
	motorSelector = 2;
	RIT_start(pd.axisStepCountY, 250);



	pd.absoluteCurrentX = 0.0;
	pd.absoluteCurrentY = 0.0;

	pd.calculateStepsPerMM();
	enablePinInterrupts();
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

	int stepAbsoluteCurrentX = pd->convertToStepsX(pd->absoluteCurrentX);
	int stepAbsoluteCurrentY = pd->convertToStepsY(pd->absoluteCurrentY);

	int stepAbsoluteTargetX = pd->convertToStepsX(pd->absoluteTargetX);
	int stepAbsoluteTargetY = pd->convertToStepsY(pd->absoluteTargetY);

	int stepDeltaX = stepAbsoluteTargetX - stepAbsoluteCurrentX;
	int stepDeltaY = stepAbsoluteTargetY - stepAbsoluteCurrentY;

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

					stepAbsoluteCurrentX += 1;
				} else {
					if(pd->dirX != false) {
						dirPinX.write(false);
						pd->dirX = false;
					}

					stepAbsoluteCurrentX -= 1;
				}

				motorSelector = 1;
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

					stepAbsoluteCurrentY += 1;
				} else {
					if(pd->dirY != true) {
						dirPinY.write(true);
						pd->dirY = true;
					}


					stepAbsoluteCurrentY -= 1;
				}

				motorSelector = 2;
				RIT_start(1, pulseInterval);
				countY -= 1;
			}
		}
		if(pulseInterval > 100) {
			pulseInterval -= 2;
		}
	}

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

		switch(motorSelector) {
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

