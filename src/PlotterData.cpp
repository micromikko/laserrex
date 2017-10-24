/*
 * PlotterData.cpp
 *
 *  Created on: 21.10.2017
 *      Author: micromikko
 */

#include "PlotterData.h"



PlotterData::PlotterData(KaksiUlotteisenLaserpiirtoLaitteentoimintamoodI kayYouElElEye) {
	this->currentX = 0;
	this->currentY = 0;

	switch(kayYouElElEye) {
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

	this->axisLengthX = 380;
	this->axisLengthY = 310;

	this->stepCountX = 0;
	this->stepCountY = 0;

	this->stepsPerMMX = 0;
	this->stepsPerMMY = 0;
}

PlotterData::PlotterData(KaksiUlotteisenLaserpiirtoLaitteentoimintamoodI kayYouElElEye, int axisX, int axisY) {
	this->currentX = 0;
	this->currentY = 0;

	switch(kayYouElElEye) {
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

	this->stepCountX = 0;
	this->stepCountY = 0;

	this->stepsPerMMX = 0;
	this->stepsPerMMY = 0;
}

PlotterData::~PlotterData() {
	// TODO Auto-generated destructor stub
}

void PlotterData::calculateStepsPerMMX() {
	this->stepsPerMMX = this->stepCountX / this->axisLengthX;
}

void PlotterData::calculateStepsPerMMY() {
	this->stepsPerMMX = this->stepCountY / this->axisLengthY;
}

void PlotterData::resetCompack() {
	this->gorm = 0;
	this->gormNum = 0;

	this->targetX = 0;
	this->targetY = 0;
	this->auxDelay = 0;

	this->targetPen = 0;
	this->targetLaser = 0;
}
