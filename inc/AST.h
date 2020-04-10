/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 21:08:58
 * @Description: file content
 */
#ifndef ___AST_H___
#define ___AST_H___

#include "Token.h"
#include "Exception.h"

namespace INTERPRETER
{
    /*******************************************************
     *                       AST base                      *
     *******************************************************/
    class AST
    {
    private:
        /* data */
    public:
        AST(void);
        ~AST();
    };
    
    /*******************************************************
     *                    derived syntax                   *
     *******************************************************/
    class BinOp: public AST
    {
    private:
        AST* left;
        AST* right;
        Token token;
        Token op;
    public:
        BinOp(void);
        BinOp(AST* left,Token op,AST* right);
        ~BinOp();
    };

    class Num: public AST
    {
    private:
        Token token;
        int value;
    public:
        Num(void);
        Num(Token token);
    };     
}

#endif