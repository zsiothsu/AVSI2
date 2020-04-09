/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-09 14:09:52
 * @Description: file content
 */
#ifndef ___INTERPRETER_H___
#define ___INTERPRETER_H___

#include <cstdlib>
#include "Token.h"
#include "Exception.h"

namespace INTERPRETER
{
    using namespace std;

    class Interpreter
    {
    private:
        string line;
        unsigned int cur;
        char currentChar;
        Token currentToken;
    public:
        Interpreter();
        Interpreter(string line);
        ~Interpreter();

        void advance();
        Token getNextToken();
        int integer();
        void skipWhiteSpace();

        int calc(int left,int right,Token opt);
        void eat(Type type);
        int expr();
        int factor();
    };
}

#endif