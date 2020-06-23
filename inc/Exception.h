/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:49:29
 * @Description: include some exception types for interpreter
 */
#ifndef ___EXCEPTION_H___
#define ___EXCEPTION_H___

#include <string>

#define __SyntaxException "SyntaxException"
#define __MathException "MathException"
#define __LogicException "LogicException"

namespace AVSI {
    using namespace std;

    class Exception : public exception
    {
      protected:
        std::string str;
        std::string eType;

      public:
        int line;
        int column;

        Exception() : str("Exception"), eType(str){};
        Exception(std::string s) : str(s), eType(str), line(-1), column(-1){};
        ~Exception(){};

        void setMsg(std::string s);
        virtual const char* what();
    };

    class SyntaxException : public Exception
    {
      public:
        using Exception::Exception;
        SyntaxException() : Exception(__SyntaxException){};
    };

    class MathException : public Exception
    {
      public:
        using Exception::Exception;
        MathException() : Exception(__MathException){};
    };

    class LogicException : public Exception
    {
      public:
        using Exception::Exception;
        LogicException() : Exception(__LogicException){};
    };

    const Exception ExceptionFactory(std::string e);
    const Exception
    ExceptionFactory(std::string e, std::string c, int line, int column);
} // namespace AVSI

#endif