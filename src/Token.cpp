/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-11 15:08:00
 * @Description: definition of tokens or interpreter
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

    Token::Token(TokenType type,any var)
    {
        this->value = var;
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

    any Token::getValue()
    {
        return this->value;
    }

    //TODO: map<TokeType,string>
    std::string typeName(TokenType type)
    {
        switch (type)
        {
        case END:
            return "EOF";
        case NONE:
            return "NONE";
        case integer_ast:
            return "integer_ast";
        case add_opt:
            return "add_opt";
        case dec_opt:
            return "dec_opt";
        case mul_opt:
            return "mul_opt";
        case div_opt:
            return "div_opt";
        default:
            return "<NOT A TYPE>";
            break;
        }
    }
}