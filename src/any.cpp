/*
 * any.cpp 2022
 *
 * "AVSI::any" is a data type which can store variable of basic types
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
        this->valueChar = (char) var;
        this->valueInt = (int) var;
        this->valueFloat = (float) var;
        this->valueDouble = (double) var;
        this->valueBool = var;
    }

    any::any(char var) {
        this->typeInfo = typeChar;
        this->valueChar = var;
        this->valueInt = (int) var;
        this->valueFloat = (float) var;
        this->valueDouble = (double ) var;
        this->valueBool = (bool) var;
    }

    any::any(int var) {
        this->typeInfo = typeInt;
        this->valueChar = (char) var;
        this->valueInt = var;
        this->valueFloat = (float) var;
        this->valueDouble = (float) var;
        this->valueBool = (bool) var;
    }

    any::any(float var) {
        this->typeInfo = typeFloat;
        this->valueDouble = (double)var;
        this->valueFloat = var;
        this->valueInt = (int) var;
        this->valueBool = (bool) var;
        this->valueChar = (char) var;
    }

    any::any(double var) {
        this->typeInfo = typeFloat;
        this->valueDouble = var;
        this->valueFloat = (float) var;
        this->valueInt = (int) var;
        this->valueBool = (bool) var;
        this->valueChar = (char) var;
    }

    any::any(char *var) {
        this->typeInfo = typeString;
        this->valueString = string(var);
    }

    any::any(string var) {
        this->typeInfo = typeString;
        this->valueString = var;
    }

    any::~any() {}

    DataType any::type(void) { return this->typeInfo.type(); }

    any any::operator=(bool var) {
        this->typeInfo = typeBool;
        this->valueDouble = (double) var;
        this->valueFloat = (float) var;
        this->valueInt = (int) var;
        this->valueChar = (char) var;
        return this->valueBool = var;
    }

    any any::operator=(char var) {
        this->typeInfo = typeChar;
        this->valueBool = (bool) var;
        this->valueFloat = (float) var;
        this->valueDouble = (double) var;
        this->valueInt = (int) var;
        return this->valueChar = var;
    }

    any any::operator=(int var) {
        this->typeInfo = typeInt;
        this->valueBool = (bool) var;
        this->valueFloat = (float) var;
        this->valueDouble = (double) var;
        this->valueChar = (char) var;
        return this->valueInt = var;
    }

    any any::operator=(float var) {
        this->typeInfo = typeFloat;
        this->valueBool = (bool) var;
        this->valueChar = (char) var;
        this->valueInt = (int) var;
        this->valueDouble = (double) var;
        return this->valueFloat = var;
    }

    any any::operator=(double var) {
        this->typeInfo = typeFloat;
        this->valueBool = (bool) var;
        this->valueChar = (char) var;
        this->valueInt = (int) var;
        this->valueFloat = (float) var;
        return this->valueDouble = var;
    }

    any any::operator=(string var) {
        this->typeInfo = typeString;
        return this->valueString = var;
    }

    any any::operator+(any var) {
        checkOperand(this->type(), var.type(), "+");

        if (this->typeInfo == typeString) return any(string(this->valueString + (string) var));
        if (var.typeInfo == typeString) return any(string((string) (*this) + var.valueString));

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
        return this->valueDouble == (double) var;
    }

    bool any::operator==(int var) const {
        return this->valueDouble == (double) var;
    }

    bool any::operator==(double var) const { return this->valueDouble == var; }

    bool any::operator==(string var) const { return this->valueString == var; }

    bool any::operator>(any var) const { return (this->valueDouble - var.valueDouble) > 0; }

    bool any::operator<(any var) const { return (this->valueDouble - var.valueDouble) < 0; }

    bool any::operator>=(any var) const { return (this->valueDouble - var.valueDouble) >= 0; }

    bool any::operator<=(any var) const { return (this->valueDouble - var.valueDouble) <= 0; }

    any::operator bool() {
        return this->valueBool;
    }

    any::operator string() {
        if (this->typeInfo == typeBool) return string(this->valueBool ? "True" : "False");
        else if (this->typeInfo == typeChar) return string(1, this->valueChar);
        else if (this->typeInfo == typeInt) return to_string(this->valueInt);
        else if (this->typeInfo == typeFloat) return to_string(this->valueDouble);
        else if (this->typeInfo == typeString) return this->valueString;
        else return string();
    }

    ostream &operator<<(ostream &output, AVSI::any &d) {
        output << d.typeInfo.name() << " ";
        if (d.typeInfo == typeEmpty) output << "None";
        else if (d.typeInfo == typeBool) output << (d.any_cast<bool>() == true ? "True" : "False");
        else if (d.typeInfo == typeChar) output << d.any_cast<char>();
        else if (d.typeInfo == typeInt) output << d.any_cast<int>();
        else if (d.typeInfo == typeFloat) output << d.any_cast<double>();
        else if (d.typeInfo == typeString) output << d.valueString;
        return output;
    }

    /**
     * This was for the first generation of AVSI, but since there is no direct
     * input to variables now, this function is deprecated. If there are other
     * projects that need to use this function, you can try not to complete
     * this code, I left a todo there.
     */
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

    /**
     * deprecated by AVSI2
     */
    bool checkOperand(DataType left, DataType right, string op) {
        if (left == Empty || right == Empty) {
            string msg = "unsupported operand type(s) for " +
                         op + ": '" + typeMap[left] +
                         "' and '" + typeMap[right] + "'";
            throw ExceptionFactory<TypeException>(msg, 0, 0);
        }
        return true;
    }
} // namespace AVSI