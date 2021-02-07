/*
 * @Author: Chipen Hsiao
 * @Date: 2020-04-06
 * @LastEditTime: 2020-06-02 15:56:23
 * @Description: include Interpreter class
 */

#include "../inc/Interpreter.h"

namespace AVSI {
    Interpreter::~Interpreter() {
        if (this->symbolTable != nullptr) delete this->symbolTable;
    }

    any Interpreter::interpret(void) {
        ActivationRecord *ar = new ActivationRecord("program", program, 1);

        if (FLAGS_callStack) {
            clog << "\033[32m====================================\033[0m"
                 << endl
                 << "\033[32m               runtime\033[0m" << endl
                 << "\033[32m------------------------------------\033[0m"
                 << endl;
            clog << "CallStack: enter 'program' level: 1" << endl;
            clog << ar->__str__();
        }

        this->callStack.push(ar);
        any ret = visitor(this->root);
        this->callStack.pop();

        if (FLAGS_callStack) {
            clog << "CallStack: leave 'program'" << endl;

            clog << ar->__str__() << endl;
            delete ar;
            clog << "\033[32m====================================\033[0m"
                 << endl;
        }

        return ret;
    }

    /*******************************************************
     *                      visitor                        *
     *******************************************************/
    any Interpreter::visitor(AST *node) {
        if (node == nullptr || node == &ASTEmpty) return 0;
        any res;
        string visitorName = node->__AST_name + "Visitor";

        map<string, visitNode>::iterator iter = visitorMap.find(visitorName);
        if (iter != visitorMap.end()) res = (this->*((*iter).second))(node);

        if (getStatus(Status_ret) && node->__AST_name == "FunctionCall")
            clearStatus(Status_ret);

        return res;
    }

    any Interpreter::AssignVisitor(AST *node) {
        Assign *assign = (Assign *) node;

        Variable *var = (Variable *) assign->left;
        any value = visitor(assign->right);

        this->callStack.__setitem__(var->id, value);

        return value;
    }

    any Interpreter::BinOpVisitor(AST *node) {
        BinOp *op = (BinOp *) node;

        try {
            if (op->getOp() == PLUS)
                return visitor(op->left) + visitor(op->right);
            if (op->getOp() == MINUS)
                return visitor(op->left) - visitor(op->right);
            if (op->getOp() == STAR)
                return visitor(op->left) * visitor(op->right);
            if (op->getOp() == SLASH) {
                any right = visitor(op->right);
                if (right == 0)
                    throw ExceptionFactory(__MathException, "division by zero",
                                           op->getToken().line,
                                           op->getToken().column);
                return visitor(op->left) / visitor(op->right);
            }
            if (op->getOp() == EQ)
                return (bool)visitor(op->left) == (bool)visitor(op->right);
            if (op->getOp() == NE)
                return (bool)visitor(op->left) != (bool)visitor(op->right);
            if (op->getOp() == GT) {
                bool t = visitor(op->left) > visitor(op->right);
                return t;}
            if (op->getOp() == LT)
                return visitor(op->left) < visitor(op->right);
            if (op->getOp() == GE)
                return visitor(op->left) >= visitor(op->right);
            if (op->getOp() == LE)
                return visitor(op->left) <= visitor(op->right);
            if (op->getOp() == OR)
                return (bool)visitor(op->left) || (bool)visitor(op->right);
            if (op->getOp() == AND)
                return (bool)visitor(op->left) && (bool)visitor(op->right);
        } catch (Exception &e) {
            if (e.type() == "TypeException") {
                e.line = op->getToken().line;
                e.column = op->getToken().column;
            }
            throw e;
        }

        return 0;
    }

    any Interpreter::BooleanVisitor(AST *node) {
        Boolean *num = (class Boolean*)node;

        return num->getValue();
    }

    any Interpreter::CompoundVisitor(AST *node) {
        Compound *compound = (Compound *) node;
        any ret;
        for (AST *ast : compound->child) {
            ret = visitor(ast);
            if (getStatus(Status_ret)) break;
        }

        return ret;
    }

    any Interpreter::EchoVisitor(AST *node) {
        Echo *echo = (Echo *)node;

        any out = visitor(echo->content);
        cout << out << endl;

        return 0;
    }

