/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-27 14:42:09
 * @Description: file content
 */ 
#include "../inc/SemanticAnalyzer.h"

namespace AVSI
{
    void SemanticAnalyzer::SemanticAnalyze(AST* root)
    {
        if(FLAGS_scope == true)
        {
            clog << "\033[32m====================================\033[0m" << endl
                 << "\033[32m          Semantic Analyze\033[0m"           << endl
                 << "\033[32m------------------------------------\033[0m" << endl;
        }

        this->symbolTable.push("global");
        visitor(root);
        this->symbolTable.pop();

        if(FLAGS_scope == true)
        {
            clog << "\033[32m====================================\033[0m" << endl << endl;
        }
    }

    any SemanticAnalyzer::visitor(AST* node)
    {
        if(node == &ASTEmpty) return 0;

        string visitorName = node->__AST_name + "Visitor";
        map<string,visitNode>::iterator iter = visitorMap.find(visitorName);
        if(iter != visitorMap.end()) (this->*((*iter).second))(node);

        return 0;

    }
    any SemanticAnalyzer::AssignVisitor(AST* node)
    {
        Assign* assign = (Assign*)node;

        Variable* var = (Variable*)assign->left;
        visitor(assign->right);

        Symbol definedSymbol = this->symbolTable.find(var->id);
        if(definedSymbol.type == function_t)
        {
            string msg = "symbol '" + var->id + "' has beed defined as function";
            throw ExceptionFactory(__LogicException,msg,var->getToken().line,var->getToken().column);
        }

        this->symbolTable.insert({var->id,variable_t});

        return 0;
    }

    any SemanticAnalyzer::BinOpVisitor(AST* node)
    {
        BinOp* op = (BinOp*)node;

        visitor(op->left);
        visitor(op->right);

        return 0;
    }

    any SemanticAnalyzer::CompoundVisitor(AST* node)
    {
        Compound* compound = (Compound*)node;

        for(AST* ast : compound->child) visitor(ast);

        return 0;
    }

    any SemanticAnalyzer::FunctionDeclVisitor(AST* node)
    {
        FunctionDecl* functionDecl = (FunctionDecl*)node;

        SymbolType type = this->symbolTable.find(functionDecl->id).type;
        if(type != null_t)
        {
            string msg = "mutiple definition";
            if(type == function_t)
            {
                msg = "mutiple definition of function '"  \
                     + functionDecl->id                  \
                     + "'";
            }
            else if(type == variable_t)
            {
                msg = "function '"                           \
                     + functionDecl->id                      \
                     + "' have the same name with variable";
            }
            throw ExceptionFactory(__LogicException,msg,
                                   functionDecl->getToken().line,
                                   functionDecl->getToken().column);
        }
        this->symbolTable.insert({functionDecl->id,function_t});

        this->symbolTable.push(functionDecl->id);
        if(functionDecl->paramList != nullptr) visitor(functionDecl->paramList);
        visitor(functionDecl->compound);
        this->symbolTable.pop();

        return 0;
    }

    any SemanticAnalyzer::FunctionCallVisitor(AST* node)
    {
        return 0;
    }

    any SemanticAnalyzer::ParamVisitor(AST* node)
    {
        Param* param = (Param*)node;
        for(Variable* var:param->paramList)
        {
            Symbol definedSymbol = this->symbolTable.find(var->id);
            if(definedSymbol.type == function_t)
            {
                string msg = "symbol '" + var->id + "' has beed defined as function";
                throw ExceptionFactory(__LogicException,msg,var->getToken().line,var->getToken().column);
            }
            this->symbolTable.insert({var->id,variable_t});
        }
        return 0;
    }

    any SemanticAnalyzer::NumVisitor(AST* node)
    {
        return 0;
    }

    any SemanticAnalyzer::UnaryOpVisitor(AST* node)
    {
        UnaryOp* op = (UnaryOp*)node;

        visitor(op->right);

        return 0;
    }

    any SemanticAnalyzer::VariableVisitor(AST* node)
    {
        Variable* var = (Variable*)node;

        Symbol symbol = this->symbolTable.find(var->id);
        if(symbol.type == null_t)
        {
            string msg = "name '" + var->id + "' is not defined";
            throw ExceptionFactory(__LogicException,msg,var->getToken().line,var->getToken().column);
        }
        
        return 0;
    }
}