/*
 * @Author: Chipen Hsiao
 * @Date: 2020-04-06
 * @LastEditTime: 2020-05-28 18:15:52
 * @Description: include Interpreter class
 */

#include "../inc/Interpreter.h"

namespace AVSI
{
    Interpreter::~Interpreter()
    {
        if(this->symbolTable != nullptr) delete this->symbolTable;
    }

    any Interpreter::interpret(void)
    {
        ActivationRecord* ar = new ActivationRecord("program",program,1);



        if(FLAGS_callStack)
        {
            clog << "\033[32m====================================\033[0m" << endl
                 << "\033[32m               runtime\033[0m"               << endl
                 << "\033[32m------------------------------------\033[0m" << endl;
            clog << "CallStack: enter 'program' level: 1" << endl;
            clog << ar->__str__();
        }

        this->callStack.push(ar);
        any ret = visitor(this->root);
        this->callStack.pop();

        if(FLAGS_callStack)
        {
            clog << "CallStack: leave 'program'" << endl;

            clog << ar->__str__() << endl;
            delete ar;
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
        // current function
        FunctionCall* fun = (FunctionCall*)node;
        string funName = fun->id;
        fun->symbol_function = (Symbol_function*)this->currentSymbolTable->find(funName);
        if(fun->symbol_function == nullptr)
        {
            string msg = "undefined reference '" + funName + "'";
            throw ExceptionFactory(__LogicException,msg,fun->getToken().line,fun->getToken().column);
        }

        // Activation Record and symbol of target function
        ActivationRecord* ar = new ActivationRecord(funName,function,this->callStack.peek()->level+1);
        Symbol_function* symbol = fun->symbol_function;

        // formal and actual paremeters
        deque<Symbol*> formalParams = symbol->formalVariable;
        vector<AST*> actualParams = fun->paramList;

        if((int)(formalParams.size()) != (int)(actualParams.size()))
        {
            string msg = "function '" + fun->id + "' takes " + to_string((int)formalParams.size()) + " arguments but " + to_string((int)(actualParams.size())) + " were given";
            throw ExceptionFactory(__LogicException,msg,fun->getToken().line,fun->getToken().column);
        }

        deque<Symbol*>::iterator iterFormal = formalParams.begin();
        vector<AST*>::iterator iterActual = actualParams.begin();
        for(;iterFormal != formalParams.end();iterFormal++,iterActual++)
        {
            ar->__setitem__((*iterFormal)->name,visitor(*iterActual));
        }

        if(FLAGS_callStack)
        {
            clog << "CallStack: enter '" + fun->id + "' level: " << ar->level << endl;
            clog << ar->__str__();
        }

        this->callStack.push(ar);
        for(auto subSymbolTable: this->currentSymbolTable->child)
        {
            if(subSymbolTable->symbolMap->name == funName)
            {
                this->currentSymbolTable = subSymbolTable;
                break;
            }
        }

        visitor((AST*)symbol->node_ast);
        if(FLAGS_callStack)
        {
            clog << "CallStack: leave '" + fun->id + "'" << endl;
            clog << ar->__str__();
        }

        this->callStack.pop();
        this->currentSymbolTable = this->currentSymbolTable->father;

        delete ar;
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
