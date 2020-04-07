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
        END = -1,
        ERR = 0,
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

        Type getType();
        int getValue();
    };
    
}

#endif
