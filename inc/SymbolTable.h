/*
 * SymbolTable.h 2022
 *
 * symbol table use in code generator
 *
 * LLVM IR code generator
 *
 * MIT License
 *
 * Copyright (c) 2022 Chipen Hsiao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

#define ENTRY_NAME          "main"

namespace AVSI {
    using std::map;
    using std::deque;
    using std::vector;

    class SymbolMap {
    private:
        llvm::BasicBlock *BB;
        map<string, llvm::AllocaInst *> named_values;

    public:
        llvm::BasicBlock *loop_exit;
        llvm::BasicBlock *loop_entry;

        SymbolMap()
                : BB(nullptr),
                  named_values(map < string, llvm::AllocaInst * > ()),
                  loop_exit(nullptr),
                  loop_entry(nullptr) {named_values.clear();}

        SymbolMap(llvm::BasicBlock *BB)
                : BB(BB),
                  named_values(map < string, llvm::AllocaInst * > ()),
                  loop_exit(nullptr),
                  loop_entry(nullptr) {named_values.clear();}

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
        SymbolTable() : maps(deque<SymbolMap *>()) {maps.clear();}

        ~SymbolTable() = default;

        void push(llvm::BasicBlock *BB);

        vector<llvm::AllocaInst *> pop();

        llvm::BasicBlock *getBasicBlock() const;

        llvm::AllocaInst *find(string &name);

        llvm::BasicBlock *getLoopExit();

        llvm::BasicBlock *getLoopEntry();

        void setLoopExit(llvm::BasicBlock * BB);

        void setLoopEntry(llvm::BasicBlock * BB);

        void insert(string name, llvm::AllocaInst *addr);
    };

    string __getModuleNameByPath(vector<string> path);

    string getFunctionNameMangling(vector<string> path, string fun);

    string getpathListToUnresolved(vector<string> path);

    vector<string> getpathUnresolvedToList(string path);
}


#endif //AVSI2_SYMBOLTABLE_H
