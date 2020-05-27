/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-27 22:46:10
 * @Description: file content
 */ 
#ifndef ___CALLSTACK_H___
#define ___CALLSTACK_H___

#include <deque>
#include <map>
#include <string>
#include <sstream>
#include "any.h"
#include "flags.h"

namespace AVSI
{
    using std::deque;
    using std::map;
    using std::string;
    using std::ostringstream;


    typedef enum
    {
        program,
        function
    } ARType;

    class ActivationRecord
    {
    private:
        string name;
        ARType type;
        int level;
        map<string,any> members;
    public:
        ActivationRecord(void):
            members(map<string,any>())
        {};
        ActivationRecord(string name,ARType type,int level):
            name(name),
            type(type),
            level(level),
            members(map<string,any>())
        {};
        virtual ~ActivationRecord() {};

        void __setitem__(string key,any value);
        any __getitem__(string key);
        string __str__();
    };

    class CallStack
    {
    private:
        deque<ActivationRecord*> records;
    public:
        CallStack(void): records(deque<ActivationRecord*>()) {};
        virtual ~CallStack() {};

        void push(ActivationRecord* ar);
        ActivationRecord* pop();
        ActivationRecord* peek();
    };

    static map<ARType,string> ARTypeMap = {
        {function,"function"},
        {program,"program"}
    };
}

#endif