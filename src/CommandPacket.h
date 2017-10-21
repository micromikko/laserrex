/*
 * CommandPacket.h
 *
 *  Created on: 20.10.2017
 *      Author: micromikko
 */

#ifndef COMMANDPACKET_H_
#define COMMANDPACKET_H_

class CommandPacket {
public:
	CommandPacket();
	virtual ~CommandPacket();

	char gorm;
	int gormNum;

	double targetX;
	double targetY;
	long auxDelay;

	int targetPen;
	int targetLaser;
	
	void reset();

};

#endif /* COMMANDPACKET_H_ */
