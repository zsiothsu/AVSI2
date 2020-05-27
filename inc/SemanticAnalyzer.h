/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-28 00:17:00
 * @Description: file content
 */ 
#ifndef ___SEMANTICANALYZER_H___
#define ___SEMANTICANALYZER_H___

#include <gflags/gflags.h>
#include <gflags/gflags_declare.h>
#include "NodeVisitor.h"
#include "flags.h"

namespace AVSI
{

    using std::clog;

    class SemanticAnalyzer: public NodeVisitor
    {
    private:
        SymbolTable symbolTable;
        SymbolTable* currentSymbolTable;
    public:
        SemanticAnalyzer(void):
            symbolTable(SymbolTable(nullptr,"global",1)),
            currentSymbolTable(&symbolTable)
        {};
        virtual ~SemanticAnalyzer();

        any visitor(AST* node);
        any AssignVisitor(AST* node);
        any BinOpVisitor(AST* node);
        any CompoundVisitor(AST* node);
        any FunctionDeclVisitor(AST* node);
        any FunctionCallVisitor(AST* node);
        any ParamVisitor(AST* node);
        any NumVisitor(AST* node);
        any UnaryOpVisitor(AST* node);
        any VariableVisitor(AST* node);

        void SemanticAnalyze(AST* root);
    };
}

#endif