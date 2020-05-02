/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 18:20:01
 * @Description: file content
 */
#include "../inc/Exception.h"

namespace AVSI
{
    /*******************************************************
     *                    base exception                   *
     *******************************************************/
    Exception::Exception()
    {
        this->str = "Exception";
        this->eType = this->str;
    }

    Exception::Exception(std::string s)
    {
        Exception();
        this->str = this->eType + s + " ";
    }

    Exception::~Exception()
    {
    }

    void Exception::setMsg(std::string s)
    {
        this->str = this->eType + ": "+ s + " ";
    }

    const char* Exception::what()
    {
        return this->str.c_str();
    }
    
    /*******************************************************
     *                  derived exception                  *
     *******************************************************/
    SyntaxException::SyntaxException()
    {
        this->str = "SyntaxException";
        this->eType = this->str;
    }

    MathException::MathException()
    {
        this->str = "MathException";
        this->eType = this->str;
    }

    /*******************************************************
     *                  exception factory                  *
     *******************************************************/

    const Exception ExceptionFactory(std::string e)
    {
        if(e == "SyntaxException")
        {
            return SyntaxException();
        }
        else if(e == "MathException")
        {
            return MathException();
        }
        return Exception();
    }


    const Exception ExceptionFactory(std::string e,std::string c)
    {
        Exception exception = ExceptionFactory(e);
        
        exception.setMsg(c);

        return exception;
    }
}