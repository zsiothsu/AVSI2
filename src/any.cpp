/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:52:24
 * @Description: "AVSI::any" is a data type which can store variable of basic
 * type
 */
#include <cmath>

#include "../inc/any.h"
#include "../inc/Exception.h"

namespace AVSI {
    type_info::type_info(void) {
        this->typeId = Empty;
        map<DataType, string>::const_iterator where = typeMap.find(Empty);
        if (where != typeMap.end()) this->typeName = where->second;
    }

    type_info::type_info(DataType type) {
        this->typeId = type;
        map<DataType, string>::const_iterator where = typeMap.find(type);
        if (where != typeMap.end()) this->typeName = where->second;
    }

    std::string type_info::name(void) { return this->typeName; }

    DataType type_info::type(void) { return this->typeId; }

    bool type_info::operator==(type_info type) {
        return this->typeId == type.type();
    }

    bool type_info::operator>=(const type_info type) const {
        return this->typeId >= type.typeId;
    }

    bool type_info::operator<=(const type_info type) const {
        return this->typeId <= type.typeId;
    }


    any::any(void) { this->typeInfo = typeEmpty; }

    any::any(bool var) {
        this->typeInfo = typeBool;
        this->valueBool = var;
        this->valueChar = this->valueFloat = this->valueInt = 1;
    }

    any::any(char var) {
        this->typeInfo = typeChar;
        this->valueChar = this->valueInt = this->valueFloat = var;
        this->valueBool = (bool)var;
    }

    any::any(int var) {
        this->typeInfo = typeInt;
        this->valueInt = this->valueFloat = var;
        this->valueBool = (bool)var;
    }

    any::any(double var) {
        this->typeInfo = typeFloat;
        this->valueFloat = var;
        this->valueBool = (bool)var;
    }

    any::any(string var) {
        this->typeInfo = typeString;
        this->valueString = var;
    }

    any::~any() {}

    DataType any::type(void) { return this->typeInfo.type(); }

    any any::operator=(bool var) {
        this->typeInfo = typeBool;
        this->valueChar = this->valueFloat = this->valueInt = var;
        return this->valueBool = var;
    }

    any any::operator=(char var) {
        this->typeInfo = typeChar;
        this->valueBool = (bool)var;
        return this->valueChar = this->valueInt = this->valueFloat = var;
    }

    any any::operator=(int var) {
        this->typeInfo = typeInt;
        this->valueBool = (bool)var;
        return this->valueInt = this->valueFloat = var;
    }

    any any::operator=(double var) {
        this->typeInfo = typeFloat;
        this->valueBool = (bool)var;
        return this->valueFloat = var;
    }

    any any::operator=(string var) {
        this->typeInfo = typeString;
        return this->valueString = var;
    }

    any any::operator+(any var) {
        checkOperand(this->type(), var.type(), "+");

        if (this->typeInfo == typeString) return any(string(this->valueString + (string)var));
        if (var.typeInfo == typeString) return any(string((string)(*this) + var.valueString));

        if (this->typeInfo.type() >= var.typeInfo.type()) {
            if (this->typeInfo == typeBool)
                return any(this->any_cast<bool>() + var.any_cast<bool>());
            if (this->typeInfo == typeChar)
                return any(this->any_cast<char>() + var.any_cast<char>());
            if (this->typeInfo == typeInt)
                return any(this->any_cast<int>() + var.any_cast<int>());
            if (this->typeInfo == typeFloat)
                return any(this->any_cast<double>() + var.any_cast<double>());
        } else {
            if (var.typeInfo == typeBool)
                return any(this->any_cast<bool>() + var.any_cast<bool>());
            if (var.typeInfo == typeChar)
                return any(this->any_cast<char>() + var.any_cast<char>());
            if (var.typeInfo == typeInt)
                return any(this->any_cast<int>() + var.any_cast<int>());
            if (var.typeInfo == typeFloat)
                return any(this->any_cast<double>() + var.any_cast<double>());
        }
        return 0;
    }

    any any::operator-(any var) {
        checkOperand(this->type(), var.type(), "-");

        if (this->typeInfo.type() >= var.typeInfo.type()) {
            if (this->typeInfo == typeBool)
                return any(this->any_cast<bool>() - var.any_cast<bool>());
            if (this->typeInfo == typeChar)
                return any(this->any_cast<char>() - var.any_cast<char>());
            if (this->typeInfo == typeInt)
                return any(this->any_cast<int>() - var.any_cast<int>());
            if (this->typeInfo == typeFloat)
                return any(this->any_cast<double>() - var.any_cast<double>());
        } else {
            if (var.typeInfo == typeBool)
                return any(this->any_cast<bool>() - var.any_cast<bool>());
            if (var.typeInfo == typeChar)
                return any(this->any_cast<char>() - var.any_cast<char>());
            if (var.typeInfo == typeInt)
                return any(this->any_cast<int>() - var.any_cast<int>());
            if (var.typeInfo == typeFloat)
                return any(this->any_cast<double>() - var.any_cast<double>());
        }
        return 0;
    }

