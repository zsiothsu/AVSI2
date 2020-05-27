/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-27 21:22:17
 * @Description: include Interpreter class
 */
#ifndef ___INTERPRETER_H___
#define ___INTERPRETER_H___

#include "NodeVisitor.h"
#include "CallStack.h"

namespace AVSI
{   
    using std::map;
    using std::vector;
    
    class Interpreter: public NodeVisitor
    {
    private:
        CallStack callStack;
    public:
        Interpreter(void): callStack(CallStack()) {};
        virtual ~Interpreter() {};

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

        any interpret(AST* root);
    };

    static map<string,any> globalVariable = map<string,any>();
}

#endif