/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-18 11:44:32
 * @Description: file content
 */
#ifndef ___PARSER_H___
#define ___PARSER_H___

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

        AST* statement();
        AST* assignment();
        AST* expr();
        AST* factor();
        AST* parse();
        AST* term();
        AST* variable();
    };
}

#endif