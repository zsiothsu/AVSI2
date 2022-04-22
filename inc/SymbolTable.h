/*
 * @Author: Chipen Hsiao
 * @Date: 2022-04-22
 * @Description: Symbol table use in code generator
 */

#ifndef AVSI2_SYMBOLTABLE_H
#define AVSI2_SYMBOLTABLE_H

#include "../inc/AST.h"
#include <map>
#include <deque>
#include <vector>

namespace AVSI {
    using std::map;
    using std::deque;
    using std::vector;

    class SymbolMap {
    private:
        llvm::BasicBlock *BB;
        map<string, llvm::AllocaInst *> named_values;

    public:
        SymbolMap() : BB(nullptr), named_values(map<string, llvm::AllocaInst *>()) {}

        SymbolMap(llvm::BasicBlock *BB)
                : BB(BB),
                  named_values(map<string, llvm::AllocaInst *>()) {}

        ~SymbolMap() = default;;

        llvm::BasicBlock *getBasicBlock() const;

        llvm::AllocaInst *find(string &name);

        void insert(string &name, llvm::AllocaInst *addr);

        vector<llvm::AllocaInst *> getAllocaList();
    };

    class SymbolTable {
    private:
        deque<SymbolMap*> maps;

    public:
        SymbolTable (): maps(deque<SymbolMap*>()) {}

        ~SymbolTable() = default;

        void push(llvm::BasicBlock *BB);

        vector<llvm::AllocaInst *> pop();

        llvm::BasicBlock *getBasicBlock() const;

        llvm::AllocaInst *find(string &name);

        void insert(string name, llvm::AllocaInst *addr);
    };

    struct StructDef {
        llvm::StructType *Ty;
        map<string, int> members;

        StructDef() = default;

        StructDef(llvm::StructType *Ty): Ty(Ty), members(map<string, int>()) {}

        ~StructDef() = default;
    };
}


#endif //AVSI2_SYMBOLTABLE_H
