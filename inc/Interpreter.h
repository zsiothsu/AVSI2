/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-05-04 14:16:08
 * @Description: file content
 */
#ifndef ___INTERPRETER_H___
#define ___INTERPRETER_H___

#include <map>
#include "Parser.h"
#include "any.h"

namespace AVSI
{   
    using namespace std;

    class NodeVisitor
    {
   
    public:
        NodeVisitor(void);
        
        static any vistor(AST* node);
        static any AssignVisitor(AST* node);
        static any BinOpVisitor(AST* node);
        static any NumVisitor(AST* node);
        
    };    

    class Interpreter: public NodeVisitor
    {
    private:
        Parser* parser;
    public:
        Interpreter(void);
        Interpreter(Parser* parser);
        ~Interpreter();

        void interpret(double* ans);
    };

    typedef any (*visiteNode)(AST* node);

    static map<TokenType,visiteNode> visitorMap = {
        {INT,NodeVisitor::NumVisitor},
        {FLT,NodeVisitor::NumVisitor},
        {ADD,NodeVisitor::BinOpVisitor},
        {DEC,NodeVisitor::BinOpVisitor},
        {MUL,NodeVisitor::BinOpVisitor},
        {DIV,NodeVisitor::BinOpVisitor},
        {ASSIGN,NodeVisitor::AssignVisitor}
    };
    
}

#endif