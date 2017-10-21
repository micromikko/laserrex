/*
 * PlotterData.h
 *
 *  Created on: 21.10.2017
 *      Author: micromikko
 */

#ifndef PLOTTERDATA_H_
#define PLOTTERDATA_H_


class PlotterData {
public:

	enum KaksiUlotteisenLaserpiirtoLaitteentoimintamoodI {
		none,
		pen,
		laser
	};

	PlotterData(KaksiUlotteisenLaserpiirtoLaitteentoimintamoodI kayYouElElEye=none);
	PlotterData(KaksiUlotteisenLaserpiirtoLaitteentoimintamoodI kayYouElElEye, int axisX, int axisY);
//	PlotterData(int jaska);
//	PlotterData(int jaska, int axisX, int axisY);
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

private:

};

#endif /* PLOTTERDATA_H_ */

