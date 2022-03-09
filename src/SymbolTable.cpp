/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-06-02 15:58:45
 * @Description: file content
 */
#include "../inc/SymbolTable.h"

namespace AVSI {
    SymbolMap::~SymbolMap() {
        if (!this->symbols.empty()) {
            for (auto symbol : this->symbols) {
                if (symbol.second != nullptr) delete symbol.second;
            }
        }
    }

    void SymbolMap::insert(Symbol *symbol) {
        this->symbols[symbol->name] = symbol;
    }

    Symbol *SymbolMap::find(string name) {
        Symbol *ret = nullptr;
        map<string, Symbol *>::iterator iter = this->symbols.find(name);
        if (iter != this->symbols.end()) ret = iter->second;
        return ret;
    }

    string SymbolMap::__str() {
        string str;
        for (auto symbol : this->symbols) {
            string symbolName = symbol.second->name;
            SymbolType symbolType = symbol.second->type;

            if (symbolType == variable_t)
                str += "    <" + symbolName + "," +
                       symbol_typeName.find(variable_t)->second + ">\n";
            else if (symbolType == function_t) {
                Symbol_function *fun = (Symbol_function *) (symbol.second);

                str += "    <" + symbolName + "," +
                       symbol_typeName.find(function_t)->second;

                if (!(fun->formal_variable.empty())) {
                    str += ",param{";
                    for (auto param : fun->formal_variable) {
                        str += param->name + ",";
                    }
                    str += "\b}";
                }
                str += ">\n";
            }
        }
        return str;
    }

    SymbolTable::~SymbolTable() {
        if (this->symbol_map != nullptr) delete this->symbol_map;
        if (!this->child.empty()) {
            for (auto subtable : this->child) {
                if (subtable != nullptr) delete subtable;
            }
        }
    }

    void SymbolTable::insert(Symbol *symbol) {
        this->symbol_map->insert(symbol);
    }

    Symbol *SymbolTable::find(string name) {
        SymbolTable *ptr = this;
        while (ptr != nullptr) {
            Symbol *symbol = ptr->symbol_map->find(name);
            if (symbol != nullptr)
                return symbol;
            else
                ptr = ptr->father;
        }

        return nullptr;
    }

    void SymbolTable::mount(SymbolTable *symbolTable) {
        this->child.push_back(symbolTable);
    }

    void SymbolTable::__str() {
        if (FLAGS_scope == true) {
            // header
            clog << "\033[34mscope: " << symbol_map->name
                 << " level: " << to_string(this->level) << endl;

            clog << "\033[32m";
            for (int i = 0; i <= 14 + (int) (symbol_map->name.length()) +
                                 (int) (to_string(this->level).length());
                 i++)
                clog << '-';
            clog << endl;
            clog << "\033[0m";
            // contents
            string str = this->symbol_map->__str();
            clog << str << endl;
            // child
            for (auto subtable : this->child) { subtable->__str(); }
        }
    }
} // namespace AVSI