/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:49:29
 * @Description: include some exception types for interpreter
 */
#ifndef ___EXCEPTION_H___
#define ___EXCEPTION_H___

#include <string>

#define __SyntaxException   "SyntaxException"
#define __MathException     "MathException"
#define __TypeException     "TypeException"
#define __LogicException    "LogicException"
#define __MissingException  "MissingException"
#define __IRErrException    "IRErrException"

namespace AVSI {
    using namespace std;

    class Exception : public exception {
    protected:
        std::string str;
        std::string eType;

    public:
        int line;
        int column;

        Exception() : str("Exception"), eType(str) {};

        Exception(std::string s) : str(s), eType(str), line(-1), column(-1) {};

        ~Exception() {};

        void setMsg(std::string s);

        std::string type();

        virtual const char *what();
    };

    class SyntaxException : public Exception {
    public:
        using Exception::Exception;

        SyntaxException() : Exception(__SyntaxException) {};
    };

    class MathException : public Exception {
    public:
        using Exception::Exception;

        MathException() : Exception(__MathException) {};
    };

    class TypeException : public Exception {
    public:
        using Exception::Exception;

        TypeException() : Exception(__TypeException) {};
    };

    class LogicException : public Exception {
    public:
        using Exception::Exception;

        LogicException() : Exception(__LogicException) {};
    };

    class MissingException : public Exception {
    public:
        using Exception::Exception;

        MissingException() : Exception(__MissingException) {};
    };

    class IRErrException: public Exception {
    public:
        using Exception::Exception;

        IRErrException() : Exception(__IRErrException) {};
    };

    const Exception ExceptionFactory(std::string e);

    const Exception
    ExceptionFactory(std::string e, std::string c, int line, int column);
} // namespace AVSI

#endif