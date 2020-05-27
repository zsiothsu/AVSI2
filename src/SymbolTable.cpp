/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-27 14:40:58
 * @Description: file content
 */ 
#include "../inc/SymbolTable.h"

namespace AVSI
{
    void SymbolMap::insert(Symbol symbol)
    {
        this->symbols[symbol.name] = symbol;
    }

    Symbol SymbolMap::find(string name)
    {
        Symbol ret = {name,null_t};
        map<string,Symbol>::iterator iter = this->symbols.find(name);
        if(iter != this->symbols.end()) ret = iter->second;
        return ret;
    }

    string SymbolMap::__str()
    {
        string str;
        for(auto symbol : this->symbols)
        {
            string symbolName = symbol.second.name;
            string symbolType = (symbolTypeName.find(symbol.second.type))->second;
            str += "    <" + symbolName + "," + symbolType + ">\n";
        }
        return str;
    }

    SymbolTable::SymbolTable(void)
    {
        this->SymbolStack = deque<SymbolMap>();
        SymbolStack.push_back(SymbolMap());
        this->level = 0;
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
            Symbol symbol = symbolmap->find(name);
            if(symbol.type != null_t) return symbol;
        }
        return {name,null_t};
    }

    void SymbolTable::pop()
    {
        if(FLAGS_scope == true)
        {
            SymbolMap& symbolMap= this->SymbolStack.back();

            clog << "\033[34mscope: " << symbolMap.name  << " level: " << to_string(this->level) << endl;

            clog << "\033[32m";
            for(int i = 0;i <= 14 + (int)(symbolMap.name.length()) + (int)(to_string(this->level).length());i++)
                clog << '-';
            clog << endl;
            clog << "\033[0m";

            clog << symbolMap.__str() << endl;
        }
        this->SymbolStack.pop_back();
        this->level--;
    }

    void SymbolTable::push(string name)
    {
        this->level++;
        this->SymbolStack.push_back(SymbolMap(name));
    }
}