    any Interpreter::ForVisitor(AST *node) {
        For *forStatement = (For *)node;
        any ret;

        SymbolTable *stBeforeCall = this->currentSymbolTable;
        for(SymbolTable *subSymbolTable : this->currentSymbolTable->child)
        {
            if(subSymbolTable->symbolMap->name == "for" &&
                   subSymbolTable->symbolMap->addr == (uint64_t)&forStatement)
            {
                    this->currentSymbolTable = subSymbolTable;
                    break;
            }
        }
        ActivationRecord *ar = new ActivationRecord(
                "for", loopScope, this->callStack.peek()->level + 1);

        if (FLAGS_callStack) {
            clog << "CallStack: enter 'for' level: " << ar->level
                 << endl;
            clog << ar->__str__();
        }

        this->callStack.push(ar);

        for(visitor(forStatement->initList);
            forStatement->noCondition ? true : (bool)visitor(forStatement->condition);
            visitor(forStatement->adjustment))
        {
            ret = visitor(forStatement->compound);
        }

        this->callStack.pop();

        if (FLAGS_callStack) {
            clog << "CallStack: leave 'for'" << endl;
            clog << "\033[34mReturn " << ret << "\033[0m" << endl;
            clog << ar->__str__();
        }

        this->currentSymbolTable = stBeforeCall;

        return ret;
    }

    any Interpreter::FunctionDeclVisitor(AST *node) {
        // TODO
        return 0;
    }

    any Interpreter::FunctionCallVisitor(AST *node) {
        // current function
        SymbolTable* stBeforeCall = this->currentSymbolTable;
        FunctionCall *fun = (FunctionCall *) node;
        string funName = fun->id;
        fun->symbol_function =
                (Symbol_function *) this->currentSymbolTable->find(funName);
        if (fun->symbol_function == nullptr) {
            string msg = "undefined reference '" + funName + "'";
            throw ExceptionFactory(__LogicException, msg, fun->getToken().line,
                                   fun->getToken().column);
        }

        // Activation Record and symbol of target function
        ActivationRecord *ar = new ActivationRecord(
                funName, function, this->callStack.peek()->level + 1);
        Symbol_function *symbol = fun->symbol_function;

        // formal and actual paremeters
        deque<Symbol *> formalParams = symbol->formalVariable;
        vector<AST *> actualParams = fun->paramList;

        if ((int) (formalParams.size()) != (int) (actualParams.size())) {
            string msg = "function '" + fun->id + "' takes " +
                         to_string((int) formalParams.size()) +
                         " arguments but " +
                         to_string((int) (actualParams.size())) + " were given";
            throw ExceptionFactory(__LogicException, msg, fun->getToken().line,
                                   fun->getToken().column);
        }

        deque<Symbol *>::iterator iterFormal = formalParams.begin();
        vector<AST *>::iterator iterActual = actualParams.begin();
        for (; iterFormal != formalParams.end(); iterFormal++, iterActual++) {
            ar->__setitem__((*iterFormal)->name, visitor(*iterActual));
        }

        if (FLAGS_callStack) {
            clog << "CallStack: enter '" + fun->id + "' level: " << ar->level
                 << endl;
            clog << ar->__str__();
        }

        // load symboltable
        this->callStack.push(ar);
        for (auto subSymbolTable : this->currentSymbolTable->child) {
            if (subSymbolTable->symbolMap->name == funName) {
                this->currentSymbolTable = subSymbolTable;
                break;
            }
        }
        // visit function
        any ret = visitor((AST *) symbol->node_ast);

        if (FLAGS_callStack) {
            clog << "CallStack: leave '" + fun->id + "'" << endl;
            clog << "\033[34mReturn " << ret << "\033[0m" << endl;
            clog << ar->__str__();
        }

        this->callStack.pop();
        this->currentSymbolTable = stBeforeCall;

        delete ar;
        return ret;
    }

    any Interpreter::GlobalVisitor(AST *node) {
        Global *global = (Global *)node;
        Variable *var = (Variable *)(global->var);
        ActivationRecord *gar = this->callStack.global();

        if (gar->members.find(var->id) == gar->members.end()) {
            string msg = "name '" + var->id + "' is not defined in global scope";
            throw ExceptionFactory(__LogicException, msg, var->getToken().line,
                                   var->getToken().column);
        }

        // TODO : Link

        return 0;
    }

