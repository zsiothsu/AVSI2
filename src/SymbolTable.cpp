/*
 * @Author: Chipen Hsiao
 * @Date: 2022-04-22
 * @Description: Symbol table use in code generator
 */
#include "../inc/SymbolTable.h"

namespace AVSI {
    llvm::BasicBlock *SymbolMap::getBasicBlock() const {
        return this->BB;
    }

    llvm::AllocaInst *SymbolMap::find(string &name) {
        if(this->named_values.find(name) != this->named_values.end()) {
            return this->named_values[name.c_str()];
        }
        return nullptr;
    }

    void SymbolMap::insert(string &name, llvm::AllocaInst *addr) {
        this->named_values[name] = addr;
    }

    vector<llvm::AllocaInst *> SymbolMap::getAllocaList() {
        vector<llvm::AllocaInst *> addrs;
        for(const auto& iter : this->named_values) {
            addrs.push_back(iter.second);
        }
        return addrs;
    }

    void SymbolTable::push(llvm::BasicBlock *BB) {
        auto *symbol_map = new SymbolMap(BB);
        this->maps.push_back(symbol_map);
    }

    vector<llvm::AllocaInst *> SymbolTable::pop() {
        if(this->maps.empty()) {
            return {};
        }

        SymbolMap *old = this->maps.back();
        this->maps.pop_back();
        vector<llvm::AllocaInst *> addrs = old->getAllocaList();
        delete old;

        return addrs;
    }

    llvm::BasicBlock *SymbolTable::getBasicBlock() const {
        return this->maps.back()->getBasicBlock();
    }

    llvm::AllocaInst *SymbolTable::find(string &name) {
        for(auto iter = this->maps.rbegin(); iter != this->maps.rend(); iter++) {
            auto ret = (*iter)->find(name);
            if(ret != nullptr) {
                return ret;
            }
        }
        return nullptr;
    }

    void SymbolTable::insert(basic_string<char> name, llvm::AllocaInst *addr) {
        this->maps.back()->insert(name, addr);
    }
}