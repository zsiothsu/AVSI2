/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 19:59:16
 * @Description: file content
 */
#include "../inc/AST.h"

namespace INTERPRETER
{
    /*******************************************************
     *                       AST base                      *
     *******************************************************/
    AST::AST(void)
    {        
    }

    AST::~AST()
    {
    }
    /*******************************************************
     *                    derived syntax                   *
     *******************************************************/
    BinOp::BinOp(void)
    {
    }

    BinOp::BinOp(AST* left,Token op,AST* right)
    {
        this->left = left;
        this->right = right;
        this->token = this->op = op;
    }

    BinOp::~BinOp()
    {
        if(this->left != nullptr)
        {
            this->left->~AST();
            delete this->left;
        }
        if(this->right != nullptr)
        {
            this->right->~AST();
            delete this->right;
        }
    }

    Num::Num(void)
    {
    }

    Num::Num(Token token)
    {
        this->token = token;
        this->value = token.getValue();
    }
}