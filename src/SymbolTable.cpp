/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-28 00:16:00
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

    SymbolTable::~SymbolTable()
    {
        for(auto subtable : this->child)
        {
            if(subtable != nullptr) delete subtable;
        }
    }

    void SymbolTable::insert(Symbol symbol)
    {
        this->symbolMap.insert(symbol);
    }

    Symbol SymbolTable::find(string name)
    {
        SymbolTable* ptr = this;
        while(ptr != nullptr)
        {
            Symbol symbol = ptr->symbolMap.find(name);
            if(symbol.type != null_t) return symbol;
            else ptr = ptr->father;
        }
        
        return {name,null_t};
    }

    void SymbolTable::mount(SymbolTable* symbolTable)
    {
        this->child.push_back(symbolTable);
    }

    void SymbolTable::__str()
    {
        if(FLAGS_scope == true)
        {
            //header
            clog << "\033[34mscope: " << symbolMap.name  << " level: " << to_string(this->level) << endl;

            clog << "\033[32m";
            for(int i = 0;i <= 14 + (int)(symbolMap.name.length()) + (int)(to_string(this->level).length());i++)
                clog << '-';
            clog << endl;
            clog << "\033[0m";
            //contents
            string str = this->symbolMap.__str();
            clog << str << endl;
            //child
            for(auto subtable:this->child)
            {
                subtable->__str();
            }
        }
    }
}