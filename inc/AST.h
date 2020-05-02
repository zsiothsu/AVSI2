/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-01 18:38:03
 * @Description: file content
 */
#ifndef ___AST_H___
#define ___AST_H___

#include <typeinfo>
#include "Data.h"
#include "Token.h"
#include "Exception.h"

namespace AVSI
{
    /*******************************************************
     *                       AST base                      *
     *******************************************************/
    class AST
    {
    protected:
        Token token;
    public:
        AST(void);
        virtual ~AST();

        Token getToken(void);
    };
    
    /*******************************************************
     *                    derived syntax                   *
     *******************************************************/
    class Assign: public AST
    {
    private:
    public:
        AST* left;
        AST* right;

        Assign(void);
        Assign(AST* left,AST* right);
        virtual ~Assign();
    };

    class BinOp: public AST
    {
    private:
        Token op;
    public:
        AST* left;
        AST* right;
        
        BinOp(void);
        BinOp(AST* left,Token op,AST* right);
        virtual ~BinOp();

        TokenType getOp(void);
    };

    class Num: public AST
    {
    private:
        double value;
    public:
        Num(void);
        Num(Token token);
        virtual ~Num();

        double getValue(void);
    };

    class UnaryOp: public AST
    {
    private:
        Token op;
    public:
        AST* left;
        AST* right;

        UnaryOp(void);
        UnaryOp(AST* left,Token op,AST* right);
        virtual ~UnaryOp();

        TokenType getOp(void);
    };

    class Variable: public AST
    {
    public:
        std::string id;

        Variable(void);
        Variable(Token var);
    };

    class NoneAST: public AST
    {
    public:
        NoneAST(void);
        virtual ~NoneAST();
    };
}

#endif