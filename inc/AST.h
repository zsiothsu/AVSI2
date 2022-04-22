/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:55:23
 * @Description: constructer of abstract syntax tree (AST)
 */
#ifndef ___AST_H___
#define ___AST_H___

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Pass.h"

#include "Exception.h"
#include "Token.h"
#include <typeinfo>
#include <utility>
#include <vector>

#define __ASSIGN_NAME           "Assign"
#define __BINOP_NAME            "BinOp"
#define __BOOL_NAME             "Boolean"
#define __COMPOUND_NAME         "Compound"
#define __ECHO_NAME             "Echo"
#define __FOR_NAME              "For"
#define __FUNCTIONCALL_NAME     "FunctionCall"
#define __FUNCTIONDECL_NAME     "FunctionDecl"
#define __GLOBAL_NAME           "Global"
#define __IF_NAME               "If"
#define __INPUT_NAME            "Input"
#define __NUM_NAME              "Num"
#define __OBJECT_NAME           "Object"
#define __PARAM_NAME            "Param"
#define __PRINTF_NAME           "Printf"
#define __RETURN_NAME           "Return"
#define __STRING_NAME           "String"
#define __UNARYTOP_NAME         "UnaryOp"
#define __VARIABLE_NAME         "Variable"
#define __WHILE_NAME            "While"

#define __NONEAST_NAME "NoneAST"

namespace AVSI {
    using std::string;
    using std::vector;
    using std::unique_ptr;
    using std::map;

    using Type = pair<llvm::Type *, string>;

    /*******************************************************
     *                       AST base                      *
     *******************************************************/
    class AST {
    protected:
        Token token;

    public:
        string __AST_name;

        AST(void) {};

        AST(string name) : __AST_name(std::move(name)) {};

        AST(string name, const Token& token) : token(token), __AST_name(std::move(name)) {};

        virtual ~AST() {};

        virtual llvm::Value *codeGen();

        Token getToken(void);
    };

    /*******************************************************
     *                    derived syntax                   *
     *******************************************************/
    class Assign : public AST {
    private:
    public:
        AST *left;
        AST *right;

        Assign(void) : AST(__ASSIGN_NAME), left(nullptr), right(nullptr) {};

        Assign(const Token& token, AST *left, AST *right)
                : AST(__ASSIGN_NAME, token),
                  left(left),
                  right(right) {};

        virtual ~Assign();

        llvm::Value *codeGen() override;
    };

    class BinOp : public AST {
    private:
        Token op;

    public:
        AST *left;
        AST *right;

        BinOp(void)
                : AST(__BINOP_NAME),
                  op(emptyToken),
                  left(nullptr),
                  right(nullptr) {};

        BinOp(AST *left, const Token& op, AST *right)
                : AST(__BINOP_NAME, op), op(op), left(left), right(right) {};

        virtual ~BinOp();

        TokenType getOp(void);

        llvm::Value *codeGen() override;
    };

    class Boolean : public AST {
    private:
        any value;

    public:
        Boolean(void) : AST(__BOOL_NAME) {};

        Boolean(Token token) : AST(__BOOL_NAME, token), value((token.getType() == TRUE)) {};

        virtual ~Boolean() {};

        any getValue(void);

        llvm::Value *codeGen() override;
    };

    class Compound : public AST {
    public:
        vector<AST *> child;

        Compound(void)
                : AST(__COMPOUND_NAME, Token(COMPOUND, 0)),
                  child(vector<AST *>()) {};

        virtual ~Compound();

        llvm::Value *codeGen() override;
    };

    class For : public AST {
    public:
        AST *initList;
        AST *condition;
        AST *adjustment;
        AST *compound;
        bool noCondition;

        For(void)
                : AST(__FOR_NAME), initList(nullptr), condition(nullptr),
                  adjustment(nullptr), compound(nullptr) {};

        For(AST *initList, AST *condition, AST *adjustment, AST *compound, bool noCondition, Token token)
                : AST(__FOR_NAME, token), initList(initList), condition(condition),
                  adjustment(adjustment), compound(compound), noCondition(noCondition) {};

        virtual ~For();

        llvm::Value *codeGen() override;
    };

    class FunctionDecl : public AST {
    public:
        string id;
        Type retTy;
        AST *paramList;
        AST *compound;

        FunctionDecl(void)
                : AST(__FUNCTIONDECL_NAME), retTy(Type()), paramList(nullptr), compound(nullptr) {};

        FunctionDecl(string id, Type retTy, AST *paramList, AST *compound, const Token& token)
                : AST(__FUNCTIONDECL_NAME, token),
                  id(std::move(id)),
                  retTy(std::move(retTy)),
                  paramList(paramList),
                  compound(compound) {};

        virtual ~FunctionDecl();

        llvm::Value *codeGen() override;
    };

    class FunctionCall : public AST {
    public:
        string id;
        vector<AST *> paramList;

        FunctionCall(void)
                : AST(__FUNCTIONCALL_NAME), paramList(vector<AST *>()) {};

