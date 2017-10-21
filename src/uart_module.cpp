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


QueueHandle_t str_queue;
QueueHandle_t char_queue;
myMutex serial_guard;


extern "C"
{
void UART0_IRQHandler(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint32_t c;
	{
	std::lock_guard<myMutex> locker(serial_guard);
	c = LPC_USART0->RXDATA;
	}

	xQueueSendToBackFromISR(char_queue, &c, &xHigherPriorityTaskWoken );
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
}


/*Create queues for messaging characters from UART interrupt to reader task.  Configure UART interrupts.*/
void UARTModule_init() {
	configUARTInterrupt();
	ITM_init();

	str_queue = xQueueCreate(10, sizeof(std::string*));
	char_queue = xQueueCreate(1, sizeof(uint32_t));
}


static void configUARTInterrupt() {
	uint32_t mask = (1 << 0);
	Chip_UART_IntEnable(LPC_USART0, mask);
	NVIC_SetPriority(UART0_IRQn, 7);
	NVIC_EnableIRQ(UART0_IRQn);
}

void dtaskUARTReader(void *pvParameters) {

	while(1) {
		std::string *str = new std::string("");
		uint32_t c;

		while (1) {

			xQueueReceive(char_queue, &c, portMAX_DELAY);

			(*str).push_back((char)c);

			if(c == '\n' || c == '\r') {
				/*Prevent race conditions to the COM port by taking a mutex */
				{
				std::lock_guard<myMutex> locker(serial_guard);
				Board_UARTPutSTR("OK\n");
				}
				BaseType_t status = xQueueSendToBack(str_queue, &str, portMAX_DELAY);
				break;
			}
		}
	}
}

void taskPrinter(void *pvParameters) {
	std::string *str;
	while(1) {
		xQueueReceive(str_queue, &str, portMAX_DELAY);
		ITM_write( (*str).c_str() );
		delete str;
	}
}



