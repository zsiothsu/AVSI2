/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:51:01
 * @Description: include Lexer class
 */
#ifndef ___LEXER_H___
#define ___LEXER_H___

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "Token.h"

namespace AVSI {
    using std::ifstream;
    using std::string;

    class Lexer {
    private:
        ifstream *file;
        unsigned int linenum;
        unsigned int cur;
        std::string line;

    public:
        char currentChar;

        Lexer(void);

        Lexer(ifstream *file);

        ~Lexer();

        void advance();

        Token getNextToken();

        Token number();

        char peek();

        string peek2();

        void skipWhiteSpace();

        Token str();

        Token Id();
    };

    static map<char, TokenType> TokenMap = {
            {'+',   PLUS},
            {'-',   MINUS},
            {'*',   STAR},
            {'/',   SLASH},
            {'(',   LPAR},
            {')',   RPAR},
            {'[',   LSQB},
            {']',   RSQB},
            {'{',   LBRACE},
            {'}',   RBRACE},
            {';',   SEMI},
            {',',   COMMA},
            {'$',   DOLLAR}};

    static map<string, TokenType> reservedKeyword = {
            {"function",    FUNCTION},
            {"return",      RETURN},
            {"true",        TRUE},
            {"false",       FALSE},
            {"echo",        ECHO},
            {"if",          IF},
            {"else",        ELSE},
            {"elif",        ELIF},
            {"fi",          FI},
            {"then",        THEN},
            {"for",         FOR},
            {"while",       WHILE},
            {"do",          DO},
            {"done",        DONE},
            {"global",      GLOBAL},
            {"input",       INPUT},
            {"printf",      PRINTF}};
} // namespace AVSI

#endif