/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 21:18:00
 * @Description: file content
 */
#include "../inc/Data.h"

namespace INTERPRETER
{
    Data::Data(void)
    {
    }
    
    Data::Data(int data)
    {
        this->valueInt = data;
    }

    Data::~Data()
    {
    }

    void Data::getValue(int& num)
    {
        num = this->valueInt;
    }
}