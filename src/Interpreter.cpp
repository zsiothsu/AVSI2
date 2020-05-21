/*
 * @Author: Chipen Hsiao
 * @Date: 2020-04-06
 * @LastEditTime: 2020-05-21 16:39:38
 * @Description: include Interpreter class
 */

#include "../inc/Interpreter.h"

namespace AVSI
{
    any Interpreter::interpret(AST* root)
    {
        return visitor(root);
    }

    /*******************************************************
     *                      visitor                        *
     *******************************************************/
    any Interpreter::visitor(AST* node)
    {
        if(node == &ASTEmpty) return 0;
        any res = 0;
        string visitorName = node->__AST_name + "Visitor";
        map<string,visitNode>::iterator iter = visitorMap.find(visitorName);
        if(iter != visitorMap.end()) res = (this->*((*iter).second))(node);
        return res;
    }

    any Interpreter::AssignVisitor(AST* node)
    {
        Assign* assign = (Assign*)node;
        Variable* var = (Variable*)assign->left;
        any value = visitor(assign->right);
        symbolTable.insert({var->id,variable_t,value});
        return value;
    }

    any Interpreter::BinOpVisitor(AST* node)
    {
        BinOp* op = (BinOp*)node;
        if(op->getOp() == add_opt) return visitor(op->left) + visitor(op->right);
        if(op->getOp() == dec_opt) return visitor(op->left) - visitor(op->right);
        if(op->getOp() == mul_opt) return visitor(op->left) * visitor(op->right);
        if(op->getOp() == div_opt)
        {
            any right = visitor(op->right);
            if(right == 0) throw ExceptionFactory("MathException","division by zero",op->getToken().line,op->getToken().column);
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

    any Interpreter::FunctionVisitor(AST* node)
    {
        //TODO
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
        Symbol symbol = symbolTable.find(var->id);
        if(symbol.type == null_t)
        {
            string msg = "name '" + var->id + "' is not defined";
            throw ExceptionFactory("LogicException",msg,var->getToken().line,var->getToken().column);
        }
        return symbol.value;
    }
}
