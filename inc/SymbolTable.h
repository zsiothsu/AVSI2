/*
 * @Author: your name
 * @Date: 2020-05-19
 * @LastEditTime: 2020-05-27 23:51:29
 * @Description: file content
 */ 
#ifndef ___SYMBOLTABLE_H___
#define ___SYMBOLTABLE_H___

#include <deque>
#include "NodeVisitor.h"
#include "flags.h"

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
        SymbolMap symbolMap;
    public:
        SymbolTable* father;
        deque<SymbolTable*> child;
        int level;

        SymbolTable(void):
            father(nullptr),
            child(deque<SymbolTable*>()),
            level(1)
        {};

        SymbolTable(SymbolTable* father,string name, int level):
            symbolMap(SymbolMap(name)),
            father(father),
            child(deque<SymbolTable*>()),
            level(level)
        {};

        ~SymbolTable();
        
        void insert(Symbol symbol);
        Symbol find(string name);
        void mount(SymbolTable* symbolTable);
        void __str();
    };

    static map<SymbolType,string> symbolTypeName = {
        {variable_t ,"variable"},
        {function_t ,"function"},
        {null_t     ,"null"}
    };
}

#endif