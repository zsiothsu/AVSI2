/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-20 10:47:52
 * @Description: "AVSI::any" is a data type which can store variable of basic type
 */

#ifndef ___ANY_H___
#define ___ANY_H___

#include <string>
#include <map>
#include <iostream>
#include <regex>
#include <cmath>

namespace AVSI
{
    using std::string;
    using std::map;
    using std::istream;
    using std::ostream;
    using std::regex;
    using std::regex_match;
    using std::match_results;
    
    typedef enum
    {
        EMPTY   = 0,
        CHAR    = 1,
        INTEGER = 2,
        FLOAT   = 3,
        STRING  = 4
    } DataType;

    class type_info
    {
    private:
        DataType typeId;
        std::string typeName;
    public:
        type_info(void);
        type_info(DataType type);

        std::string name(void);
        DataType type(void);
        bool operator==(type_info type);
    };

    class any
    {
    private:
        int     valueInt;
        double  valueFloat;
        char    valueChar;
        string  valueString;
        type_info typeInfo;
    public:
        any(void);
        any(char var);
        any(int var);
        any(double var);
        any(string var);
        ~any();

        DataType type(void);

        template <typename T>
        T any_cast(void);

        template <typename T>
        static T any_cast(any var);

        any operator=(char var);
        any operator=(int var);
        any operator=(double var);
        any operator=(string var);
        bool operator==(char var) const;
        bool operator==(int var) const;
        bool operator==(double var) const;
        bool operator==(string var) const;
        any operator+(any var);
        any operator-(any var);
        any operator*(any var);
        any operator/(any var);

        friend ostream& operator<<(ostream& output,any& d);
        friend istream& operator>>(istream& input,any& d);
    };

    static map<DataType,string> typeMap = {
        {EMPTY,"empty"},
        {CHAR,"char"},
        {INTEGER,"int"},
        {FLOAT,"float"},
        {STRING,"string"}
    };

    static const regex numPattern = regex("^(\\+|-)?(0|[1-9][0-9]*)(\\.[1-9]+)?((e|E)(\\+|-)?(0|[1-9][0-9]*))?$");

    static type_info typeEMPTY = type_info(EMPTY);
    static type_info typeInt = type_info(INTEGER);
    static type_info typeFloat = type_info(FLOAT);
    static type_info typeChar = type_info(CHAR);
    static type_info typeString = type_info(STRING);

    template <typename T>
    T any::any_cast(void)
    {
        void* ret = &this->valueChar;
        if(typeid(T) == typeid(char) && \
           this->typeInfo == typeChar
        ) ret = &this->valueChar;

        else if(typeid(T) == typeid(int) &&  \
           (this->typeInfo == typeInt || \
            this->typeInfo == typeChar)
        ) ret = &this->valueInt;

        else if(typeid(T) == typeid(double) &&  \
           (this->typeInfo == typeInt || \
            this->typeInfo == typeChar || \
            this->typeInfo == typeFloat)
        ) ret = &this->valueFloat;
        else if(typeid(T) == typeid(string)) ret = &this->valueString;
        T Tval = *((T*)ret);
        return Tval;
    }

    template <typename T>
    T any::any_cast(any var)
    {
        return var.any_cast<T>();
    }
}

#endif