/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-02 11:14:11
 * @Description: file content
 */
#ifndef ___TOKEN_H___
#define ___TOKEN_H___

#include <iostream>
#include <string>

namespace AVSI
{
    using namespace std;
    typedef enum
    {
        END = EOF,
        NONE = 0,
        INT = 1,
        FLT = 2,
        ADD = 3,
        DEC = 4,
        MUL = 5,
        DIV = 6,
        LPAREN = 7,
        RPAREN = 8,
        ASSIGN = 9,
        VAR = 10
    } TokenType;
    
    typedef char opt;
    
    class Token
    {
    private:
        int     valueInt;
        double  valueFloat;
        char    valueChar;
        std::string  valueString;
        TokenType    type;
    public:
        Token();
        Token(TokenType type,int var);
        Token(TokenType type,double var);
        Token(TokenType type,char var);
        Token(TokenType type,std::string var);
        ~Token();

        static Token empty();

        TokenType getType();
        double getNum();
        char getChar();
        std::string getString();
        //std::string __str();
    };
    
    const static Token emptyToken(NONE,0);

    std::string typeName(TokenType type);
}

#endif
