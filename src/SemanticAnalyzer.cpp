/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-06-02 15:56:48
 * @Description: file content
 */
#include "../inc/SemanticAnalyzer.h"

namespace AVSI {
    SemanticAnalyzer::~SemanticAnalyzer() {
        if (this->symbolTable != nullptr) delete this->symbolTable;
    }

    SymbolTable *SemanticAnalyzer::SemanticAnalyze(AST *root) {
        if (FLAGS_scope) {
            clog << "\033[32m====================================\033[0m"
                 << endl
                 << "\033[32m          Semantic Analyze\033[0m" << endl
                 << "\033[32m------------------------------------\033[0m"
                 << endl;
        }

        visitor(root);

        this->symbolTable->__str();

        if (FLAGS_scope) {
            clog << "\033[32m====================================\033[0m"
                 << endl
                 << endl;
        }

        return this->symbolTable;
    }

    any SemanticAnalyzer::visitor(AST *node) {
        if (node == &ASTEmpty || node == nullptr) return 0;

        string visitorName = node->__AST_name + "Visitor";
        map<string, visitNode>::iterator iter = visitorMap.find(visitorName);
        if (iter != visitorMap.end()) (this->*((*iter).second))(node);

        return 0;
    }

    any SemanticAnalyzer::AssignVisitor(AST *node) {
        Assign *assign = (Assign *) node;

        Variable *var = (Variable *) assign->left;
        visitor(assign->right);

        Symbol *definedSymbol = this->currentSymbolTable->find(var->id);
        if ((definedSymbol != nullptr) && (definedSymbol->type == function_t)) {
            string msg =
                    "symbol '" + var->id + "' has beed defined as function";
            throw ExceptionFactory(__LogicException, msg, var->getToken().line,
                                   var->getToken().column);
        }

        this->currentSymbolTable->insert(new Symbol(var->id, variable_t));

        return 0;
    }

    any SemanticAnalyzer::BinOpVisitor(AST *node) {
        BinOp *op = (BinOp *) node;

        visitor(op->left);
        visitor(op->right);

        return 0;
    }

    any SemanticAnalyzer::BooleanVisitor(AST *node) {
        // pass
        return 0;
    }

    any SemanticAnalyzer::CompoundVisitor(AST *node) {
        Compound *compound = (Compound *) node;

        for (AST *ast : compound->child) visitor(ast);

        return 0;
    }

    any SemanticAnalyzer::EchoVisitor(AST *node) {
        Echo *echo = (Echo *)node;

        visitor(echo->content);

        return 0;
    }

    any SemanticAnalyzer::ForVisitor(AST *node) {
        For *forStatement = (For *)node;

        SymbolTable *subtable = new SymbolTable(this->currentSymbolTable, "for", (uint64_t)&forStatement,
                                                this->currentSymbolTable->level + 1);

        this->symbolTable->mount(subtable);
        this->symbolTable = subtable;

        visitor(forStatement->initList);
        visitor(forStatement->condition);
        visitor(forStatement->adjustment);
        visitor(forStatement->compound);

        this->currentSymbolTable = subtable->father;

        return 0;
    }

    any SemanticAnalyzer::FunctionDeclVisitor(AST *node) {
        FunctionDecl *functionDecl = (FunctionDecl *) node;

        // mutiple definition
        Symbol *symbol = this->currentSymbolTable->find(functionDecl->id);
        if (symbol != nullptr) {
            string msg = "mutiple definition";
            if (symbol->type == function_t) {
                msg =
                        "mutiple definition of function '" + functionDecl->id + "'";
            } else if (symbol->type == variable_t) {
                msg = "function '" + functionDecl->id +
                      "' have the same name with variable";
            }
            throw ExceptionFactory(__LogicException, msg,
                                   functionDecl->getToken().line,
                                   functionDecl->getToken().column);
        }

        // insert current function to current symboltable
        Symbol_function *fun =
                new Symbol_function(functionDecl->id, function_t);
        this->currentSymbolTable->insert(fun);
        fun->node_ast = (void *) functionDecl->compound;

        // create new symboltable for function scope and mount it under current
        // symboltable
        SymbolTable *subTable =
                new SymbolTable(this->currentSymbolTable, functionDecl->id,
                                this->currentSymbolTable->level + 1);
        this->currentSymbolTable->mount(subTable);
        this->currentSymbolTable = subTable;

        // add formal parameter
        if (functionDecl->paramList != nullptr) {
            Param *param = (Param *) (functionDecl->paramList);

            for (Variable *var : param->paramList) {
                Symbol *definedSymbol = this->currentSymbolTable->find(var->id);
                if (definedSymbol != nullptr &&
                    definedSymbol->type == function_t) {
                    string msg =
                            "symbol '" + var->id + "' has beed defined as function";
                    throw ExceptionFactory(__LogicException, msg,
                                           var->getToken().line,
                                           var->getToken().column);
                }
                Symbol *paramSymbol = new Symbol(var->id, variable_t);
                fun->formal_variable.push_back(paramSymbol);
                this->currentSymbolTable->insert(paramSymbol);
            }
        }

        // visit function, and set 'inFunction' status
        bool statusCodeInFunction_preDecl = getStatus(Status_inFunction);
        setStatus(Status_inFunction);
        visitor(functionDecl->compound);
        setStatus(statusCodeInFunction_preDecl ? Status_inFunction : Status_none);

        this->currentSymbolTable = this->currentSymbolTable->father;

        return 0;
    }

