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
#include "SymbolTable.h"

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

        shared_ptr<AST> program();

        shared_ptr<AST> statementList();

        shared_ptr<AST> statement();

        shared_ptr<AST> arraylist();

        shared_ptr<AST> assignment();

        shared_ptr<AST> forStatement();

        shared_ptr<AST> functionDecl();

        shared_ptr<AST> functionCall();

        shared_ptr<AST> generic();

        shared_ptr<AST> global();

        shared_ptr<AST> grad();

        shared_ptr<AST> moduleDef();

        shared_ptr<AST> moduleImport();

        shared_ptr<AST> object(bool is_mangle);

        shared_ptr<AST> IfStatement();

        shared_ptr<AST> param();

        shared_ptr<AST> expr();

        shared_ptr<AST> logic_or_expr();

        shared_ptr<AST> logic_and_expr();

        shared_ptr<AST> bit_or_expr();

        shared_ptr<AST> bit_and_expr();

        shared_ptr<AST> equivalence_expr();

        shared_ptr<AST> compare_expr();

        shared_ptr<AST> shift_expr();

        shared_ptr<AST> basic_expr();

        shared_ptr<AST> checkedExpr();

        shared_ptr<AST> factor();

        shared_ptr<AST> parse();

        shared_ptr<AST> returnExpr();

        shared_ptr<AST> term();

        shared_ptr<AST> sizeOf();

        shared_ptr<AST> variable();

        shared_ptr<AST> WhileStatement();

        shared_ptr<AST> loopCtrl();

        shared_ptr<AST> IDHead();

        Type eatType();

        vector<string> eatModule();
    };

    pair<map<string, StructDef *>::iterator, string> find_struct(vector<string> modinfo, string &name);
} // namespace AVSI

#endif