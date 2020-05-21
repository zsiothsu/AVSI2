/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-21 16:26:20
 * @Description: file content
 */ 
#include "../inc/SemanticAnalyzer.h"

namespace AVSI
{
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
        visitor(assign->left);
        visitor(assign->right);
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

    any SemanticAnalyzer::FunctionVisitor(AST* node)
    {
        //TODO
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
        Symbol symbol = {var->id,variable_t};
        symbolTable.insert(symbol);
        return 0;
    }
}