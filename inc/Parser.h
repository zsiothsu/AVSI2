/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:51:09
 * @Description: include Parser class
 */
#ifndef ___PARSER_H___
#define ___PARSER_H___

#include "Lexer.h"
#include <set>

namespace AVSI {
    using namespace std;

    class Parser {
    private:
        Lexer *lexer;
        Token currentToken;
        int parenCnt = 0;

    public:
        Parser(void);

        Parser(Lexer *lexer);

        ~Parser();

        void eat(TokenType type);

        AST *program();

        AST *statementList();

        AST *statement();

        AST *assignment();

        AST *forStatement();

        AST *functionDecl();

        AST *functionCall();

        AST *global();

        AST *IfStatement();

        AST *input();

        AST *param();

        AST *echo();

        AST *print();

        AST *expr();

        AST *factor();

        AST *parse();

        AST *returnExpr();

        AST *term();

        AST *variable();

        AST *WhileStatement();
    };
} // namespace AVSI

#endif