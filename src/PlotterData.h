/*
 * PlotterData.h
 *
 *  Created on: 21.10.2017
 *      Author: micromikko
 */

/*
 * the very newest of tests
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

	int axisLengthX;		// mm
	int axisLengthY;		// mm

	int stepdelay_min;		//=200;
	int stepdelay_max;		//=1000;

	/*
	 * Set during caribouration
	 */
	int axisStepCountX;		// steps
	int axisStepCountY;		// steps

	int stepsPerMMX;		// steps
	int stepsPerMMY;		// steps

	/*
	 * Current
	 */
	double currentX;		// mm
	double currentY;		// mm

	int currentStepsX;		// steps
	int currentStepsY;		// steps

	int currentPenPos;		// pwm
	int currentLaserPos;	// pwm

	/*
	 * CommandPacket
	 */
	char gorm;				// G or M
	int gormNum;			// G: 1, 28; M: 1, 4, 10

	double targetX;			// mm
	double targetY;			// mm

	double targetStepsX;	// steps
	double targetStepsY;	// steps

	int targetPen;			// pwm
	int targetLaser;		// pwm

	long auxDelay;			// us

	double dX;				// mm
	double dY;				// mm

	int dStepsX;			// steps
	int dStepsY;			// steps

	void calculateStepsPerMMX();
	void calculateStepsPerMMY();
	void convertToSteps(double mm);

//	CommandPacket *compack;
	void resetCompack();
private:

};

#endif /* PLOTTERDATA_H_ */

