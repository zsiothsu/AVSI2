/*
 * @Author: Chipen Hsiao
 * @Date: 2020-04-06
 * @LastEditTime: 2020-05-02 11:32:43
 * @Description: some methods for Interpreter class
 */

#include "../inc/Interpreter.h"

namespace AVSI
{
    /*******************************************************
     *                    constructor                      *
     *******************************************************/
    /**
     * @description:    default constructor
     * @param:          None
     * @return:         None
     */
    NodeVisitor::NodeVisitor(void)
    {
    }

    /**
     * @description:    default constructor
     * @param:          None
     * @return:         None
     */
    Interpreter::Interpreter(void)
    {
    }

    Interpreter::Interpreter(Parser* paser)
    {
        this->parser = paser;
    }

    /**
     * @description:    default destructor
     * @param:          None
     * @return:         None
     */
    Interpreter::~Interpreter()
    {
    }

    void Interpreter::interpret(int* ans)
    {
        try
        {
            AST* expression = this->parser->parse();
            *ans = any::any_cast<double>(vistor(expression));
        }
        catch(Exception& e)
        {
            throw e;
        }
        
    }

    /*******************************************************
     *                      visitor                        *
     *******************************************************/
    any NodeVisitor::vistor(AST* node)
    {
        try
        {
            if(node->getToken().getType() == NONE) return 0;
            any res = visitorMap[node->getToken().getType()](node);
            return res;
        }
        catch(Exception& e)
        {
            throw e;
        }
    }

    any NodeVisitor::AssignVisitor(AST* node)
    {
        
    }

    any NodeVisitor::BinOpVisitor(AST* node)
    {
        BinOp* op = (BinOp*)node;
        if(op->left->getToken().getType() == NONE ||
           op->left->getToken().getType() == NONE)
        {
            throw ExceptionFactory("SyntaxException","invalid syntax");
        }
        if(op->getOp() == ADD) return vistor(op->left) + vistor(op->right);
        if(op->getOp() == DEC) return vistor(op->left) - vistor(op->right);
        if(op->getOp() == MUL) return vistor(op->left) * vistor(op->right);
        if(op->getOp() == DIV)
        {
            any right = vistor(op->right);
            if(right == 0) throw ExceptionFactory("MathException","division by zero");
            return vistor(op->left) / vistor(op->right);
        }
        return 0;
    }

    any NodeVisitor::NumVisitor(AST* node)
    {
        Num* num = (Num*) node;
        return num->getValue();
    }
}