/*
 * functions.cpp
 *
 *  Created on: 21.10.2017
 *      Author: Simo
 */

#include "functions.h"
#include "PlotterData.h"
#include "Motor.h"
#include "DigitalIoPin.h"

void calibrate(PlotterData* p_data, Motor motor) {
	DigitalIoPin xLim1(0, 29, DigitalIoPin::pullup, true);
	DigitalIoPin xLim2(0, 9, DigitalIoPin::pullup, true);
	DigitalIoPin yLim1(1, 3, DigitalIoPin::pullup, true);
	DigitalIoPin yLim2(0, 0, DigitalIoPin::pullup, true);

	const int trips = 3; // how many trips to do for accuracy
	const int backUpStepAmount = 20; // TODO: this might need to be adjusted for proper operation on actual plotter

	bool axis = false; // quick faux enumerator for axis selection: false is X, true is Y
	bool dir = true; // stepper direction

	int stepCount = 0; // step count on single trip
	int sum = 0; // total sum of all trips on one axis

	// do for both axes
	for (int axes = 0; axes < 2; axes++) {

		// do several trips for improved accuracy
		for (int trip = 0; trip < trips; trip++) {

			// do while no limit switch is triggered
			while (!xLim1.read() && !xLim2.read() && !yLim1.read() && !yLim2.read()) {

				// check for which axis we're on, drive according motor
				if (!axis) {
					motor.driveX(dir); // pass direction as parameter?
				} else {
					motor.driveY(dir); // pass direction as parameter?
				}

				// increment step count
				stepCount++;
			}

			// change direction
			dir = !dir;

			// back up a little to get off the limit switch
			for (int backUp = 0; backup < backUpStepAmount; backUp++) {

				// check for which axis we're on, drive according motor
				if (!axis) {
					motor.driveX(dir);
				} else {
					motor.drivey(dir);
				}
			}

			// save trip step count to sum total (minus backup steps)
			sum += stepCount - backUpStepAmount;


		}


		// change axis to be calibrated, save data and reset numbers
		if (!axis) {
			axis = true;
			p_data->stepCountX = sum / trips;
			stepCount = 0;
			sum = 0;
			dir = true;
		} else {
			p_data->stepCountY = sum / trips;
		}
	}

}


