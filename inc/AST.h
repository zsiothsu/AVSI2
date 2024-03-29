/*
 * AST.h 2022
 *
 * definitions of AST types
 *
 * LLVM IR code generator
 *
 * MIT License
 *
 * Copyright (c) 2022 Chipen Hsiao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
#include "SymbolTable.h"
#include <typeinfo>
#include <utility>
#include <vector>

#define __ASSIGN_NAME           "Assign"
#define __BINOP_NAME            "BinOp"
#define __BLOCKEXPR_NAME        "BlockExpr"
#define __BOOL_NAME             "Boolean"
#define __COMPOUND_NAME         "Compound"
#define __ECHO_NAME             "Echo"
#define __FOR_NAME              "For"
#define __FUNCTIONCALL_NAME     "FunctionCall"
#define __FUNCTIONDECL_NAME     "FunctionDecl"
#define __GLOBAL_NAME           "Global"
#define __IF_NAME               "If"
#define __INPUT_NAME            "Input"
#define __LOOPCTRL_NAME         "LoopCtrl"
#define __NUM_NAME              "Num"
#define __OBJECT_NAME           "Object"
#define __PARAM_NAME            "Param"
#define __PRINTF_NAME           "Printf"
#define __RETURN_NAME           "Return"
#define __SIZEOF_NAME           "Sizeof"
#define __STRING_NAME           "String"
#define __STRUCTINIT_NAME       "StructInit"
#define __TYPETRANS_NAME        "TypeTrans"
#define __ARRAYINIT_NAME        "ArrayInit"
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

    extern llvm::Type *VOID_TY;

    /*******************************************************
     *                      structure                      *
     *******************************************************/
    struct StructDef {
        llvm::StructType *Ty;
        map<string, int> members;

        StructDef() = default;

        StructDef(llvm::StructType *Ty) : Ty(Ty), members(map<string, int>()) {}

        ~StructDef() = default;
    };

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

        AST(string name, const Token &token) : token(token), __AST_name(std::move(name)) {};

        virtual ~AST() {};

        virtual llvm::Value *codeGen();

        virtual void dump(int depth);

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

        Assign(const Token &token, AST *left, AST *right)
                : AST(__ASSIGN_NAME, token),
                  left(left),
                  right(right) {};

        virtual ~Assign();

        llvm::Value *codeGen() override;

        void dump(int depth) override;
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

        BinOp(AST *left, const Token &op, AST *right)
                : AST(__BINOP_NAME, op), op(op), left(left), right(right) {};

        virtual ~BinOp();

        TokenType getOp(void);

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class BlockExpr : public AST {
    public:
        AST *expr;

        BlockExpr(void)
                : AST(__BLOCKEXPR_NAME),
                  expr(nullptr) {};

        BlockExpr(Token token, AST *expr)
                : AST(__BLOCKEXPR_NAME, token),
                  expr(expr) {};

        virtual ~BlockExpr() {};

        llvm::Value *codeGen() override;

        void dump(int depth) override;
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

        void dump(int depth) override;
    };

    class Compound : public AST {
    public:
        vector<AST *> child;

        Compound(void)
                : AST(__COMPOUND_NAME, Token(COMPOUND, 0)),
                  child(vector<AST *>()) {};

        Compound(Token token)
                : AST(__COMPOUND_NAME, token),
                  child(vector<AST *>()) {};

        virtual ~Compound();

        llvm::Value *codeGen() override;

        void dump(int depth) override;
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

        void dump(int depth) override;
    };

    class FunctionDecl : public AST {
    public:
        string id;
        Type retTy;
        AST *paramList;
        AST *compound;
        bool is_export;
        bool is_mangle;

        FunctionDecl(void)
                : AST(__FUNCTIONDECL_NAME),
                  retTy(Type()),
                  paramList(nullptr),
                  compound(nullptr),
                  is_export(false),
                  is_mangle(true) {};

        FunctionDecl(string id, Type retTy, AST *paramList, AST *compound, const Token &token)
                : AST(__FUNCTIONDECL_NAME, token),
                  id(std::move(id)),
                  retTy(std::move(retTy)),
                  paramList(paramList),
                  compound(compound),
                  is_export(false),
                  is_mangle(true) {};

        virtual ~FunctionDecl();

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class FunctionCall : public AST {
    public:
        string id;
        vector<AST *> paramList;
        llvm::Value *param_this;

        FunctionCall(void)
                : AST(__FUNCTIONCALL_NAME), paramList(vector<AST *>()), param_this(nullptr) {};

        FunctionCall(string id, vector<AST *> paramList, const Token &token)
                : AST(__FUNCTIONCALL_NAME, token), id(std::move(id)), paramList(std::move(paramList)),
                  param_this(nullptr) {};

        virtual ~FunctionCall();

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class Global : public AST {
    public:
        AST *var;
        bool is_export;
        bool is_mangle;

        Global(void)
                : AST(__GLOBAL_NAME),
                  is_export(false),
                  is_mangle(true) {};

        Global(AST *var, Token token)
                : AST(__GLOBAL_NAME, token),
                  var(var),
                  is_export(false),
                  is_mangle(true) {};

        virtual ~Global();

        llvm::Value *codeGen() override;

        void dump(int depth) override;
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

        void dump(int depth) override;
    };

    class LoopCtrl : public AST {
    public:
        enum LoopCtrlType {
            CTRL_BREAK = 0,
            CTRL_CONTINUE = 1
        };

        LoopCtrlType type;

        LoopCtrl() : AST(__LOOPCTRL_NAME) {}

        LoopCtrl(LoopCtrlType type, Token token) : AST(__LOOPCTRL_NAME, token), type(type) {}

        llvm::Value *codeGen() override;

        void dump(int depth) override;
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

        void dump(int depth) override;
    };

    class Sizeof : public AST {
    public:
        AST *id;
        Type Ty;

        Sizeof(void) : AST(__SIZEOF_NAME) {};

        Sizeof(Token token, AST *id) : AST(__SIZEOF_NAME, token), id(id), Ty(Type(VOID_TY, "void")) {};

        Sizeof(Token token, Type Ty) : AST(__SIZEOF_NAME, token), id(nullptr), Ty(Ty) {};

        virtual ~Sizeof() {};

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class Variable : public AST {
    public:
        std::string id;
        Type Ty;
        bool is_export;

        enum offsetType {
            ARRAY = 0,
            MEMBER = 1,
            FUNCTION = 2
        };

        vector<pair<offsetType, AST *>> offset;

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
                  offset() {};

        Variable(Token var, Type ty, vector<pair<offsetType, AST *>> offset)
                : AST(__VARIABLE_NAME, var),
                  id(var.getValue().any_cast<std::string>()),
                  Ty(std::move(ty)),
                  offset(std::move(offset)) {};

        ~Variable() {};

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class Object : public AST {
    public:
        string id;
        vector<Variable *> memberList;
        bool is_export;
        bool is_mangle;

        Object(void)
                : AST(__OBJECT_NAME),
                  memberList(vector<Variable *>()),
                  is_export(false),
                  is_mangle(true) {};

        Object(const Token &token)
                : AST(__OBJECT_NAME, token),
                  memberList(vector<Variable *>()),
                  is_export(false),
                  is_mangle(true) {};

        Object(const Token &token, string id, vector<Variable *> memberList)
                : AST(__OBJECT_NAME, token),
                  id(std::move(id)),
                  memberList(std::move(memberList)),
                  is_export(false),
                  is_mangle(true) {};

        virtual ~Object() {}

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class Param : public AST {
    public:
        vector<Variable *> paramList;

        Param(void) : AST(__PARAM_NAME), paramList(vector<Variable *>()) {};

        Param(Token token) : AST(__PARAM_NAME, token), paramList(vector<Variable *>()) {};

        void dump(int depth) override;
    };

    class Return : public AST {
    public:
        AST *ret;

        Return(void) : AST(__RETURN_NAME) {};

        Return(const Token &token, AST *ret) : AST(__RETURN_NAME, token), ret(ret) {};

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class String : public AST {
    private:
        any value;

    public:
        String(void) : AST(__STRING_NAME) {};

        String(Token token) : AST(__STRING_NAME, token), value(token.getValue()) {};

        any getValue(void);

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class StructInit : public AST {
    public:
        string id;
        vector<AST *> paramList;

        StructInit(void)
                : AST(__STRUCTINIT_NAME), paramList(vector<AST *>()) {};

        StructInit(string id, vector<AST *> paramList, const Token &token)
                : AST(__STRUCTINIT_NAME, token), id(std::move(id)), paramList(std::move(paramList)) {};

        virtual ~StructInit();

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class TypeTrans : public AST {
    public:
        AST *factor;
        Type Ty;

        TypeTrans(void)
                : AST(__TYPETRANS_NAME), factor(nullptr) {};

        TypeTrans(AST *factor, Type Ty, Token token)
                : AST(__TYPETRANS_NAME, token), factor(factor), Ty(Ty) {};

        virtual ~TypeTrans();

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class ArrayInit : public AST {
    public:
        vector<AST *> paramList;
        Type Ty;
        uint32_t num;

        ArrayInit(void)
                : AST(__ARRAYINIT_NAME), paramList(vector<AST *>()) {};

        ArrayInit(vector<AST *> paramList, uint32_t num, const Token &token)
                : AST(__ARRAYINIT_NAME, token),
                  paramList(std::move(paramList)),
                  Ty(Type(VOID_TY, "void")),
                  num(num) {};

        ArrayInit(Type Ty, uint32_t num, const Token &token)
                : AST(__ARRAYINIT_NAME, token),
                  Ty(Ty),
                  num(num) {};

        virtual ~ArrayInit();

        llvm::Value *codeGen() override;

        void dump(int depth) override;
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

        void dump(int depth) override;
    };

    class While : public AST {
    public:
        AST *condition;
        AST *compound;

        While(void)
                : AST(__WHILE_NAME), condition(nullptr), compound(nullptr) {};

        While(AST *condition, AST *compound, const Token &token)
                : AST(__WHILE_NAME, token), condition(condition), compound(compound) {};

        virtual ~While();

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    class NoneAST : public AST {
    public:
        NoneAST(void) : AST(__NONEAST_NAME, Token(NONE, 0)) {};

        virtual ~NoneAST() {};

        llvm::Value *codeGen() override;

        void dump(int depth) override;
    };

    extern AST *ASTEmpty;
    extern AST *ASTEmptyNotEnd;

    pair<map<string, StructDef *>::iterator, string> find_struct(vector<string> modinfo, string &name);

    void llvm_import_module(vector<string> path, string mod, int line, int col, string as = string());

    void llvm_global_context_reset();

    void llvm_module_fpm_init();

    void llvm_machine_init();

    void llvm_emit_obj();

    void llvm_emit_asm();

    void llvm_emit_bitcode();

    void llvm_emit_ir();

    void llvm_create_dir(string dir);
} // namespace AVSI

#endif