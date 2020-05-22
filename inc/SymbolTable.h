/*
 * @Author: your name
 * @Date: 2020-05-19
 * @LastEditTime: 2020-05-22 20:25:52
 * @Description: file content
 */ 
#ifndef ___SYMBOLTABLE_H___
#define ___SYMBOLTABLE_H___

#include <deque>
#include "NodeVisitor.h"

namespace AVSI
{
    using std::map;
    using std::deque;
    using std::string;
    using std::clog;
    using std::endl;

    typedef enum
    {
        null_t,
        variable_t,
        function_t
    } SymbolType;

    typedef struct
    {
        string name;
        SymbolType type;
        any value;
    } Symbol;

    class SymbolMap
    {
    private:
        map<string,Symbol> symbols;
    public:
        string name;

        SymbolMap(void): symbols(map<string,Symbol>()) {};
        SymbolMap(string name):
            symbols(map<string,Symbol>()),
            name(name)
        {};
        ~SymbolMap() {};

        void insert(Symbol symbol);
        Symbol find(string name);
        string __str();
    };

    class SymbolTable
    {
    private:
        deque<SymbolMap> SymbolStack;
    public:
        int level;

        SymbolTable(void);
        ~SymbolTable() {};
        
        void insert(Symbol symbol);
        Symbol find(string name);
        
        void pop(void);
        void push(string name);
    };

    static map<SymbolType,string> symbolTypeName = {
        {variable_t ,"variable"},
        {function_t ,"function"},
        {null_t     ,"null"}
    };
}

#endif