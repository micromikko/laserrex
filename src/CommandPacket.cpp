/*
 * CommandPacket.cpp
 *
 *  Created on: 20.10.2017
 *      Author: micromikko
 */

/*
 * hih hii, kutittaa
 */

#include "CommandPacket.h"

CommandPacket::CommandPacket() {
	this->gorm = 0;
	this->gormNum = 0;

	this->targetX = 0;
	this->targetY = 0;
	this->auxDelay = 0;

	this->targetPen = 0;
	this->targetLaser = 0;

}

CommandPacket::~CommandPacket() {
	// TODO Auto-generated destructor stub
}

void CommandPacket::reset() {
	this->gorm = 0;
	this->gormNum = 0;

	this->targetX = 0;
	this->targetY = 0;
	this->auxDelay = 0;

	this->targetPen = 0;
	this->targetLaser = 0;
}

