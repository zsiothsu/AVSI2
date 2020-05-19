/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-19 21:29:30
 * @Description: file content
 */ 
#include "../inc/SymbolTable.h"

namespace AVSI
{
    SymbolMap::SymbolMap(void)
    {
        this->symbols = map<string,Symbol>();
    }

    void SymbolMap::insert(Symbol symbol)
    {
        this->symbols[symbol.name] = symbol;
    }

    Symbol SymbolMap::find(string name)
    {
        Symbol ret = {name,null_t};
        map<string,Symbol>::iterator iter = this->symbols.find(name);
        if(iter != this->symbols.end()) ret = (*iter).second;
        return ret;
    }

    SymbolTable::SymbolTable(void)
    {
        this->SymbolStack = deque<SymbolMap>();
        SymbolStack.push_back(SymbolMap());
    }

    void SymbolTable::insert(Symbol symbol)
    {
        SymbolMap& symbolmap = this->SymbolStack.back();
        symbolmap.insert(symbol);
    }

    Symbol SymbolTable::find(string name)
    {
        for(auto symbolmap = this->SymbolStack.rbegin();
            symbolmap != this->SymbolStack.rend();
            symbolmap++)
        {
            Symbol symbol = (*symbolmap).find(name);
            if(symbol.type != null_t) return symbol;
        }
        return {name,null_t};
    }

    void SymbolTable::pop()
    {
        this->SymbolStack.pop_back();
    }

    void SymbolTable::push()
    {
        this->SymbolStack.push_back(SymbolMap());
    }
}