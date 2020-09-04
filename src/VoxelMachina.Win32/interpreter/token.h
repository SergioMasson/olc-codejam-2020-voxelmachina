#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum token_type
{
	integer,
	text,
	plus,
	minus,
	eof,
};

class Token
{
public:
	Token(token_type type, std::string value);

	token_type get_type();
	std::string get_value();

private:
	token_type type;
	std::string value;
};

#endif
