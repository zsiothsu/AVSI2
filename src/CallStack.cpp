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

    any ActivationRecord::__getitem__(string key) {
        for(auto it = members.begin();it != members.end();it++)
        {
            if(it->first == key) return it->second;
        }
        return any();
    }

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

    ActivationRecord *CallStack::global() {
        return this->records.front();
    }

    void CallStack::__setitem__(string key, any value) {
        auto it = this->records.rbegin();
        do
        {
            any ret = (*it)->__getitem__(key);
            if(ret.type() != Empty){
                (*it)->__setitem__(key,value);
                return;
            }
            
            if ((*it)->type == ifScope || (*it)->type == loopScope){
                it++;
            }
            else break;
        } while(it != this->records.rend());

        this->records.back()->__setitem__(key,value);
    }

    any CallStack::__getitem__(string key) {
        auto it = this->records.rbegin();
        do
        {
            any ret = (*it)->__getitem__(key);
            if(ret.type() != Empty) return ret;

            if ((*it)->type == ifScope || (*it)->type == loopScope){
                it++;
            }
            else break;
        } while(it != this->records.rend());

        return any();
    }

} // namespace AVSI