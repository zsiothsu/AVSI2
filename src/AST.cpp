/*
 * AST.cpp 2022
 *
 * constructer of abstract syntax tree (AST)
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

#include "../inc/AST.h"

namespace AVSI {
    /*******************************************************
     *                       AST base                      *
     *******************************************************/
    Token AST::getToken(void) { return this->token; }

    /*******************************************************
     *                    derived syntax                   *
     *******************************************************/
    Assign::~Assign() {}

    TokenType BinOp::getOp(void) { return this->op.getType(); }

    BinOp::~BinOp() {}

    any Boolean::getValue(void) {
        return this->value;
    }

    For::~For() {}

    FunctionDecl::~FunctionDecl() {}

    FunctionCall::~FunctionCall() {}

    Generic::~Generic() {}

    Global::~Global() {}

    any Num::getValue(void) { return this->value; }

    any String::getValue(void) { return this->value; }

    StructInit::~StructInit() {}

    TypeTrans::~TypeTrans() {}

    ArrayInit::~ArrayInit() {}

    If::~If() {}

    UnaryOp::~UnaryOp() {}

    TokenType UnaryOp::getOp(void) { return this->op.getType(); }

    Compound::~Compound() {}

    While::~While() {}
} // namespace AVSI