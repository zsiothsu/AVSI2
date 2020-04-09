#include "Token.h"

namespace INTERPRETER
{
    /*******************************************************
     *                    constructor                      *
     *******************************************************/
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

    /*******************************************************
     *                  static method                      *
     *******************************************************/
    Token Token::empty()
    {
        return emptyToken;
    }

    /*******************************************************
     *                      method                         *
     *******************************************************/
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

    string Token::__str()
    {
        string str = "Token(" + typeName(this->type) + "," + to_string(getValue()) + ")";
        return str;
    }

    string typeName(Type type)
    {
        switch (type)
        {
        case END:
            return "EOF";
            break;
        case NONE:
            return "NONE";
        case INT:
            return "INT";
        case OPT:
            return "OPT";
        default:
            return "<NOT A TYPE>";
            break;
        }
    }
}