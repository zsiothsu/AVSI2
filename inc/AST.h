/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-18 17:25:26
 * @Description: constructer of abstract syntax tree (AST)
 */
#ifndef ___AST_H___
#define ___AST_H___

#include <typeinfo>
#include <vector>
#include "Token.h"
#include "Exception.h"

namespace AVSI
{
    using std::vector;
    
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
        any value;
    public:
        Num(void);
        Num(Token token);
        virtual ~Num();

        any getValue(void);
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

    class Compound: public AST
    {
    public:
        vector<AST*> child;

        Compound(void);
        virtual ~Compound();
    };

    class NoneAST: public AST
    {
    public:
        NoneAST(void);
        virtual ~NoneAST();
    };

    static AST ASTEmpty = NoneAST();
}

#endif