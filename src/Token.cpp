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
    
    bool Token::isExpr()
    {
        return  this->type == INTEGER   || \
                this->type == FLOAT     || \
                this->type == PLUS      || \
                this->type == MINUS     || \
                this->type == ID        || \
                this->type == LPAR;
    }
} // namespace AVSI