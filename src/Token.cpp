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

    void Token::setModInfo(vector<string> info) { this->__MOD_INFO = info; }

    vector<string> Token::getModInfo() { return this->__MOD_INFO; }

    bool Token::isExpr() {
        for (auto t: ExprOp) {
            if (this->type == t) return true;
        }

        return false;
    }

    bool Token::isReOp() {
        for (auto t: ReOp) {
            if (this->type == t) return true;
        }

        return false;
    }
} // namespace AVSI