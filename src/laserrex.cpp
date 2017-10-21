#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#include <string>
#include <cstring>
#include <cstdlib>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"




#include "DigitalIoPin.h"
#include "myMutex.h"
#include "ITM_write.h"

#include "uart_module.h"
#include "Motor.h"
#include "Parser.h"
#include "CommandPacket.h"


// TODO: insert other definitions and declarations here

/*
myMutex exampleMutex;
QueueHandle_t exampleQueue;
xSemaphoreHandle exampleSemaphore;
// eventgroup?
*/

QueueHandle_t commandQueue;

extern "C" {
	void vConfigureTimerForRunTimeStats( void ) {
		Chip_SCT_Init(LPC_SCTSMALL1);
		LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
		LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
	}
}

static void prvSetupHardware(void) {
	SystemCoreClockUpdate();
	Board_Init();
}



void taskExecute(void *pvParameters) {
	Parser parsakaali;
	const char *debugCommand = "G28\r\n";		//enter gcode
	parsakaali.debug(debugCommand, true);		// set true or false to see all info in compack or given command
	for(;;) {
//		status = xQueueSendToFront(commandQueue, &commandBuffer, 0);

		/*
		 * read command from queue
		 * parse command and assign parts to compack
		 * pass info from compack to motors, pen or laser
		 */
	}
}

void dtaskLimit(void *pvParameters) {

	for(;;) {
		
	}
}

void dtaskButton(void *pvParameters) {
	
	for(;;) {
	}
}

int main(void) {
	prvSetupHardware();
	ITM_init();

	/*
	 * tasks
	 */
	xTaskCreate(taskExecute, "taskExecute", 200, /*&exampleParameter*/ NULL, (tskIDLE_PRIORITY + 1UL), NULL);
	
	/*
	 * dtasks
	 */
//	xTaskCreate(dtaskLimit, "dtaskLimit", 100, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
//	xTaskCreate(dtaskButton, "dtaskButton", 100, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
//	xTaskCreate(dtaskUARTReader, "dtaskUARTReader", 256, NULL, (tskIDLE_PRIORITY +2UL), NULL);
//	xTaskCreate(taskPrinter, "taskPrinter", 256, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
	
//	UARTModule_init();

	vTaskStartScheduler();

	for(;;);

    return 0 ;
}

/*
exampleMutex =xSemaphoreCreateMutex();
exampleSemaphore = xSemaphoreCreateBinary();
xTaskCreate(exampleTask, "taskExample", 200, &exampleParameter NULL, (tskIDLE_PRIORITY + 1UL), NULL);
*/

/*
 * PINS TO USE
 */
// [on board]	[port_pin]		[description]
// D6			1_3				Limit Y Min
// D7			0_0				Limit Y Max
// D3			0_9				Limit X Max
// D2			0_29			Limit X Min
// D12			0_12			Laser
// D4			0_10			Pen
// D10			0_27			XMotor
// D11			0_28			Xmotor Direction
// D8			0_24			YMotor
// D9			1_0				YMotor Direction
// A0			0_8				SW1
// A1			1_6				SW2
// A2			1_8				SW3

// Note:
// - Limit switches pins should be inverted (by default they are high)
// - LASER PIN IS BY DEFAULT HIGH which powers on the laser at maximum power. Driver the pin low right after your program starts

/*
struct exampleStruct {
	DigitalIoPin *switch1 = new DigitalIoPin(0, 17, DigitalIoPin::pullup, true);
	DigitalIoPin *switch3 = new DigitalIoPin(1, 9, DigitalIoPin::pullup, true);
	DigitalIoPin *limitSwitch1 = new DigitalIoPin(0, 27, DigitalIoPin::pullup, true);
	DigitalIoPin *limitSwitch2 = new DigitalIoPin(0, 28, DigitalIoPin::pullup, true);
	Motor *mmm = new Motor;
};
*/

/*
void exampleTask(void *pvParameters) {
	exampleStruct *p_SRTSRT = (exampleStruct*) pvParameters;
	xSemaphoreTake(exampleSemaphore, portMAX_DELAY);
	xSemaphoreGive(exampleSemaphore);
	for(;;) {

	}
}
*/

