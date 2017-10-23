///*
// * Motor.cpp
// *
// *  Created on: 12.9.2017
// *      Author: micromikko
// */
//
//#include "Motor.h"
//#include "dtaskMotor.h"
//
////#include "DigitalIoPin.h"
////#include "task.h"
//
//extern "C" {
//	void RIT_IRQHandler(void) {
//		// This used to check if a context switch is required
//		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
//		// Tell timer that we have processed the interrupt.
//		// Timer then removes the IRQ until next match occurs
//		Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
//		if(RIT_count > 0) {
////			char asd = RIT_count + '0';
////			Board_UARTPutSTR("OLLAA IRSSISSA!\r\n");
////			Board_UARTPutChar(asd);
////			Board_UARTPutSTR("\r\n");
//			RIT_count--;
//			// do something useful here...
//			xSemaphoreGiveFromISR( motorSemaphore, &xHigherPriorityTaskWoken );
//		} else {
//			Chip_RIT_Disable(LPC_RITIMER); // disable timer
//			// Give semaphore and set context switch flag if a higher priority task was woken up
//			xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityTaskWoken);
//		}
//		// End the ISR and (possibly) do a context switch
//		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
//	}
//}
//
//Motor::Motor() {
//	this->stepPinX = new DigitalIoPin(0, 27, DigitalIoPin::output, false);
//	this->dirPinX = new DigitalIoPin(0, 28, DigitalIoPin::output, false);
//
//	this->stepPinY = new DigitalIoPin(0, 24, DigitalIoPin::output, false);
//	this->dirPinY = new DigitalIoPin(1, 0, DigitalIoPin::output, false);
//
//	this->ritCountX = 0;
//	this->ritCountY = 0;
//	this->dirX = true;
//	this->dirY = true;
//}
//
//Motor::~Motor() {
//	// TODO Auto-generated destructor stub
//	delete stepPinX;
//	delete dirPinX;
//	delete stepPinY;
//	delete dirPinY;
//}
//
////void Motor::drive() {
////	switch(this->motorAxis) {
////	case 'X':
////		driveX();
////		break;
////	case 'Y':
////		driveY();
////		break;
////	}
////}
////
////void Motor::driveX() {
////	this->dirPinX->write(dirX);
////	this->stepPinX->write(true);
////	this->stepPinX->write(false);
////}
////
////void Motor::driveY() {
////	this->dirPinY->write(dirY);
////	this->stepPinY->write(true);
////	this->stepPinY->write(false);
////}
////
////bool Motor::changeDir(char ) {
////	this->dirPin->write(!dir);
////}
////
////int Motor::stop() {
////	this->stepPin->write(false);
////}
//
//
//
//void RIT_start(int count, int us) {
//	uint64_t cmp_value;
//	// Determine approximate compare value based on clock rate and passed interval
//	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
//
//	// disable timer during configuration
//	Chip_RIT_Disable(LPC_RITIMER);
//	RIT_count = count;
//	// enable automatic clear on when compare value==timer value
//	// this makes interrupts trigger periodically
//	Chip_RIT_EnableCompClear(LPC_RITIMER);
//	// reset the counter
//	Chip_RIT_SetCounter(LPC_RITIMER, 0);
//	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
//	// start counting
//	Chip_RIT_Enable(LPC_RITIMER);
//	// Enable the interrupt signal in NVIC (the interrupt controller)
//	NVIC_EnableIRQ(RITIMER_IRQn);
//	// wait for ISR to tell that we're done
//	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
//		// Disable the interrupt signal in NVIC (the interrupt controller)
//		NVIC_DisableIRQ(RITIMER_IRQn);
////		Board_UARTPutSTR("SEMAFOORI OTETTU\r\n");
//	} else {
//		// unexpected error
//	}
//}
