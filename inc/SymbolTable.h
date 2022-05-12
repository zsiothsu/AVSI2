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

#define NAME_MANGLING(x)    ("_ZN" + \
to_string(module_name.size()) + module_name + \
to_string(string(x).size()) + string(x))

#if (__SIZEOF_POINTER__ == 4)
#define PTR_SIZE 4
#define MACHINE_WIDTH_TY (llvm::Type::getInt32Ty((*the_context)))
#elif (__SIZEOF_POINTER__ == 8)
#define PTR_SIZE 8
#define MACHINE_WIDTH_TY (llvm::Type::getInt64Ty((*the_context)))
#else
#err "unsupported machine"
#endif

#define ENTRY_NAME          "entry"

namespace AVSI {
    using std::map;
    using std::deque;
    using std::vector;

    class SymbolMap {
    private:
        llvm::BasicBlock *BB;
        map<string, llvm::AllocaInst *> named_values;

    public:
        llvm::BasicBlock *break_to;
        llvm::BasicBlock *continue_to;

        SymbolMap()
                : BB(nullptr),
                  named_values(map < string, llvm::AllocaInst * > ()),
                  break_to(nullptr),
                  continue_to(nullptr) {}

        SymbolMap(llvm::BasicBlock *BB)
                : BB(BB),
                  named_values(map < string, llvm::AllocaInst * > ()),
                  break_to(nullptr),
                  continue_to(nullptr) {}

        ~SymbolMap() = default;;

        llvm::BasicBlock *getBasicBlock() const;

        llvm::AllocaInst *find(string &name);

        void insert(string &name, llvm::AllocaInst *addr);

        vector<llvm::AllocaInst *> getAllocaList();
    };

    class SymbolTable {
    private:
        deque<SymbolMap *> maps;

    public:
        SymbolTable() : maps(deque<SymbolMap *>()) {}

        ~SymbolTable() = default;

        void push(llvm::BasicBlock *BB);

        vector<llvm::AllocaInst *> pop();

        llvm::BasicBlock *getBasicBlock() const;

        llvm::AllocaInst *find(string &name);

        llvm::BasicBlock *getBreakTo();

        llvm::BasicBlock *getContinueTo();

        void setBreakTo(llvm::BasicBlock * BB);

        void setContinueTo(llvm::BasicBlock * BB);

        void insert(string name, llvm::AllocaInst *addr);
    };

    struct StructDef {
        llvm::StructType *Ty;
        map<string, int> members;

        StructDef() = default;

        StructDef(llvm::StructType *Ty) : Ty(Ty), members(map<string, int>()) {}

        ~StructDef() = default;
    };

    string __getModuleNameByPath(vector<string> path);

    string getFunctionNameMangling(vector<string> path, string fun);

    string getpathListToUnresolved(vector<string> path);

    vector<string> getpathUnresolvedToList(string path);
}


#endif //AVSI2_SYMBOLTABLE_H
