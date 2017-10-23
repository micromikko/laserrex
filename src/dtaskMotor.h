/*
 * dtaskMotor.h
 *
 *  Created on: 21.10.2017
 *      Author: micromikko
 */

#ifndef DTASKMOTOR_H_
#define DTASKMOTOR_H_

//#include "FreeRTOS.h"
//#include "semphr.h"
//
//xSemaphoreHandle motorSemaphore;

//extern "C" { void UART0_IRQHandler(void); }
extern "C" { void RIT_IRQHandler(void); }
void RIT_start(int count, int us);
void dtaskMotor(void *pvParameters);
void dtaskMotor_init();

#endif /* DTASKMOTOR_H_ */
