#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "DigitalIoPin.h"
#include "semphr.h"
#include "ITM_write.h"
#include "Motor.h"
#include "myMutex.h"
#include <cstring>

// TODO: insert other definitions and declarations here

//myMutex exampleMutex;
//QueueHandle_t exampleQueue;
//xSemaphoreHandle exampleSemaphore;

/* the following is required if runtime statistics are to be collected */
extern "C" {
	void vConfigureTimerForRunTimeStats( void ) {
		Chip_SCT_Init(LPC_SCTSMALL1);
		LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
		LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
	}
}
/* end runtime statictics collection */

static void prvSetupHardware(void) {
	SystemCoreClockUpdate();
	Board_Init();
}

//struct dippadei {
//	DigitalIoPin *switch1 = new DigitalIoPin(0, 17, DigitalIoPin::pullup, true);
//	DigitalIoPin *switch3 = new DigitalIoPin(1, 9, DigitalIoPin::pullup, true);
//	DigitalIoPin *limitSwitch1 = new DigitalIoPin(0, 27, DigitalIoPin::pullup, true);
//	DigitalIoPin *limitSwitch2 = new DigitalIoPin(0, 28, DigitalIoPin::pullup, true);
//	Motor *mmm = new Motor;
//};


void exampleTask(void *pvParameters) {
//	exampleStruct *p_SRTSRT = (exampleStruct*) pvParameters;

	for(;;) {

	}
}
//xSemaphoreTake(exampleSemaphore, portMAX_DELAY);
//xSemaphoreGive(exampleSemaphore);


int main(void) {
	prvSetupHardware();

//	ITM_init();
//	ITM_write("[Program started]\n");

//	exampleMutex =xSemaphoreCreateMutex();
//	exampleSemaphore = xSemaphoreCreateBinary();
//	exampleQueue = xQueueCreate(/*queueSize*/ 20, sizeof(exampleStruct));

	xTaskCreate(exampleTask, "taskExample", 200, /*&exampleParameter*/ NULL, (tskIDLE_PRIORITY + 1UL), NULL);

	vTaskStartScheduler();

	for(;;);

    return 0 ;
}
