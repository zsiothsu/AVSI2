/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:56:00
 * @Description: constructer of abstract syntax tree (AST)
 */
#include "../inc/AST.h"

namespace AVSI {
    /*******************************************************
     *                       AST base                      *
     *******************************************************/

    Token AST::getToken(void) { return this->token; }

    /*******************************************************
     *                    derived syntax                   *
     *******************************************************/
    Assign::~Assign() {
        if (this->left != nullptr) { delete this->left; }
        if (this->right != nullptr) { delete this->right; }
    }

    TokenType BinOp::getOp(void) { return this->op.getType(); }

    BinOp::~BinOp() {
        if (this->left != nullptr) { delete this->left; }
        if (this->right != nullptr) { delete this->right; }
    }

    any Boolean::getValue(void) {
        return this->value;
    }

    For::~For() {
        if (this->initList != nullptr) delete this->initList;
        if (this->condition != nullptr) delete this->condition;
        if (this->adjustment != nullptr) delete this->adjustment;
        if (this->compound != nullptr) delete this->compound;
    }

    FunctionDecl::~FunctionDecl() {
        if (this->compound != nullptr) { delete this->compound; }
        if (this->paramList != nullptr) { delete this->paramList; }
    }

    FunctionCall::~FunctionCall() {
        for (auto param : this->paramList) { delete param; }
    }

    any Num::getValue(void) { return this->value; }

    If::~If() {
        if (this->condition != nullptr) { delete this->condition; }
        if (this->compound != nullptr) { delete this->compound; }
        if (this->next != nullptr) { delete this->next; }
    }

    UnaryOp::~UnaryOp() {
        if (this->right != nullptr) { delete this->right; }
    }

    TokenType UnaryOp::getOp(void) { return this->op.getType(); }

    Compound::~Compound() {
        for (AST *ast : this->child) delete ast;
    }

    While::~While() {
        if (this->condition != nullptr) delete this->condition;
        if (this->compound != nullptr) delete this->compound;
    }
} // namespace AVSI