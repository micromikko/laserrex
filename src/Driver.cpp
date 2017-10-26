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
//#include "PlotterData.h"
#include "Parser.h"
#include "Handles.h"
#include "stdlib.h"
#include <cmath>

xSemaphoreHandle sbRIT;
xSemaphoreHandle motorSemaphore;
volatile uint32_t RIT_count;
volatile uint8_t kumpi;
volatile bool xuunta;
volatile bool yuunta;

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
	} else if(pd->gorm == 'M' && pd->gormNum == 1) {
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
//	BaseType_t status;

	// caribourate()
	// init()
	/*
	 * test
	 */
//	std::string *rawCommand = new std::string("G1 X0 Y0 A0");
//	parsakaali.generalParse(plotdat, *rawCommand);
//	executeCommand(plotdat, penServo);
//	delete rawCommand;

//	vTaskDelay(100);

//	std::string *com2 = new std::string("G1 X40 Y80 A0");
//	parsakaali.generalParse(plotdat, *com2);
//	executeCommand(plotdat, penServo);
//	delete com2;
////	vTaskDelay(100);
//
//	std::string *com3 = new std::string("G1 X80 Y0 A0");
//	parsakaali.generalParse(plotdat, *com3);
//	calculateDrive(plotdat);
//	delete com3;
////	vTaskDelay(100);
//
//	std::string *com4 = new std::string("G1 X0 Y0 A0");
//	parsakaali.generalParse(plotdat, *com4);
//	executeCommand(plotdat, penServo);
//	delete com4;

	for(;;) {
		std::string *rawCommand;
		xQueueReceive(commonHandles->commandQueue_raw, &rawCommand, portMAX_DELAY);
		parsakaali.generalParse(plotdat, *rawCommand);
		executeCommand(plotdat, penServo);
//		vTaskDelay(10);
//		parsakaali.debug(plotdat, *rawCommand, false);
		delete rawCommand;
		xSemaphoreGive(commonHandles->readyToReceive);
	}
}

//void taskExecute(void *pvParameters) {
//	Handles *commonHandles = (Handles*) pvParameters;
////	PlotterData plotdat(plotdat.pen, 380, 310);
//	PlotterData *plotdat = new PlotterData(plotdat->pen, 380, 310);
//	Parser parsakaali;
//
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




//	pd->absoluteCurrentX = pd->absoluteTargetX;
//	pd->absoluteCurrentY = pd->absoluteTargetY;
//	pd->resetCompack();
	justDrive(pd, stepAbsoluteCurrentX, stepAbsoluteCurrentY, stepAbsoluteTargetX, stepAbsoluteTargetY, stepDeltaX, stepDeltaY, ratioX, ratioY);
//	pd->absoluteCurrentX = pd->absoluteTargetX;
//	pd->absoluteCurrentY = pd->absoluteTargetY;
}

void justDrive(PlotterData *pd,
		int stepAbsoluteCurrentX, int stepAbsoluteCurrentY,
		int stepAbsoluteTargetX, int stepAbsoluteTargetY,
		int stepDeltaX, int stepDeltaY,
		double ratioX, double ratioY) {

	DigitalIoPin dirPinX(0, 28, DigitalIoPin::output, false);
	DigitalIoPin dirPinY(1, 0, DigitalIoPin::output, false);
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
////			"dMax: %.2f\r\n"
//			"ratioX: %.2f\r\n"
//			"ratioY: %.2f\r\n";
////	memset(commandBuffer, 0, sizeof(commandBuffer));
//	snprintf(commandBuffer, sizeof(commandBuffer), format,
//			pd->absoluteCurrentX,
//			pd->absoluteCurrentY,
//			pd->absoluteTargetX,
//			pd->absoluteTargetY,
//
//			stepDeltaX,
//			stepDeltaY,
////			stepDeltaMax,
//			ratioX,
//			ratioY);
//
//	ITM_write(commandBuffer);
	int poro = 500;
	double countX = 0;
	double countY = 0;

	bool dirx = true;
	bool diry = true;
	dirPinX.write(dirx);
	dirPinY.write(dirx);

	while((stepAbsoluteCurrentX != stepAbsoluteTargetX) || (stepAbsoluteCurrentY != stepAbsoluteTargetY)) {

		if(stepAbsoluteCurrentX != stepAbsoluteTargetX){
			countX += ratioX;

			if(countX >= 1) {
				if(stepDeltaX > 0) {
//					xuunta = true;
					if(dirx != true) {
						dirPinX.write(true);
					}
					stepAbsoluteCurrentX += 1;
				} else {
//					xuunta = false;
					if(dirx != false) {
						dirPinX.write(false);
					}
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
					if(diry != false) {
						dirPinY.write(false);
					}
					stepAbsoluteCurrentY += 1;
				} else {
//					yuunta = false;
					if(diry != true) {
						dirPinY.write(true);
					}
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



//	if(!((pd.absoluteCurrentX == pd.absoluteTargetX) && (pd.absoluteCurrentY = pd.absoluteTargetY))) {
//		/*
//		 * IT'S A TRAP!
//		 */
//		while(1);
//	}

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



	}
}

