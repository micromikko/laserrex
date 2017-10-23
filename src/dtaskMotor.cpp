/*
 * dtaskMotor.cpp
 *
 *  Created on: 21.10.2017
 *      Author: micromikko
 */

#include "dtaskMotor.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "DigitalIoPin.h"
#include "Motor.h"


xSemaphoreHandle sbRIT;
xSemaphoreHandle motorSemaphore;
volatile uint32_t RIT_count;

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

void dtaskMotor(void *pvParameters) {
	DigitalIoPin stepPinX(0, 27, DigitalIoPin::output, false);
	DigitalIoPin dirPinX(0, 28, DigitalIoPin::output, false);

	DigitalIoPin stepPinY(0, 24, DigitalIoPin::output, false);
	DigitalIoPin dirPinY(1, 0, DigitalIoPin::output, false);


	while(1) {
		xSemaphoreTake(motorSemaphore, (TickType_t) portMAX_DELAY );

//		read event flag
//		if(bling, then x, if blong, then y)
//		if äx
//			stepPinX.write(true);
//			stepPinX.write(false);
//		if yy
//			stepPinY.write(true);
//			stepPinY.write(false);

	}
}

void dtaskMotor_init() {
	Chip_RIT_Init(LPC_RITIMER);
	NVIC_SetPriority(RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3 );
	sbRIT = xSemaphoreCreateBinary();
	motorSemaphore = xSemaphoreCreateBinary();
}

