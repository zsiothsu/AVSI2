/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-22 19:07:01
 * @Description: include Interpreter class
 */
#ifndef ___INTERPRETER_H___
#define ___INTERPRETER_H___

#include "NodeVisitor.h"

namespace AVSI
{   
    using std::map;
    using std::vector;
    
    class Interpreter: public NodeVisitor
    {
    private:
        SymbolTable symbolTable;
    public:
        Interpreter(void): symbolTable(SymbolTable()) {};
        virtual ~Interpreter() {};

        any visitor(AST* node);
        any AssignVisitor(AST* node);
        any BinOpVisitor(AST* node);
        any CompoundVisitor(AST* node);
        any FunctionDeclVisitor(AST* node);
        any NumVisitor(AST* node);
        any UnaryOpVisitor(AST* node);
        any VariableVisitor(AST* node);

        any interpret(AST* root);
    };

    static map<string,any> globalVariable = map<string,any>();
}

#endif