/*
 * PlotterData.cpp
 *
 *  Created on: 21.10.2017
 *      Author: micromikko
 */

#include "PlotterData.h"


PlotterData::PlotterData(KaksiUlotteisenLaserpiirtoLaitteentoimintamoodI kayYouElElEye, int axisX, int axisY) {
	switch(kayYouElElEye) {
		case none:
			this->currentPenPos = 90;	// -.,-.,-.,
			this->currentLaserPos = 0;
			break;
		case pen:
			this->currentPenPos = 130;
			this->currentLaserPos = 0;
			break;
		case laser:
			this->currentPenPos = 130;
			this->currentLaserPos = 0;
			break;
	}

	this->axisLengthX = axisX;		// mm
	this->axisLengthY = axisY;		// mm

	this->stepdelay_min = 0;		//=200;
	this->stepdelay_max = 0;		//=1000;

	/*
	 * Set during caribouration
	 */
	this->axisStepCountX = 0;		// steps
	this->axisStepCountY = 0;		// steps

	// #define STEPS_PER_MM 87.58
	// 87.58
	this->stepsPerMM = 87.58; // -.,-.,-.,

	/*
	 * Current
	 */
	this->absoluteCurrentX = 0;		// mm
	this->absoluteCurrentY = 0;		// mm

	/*
	 * CommandPacket
	 */
	this->gorm = 0;				// G or M
	this->gormNum = 0;			// G: 1, 28; M: 1, 4, 10

	this->absoluteTargetX = 0;			// mm
	this->absoluteTargetY = 0;			// mm

	this->targetPen = 0;			// pwm
	this->targetLaser = 0;		// pwm

	this->dirX = true;
	this->dirY = true;
}

PlotterData::~PlotterData() {
	// TODO Auto-generated destructor stub
}

void PlotterData::calculateStepsPerMM() {
	int stepsPerMMX = this->axisStepCountX / this->axisLengthX;
	int stepsPerMMY = this->axisStepCountY / this->axisLengthY;

	if(stepsPerMMX == stepsPerMMY) {
		this->stepsPerMM = stepsPerMMX;
	} else {
		while(1);
	}
}

void PlotterData::resetCompack() {
	this->gorm = 0;
	this->gormNum = 0;

	this->absoluteTargetX = 0;
	this->absoluteTargetY = 0;

	this->auxDelay = 0;

	this->targetPen = 0;
	this->targetLaser = 0;
}

int PlotterData::convertToSteps(const double before) {
//	int jaska = (int) (before * this->stepsPerMM);
//	return jaska;
	return (int) (before * this->stepsPerMM);
}

