/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-02 11:13:54
 * @Description: file content
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
        Exception();
        Exception(std::string s);
        ~Exception();

        void setMsg(std::string s);
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

    const Exception ExceptionFactory(std::string e);
    const Exception ExceptionFactory(std::string e,std::string c);
}

#endif