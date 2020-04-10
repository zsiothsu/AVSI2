/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 20:07:16
 * @Description: file content
 */
#ifndef ___PARSER_H___
#define ___PARSER_H___

#include "Lexer.h"

namespace INTERPRETER
{
    using namespace std;

    class Parser
    {
    private:
        Lexer* lexer;
        Token currentToken;
    public:
        Parser(void);
        Parser(Lexer& lexer);
        ~Parser();

        void eat(CharType type);
        AST* expr();
        AST* factor();
        AST* parse();
        AST* term();
    };
}

#endif