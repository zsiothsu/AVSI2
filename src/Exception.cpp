/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:56:14
 * @Description: include some exception types for interpreter
 */
#include "../inc/Exception.h"

namespace AVSI {
    /*******************************************************
     *                    base exception                   *
     *******************************************************/
    void Exception::setMsg(std::string s)
    {
        this->str = this->eType + ": " + s + " ";
    }

    const char* Exception::what() { return this->str.c_str(); }

    /*******************************************************
     *                  exception factory                  *
     *******************************************************/

    const Exception ExceptionFactory(std::string e)
    {
        if(e == __SyntaxException) { return SyntaxException(); }
        else if(e == __MathException) {
            return MathException();
        }
        else if(e == __LogicException) {
            return LogicException();
        }
        return Exception();
    }

    const Exception
    ExceptionFactory(std::string e, std::string c, int line, int column)
    {
        Exception exception = ExceptionFactory(e);

        exception.setMsg(c);
        exception.line = line;
        exception.column = column;

        return exception;
    }
} // namespace AVSI