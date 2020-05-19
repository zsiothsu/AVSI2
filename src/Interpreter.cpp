/*
 * @Author: Chipen Hsiao
 * @Date: 2020-04-06
 * @LastEditTime: 2020-05-19 23:26:27
 * @Description: include Interpreter class
 */

#include "../inc/Interpreter.h"

namespace AVSI
{
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
    any Interpreter::visitor(AST* node)
    {
        try
        {
            if(node == &ASTEmpty) return 0;
            any res = 0;
            string functionName = node->__AST_name + "Visitor";
            map<string,visitNode>::iterator iter = visitorMap.find(functionName);
            if(iter != visitorMap.end()) res = (this->*((*iter).second))(node);
            return res;
        }
        catch(Exception& e)
        {
            throw e;
        }
    }

    any Interpreter::AssignVisitor(AST* node)
    {
        Assign* assign = (Assign*)node;
        Variable* var = (Variable*)assign->left;
        any value = visitor(assign->right);
        globalVariable[var->id] = value;
        return value;
    }

    any Interpreter::BinOpVisitor(AST* node)
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

    any Interpreter::CompoundVisitor(AST* node)
    {
        Compound* compound = (Compound*)node;
        for(AST* ast : compound->child) visitor(ast);
        return 0;
    }

    any Interpreter::NumVisitor(AST* node)
    {
        Num* num = (Num*) node;
        return num->getValue();
    }

    any Interpreter::UnaryOpVisitor(AST* node)
    {
        UnaryOp* op = (UnaryOp*)node;
        if(op->getOp() == add_opt) return (any)0 + visitor(op->right);
        if(op->getOp() == dec_opt) return (any)0 - visitor(op->right);
        return 0;
    }

    any Interpreter::VariableVisitor(AST* node)
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