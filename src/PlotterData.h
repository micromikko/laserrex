/*
 * PlotterData.h
 *
 *  Created on: 21.10.2017
 *      Author: micromikko
 */

#ifndef SRC_PLOTTERDATA_H_
#define SRC_PLOTTERDATA_H_

class PlotterData {
public:

	enum plotterMode {
		none,
		pen,
		laser
	};

	PlotterData(plotterMode plotMode = none);
	PlotterData(plotterMode plotMode, int axisX, int axisY);

	virtual ~PlotterData();

	int plotterMode;

	double currentX;
	double currentY;

	int currentPenPos;
	int currentLaserPos;

	int axisLengthX;
	int axisLengthY;

	int stepCountX;
	int stepCountY;

	int stepsPerMMX;
	int stepsPerMMY;

	void setStepsPerMMX();
	void setStepsPerMMY();


};

#endif /* SRC_PLOTTERDATA_H_ */

