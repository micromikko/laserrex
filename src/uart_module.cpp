/*
 * uart_module.cpp
 *
 *  Created on: Oct 21, 2017
 *      Author: Tuomas-laptop
 */
#include "uart_module.h"

#include "FreeRTOS.h"
#include "task.h"
#include <string>
#include "ITM_write.h"
#include <mutex>
#include "Handles.h"


QueueHandle_t char_queue;
myMutex serial_guard;

#define INT_MASK  (uint32_t)(1 << 0)



extern "C"
{
void UART0_IRQHandler(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint32_t c;
//	{
//	std::lock_guard<myMutex> locker(serial_guard);
	c = LPC_USART0->RXDATA;
//	}

	xQueueSendToBackFromISR(char_queue, &c, &xHigherPriorityTaskWoken );
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
	Chip_UART_IntDisable(LPC_USART0, INT_MASK);
}
}


/*Run UART interrupt configuration and create char queue for messaging chars to reader task from ISR .*/
void UARTModule_init() {
	configUARTInterrupt();

	char_queue = xQueueCreate(1, sizeof(uint32_t));
}


static void configUARTInterrupt() {
	Chip_UART_IntEnable(LPC_USART0, INT_MASK);
	/* I added the macro here for safety so we don't interfere with system interrupts
	 * - Simo */
	NVIC_SetPriority(UART0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2);
	NVIC_EnableIRQ(UART0_IRQn);
}

void dtaskUARTReader(void *pvParameters) {
	Handles *commonHandles = (Handles*) pvParameters;

	while(1) {
		std::string *str = new std::string("");
		uint32_t c;

		while (1) {
			//Sit here waiting for UART interrupt to fire and give us a character
			xQueueReceive(char_queue, &c, portMAX_DELAY);
			
			if(c == '\n' || c == '\r') {
				//We have received the whole command, send it to parser queue
				xQueueSendToBack(commonHandles->commandQueue_raw, &str, portMAX_DELAY);
				Chip_UART_IntEnable(LPC_USART0, INT_MASK);
				break;
			}
			else {
				(*str).push_back((char)c);
				Chip_UART_IntEnable(LPC_USART0, INT_MASK);
			}
		}
	}
}



void taskSendOK(void *pvParameters) {
	Handles *commonHandles = (Handles*) pvParameters;
	while(1) {

		if( xSemaphoreTake(commonHandles->readyToReceive, portMAX_DELAY)  == pdTRUE ) {
			/*We are done processing the previous command, send "OK" to mDraw to get new instruction*/
//			{
			/*Take mutex to prevent race conditions when writing to COM port*/
//			std::lock_guard<myMutex> locker(serial_guard);
			Board_UARTPutSTR("OK\r\n");
//			}
			/*Simulate delay caused by operating motors etc.*/
//			vTaskDelay(5); // -.,-.,-.,
		}
	}
}




