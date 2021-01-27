/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-28 18:16:17
 * @Description: include Interpreter class
 */
#ifndef ___INTERPRETER_H___
#define ___INTERPRETER_H___

#include "CallStack.h"
#include "NodeVisitor.h"

namespace AVSI {
    using std::map;
    using std::vector;

    class Interpreter : public NodeVisitor {
    private:
        CallStack callStack;
        SymbolTable *symbolTable;
        SymbolTable *currentSymbolTable;
        AST *root;

    public:
        Interpreter(void) : callStack(CallStack()) {};

        Interpreter(AST *root, SymbolTable *symbolTable)
                : symbolTable(symbolTable), currentSymbolTable(symbolTable),
                  root(root) {};

        virtual ~Interpreter();

        any visitor(AST *node);

        any AssignVisitor(AST *node);

        any BinOpVisitor(AST *node);

        any BooleanVisitor(AST *node);

        any CompoundVisitor(AST *node);

        any EchoVisitor(AST *node);

        any FunctionDeclVisitor(AST *node);

        any FunctionCallVisitor(AST *node);

        any NumVisitor(AST *node);

        any ReturnVisitor(AST *node);

        any UnaryOpVisitor(AST *node);

        any VariableVisitor(AST *node);

        any interpret(void);
    };

    static map<string, any> globalVariable = map<string, any>();
} // namespace AVSI

#endif