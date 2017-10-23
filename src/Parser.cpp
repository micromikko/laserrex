/*
 * Parser.cpp
 *
 *  Created on: 20.10.2017
 *      Author: micromikko
 */

#include "Parser.h"
#include <cstring>
#include <stdlib.h>
//#include "Motor.h"		// ???
#include <cstdio>			// DEBUG
#include "ITM_write.h"		// DEBUG

Parser::Parser() {
	 this->compack = new CommandPacket();

}

Parser::~Parser() {
	delete compack;
}

void Parser::m10Parse(char *c_comstr) {
	/*
	 * -.,-.,-., printtaa vaan initialization juttuja.
	 */
}

void Parser::laserParse(char *c_comstr) {
	this->compack->targetLaser = atoi(c_comstr);
}

void Parser::penParse(char *c_comstr) {
	this->compack->targetPen = atoi(c_comstr);
}

void Parser::mcodeParse(char *c_comstr) {
	char *pEnd;
	uint8_t comNum = strtod(c_comstr, &pEnd);
	c_comstr = pEnd;

	switch(comNum) {
	case 1:
		this->compack->gormNum = 1;
		penParse(c_comstr + 1);
		break;
	case 4:
		this->compack->gormNum = 4;
		laserParse(c_comstr + 1);
		break;
	case 10:
		this->compack->gormNum = 10;
		m10Parse(c_comstr + 1);
	}
}

void Parser::xyParse(char *c_comstr) {

	char *pEnd;
	this->compack->targetX = strtod(c_comstr, &pEnd);
	c_comstr = pEnd + 2;

	this->compack->targetY = strtod(c_comstr, &pEnd);
	c_comstr = pEnd + 2;
	
	this->compack->auxDelay = strtol(c_comstr, &pEnd, 10);
	//  -.,-.,-., consider atoi
//	this->compack->auxDelay = atoi(c_comstr);
}

void Parser::gcodeParse(char *c_comstr) {
	char *pEnd;
	uint8_t comNum = strtod(c_comstr, &pEnd);
	c_comstr = pEnd;

	switch(comNum) {
	case 1:
		this->compack->gormNum = 1;
		xyParse(c_comstr + 2);
		break;
	case 28:
		this->compack->gormNum = 28;
		// TODO: home()
	}
}

CommandPacket Parser::generalParse(std::string commandString) {
	char c_comstr[commandString.length() + 1];
	strcpy(c_comstr, commandString.c_str());

	switch(c_comstr[0]) {
	case 'G':
		this->compack->gorm = 'G';
		gcodeParse(c_comstr + 1);
		break;
	case 'M':
		this->compack->gorm = 'M';
		mcodeParse(c_comstr + 1);
	}
	return *this->compack;
}

void Parser::debug(const char *str, bool showAll) {

		char commandBuffer[200];
		memset(commandBuffer, 0, sizeof(commandBuffer));
		strcpy(commandBuffer, str);

		*this->compack = this->generalParse(commandBuffer);
	
		if(showAll) {
			const char *format = "gorm: %c\r\ngormNum: %d\r\ntargetX: %.2f\r\ntargetY: %.2f\r\nauxDelay: %d\r\ntargetPen: %d\r\ntargetLaser: %d\r\n";
			memset(commandBuffer, 0, sizeof(commandBuffer));
			snprintf(commandBuffer, sizeof(commandBuffer), format, this->compack->gorm, this->compack->gormNum,
					this->compack->targetX, this->compack->targetY, this->compack->auxDelay, this->compack->targetPen, this->compack->targetLaser);
			ITM_write(commandBuffer);
		} else {
			if(this->compack->gorm == 'G') {
				if(this->compack->gormNum == 1) {
					const char *format = "%c%d X%.2f Y%.2f A%d\r\n";
					memset(commandBuffer, 0, sizeof(commandBuffer));
					snprintf(commandBuffer, sizeof(commandBuffer), format, this->compack->gorm, this->compack->gormNum,
							this->compack->targetX, this->compack->targetY, this->compack->auxDelay);
					ITM_write(commandBuffer);
				} else if(this->compack->gormNum == 28) {
					const char *format = "%c%d\r\n";
					memset(commandBuffer, 0, sizeof(commandBuffer));
					snprintf(commandBuffer, sizeof(commandBuffer), format, this->compack->gorm, this->compack->gormNum);
					ITM_write(commandBuffer);
				}
			} else if(this->compack->gorm == 'M') {
				if(this->compack->gormNum == 1) {
					const char *format = "%c%d %d\r\n";
					memset(commandBuffer, 0, sizeof(commandBuffer));
					snprintf(commandBuffer, sizeof(commandBuffer), format, this->compack->gorm, this->compack->gormNum, this->compack->targetPen);
					ITM_write(commandBuffer);
				} else if(this->compack->gormNum == 4) {
					const char *format = "%c%d %d\r\n";
					memset(commandBuffer, 0, sizeof(commandBuffer));
					snprintf(commandBuffer, sizeof(commandBuffer), format, this->compack->gorm, this->compack->gormNum, this->compack->targetPen);
					ITM_write(commandBuffer);
				} else if(this->compack->gormNum == 10) {
					const char *format = "%c%d\r\n";
					memset(commandBuffer, 0, sizeof(commandBuffer));
					snprintf(commandBuffer, sizeof(commandBuffer), format, this->compack->gorm, this->compack->gormNum);
					ITM_write(commandBuffer);
				}
			}
		}

}
