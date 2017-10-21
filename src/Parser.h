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

	CommandPacket generalParse(std::string commandString);
	void gcodeParse(char *c_comstr);
	void mcodeParse(char *c_comstr);
	void xyParse(char *c_comstr);

private:
	CommandPacket *compack;

};

#endif /* PARSER_H_ */
