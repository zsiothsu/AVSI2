/*
 * CGDerivator.cpp 2025
 *
 * utils for code generator
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

#include <cstdlib>
#include <set>
#include <cstdint>
#include <unistd.h>
#include <sys/wait.h>

#include "../inc/AST.h"
#include "../inc/SymbolTable.h"
#include "../inc/FileName.h"
#include <filesystem>
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/AggressiveInstCombine/AggressiveInstCombine.h"
#if (LLVM_VERSION_MAJOR >= 14)
#include "llvm/MC/TargetRegistry.h"
#else
#include "llvm/Support/TargetRegistry.h"
#endif

extern bool opt_ir;
extern bool opt_module;
extern bool opt_reliance;
extern bool opt_verbose;
extern bool opt_warning;
extern bool opt_optimize;
extern bool opt_pic;

namespace AVSI {
    using namespace std;

    /*******************************************************
     *                      llvm base                      *
     *******************************************************/
    extern string module_name;
    extern string module_name_nopath;
    extern vector<string> module_path;

    extern llvm::LLVMContext *the_context;
    extern llvm::Module *the_module;
    extern llvm::IRBuilder<> *builder;
    extern llvm::legacy::FunctionPassManager *the_function_fpm;
    extern llvm::legacy::PassManager *the_module_fpm;

    extern llvm::TargetMachine *TheTargetMachine;

    extern llvm::BasicBlock *global_insert_point;

    /*******************************************************
     *               protos & definition                   *
     *******************************************************/
    extern llvm::Type *F64_TY;
    extern llvm::Type *F32_TY;
    extern llvm::Type *I128_TY;
    extern llvm::Type *I64_TY;
    extern llvm::Type *I32_TY;
    extern llvm::Type *I16_TY;
    extern llvm::Type *I8_TY;
    extern llvm::Type *I1_TY;
    extern llvm::Type *VOID_TY;
    extern llvm::Type *ISIZE_TY;

    extern shared_ptr<AST> ASTEmpty;
    extern shared_ptr<AST> ASTEmptyNotEnd;

    extern SymbolTable *symbol_table;

    extern map<string, StructDef *> struct_types;
    extern map<string, GenericDef *> generic_function;
    extern map<std::string, llvm::FunctionType *> function_protos;
    extern set<llvm::Type *> simple_types;
    extern map<llvm::Type *, uint8_t> simple_types_map;

    extern map<llvm::Type *, string> type_name;
    extern map<llvm::Type *, uint32_t> type_size;

    extern map<string, string> module_name_alias;

    extern map<TokenType, llvm::Type *> token_to_simple_types;

    /*******************************************************
     *                     function                        *
     *******************************************************/
    shared_ptr<AST> derivator(shared_ptr<AST> ast, string name) {
        if (ast->__AST_name == __BINOP_NAME) {
            shared_ptr<BinOp> binop = static_pointer_cast<BinOp>(ast);
            auto left = derivator(binop->left, name);
            auto right = derivator(binop->right, name);

            if (left->__AST_name != __NONEAST_NAME && right->__AST_name != __NONEAST_NAME) {
                auto type = ast->getToken().getType();
                if (type == PLUS) {
                    // (f(x) + g(x))' = f'(x) + g'(x)
                    return make_shared<BinOp>(
                        BinOp(left, Token(PLUS, "+", ast->getToken().line, ast->getToken().column), right)
                    ); 
                } else if (type == MINUS) {
                    // (f(x) - g(x))' = f'(x) - g'(x)
                    return make_shared<BinOp>(
                        BinOp(left, Token(MINUS, "-", ast->getToken().line, ast->getToken().column), right)
                    ); 
                } else if (type == STAR) {
                    // (f(x) * g(x))' = f'(x) * g(x) + f(x) * g'(x)
                    auto tmp1 = make_shared<BinOp>(
                        BinOp(left, Token(STAR, "*", ast->getToken().line, ast->getToken().column), binop->right)
                    );
                    auto tmp2 = make_shared<BinOp>(
                        BinOp(binop->left, Token(STAR, "*", ast->getToken().line, ast->getToken().column), right)
                    );
                    return make_shared<BinOp>(
                        BinOp(tmp1, Token(PLUS, "+", ast->getToken().line, ast->getToken().column), tmp2)
                    );
                } else if (type == SLASH) {
                    // (f(x) / g(x))' = (f'(x) * g(x) - f(x) * g'(x)) / g(x)^2
                    auto tmp1 = make_shared<BinOp>(
                        BinOp(left, Token(STAR, "*", ast->getToken().line, ast->getToken().column), binop->right)
                    );
                    auto tmp2 = make_shared<BinOp>(
                        BinOp(binop->left, Token(STAR, "*", ast->getToken().line, ast->getToken().column), right)
                    );
                    auto tmp3 = make_shared<BinOp>(
                        BinOp(tmp1, Token(MINUS, "-", ast->getToken().line, ast->getToken().column), tmp2)
                    );
                    return make_shared<BinOp>(
                        BinOp(tmp3, Token(SLASH, "/", ast->getToken().line, ast->getToken().column), make_shared<BinOp>(
                            BinOp(binop->right, Token(STAR, "*", ast->getToken().line, ast->getToken().column), binop->right)
                        ))
                    );
                } else if (
                    type == EQ ||
                    type == NE ||
                    type == GT ||
                    type == GE ||
                    type == LT ||
                    type == LE
                ) {
                    return make_shared<NoneAST>(NoneAST());
                }
            } else if (left->__AST_name != __NONEAST_NAME) {
                auto type = ast->getToken().getType();
                if (type == PLUS) {
                    return left;
                } else if (type == MINUS) {
                    return left;
                } else if (type == STAR) {
                    return make_shared<BinOp>(
                        BinOp(left, Token(STAR, "*", ast->getToken().line, ast->getToken().column), binop->right)
                    );
                } else if (type == SLASH) {
                    return make_shared<BinOp>(
                        BinOp(left, Token(SLASH, "/", ast->getToken().line, ast->getToken().column), binop->right)
                    );
                } else if (
                    type == EQ ||
                    type == NE ||
                    type == GT ||
                    type == GE ||
                    type == LT ||
                    type == LE
                ) {
                    return make_shared<NoneAST>(NoneAST());
                }
            } else if (right->__AST_name != __NONEAST_NAME) {
                auto type = ast->getToken().getType();
                if (type == PLUS) {
                    return right;
                } else if (type == MINUS) {
                    return make_shared<UnaryOp>(
                        UnaryOp(Token(MINUS, "-", ast->getToken().line, ast->getToken().column), right)
                    );
                } else if (type == STAR) {
                    return make_shared<BinOp>(
                        BinOp(binop->left, Token(STAR, "*", ast->getToken().line, ast->getToken().column), right)
                    );
                } else if (type == SLASH) {
                    auto tmp1 = make_shared<BinOp>(
                        BinOp(binop->left, Token(STAR, "*", ast->getToken().line, ast->getToken().column), right)
                    );
                    auto tmp2 = make_shared<BinOp>(
                        BinOp(binop->right, Token(STAR, "*", ast->getToken().line, ast->getToken().column), binop->right)
                    );
                    auto tmp3 = make_shared<BinOp>(
                        BinOp(tmp1, Token(SLASH, "/", ast->getToken().line, ast->getToken().column), tmp2)
                    );
                    return make_shared<UnaryOp>(
                        UnaryOp(Token(MINUS, "-", ast->getToken().line, ast->getToken().column), tmp3)
                    );
                } else if (
                    type == EQ ||
                    type == NE ||
                    type == GT ||
                    type == GE ||
                    type == LT ||
                    type == LE
                ) {
                    return make_shared<NoneAST>(NoneAST());
                }
            } else {
                return make_shared<NoneAST>(NoneAST());
            }
        } else if (ast->__AST_name == __UNARYTOP_NAME) {
            shared_ptr<UnaryOp> unaryop = static_pointer_cast<UnaryOp>(ast);
            auto expr = derivator(unaryop->right, name);
            
            if (expr->__AST_name != __NONEAST_NAME) {
                auto type = ast->getToken().getType();
                if (type == PLUS) {
                    return expr;
                } else if (type == MINUS) {
                    return make_shared<UnaryOp>(
                        UnaryOp(Token(MINUS, "-", ast->getToken().line, ast->getToken().column), expr)
                    );
                }
            } else {
                return make_shared<NoneAST>(NoneAST());
            }
        } else if (ast->__AST_name == __FUNCTIONCALL_NAME) {
            return derivator_for_function_call(ast, name);
        } else if (ast->__AST_name == __NUM_NAME) {
            return make_shared<NoneAST>(NoneAST());
        } else if (ast->__AST_name == __VARIABLE_NAME) {
            auto var = static_pointer_cast<Variable>(ast);

            llvm::Value *v = symbol_table->find(var->id);

            if (!v) {
                Warning(
                    "variable " + var->id + " is not defined in current function. "
                    "please check the variable name, global variable is not supported in derivation."
                    "the gradiant of this variable will be set to 0",
                    var->getToken().line,
                    var->getToken().column
                );
                return make_shared<NoneAST>(NoneAST());
            }

            if (var->id == name) {
                if (!var->offset.empty() && var->offset.rbegin()->first != Variable::offsetType::FUNCTION) {
                    throw ExceptionFactory<LogicException>(
                        "derivation of variable with offset is not supported, "
                        "unsupported variable is in line " + to_string(var->getToken().line) + " column " + to_string(var->getToken().column),
                        var->getToken().line,
                        var->getToken().column
                    );
                } else if (!var->offset.empty() && var->offset.rbegin()->first == Variable::offsetType::FUNCTION) {
                    // TODO
                } else {
                    return make_shared<Num>(Num(Token(INTEGER, 1, ast->getToken().line, ast->getToken().column)));
                }
            } else {
                auto variable_assign_ast = symbol_table->getAssignedAST(var->id);
                return derivator(variable_assign_ast, name);
            }
        } else {
            return make_shared<NoneAST>(NoneAST());
        }
    }

    /**
     * @description:    derivator for function call
     * @param:          ast: AST node
     *                  name: name of independent variable
     * @return:         derivated AST
     */
    shared_ptr<AST> derivator_for_function_call(shared_ptr<AST> ast, string name) {
        shared_ptr<FunctionCall> func = static_pointer_cast<FunctionCall>(ast);
        // store f'1(a, b, ...) a', f'2(a, b, ...) b', ...
        vector<shared_ptr<AST>> tmp_ast;    

        for (int i = 0; i < func->paramList.size(); i++) {
            auto tmp = derivator(func->paramList[i], name);
            if (tmp->__AST_name != __NONEAST_NAME) {
                tmp_ast.push_back(
                    make_shared<BinOp>(
                        BinOp(
                            tmp,
                            Token(STAR, "*", func->paramList[i]->getToken().line, func->paramList[i]->getToken().column),
                            derivator_for_special_function(func, i)
                        )
                    )
                );
            }   
        }

        if (tmp_ast.empty()) {
            return make_shared<NoneAST>(NoneAST());
        } else if (tmp_ast.size() == 1) {
            return tmp_ast[0];
        } else {
            auto tmp = tmp_ast[0];
            for (int i = 1; i < tmp_ast.size(); i++) {
                tmp = make_shared<BinOp>(
                    BinOp(
                        tmp,
                        Token(PLUS, "+", func->getToken().line, func->getToken().column),
                        tmp_ast[i]
                    )
                );
            }
            return tmp;
        }
    }

    /**
     * @description:    derivator for special function
     * @param:          ast: AST node
     *                  index: index of parameter
     * @return:         derivated AST for function
     * @note:           You might wonder why I have so much free time to list all these
     *                  functions. But actually, these were all generated by AI. This is
     *                  the first time I've been truly amazed by AI.
     */
    shared_ptr<AST> derivator_for_special_function(shared_ptr<AST> ast, int index) {
        shared_ptr<FunctionCall> func = static_pointer_cast<FunctionCall>(ast);

        if (func->id == "sin") {
            // d/dx sin(x) = cos(x)
            return make_shared<FunctionCall>(
                FunctionCall(
                    "cos",
                    func->paramList,
                    func->getToken()
                )
            );
        } else if (func->id == "cos") {
            // d/dx cos(x) = -sin(x)
            return make_shared<UnaryOp>(
                UnaryOp(
                    Token(MINUS, "-", func->getToken().line, func->getToken().column),
                    make_shared<FunctionCall>(
                        FunctionCall(
                            "sin",
                            func->paramList,
                            func->getToken()
                        )
                    )
                )
            );
        } else if (func->id == "tan") {
            // d/dx tan(x) = sec^2(x)
            return make_shared<BinOp>(
                BinOp(
                    make_shared<FunctionCall>(
                        FunctionCall(
                            "sec",
                            func->paramList,
                            func->getToken()
                        )
                    ),
                    Token(STAR, "*", func->getToken().line, func->getToken().column),
                    make_shared<FunctionCall>(
                        FunctionCall(
                            "sec",
                            func->paramList,
                            func->getToken()
                        )
                    )
                )
            );
        } else if (func->id == "cot") {
            // d/dx cot(x) = -csc^2(x)
            return make_shared<UnaryOp>(
                UnaryOp(
                    Token(MINUS, "-", func->getToken().line, func->getToken().column),
                    make_shared<BinOp>(
                        BinOp(
                            make_shared<FunctionCall>(
                                FunctionCall(
                                    "csc",
                                    func->paramList,
                                    func->getToken()
                                )
                            ),
                            Token(STAR, "*", func->getToken().line, func->getToken().column),
                            make_shared<FunctionCall>(
                                FunctionCall(
                                    "csc",
                                    func->paramList,
                                    func->getToken()
                                )
                            )
                        )
                    )
                )
            );
        } else if (func->id == "sec") {
            // d/dx sec(x) = sec(x) * tan(x)
            return make_shared<BinOp>(
                BinOp(
                    make_shared<FunctionCall>(
                        FunctionCall(
                            "sec",
                            func->paramList,
                            func->getToken()
                        )
                    ),
                    Token(STAR, "*", func->getToken().line, func->getToken().column),
                    make_shared<FunctionCall>(
                        FunctionCall(
                            "tan",
                            func->paramList,
                            func->getToken()
                        )
                    )
                )
            );
        } else if (func->id == "csc") {
            // d/dx csc(x) = -csc(x) * cot(x)
            return make_shared<UnaryOp>(
                UnaryOp(
                    Token(MINUS, "-", func->getToken().line, func->getToken().column),
                    make_shared<BinOp>(
                        BinOp(
                            make_shared<FunctionCall>(
                                FunctionCall(
                                    "csc",
                                    func->paramList,
                                    func->getToken()
                                )
                            ),
                            Token(STAR, "*", func->getToken().line, func->getToken().column),
                            make_shared<FunctionCall>(
                                FunctionCall(
                                    "cot",
                                    func->paramList,
                                    func->getToken()
                                )
                            )
                        )
                    )
                )
            );
        } else if (func->id == "log") {
            // d/dx log(x) = 1 / x
            return make_shared<BinOp>(
                BinOp(
                    make_shared<Num>(
                        Num(Token(INTEGER, 1, func->getToken().line, func->getToken().column))
                    ),
                    Token(SLASH, "/", func->getToken().line, func->getToken().column),
                    func->paramList[0]
                )
            );
        } else if (func->id == "exp") {
            // d/dx exp(x) = exp(x)
            return func;
        } else if (func->id == "sqrt") {
            // d/dx sqrt(x) = 1 / (2 * sqrt(x))
            return make_shared<BinOp>(
                BinOp(
                    make_shared<Num>(
                        Num(Token(INTEGER, 1, func->getToken().line, func->getToken().column))
                    ),
                    Token(SLASH, "/", func->getToken().line, func->getToken().column),
                    make_shared<BinOp>(
                        BinOp(
                            make_shared<Num>(
                                Num(Token(INTEGER, 2, func->getToken().line, func->getToken().column))
                            ),
                            Token(STAR, "*", func->getToken().line, func->getToken().column),
                            make_shared<FunctionCall>(
                                FunctionCall(
                                    "sqrt",
                                    func->paramList,
                                    func->getToken()
                                )
                            )
                        )
                    )
                )
            );
        } else if (func->id == "arcsin") {
            // d/dx arcsin(x) = 1 / sqrt(1 - x^2)
            return make_shared<BinOp>(
                BinOp(
                    make_shared<Num>(
                        Num(Token(INTEGER, 1, func->getToken().line, func->getToken().column))
                    ),
                    Token(SLASH, "/", func->getToken().line, func->getToken().column),
                    make_shared<FunctionCall>(
                        FunctionCall(
                            "sqrt",
                            {
                                make_shared<BinOp>(
                                    BinOp(
                                        make_shared<Num>(
                                            Num(Token(INTEGER, 1, func->getToken().line, func->getToken().column))
                                        ),
                                        Token(MINUS, "-", func->getToken().line, func->getToken().column),
                                        make_shared<BinOp>(
                                            BinOp(
                                                func->paramList[0],
                                                Token(STAR, "*", func->getToken().line, func->getToken().column),
                                                func->paramList[0]
                                            )
                                        )
                                    )
                                )
                            },
                            func->getToken()
                        )
                    )
                )
            );
        } else if (func->id == "arccos") {
            // d/dx arccos(x) = -1 / sqrt(1 - x^2)
            return make_shared<UnaryOp>(
                UnaryOp(
                    Token(MINUS, "-", func->getToken().line, func->getToken().column),
                    make_shared<BinOp>(
                        BinOp(
                            make_shared<Num>(
                                Num(Token(INTEGER, 1, func->getToken().line, func->getToken().column))
                            ),
                            Token(SLASH, "/", func->getToken().line, func->getToken().column),
                            make_shared<FunctionCall>(
                                FunctionCall(
                                    "sqrt",
                                    {
                                        make_shared<BinOp>(
                                            BinOp(
                                                make_shared<Num>(
                                                    Num(Token(INTEGER, 1, func->getToken().line, func->getToken().column))
                                                ),
                                                Token(MINUS, "-", func->getToken().line, func->getToken().column),
                                                make_shared<BinOp>(
                                                    BinOp(
                                                        func->paramList[0],
                                                        Token(STAR, "*", func->getToken().line, func->getToken().column),
                                                        func->paramList[0]
                                                    )
                                                )
                                            )
                                        )
                                    },
                                    func->getToken()
                                )
                            )
                        )
                    )
                )
            );
        } else if (func->id == "arctan") {
            // d/dx arctan(x) = 1 / (1 + x^2)
            return make_shared<BinOp>(
                BinOp(
                    make_shared<Num>(
                        Num(Token(INTEGER, 1, func->getToken().line, func->getToken().column))
                    ),
                    Token(SLASH, "/", func->getToken().line, func->getToken().column),
                    make_shared<BinOp>(
                        BinOp(
                            make_shared<Num>(
                                Num(Token(INTEGER, 1, func->getToken().line, func->getToken().column))
                            ),
                            Token(PLUS, "+", func->getToken().line, func->getToken().column),
                            make_shared<BinOp>(
                                BinOp(
                                    func->paramList[0],
                                    Token(STAR, "*", func->getToken().line, func->getToken().column),
                                    func->paramList[0]
                                )
                            )
                        )
                    )
                )
            );
        } else if (func->id == "arccot") {
            // d/dx arccot(x) = -1 / (1 + x^2)
            return make_shared<UnaryOp>(
                UnaryOp(
                    Token(MINUS, "-", func->getToken().line, func->getToken().column),
                    make_shared<BinOp>(
                        BinOp(
                            make_shared<Num>(
                                Num(Token(INTEGER, 1, func->getToken().line, func->getToken().column))
                            ),
                            Token(SLASH, "/", func->getToken().line, func->getToken().column),
                            make_shared<BinOp>(
                                BinOp(
                                    make_shared<Num>(
                                        Num(Token(INTEGER, 1, func->getToken().line, func->getToken().column))
                                    ),
                                    Token(PLUS, "+", func->getToken().line, func->getToken().column),
                                    make_shared<BinOp>(
                                        BinOp(
                                            func->paramList[0],
                                            Token(STAR, "*", func->getToken().line, func->getToken().column),
                                            func->paramList[0]
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            );
        } else if (func->id == "min") {
            if (index == 0) {
                // min(a,b) == a
                return make_shared<BinOp>(
                    BinOp(
                        func->paramList[0],
                        Token(EQ, "==", func->getToken().line, func->getToken().column),
                        make_shared<FunctionCall>(
                            FunctionCall(
                                "min",
                                func->paramList,
                                func->getToken()
                            )
                        )
                    )
                );
            } else if (index == 1) {
                // min(a,b) != a
                // for optimization better
                return make_shared<BinOp>(
                    BinOp(
                        func->paramList[0],
                        Token(NE, "!=", func->getToken().line, func->getToken().column),
                        make_shared<FunctionCall>(
                            FunctionCall(
                                "min",
                                func->paramList,
                                func->getToken()
                            )
                        )
                    )
                );
            } else {
                throw ExceptionFactory<LogicException>(
                    "function min must have 2 arguments",
                    func->getToken().line,
                    func->getToken().column
                );
            }
        } else if (func->id == "max") {
            if (index == 0) {
                // min(a,b) == a
                return make_shared<BinOp>(
                    BinOp(
                        func->paramList[0],
                        Token(EQ, "==", func->getToken().line, func->getToken().column),
                        make_shared<FunctionCall>(
                            FunctionCall(
                                "max",
                                func->paramList,
                                func->getToken()
                            )
                        )
                    )
                );
            } else if (index == 1) {
                // min(a,b) != a
                // for optimization better
                return make_shared<BinOp>(
                    BinOp(
                        func->paramList[0],
                        Token(NE, "!=", func->getToken().line, func->getToken().column),
                        make_shared<FunctionCall>(
                            FunctionCall(
                                "max",
                                func->paramList,
                                func->getToken()
                            )
                        )
                    )
                );
            } else {
                throw ExceptionFactory<LogicException>(
                    "function max must have 2 arguments",
                    func->getToken().line,
                    func->getToken().column
                );
            }
        } else {
            return make_shared<FunctionCall>(
                FunctionCall(
                    func->id + "_diff" + to_string(index),
                    func->paramList,
                    func->getToken()
                )
            );
        }
    }

    shared_ptr<AST> derivator_for_member_function(shared_ptr<AST> ast, int index) {
        shared_ptr<Variable> var = static_pointer_cast<Variable>(ast);
        
        // TODO

        for (auto i = var->offset.rbegin(); i != var->offset.rend(); i++) {
            if (i->first == Variable::offsetType::FUNCTION) {
                auto func = static_pointer_cast<FunctionCall>(i->second);
                func->id = func->id + "_diff" + to_string(index);
                break;
            }
        }

        return var;
    }
}
