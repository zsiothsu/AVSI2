/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-25 17:30:20
 * @Description: definition of tokens or interpreter
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
        DQUO,
        SQUO,
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
        CONTINUE
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

        bool isReOp();
        // std::string __str();
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
            NOT
    };
    const static TokenType ReOp[] = {
            EQ,
            NE,
            GT,
            LT,
            GE,
            LE,
            OR,
            AND
    };

    const static TokenType FIRST_STATEMENT[] = {
            EXPORT, FUNCTION, ID, RETURN, IF, FOR, WHILE, OBJ, MODULE, IMPORT
    };

    static map<TokenType, string> token_name = {
            {END,      "END"},
            {INTEGER,  "INTEGER"},
            {FLOAT,    "FLOAT"},
            {STRING,   "STRING"},
            {CHAR,     "CHAR"},
            {COMPOUND, "COMPOUND"},
            {ID,       "ID"},
            {PLUS,     "PLUS"},
            {MINUS,    "MINUS"},
            {STAR,     "STAR"},
            {SLASH,    "SLASH"},
            {EQUAL,    "EQUAL"},
            {NOT,      "NOT"},
            {DOT,      "DOT"},
            {EQ,       "EQ"},
            {NE,       "NE"},
            {GT,       "GT"},
            {LT,       "LT"},
            {GE,       "GE"},
            {LE,       "LE"},
            {OR,       "OR"},
            {AND,      "AND"},
            {LPAR,     "LPAR"},
            {RPAR,     "RPAR"},
            {LSQB,     "LSQB"},
            {RSQB,     "RSQB"},
            {LBRACE,   "LBRACE"},
            {RBRACE,   "RBRACE"},
            {SEMI,     "SEMI"},
            {COMMA,    "COMMA"},
            {DOLLAR,   "DOLLAR"},
            {COLON,    "COLON"},
            {TO,       "TO"},
            {FUNCTION, "FUNCTION"},
            {RETURN,   "RETURN"},
            {TRUE,     "TRUE"},
            {FALSE,    "FALSE"},
            {IF,       "IF"},
            {ELIF,     "ELIF"},
            {ELSE,     "ELSE"},
            {FI,       "FI"},
            {THEN,     "THEN"},
            {FOR,      "FOR"},
            {WHILE,    "WHILE"},
            {DO,       "DO"},
            {DONE,     "DONE"},
            {GLOBAL,   "GLOBAL"},
            {IMPORT,   "IMPORT"},
            {EXPORT,   "EXPORT"},
            {MODULE,   "MODULE"},
            {SIZEOF,   "SIZEOF"},
            {TYPENAME, "TYPENAME"},
            {REAL,     "REAL"},
            {VEC,      "VEC"},
            {OBJ,      "OBJ"},
            {BREAK,    "BREAK"},
            {CONTINUE, "CONTINUE"}
    };
} // namespace AVSI

#endif