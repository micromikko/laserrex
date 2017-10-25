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
		justDrive(plotdat);
//				parsakaali.debug(*rawCommand, false);		// set true or false to see all info in compack or given command
		plotdat.resetCompack(); // -.,-.,-.,
		delete rawCommand;

		xSemaphoreGive(commonHandles->readyToReceive);
	}
}
void caribourate(PlotterData &pd) {

}

//void taskExecute(void *pvParameters) {
//	Handles *commonHandles = (Handles*) pvParameters;
//	PlotterData plotdat(plotdat.pen, 380, 310);
//	Parser parsakaali;
//	std::string *rawCommand = new std::string("G1 X30 Y30 A0");
//	BaseType_t status;
//
////	 caribourate(plotdat);
//	// init()
//	parsakaali.generalParse(plotdat, *rawCommand);
//	parsakaali.debug(plotdat, *rawCommand, false);
//	calculateDrive(plotdat);
//	justDrive(plotdat);
////	plotdat.resetCompack();
//
//	std::string *com2 = new std::string("G1 X0 Y-20 A0");
//	parsakaali.generalParse(plotdat, *com2);
//	parsakaali.debug(plotdat, *com2, false);
//	calculateDrive(plotdat);
//	justDrive(plotdat);
////	plotdat.resetCompack();
//
//	std::string *com3 = new std::string("G1 X30 Y0 A0");
//	parsakaali.generalParse(plotdat, *com3);
//	parsakaali.debug(plotdat, *com3, false);
//	calculateDrive(plotdat);
//	justDrive(plotdat);
////	plotdat.resetCompack();
////	parsakaali.debug(plotdat, *rawCommand, false);		// set true or false to see all info in compack or given command
////	plotdat.resetCompack(); // -.,-.,-.,
////	RIT_count = 0;
//	ITM_write("VALMIS\r\n");
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

void calculateDrive(PlotterData &pd) {
	ITM_write("HEI HOI\r\n");
	pd.dX = pd.targetX - pd.currentX;
	pd.dY = pd.targetY - pd.currentY;

//	-.,-.,-.,
//	if((sqrt(pd.dX * pd.dX + pd.dY * pd.dY) < 0.001)) {
//		return;
//	}

	pd.currentStepsX = pd.convertToSteps(pd.currentX);
	pd.currentStepsY = pd.convertToSteps(pd.currentY);

	pd.targetStepsX = pd.convertToSteps(pd.targetX);
	pd.targetStepsY = pd.convertToSteps(pd.targetY);
//	pd.targetStepsX = pd.stepsPerMM * pd.targetX;
//	pd.targetStepsY = pd.stepsPerMM * pd.targetY;

	pd.dStepsX = pd.targetStepsX - pd.currentStepsX;
	pd.dStepsY = pd.targetStepsY - pd.currentStepsY;

	pd.dStepsMax = std::max(abs(pd.dStepsX), abs(pd.dStepsY));

	pd.stepIntervalX = (double) abs(pd.dStepsX) / (double) pd.dStepsMax;
	pd.stepIntervalY = (double) abs(pd.dStepsY) / (double) pd.dStepsMax;

//	char commandBuffer[400];
//	memset(commandBuffer, 0, sizeof(commandBuffer));
//	const char *format =
//			"curX: %.2f\r\n"
//			"curY: %.2f\r\n"
//			"tarX: %.2f\r\n"
//			"tarY: %.2f\r\n"
//			"dX: %.2f\r\n"
//			"dY: %.2f\r\n"
//			"curA: %d\r\n"
//			"curB: %d\r\n"
//			"tarA: %ld\r\n"
//			"tarB: %ld\r\n"
//			"dA: %d\r\n"
//			"dB: %d\r\n"
//			"dMax: %.2f\r\n"
//			"stepA: %.2f\r\n"
//			"stepB: %.2f\r\n";
//	memset(commandBuffer, 0, sizeof(commandBuffer));
//	snprintf(commandBuffer, sizeof(commandBuffer), format,
//			pd.currentX,
//			pd.currentY,
//			pd.targetX,
//			pd.targetY,
//			pd.dX,
//			pd.dY,
//			pd.currentStepsX,
//			pd.currentStepsY,
//			pd.targetStepsX,
//			pd.targetStepsY,
//			pd.dStepsX,
//			pd.dStepsY,
//			pd.dStepsMax,
//			pd.stepIntervalX,
//			pd.stepIntervalY);
//	ITM_write(commandBuffer);
//	pd.currentX = pd.targetX;
//	pd.currentY = pd.targetY;
}

