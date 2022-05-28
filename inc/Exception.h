/*
 * Exception.h 2022
 *
 * include some exception types for compiler
 *
 * LLVM IR code generator
 *
 * MIT License
 *
 * Copyright (c) 2022 Chipen Hsiao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef ___EXCEPTION_H___
#define ___EXCEPTION_H___

#include <string>
#include <iostream>
#include "FileName.h"

#define __SyntaxException   "SyntaxException"
#define __MathException     "MathException"
#define __TypeException     "TypeException"
#define __LogicException    "LogicException"
#define __MissingException  "MissingException"
#define __IRErrException    "IRErrException"
#define __SysErrException   "SysErrException"
#define __ErrReport         "ErrReport"

#define __Warning           "Warning"


#define __COLOR_RESET       "\033[0m"
#define __COLOR_RED         "\033[31m"
#define __COLOR_GREEN       "\033[32m"
#define __COLOR_YELLOW      "\033[33m"

extern uint16_t warn_count;
extern uint16_t err_count;

namespace AVSI {
    using namespace std;

    /*******************************************************
     *                    base exception                   *
     *******************************************************/
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

        void setMsg(std::string s) {
            this->str = this->eType + ": " + s + " ";
        }

        std::string type() {
            return this->eType;
        }

        using exception::what;
        const char *what() {
            return this->str.c_str();
        }
    };

    /*******************************************************
     *                     exceptions                      *
     *******************************************************/
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

    class SysErrException: public Exception {
    public:
        using Exception::Exception;

        SysErrException() : SysErrException(__SysErrException) {};
    };

    class ErrReport : public Exception {
    public:
        using Exception::Exception;

        ErrReport() : Exception(__ErrReport) {};
    };

    /*******************************************************
     *                     functions                       *
     *******************************************************/
    template<typename T>
    inline const Exception ExceptionFactory() {
        return T();
    }

    template<typename T>
    inline const Exception
    ExceptionFactory(std::string msg, int line, int column) {
        Exception exception = ExceptionFactory<T>();

        exception.setMsg(msg);
        exception.line = line;
        exception.column = column;

        if(exception.type() != __ErrReport) err_count++;

        return exception;
    }

    static inline void Warning(std::string msg, int line, int column) {
        warn_count++;
        std::cerr << __COLOR_YELLOW
                  << input_file_name
                  << ":" << line  << ":" << column + 1 << ": "
                  << __Warning << ": "
                  << msg
                  << __COLOR_RESET << std::endl;
    }
} // namespace AVSI

#endif