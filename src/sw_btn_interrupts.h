/*
 * sw_btn_interrupts.h
 *
 *  Created on: 20.10.2017
 *      Author: Simo
 */

#ifndef SW_BTN_INTERRUPTS_H_
#define SW_BTN_INTERRUPTS_H_

#include "DigitalIoPin.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"


extern "C" {
void PIN_INT0_IRQHandler(void);
void PIN_INT1_IRQHandler(void);
void PIN_INT2_IRQHandler(void);
void PIN_INT3_IRQHandler(void);
void PIN_INT4_IRQHandler(void);
void PIN_INT5_IRQHandler(void);
}

void dtaskHardStop(void *pvParameters);

void GPIO_interrupt_init(void);

#endif /* SW_BTN_INTERRUPTS_H_ */
