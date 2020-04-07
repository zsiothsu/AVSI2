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
        int cur;
        Token currentToken;
    public:
        Interpreter();
        Interpreter(string line);
        ~Interpreter();

        void eat(Type type);
        int calc(int left,int right,char opt);
        int expr();
        Token getNextToken();
    };
}

#endif