/*
 * Driver.h
 *
 *  Created on: 23.10.2017
 *      Author: micromikko
 */

#ifndef DRIVER_H_
#define DRIVER_H_

#include "PlotterData.h"

extern "C" { void RIT_IRQHandler(void); }
void RIT_start(int count, int us);
void RIT_init();

void caribourate(PlotterData &pd);

void taskExecute(void *pvParameters);
void calculateDrive(PlotterData &pd);
void justDrive(PlotterData &pd, int stepDeltaX, int stepDeltaY, double ratioX, double ratioY);
void dtaskMotor(void *pvParameters);


#endif /* DRIVER_H_ */
