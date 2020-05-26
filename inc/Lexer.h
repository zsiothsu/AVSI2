/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-26 15:33:38
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
        std::string line;
    public:
        char currentChar;

        Lexer(void);
        Lexer(ifstream* file);
        ~Lexer();

        void advance();
        Token getNextToken();
        Token number();
        char peek();
        void skipWhiteSpace();
        Token Id();
    };

    static map<char,TokenType> TokenMap = {
        {'+',add_opt},
        {'-',dec_opt},
        {'*',mul_opt},
        {'/',div_opt},
        {'(',left_parenthese_keyword},
        {')',right_parenthese_keyword},
        {'[',left_bracket_keyword},
        {']',right_bracket_keyword},
        {'{',left_brace_keyword},
        {'}',right_brace_keyword},
        {';',semi_keyword},
        {',',comma_keyword}
    };

    static map<string,TokenType> reservedKeyword = {
        {"function",function_keyword}
    };
}

#endif