    any SemanticAnalyzer::FunctionCallVisitor(AST *node) {
        FunctionCall *functionCall = (FunctionCall *) node;

        for (auto paramNode : functionCall->paramList) visitor(paramNode);

        return 0;
    }

    any SemanticAnalyzer::GlobalVisitor(AST *node) {
        Global *global = (Global *)node;

        Variable *var = (Variable *)global->var;
        
        Symbol *definedSymbol = this->currentSymbolTable->find(var->id);
        if ((definedSymbol != nullptr) && (definedSymbol->type == function_t)) {
            string msg =
                    "symbol '" + var->id + "' has beed defined as function";
            throw ExceptionFactory(__LogicException, msg, var->getToken().line,
                                   var->getToken().column);
        }

        this->currentSymbolTable->insert(new Symbol(var->id, variable_t));

        return 0;
    }

    any SemanticAnalyzer::IfVisitor(AST *node) {
        If *ifStatement = (If *)node;

        // visit expression
        if(!ifStatement->noCondition) visitor(ifStatement->condition);
        
        // mount symbol table
        SymbolTable *subtable = new SymbolTable(this->currentSymbolTable,"if", (uint64_t)&ifStatement,
                                                this->currentSymbolTable->level + 1);

        this->currentSymbolTable->mount(subtable);
        this->currentSymbolTable = subtable;

        visitor(ifStatement->compound);

        this->currentSymbolTable = this->currentSymbolTable->father;

        if(ifStatement->next != nullptr) visitor(ifStatement->next);

        return 0;       
    }

    any SemanticAnalyzer::InputVisitor(AST *node) {
        Input *input = (Input *)node;

        visitor(input->var);

        return 0;
    }

    any SemanticAnalyzer::PrintfVisitor(AST *node) {
        Printf *output = (Printf *)node;

        visitor(output->content);

        return 0;
    }

    any SemanticAnalyzer::NumVisitor(AST *node) {
        // pass
        return 0;
    }

    any SemanticAnalyzer::ReturnVisitor(AST *node) {
        Return *ret = (Return *) node;

        if (!getStatus(Status_inFunction)) {
            throw ExceptionFactory(__LogicException, "'return' outside function",
                                   ret->getToken().line, ret->getToken().column);
        }

        if (ret->ret != nullptr) visitor(ret->ret);

        return 0;
    }

    any SemanticAnalyzer::StringVisitor(AST *node) {
        // pass
        return 0;
    }

    any SemanticAnalyzer::UnaryOpVisitor(AST *node) {
        UnaryOp *op = (UnaryOp *) node;

        visitor(op->right);

        return 0;
    }

    any SemanticAnalyzer::VariableVisitor(AST *node) {
        Variable *var = (Variable *) node;

        Symbol *symbol = this->currentSymbolTable->find(var->id);
        if (symbol == nullptr) {
            string msg = "name '" + var->id + "' is not defined";
            throw ExceptionFactory(__LogicException, msg, var->getToken().line,
                                   var->getToken().column);
        }

        return 0;
    }

    any SemanticAnalyzer::WhileVisitor(AST *node) {
        While *whileStatement  = (While *)node;

        visitor(whileStatement->condition);

        SymbolTable *subtable = new SymbolTable(this->currentSymbolTable, "while", (uint64_t)&whileStatement,
                                                this->currentSymbolTable->level + 1);

        this->symbolTable->mount(subtable);
        this->symbolTable = subtable;

        visitor(whileStatement->compound);

        this->currentSymbolTable = subtable->father;

        return 0;
    }
} // namespace AVSI