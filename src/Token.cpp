/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 18:20:21
 * @Description: file content
 */
#include "../inc/Token.h"

namespace INTERPRETER
{
    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    Token::Token()
    {
    }

    Token::Token(CharType type,int var)
    {
        this->valueInt = var;
        this->type = type;
    }

    Token::Token(CharType type,char var)
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
    CharType Token::getType()
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

    string typeName(CharType type)
    {
        switch (type)
        {
        case END:
            return "EOF";
        case NONE:
            return "NONE";
        case INT:
            return "INT";
        case ADD:
            return "ADD";
        case DEC:
            return "DEC";
        case MUL:
            return "MUL";
        case DIV:
            return "DIV";
        default:
            return "<NOT A TYPE>";
            break;
        }
    }
}