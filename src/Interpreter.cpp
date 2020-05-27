/*
 * @Author: Chipen Hsiao
 * @Date: 2020-04-06
 * @LastEditTime: 2020-05-27 22:51:35
 * @Description: include Interpreter class
 */

#include "../inc/Interpreter.h"

namespace AVSI
{
    any Interpreter::interpret(AST* root)
    {
        ActivationRecord* ar = new ActivationRecord("program",program,1);



        if(FLAGS_callStack)
        {
            clog << "\033[32m====================================\033[0m" << endl
                 << "\033[32m               runtime\033[0m"               << endl
                 << "\033[32m------------------------------------\033[0m" << endl;
            clog << "CallStack: enter 'program'" << endl;
        }

        this->callStack.push(ar);
        any ret = visitor(root);
        this->callStack.pop();

        if(FLAGS_callStack)
        {
            clog << "CallStack: leave 'program'" << endl <<endl;

            clog << ar->__str__();

            clog << "\033[32m====================================\033[0m" << endl;
        }

        return ret;
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

        ActivationRecord* ar = this->callStack.peek();
        ar->__setitem__(var->id,value);

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
            if(right == 0) throw ExceptionFactory(__MathException,"division by zero",op->getToken().line,op->getToken().column);
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

    any Interpreter::FunctionDeclVisitor(AST* node)
    {
        //TODO
        return 0;
    }

    any Interpreter::FunctionCallVisitor(AST* node)
    {
        //TODO
        return 0;
    }

    any Interpreter::ParamVisitor(AST* node)
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

        ActivationRecord* ar = this->callStack.peek();
        any value = ar->__getitem__(var->id);
        
        return value;
    }
}
