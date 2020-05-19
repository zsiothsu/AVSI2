/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-05-19 23:40:04
 * @Description: include Interpreter class
 */
#ifndef ___INTERPRETER_H___
#define ___INTERPRETER_H___

#include "NodeVisitor.h"

namespace AVSI
{   
    using std::map;
    using std::vector;
    
    class Interpreter: public AVSI::NodeVisitor
    {
    private:
        Parser* parser;
    public:
        Interpreter(void) {};
        Interpreter(Parser* parser): parser(parser) {};
        virtual ~Interpreter() {};

        any visitor(AST* node);
        any AssignVisitor(AST* node);
        any BinOpVisitor(AST* node);
        any CompoundVisitor(AST* node);
        any NumVisitor(AST* node);
        any UnaryOpVisitor(AST* node);
        any VariableVisitor(AST* node);

        any interpret(void);
    };

    static map<string,any> globalVariable = map<string,any>();
}

#endif