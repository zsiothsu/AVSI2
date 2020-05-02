/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-01 16:38:31
 * @Description: file content
 */
#include "../inc/Token.h"

namespace AVSI
{
    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    Token::Token()
    {
    }

    Token::Token(TokenType type,int var)
    {
        this->valueInt = var;
        this->valueFloat = var;
        this->type = type;
    }

    Token::Token(TokenType type,double var)
    {
        this->valueInt = (int)var;
        this->valueFloat = var;
        this->type = type;
    }

    Token::Token(TokenType type,char var)
    {
        this->valueChar = var;
        this->type = type;
    }

    Token::Token(TokenType type,std::string var)
    {
        this->valueString = var;
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
    TokenType Token::getType()
    {
        return this->type;
    }

    double Token::getNum()
    {
        return this->valueFloat;
    }

    char Token::getChar()
    {
        return this->valueChar;
    }

    std::string Token::getString()
    {
        return this->valueString;
    }

    //TODO
    std::string typeName(TokenType type)
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