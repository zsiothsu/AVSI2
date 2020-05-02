/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-01 18:36:40
 * @Description: file content
 */
#include "../inc/AST.h"

namespace AVSI
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

    Token AST::getToken(void)
    {
        return this->token;
    }
    /*******************************************************
     *                    derived syntax                   *
     *******************************************************/
    Assign::Assign(void)
    {
        this->left = nullptr;
        this->right = nullptr;
    }

    Assign::Assign(AST* left,AST* right)
    {
        this->left = left;
        this->right = right;
        this->token = Token(ASSIGN,'=');
    }

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

    BinOp::BinOp(void)
    {
        this->left = nullptr;
        this->right = nullptr;
    }

    BinOp::BinOp(AST* left,Token op,AST* right)
    {
        this->left = left;
        this->right = right;
        this->token = this->op = op;
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

    Num::Num(void)
    {
    }

    Num::Num(Token token)
    {
        this->token = token;
        this->value = token.getNum();
    }

    double Num::getValue(void)
    {
        return this->value;
    }

    UnaryOp::UnaryOp(void)
    {
        this->right = nullptr;
    }

    UnaryOp::UnaryOp(AST* left,Token op,AST* right)
    {
        this->token = this->op = op;
        this->left = left;
        this->right = right;
    }

    UnaryOp::~UnaryOp()
    {
        if(this->left != nullptr)
        {
            delete this->right;
        }
        if(this->right != nullptr)
        {
            delete this->right;
        }
    }

    TokenType UnaryOp::getOp(void)
    {
        return this->op.getType();
    }

    Variable::Variable(void)
    {
    }

    Variable::Variable(Token var)
    {
        this->token = var;
        this->id = var.getString();
    }

    NoneAST::NoneAST(void)
    {
        this->token = Token(NONE,0);
    }
}