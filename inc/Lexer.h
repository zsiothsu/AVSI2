/*
 * Lexer.h 2022
 *
 * include Lexer class
 *
 * LLVM IR code generator
 *
 * MIT License
 *
 * Copyright (c) 2022 Chipen Hsiao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

        char getEscapeChar();

        Token character();


        Token Id();
    };

    static map<char, TokenType> TokenMap = {
            {'+', PLUS},
            {'-', MINUS},
            {'*', STAR},
            {'/', SLASH},
            {'(', LPAR},
            {')', RPAR},
            {'[', LSQB},
            {']', RSQB},
            {'{', LBRACE},
            {'}', RBRACE},
            {';', SEMI},
            {',', COMMA},
            {'$', DOLLAR},
            {':', COLON},
            {'.', DOT}};

    static map<string, TokenType> reservedKeyword = {
            {"function",  FUNCTION},
            {"return",    RETURN},
            {"true",      TRUE},
            {"false",     FALSE},
            {"if",        IF},
            {"else",      ELSE},
            {"elif",      ELIF},
            {"fi",        FI},
            {"then",      THEN},
            {"for",       FOR},
            {"while",     WHILE},
            {"do",        DO},
            {"done",      DONE},
            {"global",    GLOBAL},
            {"import",    IMPORT},
            {"export",    EXPORT},
            {"mod",       MODULE},
            {"real",      REAL},
            {"vec",       VEC},
            {"obj",       OBJ},
            {"char",      CHAR},
            {"sizeof",    SIZEOF},
            {"typename",  TYPENAME},
            {"break",     BREAK},
            {"continue",  CONTINUE},
            {"no_mangle", NOMANGLE},
            {"as",        AS}
    };
} // namespace AVSI

#endif