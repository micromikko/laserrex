/*
 * Parser.h
 *
 *  Created on: 20.10.2017
 *      Author: micromikko
 */

#ifndef PARSER_H_
#define PARSER_H_

class Parser {
public:
	Parser();
	virtual ~Parser();

	void generalParser(char * cmd);
	void gcodeParser();
	void mcodeParser();

	/*
	 * TESTII
	 */


};

#endif /* PARSER_H_ */
