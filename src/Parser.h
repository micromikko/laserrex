/*
 * Parser.h
 *
 *  Created on: 20.10.2017
 *      Author: micromikko
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include "CommandPacket.h"


class Parser {
public:
	Parser();
	virtual ~Parser();

	CommandPacket generalParse(const std::string commandString);
	void gcodeParse(char *c_comstr);
	void mcodeParse(char *c_comstr);
	void xyParse(char *c_comstr);

	void penParse(char *c_comstr);
	void laserParse(char *c_comstr);
	void m10Parse(char *c_comstr);

	void debug(const char *str, bool showAll=false);
	//void debugG28();

	//void debugM1();
	//void debugM4();
	//void debugM10();

private:
	CommandPacket *compack;

};

#endif /* PARSER_H_ */
