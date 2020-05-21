/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-21 16:47:32
 * @Description: include some exception types for interpreter
 */
#ifndef ___EXCEPTION_H___
#define ___EXCEPTION_H___

#include <string>

namespace AVSI
{
    using namespace std;

    class Exception: public exception
    {
    protected:
        std::string str;
        std::string eType;
    public:
        int line;
        int column;

        Exception():
            str("Exception"),
            eType(str)
        {};
        Exception(std::string s):
            str(s),
            eType(str),
            line(-1),
            column(-1)
        {};
        ~Exception() {};

        void setMsg(std::string s);
        virtual const char* what();
    };

    class SyntaxException: public Exception
    {
    public:
        using Exception::Exception;
        SyntaxException(): Exception("SyntaxException") {};
    };
    
    class MathException: public Exception
    {
    public:
        using Exception::Exception;
        MathException(): Exception("MathException") {};
    };

    class LogicException: public Exception
    {
    public:
        using Exception::Exception;
        LogicException(): Exception("LogicException") {};
    };

    const Exception ExceptionFactory(std::string e);
    const Exception ExceptionFactory(std::string e,std::string c,int line,int column);
}

#endif