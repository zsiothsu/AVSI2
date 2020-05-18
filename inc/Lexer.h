/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-18 17:27:52
 * @Description: include Lexer class
 */
#ifndef ___LEXER_H___
#define ___LEXER_H___

#include <fstream>
#include <cstdlib>
#include "AST.h"

namespace AVSI
{
    class Lexer
    {
    private:
        ifstream* file;
        unsigned int linenum;
        unsigned int cur;
        char currentChar;
        std::string line;
    public:
        Lexer(void);
        Lexer(ifstream* file);
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