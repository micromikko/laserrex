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

	// caribourate()


	for(;;) {
//		penServo.penUp();
//		vTaskDelay(500);
//		penServo.penDown();
//		vTaskDelay(500);
//		penServo.penUp();
//		vTaskDelay(500);
//		penServo.penDown();
//		std::string *rawCommand = new std::string("G1 X0 Y0 A0");
//		parsakaali.generalParse(plotdat, *rawCommand);
//		parsakaali.debug(plotdat, *rawCommand, false);
//
//		executeCommand(plotdat, penServo);
//		delete rawCommand;
//
//		vTaskDelay(10);
//
//		std::string *com2 = new std::string("G1 X40 Y80 A0");
//		parsakaali.generalParse(plotdat, *com2);
//		parsakaali.debug(plotdat, *rawCommand, false);
//
//		executeCommand(plotdat, penServo);
//		delete com2;
//		vTaskDelay(10);
//
//		std::string *com3 = new std::string("G1 X80 Y0 A0");
//		parsakaali.generalParse(plotdat, *com3);
//		parsakaali.debug(plotdat, *rawCommand, false);
//
//		calculateDrive(plotdat);
//		delete com3;
//		vTaskDelay(10);
//
//		std::string *com4 = new std::string("G1 X0 Y0 A0");
//		parsakaali.generalParse(plotdat, *com4);
//		parsakaali.debug(plotdat, *rawCommand, false);
//
//		executeCommand(plotdat, penServo);
//		delete com4;
//		vTaskDelay(10);



		std::string *rawCommand;
		xQueueReceive(commonHandles->commandQueue_raw, &rawCommand, portMAX_DELAY);
		parsakaali.generalParse(plotdat, *rawCommand);


//		char commandBuffer[200];
//		memset(commandBuffer, 0, sizeof(commandBuffer));
//
//		const char *format = "gorm: %c\r\ngormNum: %d\r\ncurX: %.2f\r\ncurY: %.2f\r\ntarX: %.2f\r\ntarY: %.2f\r\naux: %d\r\ntargetPen: %d\r\ntargetLaser: %d\r\n";
//		snprintf(commandBuffer, sizeof(commandBuffer), format, plotdat->gorm, plotdat->gormNum,
//				plotdat->absoluteCurrentX, plotdat->absoluteCurrentY, plotdat->absoluteTargetX, plotdat->absoluteTargetY,  plotdat->auxDelay, plotdat->targetPen, plotdat->targetLaser);
//		ITM_write(commandBuffer);
//		parsakaali.debug(plotdat, *rawCommand, false);

		executeCommand(plotdat, penServo);
//		vTaskDelay(5);
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

	int poro = 350;
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
				RIT_start(1, poro);
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
				RIT_start(1, poro);
				countY -= 1;
			}
		}
		if(poro > 100) {
			poro -= 2;
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

