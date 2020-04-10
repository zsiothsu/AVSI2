/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-09 20:48:54
 * @Description: file content
 */
#ifndef ___EXCEPTION_H___
#define ___EXCEPTION_H___

#include <string>

namespace INTERPRETER
{
    using namespace std;

    class Exception: public exception
    {
    protected:
        string str;
        string eType;
    public:
        Exception();
        Exception(string s);
        ~Exception();

        void setMsg(string s);
        virtual const char* what();
    };

    class SyntaxException: public Exception
    {
    public:
        using Exception::Exception;
        SyntaxException();
    };
    
    class MathException: public Exception
    {
    public:
        using Exception::Exception;
        MathException();
    };

    const Exception ExceptionFactory(string e);
    const Exception ExceptionFactory(string e,string c);
}

#endif