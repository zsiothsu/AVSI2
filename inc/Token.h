/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 20:57:54
 * @Description: file content
 */
#ifndef ___TOKEN_H___
#define ___TOKEN_H___

#include <iostream>
#include <string>

namespace INTERPRETER
{
    using namespace std;
    typedef enum
    {
        END = EOF,
        NONE = 0,
        INT = 1,
        ADD = 2,
        DEC = 3,
        MUL = 4,
        DIV = 5,
        LPAREN = 6,
        RPAREN = 7
    } CharType;
    
    typedef char opt;
    
    class Token
    {
    private:
        int     valueInt;
        char    valueChar;
        CharType    type;
    public:
        Token();
        Token(CharType type,int var);
        Token(CharType type,char var);
        ~Token();

        static Token empty();

        CharType getType();
        int getValue();
        string __str();
    };
    
    const static Token emptyToken(NONE,0);

    string typeName(CharType type);
}

#endif
