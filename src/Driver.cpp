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

void taskExecute(void *pvParameters) {
	Handles *commonHandles = (Handles*) pvParameters;
	PlotterData *plotdat = new PlotterData(plotdat->pen, 380, 310);
	Servo penServo(0, 10);
	Parser parsakaali;

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

