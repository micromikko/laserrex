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

//	PlotterData(KaksiUlotteisenLaserpiirtoLaitteentoimintamoodI kayYouElElEye=none);
	PlotterData(KaksiUlotteisenLaserpiirtoLaitteentoimintamoodI kayYouElElEye = none, int axisX=380, int axisY=310);

	virtual ~PlotterData();

	int plotterMode;

	bool dirX;
	bool dirY;

	int axisLengthX;		// mm
	int axisLengthY;		// mm

	int stepdelay_min;		//=200;
	int stepdelay_max;		//=1000;

	/*
	 * Set during caribouration
	 */
	int axisStepCountX;		// steps
	int axisStepCountY;		// steps


	double stepsPerMM;			// #define STEPS_PER_MM 87.58
//	double stepsPerMMX;
//	double stepsPerMMY;
	/*
	 * Current
	 */
	double absoluteCurrentX;		// mm
	double absoluteCurrentY;		// mm

	int currentPenPos;				// pwm
	int currentLaserPos;			// pwm

	/*
	 * CommandPacket
	 */
	char gorm;				// G or M
	int gormNum;			// G: 1, 28; M: 1, 4, 10

	/*
	 * compack
	 */
	double absoluteTargetX;			// mm
	double absoluteTargetY;			// mm

	int targetPen;			// pwm
	int targetLaser;		// pwm

	long auxDelay;			// us

	void calculateStepsPerMM();
	int convertToSteps(const double before);
//	int convertToStepsX(const double before);
//	int convertToStepsY(const double before);

	void resetCompack();
private:

};

#endif /* PLOTTERDATA_H_ */

