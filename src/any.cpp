/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-02 16:00:38
 * @Description: file content
 */
#include "any.h"

namespace AVSI
{
    type_info::type_info(void) { this->typeId = EMPTY; this->typeName = typeMap[EMPTY]; }
    type_info::type_info(DataType type) { this->typeId = type; this->typeName = typeMap[type]; }
    std::string type_info::name(void) { return this->typeName; }
    DataType type_info::type(void) { return this->typeId; }
    bool type_info::operator==(type_info type) const
    { 
        return this->typeId == type.type();
    }

    any::any(void) { this->typeInfo = typeEMPTY; }
    any::any(char var) { this->typeInfo = typeChar; this->valueChar = this->valueInt = this->valueFloat = var; }
    any::any(int var) { this->typeInfo = typeInt; this->valueInt = this->valueFloat = var; }
    any::any(double var) { this->typeInfo =typeFloat; this->valueFloat = var; }

    DataType any::type(void)
    {
        return this->typeInfo.type();
    }

    template <typename T>
    T any::any_cast(void)
    {
        if(typeid(T) == typeid(char) &&
           this->typeInfo = typeChar
        ) return this->valueChar;

        if(typeid(T) == typeid(int) &&  \
           (this->typeInfo == typeInt || \
            this->typeInfo == typeChar)
        ) return this->valueInt;

        if(typeid(T) == typeid(double) &&  \
           (this->typeInfo.type() == typeInt || \
            this->typeInfo.type() == typeChar || \
            this->typeInfo.type() == typeFloat)
        ) return this->valueFloat;
    }

    template <typename T>
    T any::any_cast(any var)
    {
        return var.any_cast<T>();
    }

    any any::operator=(char var) { return this->valueChar = var;}
    any any::operator=(int var) { return this->valueInt = var;}
    any any::operator=(double var) { return this->valueFloat = var;}

    bool any::operator==(char var) const { return this->valueChar == var;}
    bool any::operator==(int var) const { return this->valueInt == var;}
    bool any::operator==(double var) const { return this->valueFloat == var;}

    any any::operator+(any var) 
    {
        if(this->typeInfo.type() >= var.typeInfo.type())
        {
            if(this->typeInfo == typeChar) return any(this->any_cast<char>() + var.any_cast<char>());
            if(this->typeInfo == typeInt) return any(this->any_cast<int>() + var.any_cast<int>());
            if(this->typeInfo == typeFloat) return any(this->any_cast<double>() + var.any_cast<double>());
        }
        else
        {
            if(var.typeInfo == typeChar) return any(this->any_cast<char>() + var.any_cast<char>());
            if(var.typeInfo == typeInt) return any(this->any_cast<int>() + var.any_cast<int>());
            if(var.typeInfo == typeFloat) return any(this->any_cast<double>() + var.any_cast<double>());
        }
    }

    any any::operator-(any var)
    {
        if(this->typeInfo.type() >= var.typeInfo.type())
        {
            if(this->typeInfo == typeChar) return any(this->any_cast<char>() - var.any_cast<char>());
            if(this->typeInfo == typeInt) return any(this->any_cast<int>() - var.any_cast<int>());
            if(this->typeInfo == typeFloat) return any(this->any_cast<double>() - var.any_cast<double>());
        }
        else
        {
            if(var.typeInfo == typeChar) return any(this->any_cast<char>() - var.any_cast<char>());
            if(var.typeInfo == typeInt) return any(this->any_cast<int>() - var.any_cast<int>());
            if(var.typeInfo == typeFloat) return any(this->any_cast<double>() - var.any_cast<double>());
        }
    }

    any any::operator*(any var)
    {
        if(this->typeInfo.type() >= var.typeInfo.type())
        {
            if(this->typeInfo == typeChar) return any(this->any_cast<char>() * var.any_cast<char>());
            if(this->typeInfo == typeInt) return any(this->any_cast<int>() * var.any_cast<int>());
            if(this->typeInfo == typeFloat) return any(this->any_cast<double>() * var.any_cast<double>());
        }
        else
        {
            if(var.typeInfo == typeChar) return any(this->any_cast<char>() * var.any_cast<char>());
            if(var.typeInfo == typeInt) return any(this->any_cast<int>() * var.any_cast<int>());
            if(var.typeInfo == typeFloat) return any(this->any_cast<double>() * var.any_cast<double>());
        }
    }

    any any::operator/(any var)
    {
        if(this->typeInfo.type() >= var.typeInfo.type())
        {
            if(this->typeInfo == typeChar) return any(this->any_cast<char>() / var.any_cast<char>());
            if(this->typeInfo == typeInt) return any(this->any_cast<int>() / var.any_cast<int>());
            if(this->typeInfo == typeFloat) return any(this->any_cast<double>() / var.any_cast<double>());
        }
        else
        {
            if(var.typeInfo == typeChar) return any(this->any_cast<char>() / var.any_cast<char>());
            if(var.typeInfo == typeInt) return any(this->any_cast<int>() / var.any_cast<int>());
            if(var.typeInfo == typeFloat) return any(this->any_cast<double>() / var.any_cast<double>());
        }
    }
}