void justDrive(PlotterData &pd) {
//	 curX = currentX;
//	 curY = currentY;
//	 tarX = targetX;
//	 tarY = targetY;
//
//	 posA = currentStepsX;
//	 posB = currentStepsY;
//	 tarA = targetStepsX;
//	 tarB = targetStepsY;
//
//	 stepA = stepIntervalX;
//	 stepB = stepIntervalY;
//
//	 cntA = countX;
//	 cntB = countY;

	int countX = 0;
	int countY = 0;

	while((pd.currentStepsX != pd.targetStepsX) || (pd.currentStepsY != pd.targetStepsY)) {
		// move A
		if(pd.currentStepsX != pd.targetStepsX){
			countX += pd.stepIntervalX;

			if(countX >= 1) {
				if(pd.dStepsX > 0) {
//					pd.dirX = true;
					xuunta = true;
				} else {
//					pd.dirX = false;
					xuunta = false;
				}

				if(pd.dStepsX > 0) {
					pd.currentStepsX += 1;
				} else {
					pd.currentStepsX -= 1;
				}
				kumpi = 1;
				RIT_start(1, 750);
				countX -= 1;
			}
		}

		if(pd.currentStepsY != pd.targetStepsY){
			countY += pd.stepIntervalY;

			if(countY >= 1) {
				if(pd.dStepsY > 0) {
//					pd.dirY = true;
					yuunta = true;
				} else {
//					pd.dirY = false;
					yuunta = false;
				}

				if(pd.dStepsY > 0) {
					pd.currentStepsY += 1;
				} else {
					pd.currentStepsY -= 1;
				}
				kumpi = 2;
				RIT_start(1, 750);
				countY -= 1;
			}
		}
		kumpi = 0;
	}
//	for(int i = 0 ;(pd.currentStepsX != pd.targetStepsX)||(pd.currentStepsY != pd.targetStepsY); i++) {
//		// move A
//		if(pd.currentStepsX != pd.targetStepsX){
//			countX += pd.stepIntervalX;
//
//			if(countX >= 1) {
//				if(pd.dStepsX > 0) {
//					pd.dirX = true;
//				} else {
//					pd.dirX = false;
//				}
//
//				if(pd.dStepsX > 0) {
//					pd.currentStepsX += 1;
//				} else {
//					pd.currentStepsX -= 1;
//				}
//				kumpi = 1;
//				RIT_start(1, 1000);
//				countX -= 1;
//			}
//		}
//
//		if(pd.currentStepsY != pd.targetStepsY){
//			countY += pd.stepIntervalY;
//
//			if(countY >= 1) {
//				if(pd.dStepsY > 0) {
//					pd.dirY = true;
//				} else {
//					pd.dirY = false;
//				}
//
//				if(pd.dStepsY > 0) {
//					pd.currentStepsY += 1;
//				} else {
//					pd.currentStepsY -= 1;
//				}
//				kumpi = 1;
//				RIT_start(1, 1000);
//				countX -= 1;
//			}
//		}
//		// move B
////		if(posB!=tarB){
////			cntB+=stepB;
////			if(cntB>=1){
////				d = dB>0?motorBfw:motorBbk;
////				posB+=(dB>0?1:-1);
////				stepperMoveB(d);
////				cntB-=1;
////			}
////		}
//	}

	pd.currentStepsX = pd.targetStepsX;
	pd.currentStepsY = pd.targetStepsY;
	pd.currentX = pd.targetX;
	pd.currentY = pd.targetY;
	pd.resetCompack();
}


void dtaskMotor(void *pvParameters) {

	DigitalIoPin stepPinX(0, 27, DigitalIoPin::output, false);
	DigitalIoPin dirPinX(0, 28, DigitalIoPin::output, false);

	DigitalIoPin stepPinY(0, 24, DigitalIoPin::output, false);
	DigitalIoPin dirPinY(1, 0, DigitalIoPin::output, false);

	while(1) {
		xSemaphoreTake(motorSemaphore, (TickType_t) portMAX_DELAY );

		switch(kumpi) {
		case 1:
			dirPinX.write(xuunta);
			stepPinX.write(true);
			stepPinX.write(false);
			break;
		case 2:
			dirPinY.write(yuunta);
			stepPinY.write(true);
			stepPinY.write(false);
			break;
		}
//		if(kumpi == 1) {
//			stepPinX.write(true);
//			stepPinX.write(false);
//		}
//
//		if(kumpi == 2) {
//			stepPinY.write(true);
//			stepPinY.write(false);
//		}


	}
}



