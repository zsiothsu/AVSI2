/*
 * @Author: your name
 * @Date: 2020-05-19
 * @LastEditTime: 2020-05-28 15:55:54
 * @Description: file content
 */
#ifndef ___NODEVISITOR_H___
#define ___NODEVISITOR_H___

#include "Parser.h"
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

        virtual any EchoVisitor(AST *node) = 0;

        virtual any ForVisitor(AST *node) = 0;

        virtual any FunctionDeclVisitor(AST *node) = 0;

        virtual any FunctionCallVisitor(AST *node) = 0;

        virtual any GlobalVisitor(AST *node) = 0;

        virtual any IfVisitor(AST *node) = 0;

        virtual any InputVisitor(AST *node) = 0;

        virtual any NumVisitor(AST *node) = 0;

        virtual any PrintfVisitor(AST *node) = 0;

        virtual any ReturnVisitor(AST *node) = 0;

        virtual any StringVisitor(AST *node) = 0;

        virtual any UnaryOpVisitor(AST *node) = 0;

        virtual any VariableVisitor(AST *node) = 0;

        virtual any WhileVisitor(AST *node) = 0;
    };

    typedef any (NodeVisitor::*visitNode)(AST *node);

    static map<string, visitNode> visitorMap = {
            {"AssignVisitor",       &NodeVisitor::AssignVisitor},
            {"BinOpVisitor",        &NodeVisitor::BinOpVisitor},
            {"BooleanVisitor",      &NodeVisitor::BooleanVisitor},
            {"CompoundVisitor",     &NodeVisitor::CompoundVisitor},
            {"EchoVisitor",         &NodeVisitor::EchoVisitor},
            {"ForVisitor",          &NodeVisitor::ForVisitor},
            {"FunctionCallVisitor", &NodeVisitor::FunctionCallVisitor},
            {"FunctionDeclVisitor", &NodeVisitor::FunctionDeclVisitor},
            {"GlobalVisitor",       &NodeVisitor::GlobalVisitor},
            {"IfVisitor",           &NodeVisitor::IfVisitor},
            {"InputVisitor",         &NodeVisitor::InputVisitor},
            {"NumVisitor",          &NodeVisitor::NumVisitor},
            {"PrintfVisitor",       &NodeVisitor::PrintfVisitor},
            {"ReturnVisitor",       &NodeVisitor::ReturnVisitor},
            {"StringVisitor",       &NodeVisitor::StringVisitor},
            {"UnaryOpVisitor",      &NodeVisitor::UnaryOpVisitor},
            {"VariableVisitor",     &NodeVisitor::VariableVisitor},
            {"WhileVisitor",        &NodeVisitor::WhileVisitor}};
} // namespace AVSI

#endif