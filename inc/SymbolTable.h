/*
 * @Author: your name
 * @Date: 2020-05-19
 * @LastEditTime: 2020-05-19 22:54:16
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

    typedef enum
    {
        null_t,
        variable_t
    } SymbolType;

    typedef struct
    {
        string name;
        SymbolType type;
    } Symbol;

    class SymbolMap
    {
    private:
        map<string,Symbol> symbols;
    public:
        SymbolMap(void);
        ~SymbolMap() {};

        void insert(Symbol symbol);
        Symbol find(string name);
    };

    class SymbolTable
    {
    private:
        deque<SymbolMap> SymbolStack;
    public:
        SymbolTable(void);
        ~SymbolTable() {};
        
        void insert(Symbol symbol);
        Symbol find(string name);
        
        void pop(void);
        void push(void);
    };
}

#endif