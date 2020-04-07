#include "Token.h"

namespace INTERPRETER
{
    Token::Token()
    {
    }

    Token::Token(Type type,int var)
    {
        this->valueInt = var;
        this->type = type;
    }

    Token::Token(Type type,char var)
    {
        this->valueChar = var;
        this->type = type;
    }

    Token::~Token()
    {
    }

    Type Token::getType()
    {
        return this->type;
    }

    int Token::getValue()
    {
        if(this->type == END)
        {
            return -1;
        }
        else if(this->type == INT)
        {
            return this->valueInt;
        }
        else
        {
            return (int)this->valueChar;
        }
    }
}