/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-02 11:32:09
 * @Description: file content
 */

#ifndef ___ANY_H___
#define ___ANY_H___

#include <string>
#include "Exception.h"

namespace AVSI
{
    typedef enum
    {
        EMPTY   = 0,
        CHAR    = 1,
        INTEGER = 2,
        FLOAT   = 3
    } DataType;

    class type_info
    {
    private:
        DataType type;
        std::string name;
    public:
        type_info(void);
        type_info(std::string type);

        std::string name();
        bool operator==(type_info type) const;
    };

    class any
    {
    private:
        int     valueInt;
        double  valueFloat;
        char    valueChar;
        type_info type;
    public:
        any(void);
        any(char var);
        any(int var);
        any(double var);
        ~any();

        template <typename T>
        T any_cast();
        template <typename T>
        T static any_cast(any var);

        any operator=(char var) const;
        any operator=(int var) const;
        any operator=(double var) const;
        bool operator==(char var) const;
        bool operator==(int var) const;
        bool operator==(double var) const;
        any operator+(const any var) const;
        any operator-(const any var) const;
        any operator*(const any var) const;
        any operator/(const any var) const;
    };

    static const type_info typeInt = type_info("integer");
    static const type_info typeFloat = type_info("float");
    static const type_info typeChar = type_info("char");
}

#endif