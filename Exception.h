#ifndef ___EXCEPTION_H___
#define ___EXCEPTION_H___

#include <string>

namespace INTERPRETER
{
    using namespace std;

    class SyntaxException
    {
    private:
        string str;
        int line;
    public:
        SyntaxException();
        SyntaxException(string str);
        SyntaxException(string str,int index);
        ~SyntaxException();

        string __str();
        int __line();
    };
    
}

#endif