    any Interpreter::IfVisitor(AST *node)
    {
        If *ifStatement = (If *)node;
        any ret;

        if((!ifStatement->noCondition && visitor(ifStatement->condition)) ||
            ifStatement->noCondition)
        {
            SymbolTable* stBeforeCall = this->currentSymbolTable;
            for(SymbolTable *subSymbolTable : this->currentSymbolTable->child)
            {
                if(subSymbolTable->symbolMap->name == "if" &&
                   subSymbolTable->symbolMap->addr == (uint64_t)&ifStatement)
                {
                    this->currentSymbolTable = subSymbolTable;
                    break;
                }
            }
            ActivationRecord *ar = new ActivationRecord(
                    "if",ifScope,this->callStack.peek()->level + 1);

            if (FLAGS_callStack) {
            clog << "CallStack: enter 'if' level: " << ar->level
                 << endl;
            clog << ar->__str__();
            
            }

            this->callStack.push(ar);
            ret = visitor(ifStatement->compound);
            this->callStack.pop();

            if (FLAGS_callStack) {
            clog << "CallStack: leave 'if'" << endl;
            clog << "\033[34mReturn " << ret << "\033[0m" << endl;
            clog << ar->__str__();
            }

            this->currentSymbolTable = stBeforeCall;
        }
        else
        {
            ret = visitor(ifStatement->next);
        }

        return ret;
    }

    any Interpreter::InputVisitor(AST *node) {
        Input *input = (Input *)node;
        Variable *var = (Variable *)input->var;

        any val;
        std::cin >> val;

        this->callStack.__setitem__(var->id, val);

        return val;
    }

    any Interpreter::NumVisitor(AST *node) {
        Num *num = (Num *) node;

        return num->getValue();
    }

    any Interpreter::PrintfVisitor(AST *node) {
        Printf *output = (Printf *)node;

        any out = visitor(output->content);
        cout << out;

        return 0;
    }

    any Interpreter::ReturnVisitor(AST *node) {
        Return *ret = (Return *) node;
        any retval;

        if (ret->ret != nullptr) retval = visitor(ret->ret);
        setStatus(Status_ret);

        return retval;
    }

    any Interpreter::StringVisitor(AST *node) {
        class String *ret = (class String *)node;
        
        return ret->getValue();
    }

    any Interpreter::UnaryOpVisitor(AST *node) {
        UnaryOp *op = (UnaryOp *) node;

        if (op->getOp() == PLUS) return (any) 0 + visitor(op->right);
        if (op->getOp() == MINUS) return (any) 0 - visitor(op->right);
        if (op->getOp() == NOT) return !(bool)visitor(op->right);
        return 0;
    }

    any Interpreter::VariableVisitor(AST *node) {
        Variable *var = (Variable *) node;

        any value = this->callStack.__getitem__(var->id);

        return value;
    }

    any Interpreter::WhileVisitor(AST *node) {
        While *whileStatement = (While *)node;
        any ret;

        SymbolTable *stBeforeCall = this->currentSymbolTable;
        for(SymbolTable *subSymbolTable : this->currentSymbolTable->child)
        {
            if(subSymbolTable->symbolMap->name == "while" &&
                   subSymbolTable->symbolMap->addr == (uint64_t)&whileStatement)
            {
                    this->currentSymbolTable = subSymbolTable;
                    break;
            }
        }
        ActivationRecord *ar = new ActivationRecord(
                "while", loopScope, this->callStack.peek()->level + 1);

        if (FLAGS_callStack) {
            clog << "CallStack: enter 'while' level: " << ar->level
                 << endl;
            clog << ar->__str__();
        }

        this->callStack.push(ar);

        while(visitor(whileStatement->condition))
        {
            ret = visitor(whileStatement->compound);
        }

        this->callStack.pop();

        if (FLAGS_callStack) {
            clog << "CallStack: leave 'while'" << endl;
            clog << "\033[34mReturn " << ret << "\033[0m" << endl;
            clog << ar->__str__();
        }

        this->currentSymbolTable = stBeforeCall;

        return ret;
    }
} // namespace AVSI
