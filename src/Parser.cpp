/*
 * Parser.cpp
 *
 *  Created on: 20.10.2017
 *      Author: micromikko
 */

#include "Parser.h"

Parser::Parser() {
	// TODO Auto-generated constructor stub

}

/*
 * JASKA
 */

Parser::~Parser() {
	// TODO Auto-generated destructor stub
}

void Parser::generalParser(char *command) {

	switch(command[0]) {
	case 'G':
		//gcodeparser
		break;
	case 'M':
		//mcodeparser
		break;
	default:
		//jaska
	}
}
