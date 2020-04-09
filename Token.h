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
        OPT = 2
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
