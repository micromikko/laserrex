/*
 * Driver.cpp
 *
 *  Created on: 23.10.2017
 *      Author: micromikko
 */

#include "Driver.h"
#include "PlotterData.h"
#include "CommandPacket.h"
#include "Parser.h"
#include "Handles.h"
#include <cmath>

void taskExecute(void *pvParameters) {
	Handles *commonHandles = (Handles*) pvParameters;
	CommandPacket compack;
	PlotterData plotdat;
	Parser parsakaali;
	std::string *rawCommand;
	BaseType_t status;

	// caribourate()
	// init()
	for(;;) {

		status = xQueueReceive(commonHandles->commandQueue_raw, &rawCommand, portMAX_DELAY);
		CommandPacket cp = parsakaali.generalParse(*rawCommand);



		//		parsakaali.debug(*rawCommand, false);		// set true or false to see all info in compack or given command
		xSemaphoreGive(commonHandles->readyToReceive);
		compack.reset();
	}
}

void calculateDrive() {

}

void justDrive(CommandPacket &compack, PlotterData &plotdat) {
	  float dX = compack.targetX - plotdat.currentX;
	  float dY = compack.targetY - plotdat.currentY;
	  float distance = std::sqrt(dX*dX+dY*dY);
	  if (distance < 0.001)
	    return;

	  tarA = tarX*STEPS_PER_MM;
	  tarB = tarY*STEPS_PER_MM;
	  //Serial.print("tarL:");Serial.print(tarL);Serial.print(' ');Serial.print("tarR:");Serial.println(tarR);
	  //Serial.print("curL:");Serial.print(curL);Serial.print(' ');Serial.print("curR:");Serial.println(curR);
	  //Serial.printf("tar Pos %ld %ld\r\n",tarA,tarB);
	  doMove();
	  curX = tarX;
	  curY = tarY;

}