    any any::operator*(any var) {
        checkOperand(this->type(), var.type(), "*");

        if (this->typeInfo.type() >= var.typeInfo.type()) {
            if (this->typeInfo == typeBool)
                return any(this->any_cast<bool>() * var.any_cast<bool>());
            if (this->typeInfo == typeChar)
                return any(this->any_cast<char>() * var.any_cast<char>());
            if (this->typeInfo == typeInt)
                return any(this->any_cast<int>() * var.any_cast<int>());
            if (this->typeInfo == typeFloat)
                return any(this->any_cast<double>() * var.any_cast<double>());
        } else {
            if (var.typeInfo == typeBool)
                return any(this->any_cast<bool>() * var.any_cast<bool>());
            if (var.typeInfo == typeChar)
                return any(this->any_cast<char>() * var.any_cast<char>());
            if (var.typeInfo == typeInt)
                return any(this->any_cast<int>() * var.any_cast<int>());
            if (var.typeInfo == typeFloat)
                return any(this->any_cast<double>() * var.any_cast<double>());
        }
        return 0;
    }

    any any::operator/(any var) {
        checkOperand(this->type(), var.type(), "/");

        if (this->typeInfo.type() >= var.typeInfo.type()) {
            if (this->typeInfo == typeBool)
                return any(this->any_cast<bool>() / var.any_cast<bool>());
            if (this->typeInfo == typeChar)
                return any(this->any_cast<char>() / var.any_cast<char>());
            if (this->typeInfo == typeInt)
                return any(this->any_cast<int>() / var.any_cast<int>());
            if (this->typeInfo == typeFloat)
                return any(this->any_cast<double>() / var.any_cast<double>());
        } else {
            if (var.typeInfo == typeBool)
                return any(this->any_cast<bool>() / var.any_cast<bool>());
            if (var.typeInfo == typeChar)
                return any(this->any_cast<char>() / var.any_cast<char>());
            if (var.typeInfo == typeInt)
                return any(this->any_cast<int>() / var.any_cast<int>());
            if (var.typeInfo == typeFloat)
                return any(this->any_cast<double>() / var.any_cast<double>());
        }
        return 0;
    }

    bool any::operator==(bool var) const {
        return this->valueBool == var;
    }

    bool any::operator==(char var) const {
        return this->valueFloat == (double) var;
    }

    bool any::operator==(int var) const {
        return this->valueFloat == (double) var;
    }

    bool any::operator==(double var) const { return this->valueFloat == var; }

    bool any::operator==(string var) const { return this->valueString == var; }

    bool any::operator>(any var) const { return (this->valueFloat - var.valueFloat) > 0;}

    bool any::operator<(any var) const { return (this->valueFloat - var.valueFloat) < 0;}

    bool any::operator>=(any var) const { return (this->valueFloat - var.valueFloat) >= 0;}

    bool any::operator<=(any var) const { return (this->valueFloat - var.valueFloat) <= 0;}

    any::operator bool() {
        return this->valueBool;
    }

    any::operator string() {
        if (this->typeInfo == typeBool) return string((this->valueBool == true) ? "true" : "false");
        else if (this->typeInfo == typeChar) return to_string(this->valueChar);
        else if (this->typeInfo == typeInt) return to_string(this->valueInt);
        else if (this->typeInfo == typeFloat) return to_string(this->valueFloat);
        else if (this->typeInfo == typeString) return this->valueString;
        else return string();
    }

    ostream &operator<<(ostream &output, AVSI::any &d) {
        if (d.typeInfo == typeEmpty) output << "None";
        else if (d.typeInfo == typeBool) output << (d.any_cast<bool>() == true ? "True" : "False");
        else if (d.typeInfo == typeChar) output << d.any_cast<char>();
        else if (d.typeInfo == typeInt) output << d.any_cast<int>();
        else if (d.typeInfo == typeFloat) output << d.any_cast<double>();
        else if (d.typeInfo == typeString) output << d.valueString;
        return output;
    }

    istream &operator>>(istream &input, AVSI::any &d) {
        string str;
        input >> str;
        int len = str.length();
        if (regex_match(str, numPattern)) {
            int index = 0;
            double num = 0, scale = 0;
            int subscale = 0, signsubscale = 1;
            if (str[index] == '0') index++; // is zero
            if (index < len && str[index] >= '1' && str[index] <= '9')
                do {
                    num = num * 10.0 + (str[index] - '0');
                    index++;
                } while (index < len && str[index] >= '0' &&
                         str[index] <= '9'); // is number ?
            if (index < len && str[index] == '.' && str[index + 1] >= '0' &&
                str[index + 1] <= '9') {
                index++;
                do {
                    num = num * 10.0 + (str[index] - '0');
                    scale--;
                    index++;
                } while (index < len && str[index] >= '0' && str[index] <= '9');
            } // fractional part?
            if ((index < len) && ((str[index] == 'e') || str[index] == 'E')) {
                index++;
                if (str[index] == '+')
                    index++;
                else if (str[index] == '-')
                    signsubscale = -1, index++;
                while (index < len && str[index] >= '0' && str[index] <= '9')
                    subscale = subscale * 10 + (str[index] - '0'), index++;
            } // exponent?
            num = num * pow(10.0, (scale + subscale * signsubscale));
            if (scale == 0 && signsubscale == 1)
                d = (int) num;
            else
                d = num;
        } else {
            // TODO string
            /* code */
        }

        return input;
    }

    bool checkOperand(DataType left, DataType right, string op) {
        if (left == Empty || right == Empty) {
            string msg = "unsupported operand type(s) for " +
                         op + ": '" + typeMap[left] +
                         "' and '" + typeMap[right] + "'";
            throw ExceptionFactory(__TypeException, msg, 0, 0);
        }
        return true;
    }
} // namespace AVSI