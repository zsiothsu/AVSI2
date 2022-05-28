/*
 * Token.cpp 2022
 *
 * Operations for Token
 *
 * LLVM IR code generator
 *
 * MIT License
 *
 * Copyright (c) 2022 Chipen Hsiao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

    /**
     * @description:    check is FIRST(expr)
     * @return:         is expr token(true) or not(false)
     */
    bool Token::isExpr() {
        for (auto t: ExprOp) {
            if (this->type == t) return true;
        }

        return false;
    }
} // namespace AVSI