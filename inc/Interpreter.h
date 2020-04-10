/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-04-10 20:52:41
 * @Description: file content
 */
#ifndef ___INTERPRETER_H___
#define ___INTERPRETER_H___

#include "Parser.h"

namespace INTERPRETER
{
    class NodeVisitor
    {
    public:
        NodeVisitor(void);
        
        int vistor(AST* root);
    };    

    class Interpreter
    {
    private:
        Parser* parser;
    public:
        Interpreter(void);
        Interpreter(Parser* parser);
        ~Interpreter();

        void interpret(int* ans);
    };    
}

#endif