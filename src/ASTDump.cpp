/*
 * ASTDump.cpp 2022
 *
 *  AST dump
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

#include <iostream>

#include "../inc/AST.h"

#define PRINT_LINE_COLUNM() \
do { \
    cout << __AST_name << " " << this->token.line << ":" << this->token.column << ":" << endl; \
} while(false)

namespace AVSI {
    // type name to be displayed in debug message
    extern map<llvm::Type *, string> type_name;

    void printBlank(int depth) {
        for (int i = 0; i < depth; i++) {
            cout << "  ";
        }
    }

    void Assign::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        if (left) this->left->dump(depth + 1);
        if (right) this->right->dump(depth + 1);
    }

    void BinOp::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- op: " << token_name[this->op.getType()] << endl;
        if (left) this->left->dump(depth + 1);
        if (right) this->right->dump(depth + 1);
    }

    void BlockExpr::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        if (expr) this->expr->dump(depth + 1);
    }

    void Boolean::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- value: " << value << endl;
    }

    void Compound::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        for (auto i: child) {
            i->dump(depth + 1);
        }
    }

    void For::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- initList:" << endl;
        if (initList) this->initList->dump(depth + 1);
        printBlank(depth + 1);
        cout << "- condition:" << endl;
        if (condition) this->condition->dump(depth + 1);
        printBlank(depth + 1);
        cout << "- adjustment:" << endl;
        if (adjustment) this->adjustment->dump(depth + 1);
        printBlank(depth + 1);
        cout << "- compound:" << endl;
        if (compound) this->compound->dump(depth + 1);
    }

    void FunctionDecl::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- id: " << id << endl;
        auto modinfo = token.getModInfo();
        if(!modinfo.empty()) {
            printBlank(depth + 1);
            cout << "- modinfo: ";
            bool first_flag = true;
            for(auto &i : modinfo) {
                if(!first_flag) {
                    cout << "::";
                }
                first_flag = false;
                cout << i;
            }
            cout << endl;
        }
        printBlank(depth + 1);
        cout << "- is_export: " << is_export << endl;
        printBlank(depth + 1);
        cout << "- is_mangle: " << is_mangle << endl;
        printBlank(depth + 1);
        cout << "- paramList:" << endl;
        if (paramList) this->paramList->dump(depth + 1);
        printBlank(depth + 1);
        cout << "- compound:" << endl;
        if (compound) this->compound->dump(depth + 1);
    }

    void FunctionCall::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- id: " << id << endl;
        auto modinfo = token.getModInfo();
        if(!modinfo.empty()) {
            printBlank(depth + 1);
            cout << "- modinfo: ";
            bool first_flag = true;
            for(auto &i : modinfo) {
                if(!first_flag) {
                    cout << "::";
                }
                first_flag = false;
                cout << i;
            }
            cout << endl;
        }
        printBlank(depth + 1);
        cout << "- paramList:" << endl;
        for (auto i: paramList) {
            i->dump(depth + 1);
        }
    }

    void Generic::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- id: " << id << endl;

        if (!this->func_list.empty()) {
            printBlank(depth + 1);
            cout << "- function list:" << endl;
            for (auto i : this->func_list) {
                printBlank(depth + 2);
                cout << type_name[i.second.first] << ": " << i.first << endl;
            }
        }

        if (!this->default_func.empty()) {
            printBlank(depth + 1);
            cout << "- default: " << this->default_func << endl;
        }
        
    }

    void Global::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- is_export: " << is_export << endl;
        printBlank(depth + 1);
        cout << "- is_mangle: " << is_mangle << endl;
        if (var) this->var->dump(depth + 1);
        if (expr) this->expr->dump(depth + 1);
    }

    void Grad::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- expr: " << endl;
        if (this->expr) this->expr->dump(depth + 1);
        printBlank(depth + 1);
        cout << "- vars: " << endl;
        for (auto i : this->vars) {
            printBlank(depth + 2);
            cout << i.first << ", " << i.second << endl;
        }
    }

    void If::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- condition:" << endl;
        if (condition) this->condition->dump(depth + 1);
        printBlank(depth + 1);
        cout << "- compound:" << endl;
        if (compound) this->compound->dump(depth + 1);
        printBlank(depth + 1);
        cout << "- next:" << endl;
        if (next) this->next->dump(depth + 1);
    }

    void LoopCtrl::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- type: " << (type == CTRL_BREAK ? "break" : "continue") << endl;
    }

    void Num::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- value: " << value << endl;
    }

    void Sizeof::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        if (id) id->dump(depth + 1);
    }

    void Variable::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1); cout << "- id: " << id << endl;
        auto modinfo = token.getModInfo();
        if(!modinfo.empty()) {
            printBlank(depth + 1);
            cout << "- modinfo: ";
            bool first_flag = true;
            for(auto &i : modinfo) {
                if(!first_flag) {
                    cout << "::";
                }
                first_flag = false;
                cout << i;
            }
            cout << endl;
        }
        if (!offset.empty()) {
            printBlank(depth + 1);
            cout << "- offset: " << endl;
            for (auto &i: offset) {
                i.second->dump(depth + 1);
            }
        }
    }

    void Object::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1); cout << "- id:" << id << endl;
        printBlank(depth + 1); cout << "- is_export:" << is_export << endl;
        printBlank(depth + 1); cout << "- is_mangle:" << is_mangle << endl;
        for(auto i : memberList) {
            i->dump(depth + 1);
        }
    }

    void Param::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        for(auto i : paramList) {
            i->dump(depth + 1);
        }
        printBlank(depth + 1); cout << "- is_va_arg:" << is_va_arg << endl;
    }

    void Return::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        if(ret) ret->dump(depth + 1);
    }

    void String::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1); cout << "- value: " << value << endl;
    }

    void StructInit::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1); cout << "- id: " << id << endl;
        for(auto i : paramList) {
            i->dump(depth + 1);
        }
    }

    void TypeTrans::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1); cout << "- factor:" << endl;
        if(factor) factor->dump(depth + 1);
    }

    void ArrayInit::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1); cout << "- num: " << num << endl;
        if (this->num_by_const) {
            printBlank(depth + 1);
            cout << "- num_by_const:" << endl;
            this->num_by_const->dump(depth + 2);
        }
        printBlank(depth + 1); cout << "- paramList:" << endl;
        for(auto i : paramList) {
            i->dump(depth + 1);
        }
    }

    void UnaryOp::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1); cout << "- op: " << token_name[op.getType()] << endl;
        printBlank(depth + 1); cout << "- right: " << endl;
        if(right) right->dump(depth + 1);
    }

    void While::dump(int depth) {
        printBlank(depth);
        PRINT_LINE_COLUNM();
        printBlank(depth + 1);
        cout << "- condition:" << endl;
        if (condition) this->condition->dump(depth + 1);
        printBlank(depth + 1);
        cout << "- compound:" << endl;
        if (compound) this->compound->dump(depth + 1);
    }

    void NoneAST::dump(int depth) {

    }
}