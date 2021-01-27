/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-27 22:52:41
 * @Description: file content
 */
#include "../inc/CallStack.h"

namespace AVSI {
    void ActivationRecord::__setitem__(string key, any value) {
        this->members[key] = value;
    }

    any ActivationRecord::__getitem__(string key) { return members[key]; }

    string ActivationRecord::__str__() {
        ostringstream str;
        str << "\033[34mAR: " << this->name
            << "    type:" << ARTypeMap.find(this->type)->second << "\033[0m\n";
        str << "\033[32m";
        for (int i = 0; i < (int) (13 + this->name.length() +
                                   ARTypeMap.find(this->type)->second.length());
             i++)
            str << "-";
        str << "\033[0m";
        str << "\n";
        for (std::pair<std::string,any> item : this->members) {
            str << "    <" << item.first << ":" << item.second << ">\n";
        }
        str << "\n";
        return str.str();
    }

    void CallStack::push(ActivationRecord *ar) { this->records.push_back(ar); }

    ActivationRecord *CallStack::CallStack::pop() {
        ActivationRecord *ar = this->records.back();
        this->records.pop_back();
        return ar;
    }

    ActivationRecord *CallStack::CallStack::peek() {
        return this->records.back();
    }
} // namespace AVSI