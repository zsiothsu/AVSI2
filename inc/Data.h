/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 21:12:30
 * @Description: file content
 */
#ifndef ___DATA_H___
#define ___DATA_H___

#include "AST.h"

namespace INTERPRETER
{
    typedef enum
    {
        INTEGER = 0
    } DataType;

    class Data
    {
    private:
        int valueInt;
    public:
        Data(void);
        Data(int data);
        ~Data();

        void getValue(int& num);
    };
        
}

 #endif