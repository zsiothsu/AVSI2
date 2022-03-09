/*
 * @Author: your name
 * @Date: 2020-05-19
 * @LastEditTime: 2020-06-02 15:55:41
 * @Description: file content
 */
#ifndef ___SYMBOLTABLE_H___
#define ___SYMBOLTABLE_H___

#include "flags.h"
#include <cstring>
#include <deque>
#include <iostream>
#include <map>

namespace AVSI {
    using std::clog;
    using std::deque;
    using std::endl;
    using std::map;
    using std::string;
    using std::to_string;

    typedef enum {
        null_t, variable_t, function_t
    } SymbolType;

    // typedef struct
    // {
    //     string name;
    //     SymbolType type;
    //     any value;
    // } Symbol;

    class Symbol {
    public:
        string name;
        SymbolType type;

        Symbol(void) {};

        Symbol(string name, SymbolType type) : name(name), type(type) {};
    };

    class Symbol_function : public Symbol {
    public:
        deque<Symbol *> formal_variable;
        void *node_ast;

        Symbol_function(void)
                : formal_variable(deque<Symbol *>()), node_ast(nullptr) {};

        Symbol_function(string name, SymbolType type)
                : Symbol(name, type),
                  formal_variable(deque<Symbol *>()),
                  node_ast(nullptr) {};
    };

    class SymbolMap {
    public:
        map<string, Symbol *> symbols;
        string name;
        uint64_t addr;

        SymbolMap(void) : symbols(map<string, Symbol *>()) {};

        SymbolMap(string name) : symbols(map<string, Symbol *>()), name(name) {};

        SymbolMap(string name, uint64_t addr) : symbols(map<string, Symbol *>()), name(name), addr(addr) {};

        ~SymbolMap();

        void insert(Symbol *symbol);

        Symbol *find(string name);

        string __str();
    };

    class SymbolTable {
    public:
        SymbolMap *symbol_map;
        SymbolTable *father;
        deque<SymbolTable *> child;
        int level;

        SymbolTable(void)
                : symbol_map(nullptr),
                  father(nullptr),
                  child(deque<SymbolTable *>()),
                  level(1) {};

        SymbolTable(SymbolTable *father, string name, int level)
                : symbol_map(new SymbolMap(name)),
                  father(father),
                  child(deque<SymbolTable *>()),
                  level(level) {};

        SymbolTable(SymbolTable *father, string name, uint64_t addr, int level)
                : symbol_map(new SymbolMap(name, addr)),
                  father(father),
                  child(deque<SymbolTable *>()),
                  level(level) {};

        ~SymbolTable();

        void insert(Symbol *symbol);

        Symbol *find(string name);

        void mount(SymbolTable *symbolTable);

        void __str();
    };

    static map<SymbolType, string> symbol_typeName = {{variable_t, "variable"},
                                                      {function_t, "function"},
                                                      {null_t,     "null"}};
} // namespace AVSI

#endif