/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-12 11:07:07
 * @Description: file content
 */
#include "../inc/any.h"

namespace AVSI
{
    type_info::type_info(void)
    {
        this->typeId = EMPTY;
        map<DataType,string>::const_iterator where = typeMap.find(EMPTY);
        if(where != typeMap.end()) this->typeName = where->second;
    }
    type_info::type_info(DataType type)
    {
        this->typeId = type;
        map<DataType,string>::const_iterator where = typeMap.find(type);
        if(where != typeMap.end()) this->typeName = where->second;
    }
    std::string type_info::name(void) { return this->typeName; }
    DataType type_info::type(void) { return this->typeId; }
    bool type_info::operator==(type_info type)
    { 
        return this->typeId == type.type();
    }

    any::any(void) { this->typeInfo = typeEMPTY; }
    any::any(char var) { this->typeInfo = typeChar; this->valueChar = this->valueInt = this->valueFloat = var; }
    any::any(int var) { this->typeInfo = typeInt; this->valueInt = this->valueFloat = var; }
    any::any(double var) { this->typeInfo =typeFloat; this->valueFloat = var; }
    any::any(string var) {this->typeInfo = typeString; this->valueString = var; }
    any::~any() {}

    DataType any::type(void)
    {
        return this->typeInfo.type();
    }

    any any::operator=(char var) { this->typeInfo = typeChar; return this->valueChar = this->valueInt = this->valueFloat = var;}
    any any::operator=(int var) { this->typeInfo = typeInt; return this->valueInt = this->valueFloat = var;}
    any any::operator=(double var) { this->typeInfo = typeFloat; return this->valueFloat = var;}
    any any::operator=(string var) { this->typeInfo = typeString; return this->valueString = var;}

    bool any::operator==(char var) const { return this->valueFloat == (double)var;}
    bool any::operator==(int var) const { return this->valueFloat == (double)var;}
    bool any::operator==(double var) const { return this->valueFloat == var;}
    bool any::operator==(string var) const { return this->valueString == var; }

    //TODO: string
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
    }

    ostream& operator<<(ostream& output,AVSI::any& d)
    {//TODO:string
        if(d.typeInfo == typeChar) output << d.any_cast<char>();
        if(d.typeInfo == typeInt) output << d.any_cast<int>();
        if(d.typeInfo == typeFloat) output << d.any_cast<double>();
        return output;
    }

    istream& operator>>(istream& input,AVSI::any& d)
    {
        string str;
        input >> str;
        int len = str.length();
        if(regex_match(str,numPattern))
        {
            int index = 0;
            double num = 0,scale = 0;
            int subscale = 0,signsubscale = 1;
            if(str[index] == '0') index++; // is zero
            if(index < len && str[index] >= '1' && str[index] <= '9') do { num = num * 10.0 + (str[index] - '0'); index++; } while(index < len && str[index] >= '0' && str[index] <= '9'); // is number ?
            if(index < len && str[index] == '.' && str[index+1] >= '0' && str[index+1] <= '9')
            {
                index++;
                do
                {
                    num = num * 10.0 + (str[index] - '0');
                    scale--;
                    index++;
                } while(index < len && str[index] >= '0' && str[index] <= '9');
            } // fractional part?
            if((index < len) && ((str[index] == 'e') || str[index] == 'E'))
            {
                index++; if(str[index] == '+') index++; else if(str[index] == '-') signsubscale = -1,index++;
                while(index < len && str[index] >= '0' && str[index] <= '9') subscale = subscale * 10 + (str[index] - '0'),index++;
            } // exponent?
            num = num * pow(10.0,(scale + subscale * signsubscale));
            if(scale == 0 && signsubscale == 1) d = (int)num;
            else d = num;
        }
        else
        {
            //TODO string
            /* code */
        }
        
        return input;
    }
}