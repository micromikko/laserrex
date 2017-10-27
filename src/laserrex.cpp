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

#include "ITM_write.h"
#include "myMutex.h"
#include "DigitalIoPin.h"

#include "Handles.h"
#include "uart_module.h"
#include "sw_btn_interrupts.h"

#include "PlotterData.h"
#include "Driver.h"
#include "Parser.h"
#include "Servo.h"


/* delete*/
//#include "dtaskMotor.h"
//#include "CommandPacket.h"
//#include "Motor.h"

// TODO: insert other definitions and declarations here

// eventgroup?


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


int main(void) {
	prvSetupHardware();
	ITM_init();

	Handles *commonHandles = new Handles;
	commonHandles->commandQueue_raw = xQueueCreate(1, sizeof(std::string*));
	commonHandles->readyToReceive = xSemaphoreCreateBinary();

	/*
	 * tasks
	 */
	xTaskCreate(taskExecute, "taskExecute", 500, (void*) commonHandles, (tskIDLE_PRIORITY + 1UL), NULL);
	xTaskCreate(taskSendOK, "taskSendOK", 256, (void*) commonHandles, (tskIDLE_PRIORITY + 3UL), NULL);
	
	/*
	 * dtasks
	 */
	xTaskCreate(dtaskUARTReader, "UARTReaderdTask", 500, (void*) commonHandles, (tskIDLE_PRIORITY +2UL), NULL);
	xTaskCreate(dtaskHardStop, "HardStopdTask", 100, NULL, (tskIDLE_PRIORITY + 5UL), NULL); // keep at highest priority!
	xTaskCreate(dtaskMotor, "motordTask", 200, NULL, (tskIDLE_PRIORITY + 3UL), NULL); // keep at highest priority!

	xSemaphoreGive(commonHandles->readyToReceive);			 //This has to be initially available

	UARTModule_init();
	RIT_init();

//	GPIO_interrupt_init();

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

