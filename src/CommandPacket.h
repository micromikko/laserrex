/*
 * CommandPacket.h
 *
 *  Created on: 20.10.2017
 *      Author: micromikko
 */

#ifndef SRC_COMMANDPACKET_H_
#define SRC_COMMANDPACKET_H_

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
	
};

#endif /* SRC_COMMANDPACKET_H_ */
