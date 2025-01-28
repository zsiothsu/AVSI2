/*
 * Token.h 2022
 *
 * definition of tokens
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

#ifndef ___TOKEN_H___
#define ___TOKEN_H___

#include "any.h"

namespace AVSI {
    using namespace std;
    typedef enum {
        // special token
        END = EOF,
        NONE = 0,
        // terminal
        INTEGER,
        FLOAT,
        STRING,
        CHAR,
        COMPOUND,
        ID,
        // operator
        PLUS,       // +
        MINUS,      // -
        STAR,       // *
        SLASH,      // /
        EQUAL,      // =
        NOT,        // !
        DOT,        // .
        EQ,         // ==   -eq
        NE,         // !=   -ne
        GT,         // >    -gt
        LT,         // <    -lt
        GE,         // >=   -ge
        LE,         // <=   -le
        OR,         // ||   -o
        AND,        // &&   -a
        BITOR,      // |
        BITAND,     // &
        BITCPL,     // ~
        SHL,        // <<
        SHR,        // >>
        SHRU,       // >>>
        REM,        // %
        // symbol
        LPAR,       // (
        RPAR,       // )
        LSQB,       // [
        RSQB,       // ]
        LBRACE,     // {
        RBRACE,     // }
        SEMI,       // ;
        COMMA,      // ,
        DOLLAR,     // $
        COLON,      // :
        TO,         // ->
        // reserved keywork
        FUNCTION,
        RETURN,
        TRUE,
        FALSE,
        IF,
        ELIF,
        ELSE,
        FI,
        THEN,
        FOR,
        WHILE,
        DO,
        DONE,
        GLOBAL,
        IMPORT,
        EXPORT,
        MODULE,
        SIZEOF,
        TYPENAME,
        REAL,
        VEC,
        OBJ,
        BREAK,
        CONTINUE,
        NOMANGLE,
        AS,
        GENERIC,
        // types,
        F64,
        F32,
        I128,
        I64,
        I32,
        I16,
        I8,
        BOOL,
        ISIZE,
        VOID,
    } TokenType;

    class Token {
    private:
        any value;
        TokenType type;
        vector<string> __MOD_INFO;

    public:
        int line;
        int column;

        Token() {};

        Token(TokenType type, any var) : value(var), type(type) {};

        Token(TokenType type, any var, int line, int col)
                : value(var), type(type), line(line), column(col) {};
    
        ~Token() {};

        static Token empty();

        TokenType getType();

        any getValue();

        void setModInfo(vector<string> info);

        vector<string> getModInfo();

        any getNum();

        char getChar();

        std::string getString();

        bool isExpr();
    };

    const static Token emptyToken(NONE, 0);

    const static TokenType ExprOp[] = {
            INTEGER,
            FLOAT,
            STRING,
            TRUE,
            FALSE,
            PLUS,
            MINUS,
            ID,
            LPAR,
            NOT,
            LBRACE,
            IF
    };

    const static TokenType FIRST_STATEMENT[] = {
            EXPORT, FUNCTION, ID,
            RETURN, IF, FOR,
            WHILE, OBJ, MODULE,
            IMPORT
    };

    static map<TokenType, string> token_name = {
            {END,       "END"},
            {INTEGER,   "INTEGER"},
            {FLOAT,     "FLOAT"},
            {STRING,    "STRING"},
            {CHAR,      "CHAR"},
            {COMPOUND,  "COMPOUND"},
            {ID,        "ID"},
            {PLUS,      "PLUS"},
            {MINUS,     "MINUS"},
            {STAR,      "STAR"},
            {SLASH,     "SLASH"},
            {EQUAL,     "EQUAL"},
            {NOT,       "NOT"},
            {DOT,       "DOT"},
            {EQ,        "EQ"},
            {NE,        "NE"},
            {GT,        "GT"},
            {LT,        "LT"},
            {GE,        "GE"},
            {LE,        "LE"},
            {OR,        "OR"},
            {AND,       "AND"},
            {LPAR,      "LPAR"},
            {RPAR,      "RPAR"},
            {LSQB,      "LSQB"},
            {RSQB,      "RSQB"},
            {LBRACE,    "LBRACE"},
            {RBRACE,    "RBRACE"},
            {SEMI,      "SEMI"},
            {COMMA,     "COMMA"},
            {DOLLAR,    "DOLLAR"},
            {COLON,     "COLON"},
            {TO,        "TO"},
            {FUNCTION,  "FUNCTION"},
            {RETURN,    "RETURN"},
            {TRUE,      "TRUE"},
            {FALSE,     "FALSE"},
            {IF,        "IF"},
            {ELIF,      "ELIF"},
            {ELSE,      "ELSE"},
            {FI,        "FI"},
            {THEN,      "THEN"},
            {FOR,       "FOR"},
            {WHILE,     "WHILE"},
            {DO,        "DO"},
            {DONE,      "DONE"},
            {GLOBAL,    "GLOBAL"},
            {IMPORT,    "IMPORT"},
            {EXPORT,    "EXPORT"},
            {MODULE,    "MODULE"},
            {SIZEOF,    "SIZEOF"},
            {TYPENAME,  "TYPENAME"},
            {REAL,      "REAL"},
            {VEC,       "VEC"},
            {OBJ,       "OBJ"},
            {BREAK,     "BREAK"},
            {CONTINUE,  "CONTINUE"},
            {NOMANGLE,  "NOMANGLE"},
            {AS,        "AS"},
            {F64,       "F64"},
            {F32,       "F32"},
            {I128,      "I128"},
            {I64,       "I64"},
            {I32,       "I32"},
            {I16,       "I16"},
            {I8,        "I8"},
            {BOOL,      "BOOL"},
            {ISIZE,     "BOOL"},
            {VOID,      "VOID"},
            {BITOR,     "BITOR"},
            {BITAND,    "BITAND"},
            {BITCPL,    "BITCPL"},
            {SHL,       "SHL"},
            {SHR,       "SHR"},
            {SHRU,      "SHRU"},
            {REM,       "REM"},
    };
} // namespace AVSI

#endif