/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 18:23:23
 * @Description: file content
 */
#ifndef ___LEXER_H___
#define ___LEXER_H___

#include <cstdlib>
#include "AST.h"

namespace INTERPRETER
{
    class Lexer
    {
    private:
        string line;
        unsigned int cur;
        char currentChar;
    public:
        Lexer(void);
        Lexer(string line);
        ~Lexer();

        void advance();
        Token getNextToken();
        int integer();
        void skipWhiteSpace();
    };
}

#endif