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

	char gorm;		// G or M
	int gormNum;	// G or M num

	double targetX;	// target X-coordinate
	double targetY;	// target Y-coordinate
	long auxDelay;	// ?????

	int targetPen;
	int targetLaser;


};

#endif /* SRC_COMMANDPACKET_H_ */
