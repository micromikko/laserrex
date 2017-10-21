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
	int targetX;
	int targetY;

};

#endif /* SRC_COMMANDPACKET_H_ */
