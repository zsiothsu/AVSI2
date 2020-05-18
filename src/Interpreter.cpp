/*
 * @Author: Chipen Hsiao
 * @Date: 2020-04-06
 * @LastEditTime: 2020-05-18 17:27:27
 * @Description: include Interpreter class
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

    any Interpreter::interpret()
    {
        try
        {
            AST* tree = this->parser->parse();
            return visitor(tree);
        }
        catch(Exception& e)
        {
            throw e;
        }
        
    }

    /*******************************************************
     *                      visitor                        *
     *******************************************************/
    any NodeVisitor::visitor(AST* node)
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
        Assign* assign = (Assign*)node;
        Variable* var = (Variable*)assign->left;
        any value = visitor(assign->right);
        globalVariable[var->id] = value;
        return value;
    }

    any NodeVisitor::BinOpVisitor(AST* node)
    {
        BinOp* op = (BinOp*)node;
        if(op->left->getToken().getType() == NONE ||
           op->left->getToken().getType() == NONE)
        {
            throw ExceptionFactory("SyntaxException","invalid syntax");
        }
        if(op->getOp() == add_opt) return visitor(op->left) + visitor(op->right);
        if(op->getOp() == dec_opt) return visitor(op->left) - visitor(op->right);
        if(op->getOp() == mul_opt) return visitor(op->left) * visitor(op->right);
        if(op->getOp() == div_opt)
        {
            any right = visitor(op->right);
            if(right == 0) throw ExceptionFactory("MathException","division by zero");
            return visitor(op->left) / visitor(op->right);
        }
        return 0;
    }

    any NodeVisitor::CompoundVisitor(AST* node)
    {
        Compound* compound = (Compound*)node;
        for(AST* ast : compound->child) visitor(ast);
        return 0;
    }

    any NodeVisitor::NumVisitor(AST* node)
    {
        Num* num = (Num*) node;
        return num->getValue();
    }

    any NodeVisitor::VariableVisitor(AST* node)
    {
        Variable* var = (Variable*)node;
        map<string,any>::iterator iter = globalVariable.find(var->id);
        if(iter != globalVariable.end()) return iter->second;
        else
        {
            string msg = "name '" + var->id + "' is not defined";
            throw ExceptionFactory("LogicException",msg);
        }
    }
}