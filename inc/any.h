/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-04 14:14:46
 * @Description: file content
 */

#ifndef ___ANY_H___
#define ___ANY_H___

#include <string>
#include <map>
#include "Exception.h"

namespace AVSI
{
    using std::map;
    using std::string;
    
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
        type_info typeInfo;
    public:
        any(void);
        any(char var);
        any(int var);
        any(double var);
        ~any();

        DataType type(void);

        template <typename T>
        T any_cast(void);
        template <typename T>
        static T any_cast(any var);

        any operator=(char var);
        any operator=(int var);
        any operator=(double var);
        bool operator==(char var) const;
        bool operator==(int var) const;
        bool operator==(double var) const;
        any operator+(any var);
        any operator-(any var);
        any operator*(any var);
        any operator/(any var);
    };

    static map<DataType,string> typeMap = {
        {EMPTY,"empty"},
        {CHAR,"char"},
        {INTEGER,"int"},
        {FLOAT,"float"}
    };

    //static type_info typeEMPTY = type_info(EMPTY);
    static type_info typeEMPTY = type_info();
    static type_info typeInt = type_info(CHAR);
    static type_info typeFloat = type_info(INTEGER);
    static type_info typeChar = type_info(FLOAT);

}

#endif