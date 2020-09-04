#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>

#include "token.h"

class Interpreter
{
public:
	Interpreter(std::string text);

	// Lexical analyzer
	Token get_next_token();

	// Parser
	int expr();

private:
	void skip_whitespace();

	std::string m_text;
	size_t m_pos;
};

#endif
