#include "interpreter.h"

#include <assert.h>
#include <ctype.h>

Interpreter::Interpreter(std::string text):
m_text(text),
m_pos(0)
{
}

Token Interpreter::get_next_token()
{
    if (m_pos >= m_text.size())
    {
        return Token(token_type::eof, "");
    }

    skip_whitespace();

    char c = m_text.at(m_pos);
	if (isdigit(c))
	{
		int begin = m_pos;
		while (m_pos < m_text.size() && isdigit(m_text.at(m_pos)))
		{
			m_pos++;
		}
		return Token(token_type::integer, m_text.substr(begin, m_pos-begin));
	}
    else if (isalpha(c))
    {
        int begin = m_pos;
        while (m_pos < m_text.size() && isalpha(m_text.at(m_pos)))
		{
			m_pos++;
		}
		return Token(token_type::text, m_text.substr(begin, m_pos-begin));
    }
	else if (c == '+')
	{
		m_pos++;
		return Token(token_type::plus, std::string(1, c));
	}
	else if (c == '-')
	{
		m_pos++;
		return Token(token_type::minus, std::string(1, c));
	}
}

int Interpreter::expr()
{
    Token curr_token = get_next_token();
    assert (curr_token.get_type() == token_type::integer);
    int left = std::stoi(curr_token.get_value());

    curr_token = get_next_token();
    token_type op = curr_token.get_type();
    while (op == token_type::plus || op == token_type::minus)
    {
        curr_token = get_next_token();
        assert (curr_token.get_type() == token_type::integer);
        int right = std::stoi(curr_token.get_value());
        if (op == token_type::plus)
        {
            left = left + right;
        }
        else
        {
            left = left - right;
        }

        curr_token = get_next_token();
        op = curr_token.get_type();
    }

    return left;
}

void Interpreter::skip_whitespace()
{
    while (m_pos < m_text.size() && isspace(m_text.at(m_pos)))
    {
        m_pos++;
    }
}
