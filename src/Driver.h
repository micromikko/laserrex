/*
 * Driver.h
 *
 *  Created on: 23.10.2017
 *      Author: micromikko
 */

#ifndef DRIVER_H_
#define DRIVER_H_

void taskExecute(void *pvParameters);
void calculateDrive(CommandPacket &compack, PlotterData &plotdat);
void justDrive();

#endif /* DRIVER_H_ */
