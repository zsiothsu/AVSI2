/*
 * SymbolTable.cpp 2022
 *
 * Symbol table use in code generator
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

#include "../inc/SymbolTable.h"
#include <string>

namespace AVSI {
    extern std::string module_name;

    llvm::BasicBlock *SymbolMap::getBasicBlock() const {
        return this->BB;
    }

    /**
     * @description:    get allocated space of variable
     * @param:          name: variable name
     * @return:         pointer point to variable. if not exist, it will
     *                  return nullptr
     */
    llvm::AllocaInst *SymbolMap::find(string &name) {
        if (this->named_values.find(name) != this->named_values.end()) {
            return this->named_values[name.c_str()];
        }
        return nullptr;
    }

    /**
     * @description:    insert a variable to symbol map
     * @param:          name: variable name
     * @param:          addr: pointer to allocated space
     * @return:         none
     */
    void SymbolMap::insert(string &name, llvm::AllocaInst *addr) {
        this->named_values[name] = addr;
    }

    /**
     * @description:    get all variables
     * @return:         list of allocated spaces
     */
    vector<llvm::AllocaInst *> SymbolMap::getAllocaList() {
        vector<llvm::AllocaInst *> addrs;
        for (const auto &iter: this->named_values) {
            addrs.push_back(iter.second);
        }
        return addrs;
    }

    /**
     * @description:    push a new scope
     * @return:         none
     */
    void SymbolTable::push(llvm::BasicBlock *BB) {
        llvm::BasicBlock *last_breake_to = nullptr;
        llvm::BasicBlock *last_continue_to = nullptr;
        if (!this->maps.empty()) {
            last_breake_to = this->maps.back()->loop_exit;
            last_continue_to = this->maps.back()->loop_entry;
        }

        auto *symbol_map = new SymbolMap(BB);
        symbol_map->loop_exit = last_breake_to;
        symbol_map->loop_entry = last_continue_to;

        this->maps.push_back(symbol_map);
    }

    /**
     * @description:    pop a scope
     * @return:         none
     */
    vector<llvm::AllocaInst *> SymbolTable::pop() {
        if (this->maps.empty()) {
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

    /**
     * @description:    get allocated space of variable in all scopes
     * @param:          name: variable name
     * @return:         pointer point to variable. if not exist, it will
     *                  return nullptr
     */
    llvm::AllocaInst *SymbolTable::find(string &name) {
        for (auto iter = this->maps.rbegin(); iter != this->maps.rend(); iter++) {
            auto ret = (*iter)->find(name);
            if (ret != nullptr) {
                return ret;
            }
        }
        return nullptr;
    }

    /**
     * @description:    insert a variable to current scope
     * @param:          name: variable name
     * @param:          addr: pointer to allocated space
     * @return:         none
     */
    void SymbolTable::insert(basic_string<char> name, llvm::AllocaInst *addr) {
        this->maps.back()->insert(name, addr);
    }

    llvm::BasicBlock *SymbolTable::getLoopExit() {
        return this->maps.back()->loop_exit;
    }

    llvm::BasicBlock *SymbolTable::getLoopEntry() {
        return this->maps.back()->loop_entry;
    }

    void SymbolTable::setLoopExit(llvm::BasicBlock *BB) {
        this->maps.back()->loop_exit = BB;
    }

    void SymbolTable::setLoopEntry(llvm::BasicBlock *BB) {
        this->maps.back()->loop_entry = BB;
    }

    /**
     * @description:    get module name by path
     * @param:          path: module path
     * @return:         resolved name
     * @example:
     *      {a,b,c} -> a_b_c
     */
    string __getModuleNameByPath(vector<string> path) {
        if (path.empty()) {
            return module_name;
        }

        string name;
        bool flag_first = true;
        for (string i: path) {
            if (!flag_first) name += "_";
            flag_first = false;
            name += i;
        }
        return name;
    }

    /**
     * @description:    get mangled function name
     * @param:          path: module path
     * @param:          fun: function name
     * @return:         resolved name
     * @example:
     *      a::b::foo -> _ZN3a_b3foo
     */
    string getFunctionNameMangling(vector<string> path, string fun) {
        string mn = __getModuleNameByPath(path);
        return "_ZN" + to_string(mn.size()) + mn + to_string(fun.size()) + fun;
    }

    /**
     * @description:    get unresolved path by list
     * @param:          path: module path list
     * @return:         unresolved module name
     * @example:
     *      {a,b,c} -> a::b::c
     */
    string getpathListToUnresolved(vector<string> path) {
        string ret;
        bool flag_isFirst = true;
        for (auto i: path) {
            if (!flag_isFirst) ret += "::";
            flag_isFirst = false;
            ret += i;
        }
        return ret;
    }

    /**
     * @description:    get resolved path list by unresolved module path
     * @param:          path: module path
     * @return:         unresolved module name
     * @example:
     *      a::b::c -> {a,b,c}
     */
    vector<string> getpathUnresolvedToList(string path) {
        vector<string> ret;
        while (true) {
            auto index = path.find("::");
            if (index == string::npos) break;
            string part = path.substr(0, index);
            ret.push_back(part);
            path = path.substr(index + 2);
        }
        ret.push_back(path);

        return ret;
    }
}