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
        REAL,
        VEC,
        OBJ
    } TokenType;

    class Token {
    private:
        any value;
        TokenType type;

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

    std::string typeName(TokenType type);
} // namespace AVSI

#endif