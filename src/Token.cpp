/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:56:56
 * @Description: definition of tokens or interpreter
 */
#include "../inc/Token.h"

namespace AVSI {
    /*******************************************************
     *                  static method                      *
     *******************************************************/
    Token Token::empty() { return emptyToken; }

    /*******************************************************
     *                      method                         *
     *******************************************************/
    TokenType Token::getType() { return this->type; }

    any Token::getValue() { return this->value; }

    // TODO: map<TokeType,string>
    std::string typeName(TokenType type)
    {
        switch(type) {
            case END: return "EOF";
            case NONE: return "NONE";
            case INTEGER: return "INTEGER";
            case PLUS: return "PLUS";
            case MINUS: return "MINUS";
            case STAR: return "STAR";
            case SLASH: return "SLASH";
            default: return "<NOT A TYPE>"; break;
        }
    }
} // namespace AVSI