/*
 * any.h 2022
 *
 * "AVSI::any" is a data type which can store variable of basic types
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

#ifndef ___ANY_H___
#define ___ANY_H___

#include <cmath>
#include <iostream>
#include <map>
#include <regex>
#include <string>

namespace AVSI {
    using std::istream;
    using std::map;
    using std::match_results;
    using std::ostream;
    using std::regex;
    using std::regex_match;
    using std::string;

    typedef enum {
        Empty = 0,
        boolean = 10,
        Char = 20,
        Integer = 30,
        Float = 40,
        String = 50
    } DataType;

    class type_info {
    private:
        DataType typeId;
        std::string typeName;

    public:
        type_info(void);

        type_info(DataType type);

        std::string name(void);

        DataType type(void);

        bool operator==(type_info type);

        bool operator>=(const type_info type) const;

        bool operator<=(const type_info type) const;
    };

    class any {
    private:
        bool valueBool;
        int valueInt;
        double valueDouble;
        float valueFloat;
        char valueChar;
        string valueString;
        type_info typeInfo;

    public:
        any(void);

        any(char var);

        any(int var);

        any(float var);

        any(double var);

        any(char *var);

        any(string var);

        any(bool var);

        ~any();

        DataType type(void);

        template<typename T>
        T any_cast(void);

        template<typename T>
        static T any_cast(any var);

        any operator=(bool var);

        any operator=(char var);

        any operator=(int var);

        any operator=(float var);

        any operator=(double var);

        any operator=(string var);

        any operator+(any var);

        any operator-(any var);

        any operator*(any var);

        any operator/(any var);

        bool operator==(bool var) const;

        bool operator==(char var) const;

        bool operator==(int var) const;

        bool operator==(double var) const;

        bool operator==(string var) const;

        bool operator>(any var) const;

        bool operator<(any var) const;

        bool operator>=(any var) const;

        bool operator<=(any var) const;

        operator bool();

        operator string();

        friend ostream &operator<<(ostream &output, any &d);

        friend istream &operator>>(istream &input, any &d);

        friend bool checkOperand(DataType left, DataType right, string op);
    };

    static map<DataType, string> typeMap = {{Empty,   "empty"},
                                            {Char,    "char"},
                                            {Integer, "int"},
                                            {Float,   "float"},
                                            {String,  "string"}};

    static const regex numPattern = regex(
            "^(\\+|-)?(0|[1-9][0-9]*)(\\.[1-9]+)?((e|E)(\\+|-)?(0|[1-9][0-9]*))?$");

    static type_info typeEmpty = type_info(Empty);
    static type_info typeBool = type_info(boolean);
    static type_info typeInt = type_info(Integer);
    static type_info typeFloat = type_info(Float);
    static type_info typeChar = type_info(Char);
    static type_info typeString = type_info(String);

    bool checkOperand(DataType left, DataType right, string op);

    template<typename T>
    T (*cast)(void);

    template<typename T>
    T any::any_cast(void) {
        static_assert((std::is_arithmetic_v<T> || std::is_same_v<T, std::string>), "unsupported any_cast type");

        void *ret = &this->valueChar;
        if (typeid(T) == typeid(bool) && this->typeInfo <= typeFloat)
            ret = &this->valueBool;
        else if (typeid(T) == typeid(char) && this->typeInfo <= typeFloat)
            ret = &this->valueChar;
        else if (typeid(T) == typeid(int) && this->typeInfo <= typeFloat)
            ret = &this->valueInt;
        else if (typeid(T) == typeid(float) && this->typeInfo <= typeFloat)
            ret = &this->valueFloat;
        else if (typeid(T) == typeid(double) && this->typeInfo <= typeFloat)
            ret = &this->valueDouble;
        else if (typeid(T) == typeid(string))
            ret = &this->valueString;
        T Tval = *((T *) ret);
        return Tval;
    }

    template<typename T>
    T any::any_cast(any var) {
        return var.any_cast<T>();
    }

} // namespace AVSI

#endif