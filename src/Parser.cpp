/*
 * Parser.cpp
 *
 *  Created on: 20.10.2017
 *      Author: micromikko
 */

#include "Parser.h"
#include <cstring>
#include <stdlib.h>
#include "Motor.h"

Parser::Parser() {
	 this->compack = new CommandPacket();

}

Parser::~Parser() {
	delete compack;
}

void Parser::xyParse(char *c_comstr) {
	/*
	 * raksuta x
	 *
	 * raksuta y
	 */
}

void Parser::gcodeParse(char *c_comstr) {
	char * pEnd;
	uint8_t comNum = strtod(c_comstr, &pEnd);
	c_comstr = pEnd;

	switch(comNum) {
	case 1:
		this->compack->gormNum = 1;
		xyParse(c_comstr + 1);
		break;
	case 28:
		this->compack->gormNum = 28;
		// home()
	}
}

void Parser::mcodeParse(char *c_comstr) {

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





// G1 X-75.50 Y165.50 A0
//%.2f
/* strtod example */
//#include <stdio.h>      /* printf, NULL */
//#include <stdlib.h>     /* strtod */
//
//int main ()
//{
//  char szOrbits[] = "365.24 29.53";
//  char* pEnd;
//  double d1, d2;
//  d1 = strtod (szOrbits, &pEnd);
//  d2 = strtod (pEnd, NULL);
//  printf ("The moon completes %.2f orbits per Earth year.\n", d1/d2);
//  return 0;
//}

/*
void parseCordinate(char * cmd)
{
  char * tmp;
  char * str;
  str = strtok_r(cmd, " ", &tmp);
  tarX = curX;
  tarY = curY;
  while(str!=NULL){
    str = strtok_r(0, " ", &tmp);
    if(str[0]=='X'){
      tarX = atof(str+1);
    }else if(str[0]=='Y'){
      tarY = atof(str+1);
    }else if(str[0]=='Z'){
      tarZ = atof(str+1);
    }else if(str[0]=='F'){
      float speed = atof(str+1);
      tarSpd = speed/60; // mm/min -> mm/s
    }else if(str[0]=='A'){
      stepAuxDelay = atol(str+1);
    }
  }
//  Serial.print("tarX:");
//  Serial.print(tarX);
//  Serial.print(", tarY:");
//  Serial.print(tarY);
//  Serial.print(", stepAuxDelay:");
//  Serial.println(stepAuxDelay);
  prepareMove();
}
*/


