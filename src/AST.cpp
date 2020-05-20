/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-20 10:39:48
 * @Description: constructer of abstract syntax tree (AST)
 */
#include "../inc/AST.h"

namespace AVSI
{
    /*******************************************************
     *                       AST base                      *
     *******************************************************/

    Token AST::getToken(void)
    {
        return this->token;
    }
    /*******************************************************
     *                    derived syntax                   *
     *******************************************************/
    Assign::~Assign()
    {
        if(this->left != nullptr)
        {
            delete this->left;
        }
        if(this->right != nullptr)
        {
            delete this->right;
        }
    }

    TokenType BinOp::getOp(void)
    {
        return this->op.getType();
    }

    BinOp::~BinOp()
    {
        if(this->left != nullptr)
        {
            delete this->left;
        }
        if(this->right != nullptr)
        {
            delete this->right;
        }
    }

    Function::~Function()
    {
        if(this->compound != nullptr)
        {
            delete this->compound;
        }
    }

    any Num::getValue(void)
    {
        return this->value;
    }

    UnaryOp::~UnaryOp()
    {
        if(this->right != nullptr)
        {
            delete this->right;
        }
    }

    TokenType UnaryOp::getOp(void)
    {
        return this->op.getType();
    }

    Compound::~Compound()
    {
        for(AST* ast : this->child) delete ast;
    }
}