/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-01 18:27:52
 * @Description: file content
 */
#ifndef ___LEXER_H___
#define ___LEXER_H___

#include <cstdlib>
#include <cmath>
#include "AST.h"
#include "Exception.h"

namespace AVSI
{
    class Lexer
    {
    private:
        std::string line;
        unsigned int cur;
        char currentChar;
    public:
        Lexer(void);
        Lexer(std::string line);
        ~Lexer();

        void advance();
        Token getNextToken();
        Token number();
        char peek();
        void skipWhiteSpace();
        std::string Id();
    };
}

#endif