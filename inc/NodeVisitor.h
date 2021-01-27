/*
 * @Author: your name
 * @Date: 2020-05-19
 * @LastEditTime: 2020-05-28 15:55:54
 * @Description: file content
 */
#ifndef ___NODEVISITOR_H___
#define ___NODEVISITOR_H___

#include "Parser.h"
#include "SymbolTable.h"
#include "StatusCode.h"

namespace AVSI {
    using std::map;
    using std::string;

    class NodeVisitor {
    public:
        NodeVisitor(void) {};

        virtual ~NodeVisitor() {};

        virtual any visitor(AST *node) = 0;

        virtual any AssignVisitor(AST *node) = 0;

        virtual any BinOpVisitor(AST *node) = 0;

        virtual any BooleanVisitor(AST *node) = 0;

        virtual any CompoundVisitor(AST *node) = 0;

        virtual any FunctionDeclVisitor(AST *node) = 0;

        virtual any FunctionCallVisitor(AST *node) = 0;

        virtual any NumVisitor(AST *node) = 0;

        virtual any ReturnVisitor(AST *node) = 0;

        virtual any UnaryOpVisitor(AST *node) = 0;

        virtual any VariableVisitor(AST *node) = 0;
    };

    typedef any (NodeVisitor::*visitNode)(AST *node);

    static map<string, visitNode> visitorMap = {
            {"NumVisitor",          &NodeVisitor::NumVisitor},
            {"BinOpVisitor",        &NodeVisitor::BinOpVisitor},
            {"BooleanVisitor",      &NodeVisitor::BooleanVisitor},
            {"UnaryOpVisitor",      &NodeVisitor::UnaryOpVisitor},
            {"FunctionDeclVisitor", &NodeVisitor::FunctionDeclVisitor},
            {"FunctionCallVisitor", &NodeVisitor::FunctionCallVisitor},
            {"AssignVisitor",       &NodeVisitor::AssignVisitor},
            {"CompoundVisitor",     &NodeVisitor::CompoundVisitor},
            {"VariableVisitor",     &NodeVisitor::VariableVisitor},
            {"ReturnVisitor",       &NodeVisitor::ReturnVisitor}};
} // namespace AVSI

#endif