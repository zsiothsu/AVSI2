/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:51:09
 * @Description: include Parser class
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

        AST *object();

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

        Type eatType();

        vector<string> eatModule();
    };
} // namespace AVSI

#endif