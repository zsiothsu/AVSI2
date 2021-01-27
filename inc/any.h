/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:48:01
 * @Description: "AVSI::any" is a data type which can store variable of basic
 * type
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
        double valueFloat;
        char valueChar;
        string valueString;
        type_info typeInfo;

    public:
        any(void);

        any(bool var);

        any(char var);

        any(int var);

        any(double var);

        any(string var);

        ~any();

        DataType type(void);

        template<typename T>
        T any_cast(void);

        template<typename T>
        static T any_cast(any var);

        any operator=(bool var);

        any operator=(char var);

        any operator=(int var);

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

        operator bool() const;

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
        void *ret = &this->valueChar;
        if (typeid(T) == typeid(bool) && this->typeInfo <= typeBool)
            ret = &this->valueBool;
        else if (typeid(T) == typeid(char) && this->typeInfo <= typeChar)
            ret = &this->valueChar;
        else if (typeid(T) == typeid(int) && this->typeInfo <= typeInt)
            ret = &this->valueInt;
        else if (typeid(T) == typeid(double) && this->typeInfo <= typeFloat)
            ret = &this->valueFloat;
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