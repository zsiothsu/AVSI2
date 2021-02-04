/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-06-02 15:55:34
 * @Description: file content
 */
#ifndef ___CALLSTACK_H___
#define ___CALLSTACK_H___

#include "any.h"
#include "flags.h"
#include <deque>
#include <map>
#include <sstream>
#include <string>

namespace AVSI {
    using std::deque;
    using std::map;
    using std::ostringstream;
    using std::string;

    typedef enum {
        program, function, ifScope, loopScope
    } ARType;

    class ActivationRecord {
    private:
        string name;
        map<string, any> members;

    public:
        int level;
        ARType type;

        ActivationRecord(void) : members(map<string, any>()) {};

        ActivationRecord(string name, ARType type, int level)
                : name(name),
                  members(map<string, any>()),
                  level(level),
                  type(type) {};

        virtual ~ActivationRecord() {};

        void __setitem__(string key, any value);

        any __getitem__(string key);

        string __str__();
    };

    class CallStack {
    private:
        deque<ActivationRecord *> records;

    public:
        CallStack(void) : records(deque<ActivationRecord *>()) {};

        virtual ~CallStack() {};

        void push(ActivationRecord *ar);

        ActivationRecord *pop();

        ActivationRecord *peek();

        void __setitem__(string key, any value);

        any __getitem__(string key);
    };

    static map<ARType, string> ARTypeMap = {{function, "function"},
                                            {program,  "program"},
                                            {ifScope, "if"}};
} // namespace AVSI

#endif