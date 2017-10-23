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
#include "sw_btn_interrupts.h"
#include "Motor.h"
#include "dtaskMotor.h"
#include "Parser.h"
#include "CommandPacket.h"
#include "PlotterData.h"


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
	UARTModule_init();
	GPIO_interrupt_init();
}



void taskExecute(void *pvParameters) {
//	PlotterData plotdat(pen);		// init()
//	CommandPacket compack;
//	Parser parsakaali;
//	BaseType_t status;
//	caribourate(plotdat);
//	std::string commandBuffer;
//
//	for(;;) {
//		status = xQueueReceive(commandQueue, &commandBuffer, portMAX_DELAY);
//		compack = parsakaali.generalParse(commandBuffer);
//		if(move) {
//			calculate movement
//
//			do movement usin
//		}
//
//		/*
//		 * read command from queue
//		 * parse command and assign parts to compack
//		 * pass info from compack to motors, pen or laser
//		 */
//		compack.reset();
	}
}

int main(void) {
	prvSetupHardware();

	/*
	 * tasks
	 */
	xTaskCreate(taskExecute, "taskExecute", 200, /*&exampleParameter*/ NULL, (tskIDLE_PRIORITY + 1UL), NULL);
	
	/*
	 * dtasks
	 */
//	xTaskCreate(dtaskLimit, "dtaskLimit", 100, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
//	xTaskCreate(dtaskButton, "dtaskButton", 100, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
	xTaskCreate(dtaskUARTReader, "dtaskUARTReader", 256, NULL, (tskIDLE_PRIORITY +2UL), NULL);		
	xTaskCreate(taskPrinter, "taskPrinter", 256, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
	xTaskCreate(dtaskHardStop, "HardStopTask", 100, NULL, (tskIDLE_PRIORITY + 4UL), NULL); // keep at highest priority!
	

//	xTaskCreate(dtaskUARTReader, "dtaskUARTReader", 256, NULL, (tskIDLE_PRIORITY +2UL), NULL);
//	xTaskCreate(taskPrinter, "taskPrinter", 256, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
	
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

