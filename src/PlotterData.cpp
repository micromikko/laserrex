/*
 * PlotterData.cpp
 *
 *  Created on: 21.10.2017
 *      Author: micromikko
 */

#include "PlotterData.h"

PlotterData::PlotterData(plotterMode plotmode) {
	this->currentX = 0;
	this->currentY = 0;

	switch(plotterMode) {
		case none:
			this->currentPenPos = 0;
			this->currentLaserPos = 0;
			break;
		case pen:
			this->currentPenPos = 130;
			this->currentLaserPos = 0;
			break;
		case laser:
			this->currentPenPos = 0;
			this->currentLaserPos = 0;
			break;
	}

	this->axisLengthX = 0;
	this->axisLengthY = 0;

	this->stepCountX = 0;
	this->stepCountY = 0;

	this->stepsPerMMX = 0;
	this->stepsPerMMY = 0;
}

PlotterData::PlotterData(plotterMode plotMode, int axisX, int axisY) {
	this->currentX = 0;
	this->currentY = 0;

	switch(plotterMode) {
		case none:
			this->currentPenPos = 0;
			this->currentLaserPos = 0;
			break;
		case pen:
			this->currentPenPos = 130;
			this->currentLaserPos = 0;
			break;
		case laser:
			this->currentPenPos = 0;
			this->currentLaserPos = 0;
			break;
	}

	this->axisLengthX = axisX;
	this->axisLengthY = axisY;

	this->stepCountX;
	this->stepCountY;

	this->stepsPerMMX = 0;
	this->stepsPerMMY = 0;
}

PlotterData::~PlotterData() {
	// TODO Auto-generated destructor stub
}

void PlotterData::setStepsPerMMX() {
	this->stepsPerMMX = this->stepCountX / this->axisLengthX;
}

void PlotterData::setStepsPerMMY() {
	this->stepsPerMMX = this->stepCountY / this->axisLengthY;
}
