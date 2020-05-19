/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-19 22:51:09
 * @Description: constructer of abstract syntax tree (AST)
 */
#ifndef ___AST_H___
#define ___AST_H___

#include <typeinfo>
#include <vector>
#include "Token.h"
#include "Exception.h"

#define __ASSIGN_NAME   "Assign"
#define __BINOP_NAME    "BinOp"
#define __NUM_NAME      "Num"
#define __UNARYTOP_NAME "UnaryOp"
#define __VARIABLE_NAME "Variable"
#define __COMPOUND_NAME "Compound"
#define __NONEAST_NAME  "NoneAST"

namespace AVSI
{
    using std::vector;
    using std::string;
    
    /*******************************************************
     *                       AST base                      *
     *******************************************************/
    class AST
    {
    protected:
        Token token;
    public:
        string __AST_name;

        AST(void);
        AST(string name): __AST_name(name) {};
        AST(string name,Token token):
            token(token),
            __AST_name(name)
        {};
        virtual ~AST() {};

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

        Assign(void):
            AST(__ASSIGN_NAME),
            left(nullptr),
            right(nullptr)
        {};
        Assign(AST* left,AST* right):
            AST(__ASSIGN_NAME,Token(assign_opt,'=')),
            left(left),
            right(right)
        {};
        virtual ~Assign();
    };

    class BinOp: public AST
    {
    private:
        Token op;
    public:
        AST* left;
        AST* right;
        
        BinOp(void):
            AST(__BINOP_NAME),
            op(emptyToken),
            left(nullptr),
            right(nullptr)
        {};
        BinOp(AST* left,Token op,AST* right):
            AST(__BINOP_NAME,op),
            op(op),
            left(left),
            right(right)
        {};
        virtual ~BinOp();

        TokenType getOp(void);
    };

    class Num: public AST
    {
    private:
        any value;
    public:
        Num(void): AST(__NUM_NAME) {};
        Num(Token token):
            AST(__NUM_NAME,token),
            value(token.getValue())
        {};
        virtual ~Num() {};

        any getValue(void);
    };

    class UnaryOp: public AST
    {
    private:
        Token op;
    public:
        AST* right;

        UnaryOp(void): AST(__AST_name) {};
        UnaryOp(Token op,AST* right):
            AST(__UNARYTOP_NAME,op),
            op(op),
            right(right)
        {};
        virtual ~UnaryOp();

        TokenType getOp(void);
    };

    class Variable: public AST
    {
    public:
        std::string id;

        Variable(void): AST(__VARIABLE_NAME) {};
        Variable(Token var):
            AST(__VARIABLE_NAME,var),
            id(var.getValue().any_cast<std::string>())
        {};
        ~Variable() {};
    };

    class Compound: public AST
    {
    public:
        vector<AST*> child;

        Compound(void):
            AST(__COMPOUND_NAME,Token(compound_ast,0)),
            child(vector<AST*>())
        {};
        virtual ~Compound();
    };

    class NoneAST: public AST
    {
    public:
        NoneAST(void): AST(__NONEAST_NAME,Token(NONE,0)) {};
        virtual ~NoneAST() {};
    };

    static AST ASTEmpty = NoneAST();
}

#endif