/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-26 15:30:13
 * @Description: include Parser class
 */
#ifndef ___PARSER_H___
#define ___PARSER_H___

#include <set>
#include "Lexer.h"

namespace AVSI
{
    using namespace std;

    class Parser
    {
    private:
        Lexer* lexer;
        Token currentToken;
        int parenCnt = 0;
    public:
        Parser(void);
        Parser(Lexer* lexer);
        ~Parser();

        void eat(TokenType type);

        AST* program();
        AST* statementList(); 
        AST* statement();
        AST* assignment();
        AST* functionDecl();
        AST* functionCall();
        AST* param();
        AST* expr();
        AST* factor();
        AST* parse();
        AST* term();
        AST* variable();
    };
}

#endif