#include "token.h"

Token::Token(token_type type, std::string value):
    type(type),
    value(value)
{
}

token_type Token::get_type()
{
	return type;
}

std::string Token::get_value()
{
	return value;
}