        FunctionCall(string id, vector<AST *> paramList, const Token& token)
                : AST(__FUNCTIONCALL_NAME, token), id(std::move(id)), paramList(std::move(paramList)) {};

        virtual ~FunctionCall();

        llvm::Value *codeGen() override;
    };

    class Global : public AST {
    public:
        AST *var;

        Global(void) : AST(__GLOBAL_NAME) {};

        Global(AST *var, Token token)
                : AST(__GLOBAL_NAME, token), var(var) {};

        virtual ~Global();

        llvm::Value *codeGen() override;
    };

    class If : public AST {
    public:
        AST *condition;
        bool noCondition;
        AST *compound;
        AST *next;

        If(void) : AST(__IF_NAME), condition(nullptr), compound(nullptr), next(nullptr) {};

        If(AST *condition, bool noCondition, AST *compound, AST *next, Token token) :
                AST(__IF_NAME, token), condition(condition), noCondition(noCondition),
                compound(compound), next(next) {};

        virtual ~If();

        llvm::Value *codeGen() override;
    };

    class Num : public AST {
    private:
        any value;

    public:
        Num(void) : AST(__NUM_NAME) {};

        explicit Num(Token token) : AST(__NUM_NAME, token), value(token.getValue()) {};

        virtual ~Num() {};

        any getValue(void);

        llvm::Value *codeGen() override;
    };

    class Variable : public AST {
    public:
        std::string id;
        Type Ty;

        enum offsetType {
            ARRAY = 0,
            MEMBER = 1
        };

        vector<pair<offsetType, AST*>> offset;

        Variable(void)
                : AST(__VARIABLE_NAME),
                  id(""),
                  Ty(Type(nullptr, "")),
                  offset() {};

        explicit Variable(Token var)
                : AST(__VARIABLE_NAME, var),
                  id(var.getValue().any_cast<std::string>()),
                  Ty(Type(nullptr, "")),
                  offset() {};

        Variable(Token var, Type ty)
                : AST(__VARIABLE_NAME, var),
                  id(var.getValue().any_cast<std::string>()),
                  Ty(std::move(ty)),
                  offset(){};

        Variable(Token var, Type ty, vector<pair<offsetType, AST*>> offset)
                : AST(__VARIABLE_NAME, var),
                  id(var.getValue().any_cast<std::string>()),
                  Ty(std::move(ty)),
                  offset(std::move(offset)){};

        ~Variable() {};

        llvm::Value *codeGen() override;
    };

    class Object : public AST {
    public:
        string id;
        vector<Variable *> memberList;

        Object(void) : AST(__OBJECT_NAME), memberList(vector<Variable *>()) {};

        Object(const Token& token) : AST(__OBJECT_NAME, token), memberList(vector<Variable *>()) {};

        Object(const Token& token, string id, vector<Variable *> memberList)
                : AST(__OBJECT_NAME, token),
                  id(std::move(id)),
                  memberList(std::move(memberList)) {};

        virtual ~Object() {}

        llvm::Value *codeGen() override;
    };

    class Param : public AST {
    public:
        vector<Variable *> paramList;

        Param(void) : AST(__PARAM_NAME), paramList(vector<Variable *>()) {};
    };

    class Return : public AST {
    public:
        AST *ret;

        Return(void) : AST(__RETURN_NAME) {};

        Return(const Token& token, AST *ret) : AST(__RETURN_NAME, token), ret(ret) {};

        llvm::Value *codeGen() override;
    };

    class String : public AST {
    private:
        any value;

    public:
        String(void) : AST(__STRING_NAME) {};

        String(Token token) : AST(__STRING_NAME, token), value(token.getValue()) {};


        any getValue(void);
    };

    class UnaryOp : public AST {
    private:
        Token op;

    public:
        AST *right;

        UnaryOp(void) : AST(__UNARYTOP_NAME) {};

        UnaryOp(Token op, AST *right)
                : AST(__UNARYTOP_NAME, op), op(op), right(right) {};

        virtual ~UnaryOp();

        TokenType getOp(void);

        llvm::Value *codeGen() override;
    };

    class While : public AST {
    public:
        AST *condition;
        AST *compound;

        While(void)
                : AST(__WHILE_NAME), condition(nullptr), compound(nullptr) {};

        While(AST *condition, AST *compound, const Token& token)
                : AST(__WHILE_NAME, token), condition(condition), compound(compound) {};

        virtual ~While();

        llvm::Value *codeGen() override;
    };

    class NoneAST : public AST {
    public:
        NoneAST(void) : AST(__NONEAST_NAME, Token(NONE, 0)) {};

        virtual ~NoneAST() {};

        llvm::Value *codeGen() override;
    };

    static AST ASTEmpty = NoneAST();

    void llvm_module_fpm_init();

    void llvm_machine_init();

    void llvm_obj_output();

    void llvm_asm_output();

    void llvm_module_printIR();
} // namespace AVSI

#endif