/*
 * Parser.h 2022
 *
 * include Parser class
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

#ifndef ___PARSER_H___
#define ___PARSER_H___

#include <set>

#include "AST.h"
#include "Lexer.h"

namespace AVSI {
    using namespace std;
    using Type = pair<llvm::Type*, string>;

    class Parser {
    private:
        Lexer *lexer;
        Token currentToken;
        Token lastToken;
        int parenCnt = 0;

    public:
        Parser(void);

        Parser(Lexer *lexer);

        ~Parser();

        void eat(TokenType type);

        AST *program();

        AST *statementList();

        AST *statement();

        AST *arraylist();

        AST *assignment();

        AST *forStatement();

        AST *functionDecl();

        AST *functionCall();

        AST *global();

        AST *moduleDef();

        AST *moduleImport();

        AST *object(bool is_mangle);

        AST *IfStatement();

        AST *param();

        AST *expr();

        AST *logic_or_expr();

        AST *logic_and_expr();

        AST *equivalence_expr();

        AST *compare_expr();

        AST *basic_expr();

        AST *checkedExpr();

        AST *factor();

        AST *parse();

        AST *returnExpr();

        AST *term();

        AST *sizeOf();

        AST *variable();

        AST *WhileStatement();

        AST *loopCtrl();

        AST *IDHead();

        Type eatType();

        vector<string> eatModule();
    };
} // namespace AVSI

#endif