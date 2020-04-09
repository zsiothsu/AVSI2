/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-09 20:49:01
 * @Description: file content
 */
#ifndef ___TOKEN_H___
#define ___TOKEN_H___

#include <iostream>
#include <string>
#include <sstream>
#include <typeinfo>

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
        DIV = 5
    } Type;
    
    typedef char opt;

    class Token
    {
    private:
        int     valueInt;
        char    valueChar;
        Type    type;
    public:
        Token();
        Token(Type type,int var);
        Token(Type type,char var);
        ~Token();

        static Token empty();

        Type getType();
        int getValue();
        string __str();
    };
    
    const static Token emptyToken(NONE,0);

    string typeName(Type type);
}

#endif
