/*
 * CG.cpp 2025
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

#include <set>
#include <cstdint>
#include <unistd.h>

#include "../inc/AST.h"
#include "../inc/SymbolTable.h"
#include <filesystem>
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/LegacyPassManager.h"

#if (LLVM_VERSION_MAJOR >= 14)

#include "llvm/MC/TargetRegistry.h"

#else
#include "llvm/Support/TargetRegistry.h"
#endif

#define UNUSED(x) ((void)(x))

extern bool opt_reliance;
extern bool opt_verbose;
extern bool opt_warning;

namespace AVSI {
    /*******************************************************
     *                      llvm base                      *
     *******************************************************/
    string module_name;
    string module_name_nopath;
    vector<string> module_path = vector<string>();
    vector<string> module_path_with_module_name = vector<string>();

    llvm::LLVMContext *the_context;
    llvm::Module *the_module;
    llvm::IRBuilder<> *builder;
    llvm::legacy::FunctionPassManager *the_function_fpm;
    llvm::legacy::PassManager *the_module_fpm;
    llvm::TargetMachine *TheTargetMachine;

    llvm::BasicBlock *global_insert_point;

    /*******************************************************
     *               protos & definition                   *
     *******************************************************/
    llvm::Type *F64_TY;
    llvm::Type *F32_TY;
    llvm::Type *I128_TY;
    llvm::Type *I64_TY;
    llvm::Type *I32_TY;
    llvm::Type *I16_TY;
    llvm::Type *I8_TY;
    llvm::Type *I1_TY;
    llvm::Type *VOID_TY;
    llvm::Type *ISIZE_TY;

    shared_ptr<AST> ASTEmpty = make_shared<NoneAST>(NoneAST());
    shared_ptr<AST> ASTEmptyNotEnd = make_shared<NoneAST>(NoneAST());

    // global stack based symbol table
    SymbolTable *symbol_table;
    // store structure definitions. including members' name and type.
    map<string, StructDef *> struct_types;
    // store generic definitions. including function name and type.
    map<string, GenericDef *> generic_function;
    // store function defined in the context.
    map<std::string, llvm::FunctionType *> function_protos;

    // a read-only set including simple types (REAL, CHAR, etc.)
    set<llvm::Type *> simple_types;
    // map simple types to an integer
    map<llvm::Type *, uint8_t> simple_types_map;

    // type name to be displayed in debug message
    map<llvm::Type *, string> type_name;
    // store size of specific type. for an array pointer, it will
    // be the size of the space where the pointer point to
    map<llvm::Type *, uint32_t> type_size;

    // map a relative or renamed module path to absolute
    map<string, string> module_name_alias;

    // map token type to llvm simple type
    map<TokenType, llvm::Type *> token_to_simple_types;

    // map a constant string to llvm const
    map<string, llvm::Constant *> const_string_pool;

    extern void debug_type(llvm::Value *v);

    extern void debug_type(llvm::Type *v);
    /*******************************************************
     *                    IR generator                     *
     *******************************************************/
    llvm::Value *AST::codeGen() {
        return llvm::ConstantFP::getNaN(F64_TY);
    }

    void AST::dump(int depth) {

    }

    llvm::Value *Assign::codeGen() {
        llvm::Value *r_value = this->right->codeGen();

        string l_base_name = (static_pointer_cast<Variable>(this->left))->id;
        bool is_const = false;
        if (r_value->getType()->isArrayTy() || r_value->getType()->isVectorTy()) {
            auto l_alloca_addr = getAlloca(static_pointer_cast<Variable>(this->left).get(), &is_const);

            if (is_const) {
                throw ExceptionFactory<LogicException>(
                    "cannot assign to a constant variable",
                    this->getToken().line, this->getToken().column
                );
            }

            auto st = store(this, l_alloca_addr, r_value, true, l_base_name);
            symbol_table->insertAssingedAst(l_base_name, this->right, !st);
        } else {
            auto l_alloca_addr = getAlloca(static_pointer_cast<Variable>(this->left).get(), &is_const);

            if (is_const) {
                throw ExceptionFactory<LogicException>(
                    "cannot assign to a constant variable",
                    this->getToken().line, this->getToken().column
                );
            }

            if (!l_alloca_addr && !((static_pointer_cast<Variable>(this->left))->offset.empty())) {
                throw ExceptionFactory<MissingException>(
                        "missing variable '" + l_base_name + "'",
                        this->getToken().line, this->getToken().column
                );
            }

            auto st = store(this, l_alloca_addr, r_value, true, l_base_name);
            symbol_table->insertAssingedAst(l_base_name, this->right, !st);
        }
        return llvm::ConstantFP::getNaN(F64_TY);
    }

    llvm::Value *BinOp::codeGen() {
        auto token_type = this->getToken().getType();

        if (token_type != AND && token_type != OR) {
            // normal binop
            llvm::Value *lv = this->left->codeGen();
            llvm::Value *rv = this->right->codeGen();

            if (!lv || !rv) {
                return nullptr;
            }

            auto lv_const = llvm::dyn_cast<llvm::Constant>(lv);
            auto rv_const = llvm::dyn_cast<llvm::Constant>(rv);

            if (lv_const && lv_const->isNaN()) {
                throw ExceptionFactory<LogicException>(
                        "left operand must have a value",
                        this->left->getToken().line, this->left->getToken().column
                );
            }
            if (rv_const && rv_const->isNaN()) {
                throw ExceptionFactory<LogicException>(
                        "right operand must have a value",
                        this->right->getToken().line, this->right->getToken().column
                );
            }

            llvm::Type *l_type = lv->getType();
            llvm::Type *r_type = rv->getType();

            if (
                    simple_types.find(l_type) == simple_types.end() ||
                    simple_types.find(r_type) == simple_types.end()) {
                if (l_type->isVectorTy() && r_type->isVectorTy()) {
                    if (l_type != r_type) {
                        throw ExceptionFactory<MathException>(
                            "unsupported type '" + type_name[l_type] + "' and '" + type_name[r_type]
                            + "to vector expression",
                            this->token.line, this->token.column
                        );
                    }

                    switch (this->op.getType()) {
                    case PLUS:
                        return builder->CreateAdd(lv, rv, "vadd");
                    case MINUS:
                        return builder->CreateSub(lv, rv, "vsub");
                    case STAR:
                        return builder->CreateMul(lv, rv, "vmul");
                    case SLASH:
                        return builder->CreateFDiv(lv, rv, "vdiv");
                    default:
                        throw ExceptionFactory<MathException>(
                            "unsupported operation " + token_name[this->token.getType()] + "to vector expression",
                            this->token.line, this->token.column
                        );
                    // TODO more operation
                    }
                } else {
                    return nullptr;
                }
            }

            // get the "maximum" type
            int result_type_bitmap = simple_types_map[l_type] | simple_types_map[r_type];
            bool float_point = false;
            if (result_type_bitmap >= simple_types_map[F32_TY])
                float_point = true;

            llvm::Type *cast_to = l_type;
            cast_to = (result_type_bitmap >= simple_types_map[I1_TY]) ? I1_TY : cast_to;
            cast_to = (result_type_bitmap >= simple_types_map[I8_TY]) ? I8_TY : cast_to;
            cast_to = (result_type_bitmap >= simple_types_map[I16_TY]) ? I16_TY : cast_to;
            cast_to = (result_type_bitmap >= simple_types_map[I32_TY]) ? I32_TY : cast_to;
            cast_to = (result_type_bitmap >= simple_types_map[I64_TY]) ? I64_TY : cast_to;
            cast_to = (result_type_bitmap >= simple_types_map[I128_TY]) ? I128_TY : cast_to;
            cast_to = (result_type_bitmap >= simple_types_map[F32_TY]) ? F32_TY : cast_to;
            cast_to = (result_type_bitmap >= simple_types_map[F64_TY]) ? F64_TY : cast_to;

            llvm::Value *cmp_value_boolean;

            llvm::Value *lv_real = nullptr;
            llvm::Value *rv_real = nullptr;

            if (float_point) {
                lv_real = l_type->isFloatingPointTy()
                          ? builder->CreateFPExt(lv, cast_to, "conv.fp.ext")
                          : (
                            l_type == I1_TY
                            ? builder->CreateUIToFP(lv, cast_to, "conv.zi.fp")
                            : builder->CreateSIToFP(lv, cast_to, "conv.si.fp")
                          );
                rv_real = r_type->isFloatingPointTy()
                          ? builder->CreateFPExt(rv, cast_to, "conv.fp.ext")
                          : (
                            r_type == I1_TY
                            ? builder->CreateUIToFP(rv, cast_to, "conv.zi.fp")
                            : builder->CreateSIToFP(rv, cast_to, "conv.si.fp")
                          );
            } else {
                if (lv->getType() == I1_TY) {
                    lv = builder->CreateZExt(lv, cast_to, "conv.zi.ext");
                } else {
                    lv = builder->CreateSExt(lv, cast_to, "conv.si.ext");
                }
                if (rv->getType() == I1_TY) {
                    rv = builder->CreateZExt(rv, cast_to, "conv.zi.ext");
                } else {
                    rv = builder->CreateSExt(rv, cast_to, "conv.si.ext");
                }
            }

            switch (this->op.getType()) {
                case PLUS:
                    if (float_point)
                        return builder->CreateFAdd(lv_real, rv_real, "add");
                    else
                        return builder->CreateAdd(lv, rv, "add");
                case MINUS:
                    if (float_point)
                        return builder->CreateFSub(lv_real, rv_real, "sub");
                    else
                        return builder->CreateSub(lv, rv, "sub");
                case STAR:
                    if (float_point)
                        return builder->CreateFMul(lv_real, rv_real, "mul");
                    else
                        return builder->CreateMul(lv, rv, "mul");
                case SLASH:
                    if (llvm::dyn_cast<llvm::Constant>(rv) && llvm::dyn_cast<llvm::Constant>(rv)->isZeroValue()) {
                        throw ExceptionFactory<MathException>(
                                "divisor cannot be 0",
                                this->token.line, this->token.column);
                    }
                    if (float_point)
                        return builder->CreateFDiv(lv_real, rv_real, "div");
                    else
                        return builder->CreateSDiv(lv, rv, "div");
                case EQ:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOEQ(lv_real, rv_real, "eq");
                    } else {
                        cmp_value_boolean = builder->CreateICmpEQ(lv, rv, "eq");
                    }
                    return cmp_value_boolean;
                case NE:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpONE(lv_real, rv_real, "ne");
                    } else {
                        cmp_value_boolean = builder->CreateICmpNE(lv, rv, "ne");
                    }
                    return cmp_value_boolean;
                case GT:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOGT(lv_real, rv_real, "gt");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSGT(lv, rv, "gt");
                    }
                    return cmp_value_boolean;
                case LT:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOLT(lv_real, rv_real, "lt");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSLT(lv, rv, "lt");
                    }
                    return cmp_value_boolean;
                case GE:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOGE(lv_real, rv_real, "gt");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSGE(lv, rv, "gt");
                    }
                    return cmp_value_boolean;
                case LE:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOLE(lv_real, rv_real, "le");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSLE(lv, rv, "le");
                    }
                    return cmp_value_boolean;
                case BITOR:
                    if (float_point)
                        throw ExceptionFactory<MathException>(
                                "unsupported type '" + type_name[l_type] + "' and '" + type_name[r_type]
                                + "to bitor expression",
                                this->token.line, this->token.column);
                    else
                        return builder->CreateOr(lv, rv, "bitor");
                case BITAND:
                    if (float_point)
                        throw ExceptionFactory<MathException>(
                                "unsupported type '" + type_name[l_type] + "' and '" + type_name[r_type]
                                + "to bitand expression",
                                this->token.line, this->token.column);
                    else
                        return builder->CreateAnd(lv, rv, "bitand");
                case SHL:
                    if (float_point)
                        throw ExceptionFactory<MathException>(
                                "unsupported type '" + type_name[l_type] + "' and '" + type_name[r_type]
                                + "to shift expression",
                                this->token.line, this->token.column);
                    else
                        return builder->CreateShl(lv, rv, "shl");
                case SHR:
                    if (float_point)
                        throw ExceptionFactory<MathException>(
                                "unsupported type '" + type_name[l_type] + "' and '" + type_name[r_type]
                                + "to shift expression",
                                this->token.line, this->token.column);
                    else
                        return builder->CreateAShr(lv, rv, "shr");
                case SHRU:
                    if (float_point)
                        throw ExceptionFactory<MathException>(
                                "unsupported type '" + type_name[l_type] + "' and '" + type_name[r_type]
                                + "to shift expression",
                                this->token.line, this->token.column);
                    else
                        return builder->CreateLShr(lv, rv, "shr");
                case REM:
                    if (float_point)
                        throw ExceptionFactory<MathException>(
                                "unsupported type '" + type_name[l_type] + "' and '" + type_name[r_type]
                                + "to remainder expression",
                                this->token.line, this->token.column);
                    else
                        return builder->CreateSRem(lv, rv, "rem");
                default:
                    return nullptr;
            }
        } else {
            /**
             * short-circuit operator
             * it will generate if-like IR codes
             */

            if (token_type == AND) {
                auto l = this->left->codeGen();
                if (!l) {
                    return nullptr;
                }
                auto the_function = builder->GetInsertBlock()->getParent();
                auto Positive = llvm::BasicBlock::Create(*the_context, "land.lhs.true.rhs.head", the_function);
                auto Negative = llvm::BasicBlock::Create(*the_context, "land.end");

                if (l->getType() != I1_TY) {
                    if (l->getType()->isIntegerTy()) {
                        l = builder->CreateICmpNE(l, llvm::ConstantInt::get(l->getType(), 0), "toBool");
                    } else {
                        l = builder->CreateFCmpUNE(l, llvm::ConstantFP::get(l->getType(), 0.0), "toBool");
                    }
                }
                auto l_phi_path = builder->GetInsertBlock();
                builder->CreateCondBr(l, Positive, Negative);

                builder->SetInsertPoint(Positive);
                auto r = this->right->codeGen();
                if (r->getType() != I1_TY) {
                    if (r->getType()->isIntegerTy()) {
                        r = builder->CreateICmpNE(r, llvm::ConstantInt::get(r->getType(), 0), "toBool");
                    } else {
                        r = builder->CreateFCmpUNE(r, llvm::ConstantFP::get(r->getType(), 0.0), "toBool");
                    }
                }
                auto r_phi_path = builder->GetInsertBlock();
                builder->CreateBr(Negative);

                the_function->getBasicBlockList().push_back(Negative);
                builder->SetInsertPoint(Negative);

                llvm::PHINode *PN = builder->CreatePHI(I1_TY,
                                                       2);
                PN->addIncoming(l, l_phi_path);
                PN->addIncoming(r, r_phi_path);

                return PN;
            } else {
                auto l = this->left->codeGen();
                if (!l) {
                    return nullptr;
                }

                auto the_function = builder->GetInsertBlock()->getParent();
                auto Negative = llvm::BasicBlock::Create(*the_context, "lor.lhs.false.rhs.head", the_function);
                auto Positive = llvm::BasicBlock::Create(*the_context, "lor.end");

                if (l->getType() != I1_TY) {
                    if (l->getType()->isIntegerTy()) {
                        l = builder->CreateICmpNE(l, llvm::ConstantInt::get(l->getType(), 0), "toBool");
                    } else {
                        l = builder->CreateFCmpUNE(l, llvm::ConstantFP::get(l->getType(), 0.0), "toBool");
                    }
                }
                auto l_phi_path = builder->GetInsertBlock();
                builder->CreateCondBr(l, Positive, Negative);

                builder->SetInsertPoint(Negative);
                auto r = this->right->codeGen();
                if (r->getType() != I1_TY) {
                    if (r->getType()->isIntegerTy()) {
                        r = builder->CreateICmpNE(r, llvm::ConstantInt::get(r->getType(), 0), "toBool");
                    } else {
                        r = builder->CreateFCmpUNE(r, llvm::ConstantFP::get(r->getType(), 0.0), "toBool");
                    }
                }
                auto r_phi_path = builder->GetInsertBlock();
                builder->CreateBr(Positive);

                the_function->getBasicBlockList().push_back(Positive);
                builder->SetInsertPoint(Positive);

                llvm::PHINode *PN = builder->CreatePHI(I1_TY,
                                                       2);
                PN->addIncoming(l, l_phi_path);
                PN->addIncoming(r, r_phi_path);

                return PN;
            }
        }
    }

    llvm::Value *BlockExpr::codeGen() {
        return this->expr->codeGen();
    }

    llvm::Value *Boolean::codeGen() {
        if (this->value == true) {
            return llvm::ConstantInt::get(I1_TY, 1);
        } else {
            return llvm::ConstantInt::get(I1_TY, 0);
        }
    }

    llvm::Value *For::codeGen() {
        llvm::Function *the_function = builder->GetInsertBlock()->getParent();

        llvm::BasicBlock *headBB = llvm::BasicBlock::Create(*the_context, "loop.head", the_function);
        llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(*the_context, "loop.body");
        llvm::BasicBlock *adjBB = llvm::BasicBlock::Create(*the_context, "loop.adjust");
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "loop.end");

        // not region of headBB, but symbol table should be pushed before initial list
        symbol_table->push(headBB);
        symbol_table->setLoopExit(mergeBB);
        symbol_table->setLoopEntry(adjBB);

        llvm::Value *start = this->initList->codeGen();
        if (!((static_pointer_cast<Compound>(this->initList))->child.empty()) && (!start)) {
            return nullptr;
        }

        builder->CreateBr(headBB);

//        the_function->getBasicBlockList().push_back(headBB);
        builder->SetInsertPoint(headBB);

        if (!this->noCondition) {
            llvm::Value *cond = this->condition->codeGen();
            if (!cond) {
                return nullptr;
            }

            if (cond->getType() != I1_TY) {
                if (cond->getType()->isIntegerTy()) {
                    cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(cond->getType(), 0), "toBool");
                } else {
                    cond = builder->CreateFCmpUNE(cond, llvm::ConstantFP::get(cond->getType(), 0.0), "toBool");
                }
            }

            if (auto *LC = llvm::dyn_cast<llvm::ConstantInt>(cond)) {
                if (LC->isOne()) {
                    builder->CreateBr(loopBB);
                } else {
                    builder->CreateBr(mergeBB);
                }
            }

            builder->CreateCondBr(cond, loopBB, mergeBB);
            headBB = builder->GetInsertBlock();
        } else {
            builder->CreateBr(loopBB);
        }

        the_function->getBasicBlockList().push_back(loopBB);
        builder->SetInsertPoint(loopBB);

        // push headBB again, to avoid statements that referring the
        // variable defined at body in adjustment
        symbol_table->push(headBB);
        llvm::Value *body = this->compound->codeGen();
        if (!((static_pointer_cast<Compound>(this->compound))->child.empty()) && (!body)) {
            return nullptr;
        }
        symbol_table->pop();

        if (!builder->GetInsertBlock()->getTerminator()) {
            builder->CreateBr(adjBB);
        }

        the_function->getBasicBlockList().push_back(adjBB);
        builder->SetInsertPoint(adjBB);

        llvm::Value *adjust = this->adjustment->codeGen();
        if (!((static_pointer_cast<Compound>(this->adjustment))->child.empty()) && (!adjust)) {
            return nullptr;
        }
        symbol_table->pop();

        auto t = builder->GetInsertBlock()->getTerminator();
        if (!t) {
            builder->CreateBr(headBB);
        }

        the_function->getBasicBlockList().push_back(mergeBB);
        builder->SetInsertPoint(mergeBB);

        if (!adjBB->hasNPredecessorsOrMore(1)) {
            adjBB->eraseFromParent();
        }

        return llvm::ConstantFP::getNaN(F64_TY);
    }

    llvm::Value *FunctionDecl::codeGen() {
        llvm::BasicBlock *last_BB = nullptr;
        llvm::BasicBlock::iterator last_pt;
        if (builder->GetInsertBlock() && builder->GetInsertBlock()->getParent() != nullptr) {
            last_BB = builder->GetInsertBlock();
            last_pt = builder->GetInsertPoint();
        }

        bool is_a_member_funtion = !this->token.getModInfo().empty();

        string func_name =
                (this->id == ENTRY_NAME || !this->is_mangle)
                ? this->id
                : getFunctionNameMangling(module_path_with_module_name, this->id);

        // redirect to member function
        llvm::Type *struct_type = nullptr;
        if (is_a_member_funtion) {
            auto modinfo = this->token.getModInfo();
            auto struct_id = string(modinfo.back());
            vector<string> struct_path = modinfo;
            struct_path.pop_back();

            auto ty = find_struct(struct_path, struct_id);

            if (ty.first == struct_types.end()) {
                throw ExceptionFactory<MissingException>(
                        "missing type '" + id + "'",
                        this->token.line,
                        this->token.column
                );
            }

            struct_type = ty.first->second->Ty;
            auto struct_name = type_name[struct_type];
            auto index = struct_name.find('{');
            string part = struct_name.substr(0, index);
            struct_path = getpathUnresolvedToList(part);

            func_name = getFunctionNameMangling(struct_path, this->id);
        }

        // create function parameters' type
        std::vector<llvm::Type *> Tys;
        if (is_a_member_funtion) {
            Tys.push_back(struct_type->getPointerTo());
        }
        for (Variable *i: (static_pointer_cast<Param>(this->paramList))->paramList) {
            /* Passing array pointers between functions
             *
             * To be able to call external functions. Passing
             * array type by C ABI.
             *
             * e.g. [3 x double] will be represented as [3 x double]*
             * in function declaration
             */
            if (i->Ty.first->isArrayTy()) {
                Tys.push_back(i->Ty.first->getArrayElementType()->getPointerTo());
            } else if (i->Ty.first->isStructTy()) {
                Tys.push_back(i->Ty.first->getPointerTo());
            } else {
                Tys.push_back(i->Ty.first);
            }
        }

        llvm::Type *retTy = this->retTy.first->isArrayTy()
                            ? this->retTy.first->getArrayElementType()->getPointerTo()
                            : this->retTy.first;

        if (this->id == ENTRY_NAME || this->id == "_start") {
            retTy = llvm::Type::getInt32Ty(*the_context);
        }

        llvm::FunctionType *FT = llvm::FunctionType::get(
            retTy,
            Tys,
            static_pointer_cast<Param>(this->paramList)->is_va_arg
        );

        if (the_module->getFunction(this->id) == nullptr) {
            builder->ClearInsertionPoint();
            // check is re-definition
            auto funType = function_protos.find(this->id);
            if (funType != function_protos.end() &&
                ((funType->second != FT) || (the_module->getFunction(this->id) != nullptr))) {
                throw ExceptionFactory<LogicException>(
                        "function redefined there",
                        this->token.line, this->token.column);
            }

            if (generic_function.find(this->id) != generic_function.end()) {
                throw ExceptionFactory<LogicException>(
                        "function name " + this->id + " is conflict with generic function",
                        this->token.line, this->token.column);
            }

            auto link_type =
                    this->is_export
                    ? llvm::Function::ExternalLinkage
                    : llvm::Function::PrivateLinkage;

            // create function
            llvm::Function *the_function = llvm::Function::Create(
                    FT,
                    link_type,
                    func_name,
                    the_module);
            function_protos[this->id] = FT;

            if (this->is_noinline) {
                the_function->addFnAttr(llvm::Attribute::NoInline);
            } else if (this->is_always_inline) {
                the_function->addFnAttr(llvm::Attribute::AlwaysInline);
            } else if (this->is_inline) {
                the_function->addFnAttr(llvm::Attribute::InlineHint);
            }

            if (this->is_pure) {
                the_function->addFnAttr(llvm::Attribute::ReadNone);
                the_function->addFnAttr(llvm::Attribute::NoUnwind);
                the_function->addFnAttr(llvm::Attribute::WillReturn);
            }

            uint8_t param_index = 0;
            auto args_iter = the_function->args().begin();

            if (is_a_member_funtion) {
                args_iter->setName("this");
                args_iter++;
            }
            for (auto arg = args_iter; arg != the_function->args().end(); arg++) {
                Variable *var = (static_pointer_cast<Param>(this->paramList))->paramList[param_index++];
                arg->setName(var->id);
            }

            if (last_BB != nullptr) {
                builder->SetInsertPoint(last_BB, last_pt);
            }
        }

        if (this->compound != nullptr) {
            builder->ClearInsertionPoint();
            llvm::Function *the_function = the_module->getFunction(func_name);

            // create body
            llvm::BasicBlock *BB = llvm::BasicBlock::Create(*the_context, "entry", the_function);
            symbol_table->push(BB);
            symbol_table->setLoopEntry(nullptr);
            symbol_table->setLoopExit(nullptr);
            builder->SetInsertPoint(BB);

            // initialize param
            for (auto &arg: the_function->args()) {
                llvm::AllocaInst *alloca = allocaBlockEntry(the_function, arg.getName().str() + ".addr", arg.getType());
                builder->CreateStore(&arg, alloca);
                symbol_table->insert(arg.getName().str(), alloca, true);
            }

            llvm::Value *ret = nullptr;
            ret = this->compound->codeGen();
            if (ret) {
                auto endbb = builder->GetInsertBlock();
                auto t = endbb->getTerminator();
                if (!endbb->isEntryBlock() && !endbb->hasNPredecessorsOrMore(1)) {
                    endbb->eraseFromParent();
                } else if (!t) {
                    auto ret_const = llvm::dyn_cast<llvm::Constant>(ret);
                    if (the_function->getReturnType() == VOID_TY) {
                        builder->CreateRetVoid();
                    } else if (!ret_const || (ret_const && !ret_const->isNaN())) {
                        // expr without "return" keyword
                        auto ret_alloca_addr = allocaBlockEntry(the_function, "this.function.ret", the_function->getReturnType());
                        bool state = store(this, ret_alloca_addr, ret, false, "this.function.ret");

                        if (!state) {
                            // rebind variable
                            if (
                                    simple_types.find(ret->getType()) != simple_types.end()
                                    && simple_types.find(the_function->getReturnType()) != simple_types.end()
                                    ) {
                                throw ExceptionFactory<LogicException>(
                                        "unmatched return type '" + type_name[ret->getType()]
                                        + "', excepted '" + type_name[the_function->getReturnType()] + "'."
                                        + "This return expression cause a loss of precision",
                                        this->getToken().line, this->getToken().column
                                );
                            }
                            throw ExceptionFactory<LogicException>(
                                    "unmatched return type '" + type_name[ret->getType()]
                                    + "', excepted '" + type_name[the_function->getReturnType()] + "'."
                                    + "please check return expression",
                                    this->getToken().line, this->getToken().column
                            );
                        }

                        ret = builder->CreateLoad(the_function->getReturnType(), ret_alloca_addr);
                        builder->CreateRet(ret);
                    }  else if (the_function->getReturnType()->isFloatingPointTy()) {
                        builder->CreateRet(llvm::ConstantFP::get(the_function->getReturnType(), 0.0));
                    } else {
                        builder->CreateRet(llvm::ConstantInt::get(the_function->getReturnType(), 0));
                    }
                }
                symbol_table->pop();

                if (llvm::verifyFunction(*the_function, &llvm::outs())) {
                    throw ExceptionFactory<IRErrException>(
                            "some errors occurred when generating function",
                            this->token.line, this->token.column);
                }
                the_function_fpm->run(*the_function);
                the_function_fpm->run(*the_function);
                if (last_BB != nullptr) {
                    builder->SetInsertPoint(last_BB, last_pt);
                }

                return the_function;
            }
            symbol_table->pop();
        }
        if (last_BB != nullptr) {
            builder->SetInsertPoint(last_BB, last_pt);
        }
        return nullptr;
    }

    llvm::Value *FunctionCall::codeGen() {
        if (SF_is_builtin(this)) {
            return SF_builtin(this);
        }

        vector<string> modinfo = this->getToken().getModInfo();
        llvm::Function *fun = the_module->getFunction(this->id);
        vector<llvm::Value *> caller_args;

        for (int i = 0; i < this->paramList.size(); i++) {
            shared_ptr<AST> arg = paramList[i];
            llvm::Value *v = arg->codeGen();
            caller_args.push_back(v);
        }

        /* generate function names */
        vector<string> func_names;
        if (!fun) {
            if (modinfo.empty()) {
                // function in currnet module
                func_names.push_back(getFunctionNameMangling(module_path_with_module_name, this->id));
            } else if (modinfo[0] == "root") {
                // absolute path
                vector<string> p;
                modinfo.erase(modinfo.begin());
                p.insert(p.end(), package_path.begin(), package_path.end());
                p.insert(p.end(), modinfo.begin(), modinfo.end());
                func_names.push_back(getFunctionNameMangling(p, this->id));
            } else {
                // may be relative path
                auto fun_path = module_path;
                fun_path.insert(fun_path.end(), modinfo.begin(), modinfo.end());
                func_names.push_back(getFunctionNameMangling(fun_path, this->id));
                // may be alias
                string head = modinfo[0];
                if (module_name_alias.find(head) != module_name_alias.end()) {
                    auto path_cut = modinfo;
                    path_cut.erase(path_cut.begin());
                    head = module_name_alias[head];
                    auto head_to_origin = getpathUnresolvedToList(head);
                    for (auto i: path_cut) {
                        head_to_origin.push_back(i);
                    }
                    func_names.push_back(getFunctionNameMangling(head_to_origin, this->id));
                }
                // may be absolute
                func_names.push_back(getFunctionNameMangling(modinfo, this->id));
            }
        }
        func_names.push_back(this->id);

        for (auto i: func_names) {
            fun = the_module->getFunction(i);
            if (fun) {
                break;
            }
        }

        if (!fun) {
            // may be generic function
            for (auto i: func_names) {
                if (generic_function.find(i) != generic_function.end()) {
                    int idx = generic_function[i]->idx;
                    auto arg_type_name = type_name[caller_args[idx]->getType()];

                    if (generic_function[i]->function_map.find(arg_type_name) != generic_function[i]->function_map.end()) {
                        fun = the_module->getFunction(generic_function[i]->function_map[arg_type_name]);
                        break;
                    } else if (generic_function[i]->function_map.find("default") != generic_function[i]->function_map.end()){
                        fun = the_module->getFunction(generic_function[i]->function_map["default"]);
                        break;
                    } else {
                        throw ExceptionFactory<MissingException>(
                            "function '" + this->id + "' is not declared for type '" + arg_type_name + "'",
                            this->getToken().line, this->getToken().column
                        );
                    }
                }
            }
        }

        bool implicit = false;

        if (!fun) {
            if (this->param_this == nullptr) {
                Warning(
                    "implicit declaration of function '" + this->id + "'",
                    this->getToken().line, this->getToken().column
                );

                implicit = true;

                llvm::FunctionType *FT = llvm::FunctionType::get(
                    llvm::Type::getInt32Ty(*the_context),
                    true
                );

                llvm::Function::Create(
                    FT,
                    llvm::Function::ExternalLinkage,
                    this->id,
                    the_module);

                function_protos[this->id] = FT;
                fun = the_module->getFunction(this->id);
            } else {
                throw ExceptionFactory<MissingException>(
                    "function '" + this->id + "' is not declared",
                    this->getToken().line, this->getToken().column);
            }
        }

        if (
            (!fun->isVarArg() && (fun->arg_size() != this->paramList.size() + (this->param_this ? 1 : 0)))
            || (fun->isVarArg() && ((fun->arg_size() > this->paramList.size() + (this->param_this ? 1 : 0))))
        ) {
            // if function has declared, check parameters' size
            throw ExceptionFactory<LogicException>(
                    "candidate function not viable: requires " +
                    to_string(fun->arg_size()) +
                    " arguments, but " +
                    to_string(this->paramList.size() + (this->param_this ? 1 : 0)) +
                    " were provided",
                    this->token.line, this->token.column);
        }

        auto callee_arg_iter = fun->args().begin();

        if (this->param_this) {
            if (callee_arg_iter->getType() != this->param_this->getType()) {
                throw ExceptionFactory<MissingException>(
                        this->id + "is not a matched member function for type"
                        + type_name[this->param_this->getType()->getPointerElementType()],
                        this->token.line, this->token.column);
            }
            callee_arg_iter++;
        }

        for (int i = 0; i < this->paramList.size(); i++) {
            shared_ptr<AST> arg = paramList[i];
            llvm::Value *v = caller_args[i];
            if (!v) {
                return nullptr;
            }

            llvm::Type *callee_type = callee_arg_iter == fun->args().end() ? nullptr : callee_arg_iter->getType();
            llvm::Type *caller_type = v->getType();

            if (callee_arg_iter != fun->args().end() && type_size.find(callee_type) == type_size.end()) registerType(callee_type);
            if (type_size.find(caller_type) == type_size.end()) registerType(caller_type);

            if (caller_type->isArrayTy()) {
                llvm::AllocaInst *addr = (llvm::AllocaInst *) llvm::getLoadStorePointerOperand(v);

                if (!addr) {
                    llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
                    addr = allocaBlockEntry(the_scope, "array.init", caller_type);
                    builder->CreateStore(v, addr);
                }

                v = builder->CreateInBoundsGEP(
                        caller_type,
                        addr,
                        {
                                llvm::ConstantInt::get(I32_TY, 0),
                                llvm::ConstantInt::get(I32_TY, 0)
                        },
                        "arraydecay"
                );

                caller_type = v->getType();
                if (type_size.find(caller_type) == type_size.end()) registerType(caller_type);
            } else if (
                caller_type->isStructTy() &&
                (
                    (callee_arg_iter == fun->args().end()) ||
                    (
                        ((llvm::PointerType*)(callee_type))->getPointerElementType()->isStructTy() && 
                        ((llvm::StructType*)(((llvm::PointerType*)callee_type)->getPointerElementType()))->isLayoutIdentical((llvm::StructType *) caller_type)
                    )
                )
            ) {
                llvm::AllocaInst *addr = (llvm::AllocaInst *) llvm::getLoadStorePointerOperand(v);

                if (!addr) {
                    llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
                    addr = allocaBlockEntry(the_scope, "struct.init", caller_type);
                    builder->CreateStore(v, addr);
                }

                v = builder->CreateInBoundsGEP(
                        caller_type,
                        addr,
                        {
                                llvm::ConstantInt::get(I32_TY, 0),
                                llvm::ConstantInt::get(I32_TY, 0)
                        },
                        "structdecay"
                );

                v = builder->CreatePointerCast(v, caller_type->getPointerTo());                
                caller_type = v->getType();
                if (type_size.find(caller_type) == type_size.end()) registerType(caller_type);
            } else if ((callee_arg_iter == fun->args().end()) && caller_type->isFloatingPointTy()) {
                try {
                    v = type_conv(arg.get(), v, caller_type, llvm::Type::getDoubleTy(*the_context), false);
                } catch (...) {
                    throw ExceptionFactory<TypeException>(
                            "unmatched type, provided: " +
                            type_name[caller_type] +
                            ", excepted: " + type_name[callee_arg_iter->getType()],
                            arg->getToken().line, arg->getToken().column);
                }
            } else if ((callee_arg_iter != fun->args().end()) && callee_type != caller_type) {
                try {
                    v = type_conv(arg.get(), v, caller_type, callee_type, false);
                } catch (...) {
                    throw ExceptionFactory<TypeException>(
                            "unmatched type, provided: " +
                            type_name[caller_type] +
                            ", excepted: " + type_name[callee_arg_iter->getType()],
                            arg->getToken().line, arg->getToken().column);
                }
            }


            caller_args[i] = v;
            if (callee_arg_iter != fun->args().end()) callee_arg_iter++;
        }

        if (this->param_this) {
            caller_args.insert(caller_args.begin(), this->param_this);
        }

        llvm::CallInst *inst = nullptr;

        if (implicit) {
            vector<llvm::Type*> caller_types;
            for (auto i: caller_args) {
                caller_types.push_back(i->getType());
            }
            llvm::FunctionType *FT = llvm::FunctionType::get(
                fun->getReturnType(),
                caller_types,
                false
            );                
            llvm::Constant *BitcastExpr = llvm::ConstantExpr::getBitCast(
                fun,
                FT->getPointerTo()
            );

            if (FT->getReturnType() != VOID_TY) {
                inst = builder->CreateCall(
                    FT,
                    BitcastExpr,
                    caller_args,
                    "callLocal" 
                );
            } else {
                inst = builder->CreateCall(
                    FT,
                    BitcastExpr,
                    caller_args
                );
            }
        } else {
            if (fun->getFunctionType()->getReturnType() != VOID_TY) {
                inst = builder->CreateCall(fun, caller_args, "callLocal");
            } else {
                inst = builder->CreateCall(fun, caller_args);
            }
        }

        if (fun->getFunctionType()->getReturnType() != VOID_TY) {
            return inst;
        } else {
            return llvm::ConstantFP::getNaN(F64_TY);
        }
    }

    llvm::Value *Generic::codeGen() {
        bool is_a_member_funtion = !this->token.getModInfo().empty();
        auto parent_modinfo = this->token.getModInfo();

        auto get_func_name = [&](string id, vector<string> modinfo, Token token) -> string {
            string func_name =
                (id == ENTRY_NAME || !this->is_mangle)
                ? id
                : getFunctionNameMangling(module_path_with_module_name, id);  
            
            // redirect to member function
            llvm::Type *struct_type = nullptr;
            if (is_a_member_funtion) {
                auto struct_id = string(modinfo.back());
                vector<string> struct_path = modinfo;
                struct_path.pop_back();

                auto ty = find_struct(struct_path, struct_id);

                if (ty.first == struct_types.end()) {
                    throw ExceptionFactory<MissingException>(
                            "missing type '" + id + "'",
                            token.line,
                            token.column
                    );
                }

                struct_type = ty.first->second->Ty;
                auto struct_name = type_name[struct_type];
                auto index = struct_name.find('{');
                string part = struct_name.substr(0, index);
                struct_path = getpathUnresolvedToList(part);

                func_name = getFunctionNameMangling(struct_path, this->id);
            }

            return func_name;
        };

        string mapper_func_name = get_func_name(this->id, parent_modinfo, this->token);

        if (function_protos.find(mapper_func_name) != function_protos.end()) {
            throw ExceptionFactory<LogicException>(
                    "generic function '" + this->id + "' is conflict with function",
                    this->token.line, this->token.column);
        }

        llvm::NamedMDNode *meta = the_module->getOrInsertNamedMetadata("generic." + mapper_func_name);
        meta->addOperand(llvm::MDNode::get(*the_context, {llvm::MDString::get(*the_context, to_string(this->idx))}));

        GenericDef *gd = new GenericDef();
        gd->idx = this->idx;

        for (auto i: this->func_list) {
            string mapped_func_name = get_func_name(i.first, parent_modinfo, this->token);

            string ty_name = type_name[i.second.first];

            llvm::Metadata *md_args[] = {
                    llvm::MDString::get(*the_context, mapped_func_name),
                    llvm::MDString::get(*the_context, ty_name)
            };            

            meta->addOperand(llvm::MDNode::get(*the_context, md_args));
            gd->function_map[ty_name] = mapped_func_name;
        }

        if (!this->default_func.empty()) {
            llvm::Metadata *md_args[] = {
                    llvm::MDString::get(*the_context,  get_func_name(this->default_func, parent_modinfo, this->token)),
                    llvm::MDString::get(*the_context, "default")
            };            

            meta->addOperand(llvm::MDNode::get(*the_context, md_args));
            gd->function_map["default"] = this->default_func;
        }

        generic_function[mapper_func_name] = gd;

        return nullptr;
    }

    llvm::Value *StructInit::codeGen() {
        llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
        llvm::StructType *Ty = struct_types[this->id]->Ty;

        uint32_t param_num = this->paramList.size();
        uint32_t member_num = Ty->getNumElements();

        // check number of parameters
        if (param_num > member_num) {
            throw ExceptionFactory<LogicException>(
                    "candidate function not viable: requires " +
                    to_string(member_num) +
                    " or less arguments, but " +
                    to_string(param_num) +
                    " were provided",
                    this->token.line, this->token.column);
        }

        // initialize struct
        llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, "struct." + this->id + ".init", Ty);
        for (int i = 0; i < param_num; i++) {
            shared_ptr<AST> param = this->paramList[i];
            llvm::Value *rv = param->codeGen();

            auto member_addr = builder->CreateGEP(
                    llvm::cast<llvm::PointerType>(new_var_alloca->getType()->getScalarType())->getPointerElementType(),
                    new_var_alloca,
                    {
                            llvm::ConstantInt::get(I32_TY, 0),
                            llvm::ConstantInt::get(I32_TY, i),
                    },
                    id + ".member." + to_string(i));

            store(param.get(), member_addr, rv, false, id + ".member." + to_string(i));
        }

        return builder->CreateLoad(Ty, new_var_alloca);
    }

    llvm::Value *TypeTrans::codeGen() {
        auto v = this->factor->codeGen();

        auto vtype = v->getType();
        auto etype = this->Ty.first;

        return type_conv(this, v, vtype, etype);
    }

    llvm::Value *ArrayInit::codeGen() {
        bool is_in_function = builder->GetInsertBlock() ? true : false;
        llvm::Function *the_scope = is_in_function ? builder->GetInsertBlock()->getParent() : nullptr;
        uint32_t element_num = this->num;

        if (this->num_by_const.get()) {
            try {
                auto num = this->num_by_const->codeGen();
                auto cons = llvm::dyn_cast<llvm::Constant>(num);
                auto cons_data = llvm::dyn_cast<llvm::ConstantData>(cons);
                auto cons_int = llvm::dyn_cast<llvm::ConstantInt>(cons_data);
                if (cons_int) {
                    element_num = cons_int->getZExtValue();
                } else {
                    throw ExceptionFactory<LogicException>(
                        "array initializer must be a compile-time constant",
                        this->num_by_const->getToken().line, this->num_by_const->getToken().column
                    );
                }
            } catch (...) {
                throw ExceptionFactory<LogicException>(
                    "array initializer must be a compile-time constant",
                    this->num_by_const->getToken().line, this->num_by_const->getToken().column
                );
            }
        }

        /**
         * a pointer will be returned if element_num equals to 0.
         * to secure safety, the pointer will point to a newly created space
         */
        if (element_num == 0) {
            llvm::Type *contain_type = this->Ty.first;
            if (this->Ty.first == VOID_TY) {
                contain_type = I8_TY;
            }
            llvm::Type *ptr_type = contain_type->getPointerTo();
            type_name[ptr_type] = type_name[contain_type] + "*";
            type_size[ptr_type] = PTR_SIZE;
            auto nullptr_init = llvm::ConstantInt::get(ISIZE_TY, 0);
            return builder->CreateIntToPtr(nullptr_init, ptr_type);
        }

        if (this->paramList.size() == 0 && this->Ty.first == VOID_TY) {
            throw ExceptionFactory<TypeException>(
                    "array with void type is not allowed",
                    this->getToken().line, this->getToken().column);
        }

        /**
         * create array by type, return a array initialized by 0.
         */
        if (this->Ty.first != VOID_TY) {
            if (!this->is_vec) {
                auto arr_type = llvm::ArrayType::get(this->Ty.first, element_num);
                type_name[arr_type] = "arr[" + type_name[this->Ty.first] + ":" + to_string(element_num) + "]";
                type_name[arr_type->getPointerTo()] =
                        "arr[" + type_name[this->Ty.first] + ":" + to_string(element_num) + "]*";
                type_size[arr_type] = type_size[this->Ty.first] * element_num;
                type_size[arr_type->getPointerTo()] = PTR_SIZE;
                if (is_in_function) {
                    llvm::AllocaInst *array_alloca = allocaBlockEntry(the_scope, "array.init.by.type", arr_type);
                    builder->CreateMemSet(
                            array_alloca,
                            llvm::ConstantInt::get(
                                    llvm::Type::getInt8Ty(*the_context),
                                    0),
                            type_size[arr_type],
                            llvm::MaybeAlign());

                    return builder->CreateLoad(arr_type, array_alloca);
                } else {
                    return llvm::ConstantAggregateZero::get(arr_type);
                }
            } else {
                auto vec_type = llvm::VectorType::get(this->Ty.first, element_num, false);
                type_name[vec_type] = "vec[" + type_name[this->Ty.first] + ":" + to_string(element_num) + "]";
                type_name[vec_type->getPointerTo()] =
                        "vec[" + type_name[this->Ty.first] + ":" + to_string(element_num) + "]*";
                type_size[vec_type] = type_size[this->Ty.first] * element_num;
                type_size[vec_type->getPointerTo()] = PTR_SIZE;

                if (is_in_function) {
                    llvm::AllocaInst *array_alloca = allocaBlockEntry(the_scope, "vector.init.by.type", vec_type);
                    builder->CreateMemSet(
                            array_alloca,
                            llvm::ConstantInt::get(
                                    llvm::Type::getInt8Ty(*the_context),
                                    0),
                            type_size[vec_type],
                            llvm::MaybeAlign());

                    return builder->CreateLoad(vec_type, array_alloca);
                } else {
                    return llvm::ConstantAggregateZero::get(vec_type);
                }
            }
        }

        bool is_const_array = true;
        bool is_char_array = true;

        DataType const__number_array_type = DataType::Integer;

        // check type of array initializer
        for (auto i: this->paramList) {
            if (i->__AST_name != __NUM_NAME) {
                is_const_array = false;
                break;
            } else {
                const__number_array_type =
                        i->getToken().getValue().type() == DataType::Float
                        ? DataType::Float
                        : const__number_array_type;
            }
            if ((static_pointer_cast<Num>(i))->getToken().getType() != CHAR) {
                is_char_array = false;
            }
        }

        if (is_const_array) {
            // create a constant data array for constant array
            llvm::Constant *arr = nullptr;
            llvm::Type *tname;
            ::size_t tsize;
            if (is_char_array) {
                string str;
                for (auto i: this->paramList) {
                    str += (static_pointer_cast<Num>(i))->getToken().getValue().any_cast<char>();
                }
                arr = llvm::ConstantDataArray::getString(*the_context, str);
                tname = I8_TY;
                tsize = type_size[I8_TY];
                element_num += 1;
            } else {
                if (const__number_array_type == DataType::Integer) {
                    vector<int32_t> data;
                    for (auto i: this->paramList) {
                        data.push_back((static_pointer_cast<Num>(i))->getToken().getValue().any_cast<int>());
                    }
                    arr = llvm::ConstantDataArray::get(*the_context, data);
                    tname = I32_TY;
                    tsize = type_size[I32_TY];
                } else {
                    vector<float> data;
                    for (auto i: this->paramList) {
                        data.push_back((static_pointer_cast<Num>(i))->getToken().getValue().any_cast<double>());
                    }
                    arr = llvm::ConstantDataArray::get(*the_context, data);
                    tname = F32_TY;
                    tsize = type_size[F32_TY];
                }
            }

            type_name[arr->getType()] = "arr[" + type_name[tname] + ":" + to_string(element_num) + "]";
            type_name[arr->getType()->getPointerTo()] = "arr[" + type_name[tname] + ":" + to_string(element_num) + "]*";
            type_size[arr->getType()] = tsize * element_num;

            if (is_in_function) {
                string global_var_name = string("__constant.") + string(builder->GetInsertBlock()->getParent()->getName()) + string(".arr");
                llvm::function_ref<llvm::GlobalVariable *()> global_var_callback = [&] {
                    return new llvm::GlobalVariable(
                            *the_module,
                            arr->getType(),
                            false,
                            llvm::GlobalVariable::LinkageTypes::PrivateLinkage,
                            arr,
                            global_var_name);
                };
                auto ptr = the_module->getOrInsertGlobal(
                        global_var_name,
                        (llvm::Type *) arr->getType(),
                        global_var_callback
                );

                return builder->CreateLoad(arr->getType(), ptr);
            } else {
                return arr;
            }
        } else {
            if (is_in_function) {
                // else initialize an array by store action
                llvm::Value *head_rv = this->paramList[0]->codeGen();
                auto eleTy = head_rv->getType();
                auto arr_type = llvm::ArrayType::get(eleTy, element_num + 1);

                if (type_size.find(eleTy) == type_size.end()) {
                    registerType(eleTy);
                }

                type_name[arr_type] = "arr[" + type_name[eleTy] + ":" + to_string(element_num) + "]";
                type_name[arr_type->getPointerTo()] = "arr[" + type_name[eleTy] + ":" + to_string(element_num) + "]*";
                type_size[arr_type] = (eleTy->isPointerTy() ? PTR_SIZE : type_size[eleTy]) * element_num;

                // initialize array
                llvm::AllocaInst *array_alloca = allocaBlockEntry(the_scope, "array.init.by.value", arr_type);

                // store first element
                auto first_element_addr = builder->CreateGEP(
                        llvm::cast<llvm::PointerType>(array_alloca->getType()->getScalarType())->getPointerElementType(),
                        array_alloca,
                        {
                                llvm::ConstantInt::get(ISIZE_TY, 0),
                                llvm::ConstantInt::get(ISIZE_TY, 0),
                        },
                        "Array.init.element." + to_string(0));
                builder->CreateStore(head_rv, first_element_addr);

                // initialize other elements
                for (int i = 1; i < element_num; i++) {
                    shared_ptr<AST> param = this->paramList[i];
                    llvm::Value *rv = param->codeGen();
                    auto element_addr = builder->CreateGEP(
                        llvm::cast<llvm::PointerType>(array_alloca->getType()->getScalarType())->getPointerElementType(),
                        array_alloca,
                        {
                                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), 0),
                                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), i),
                        },
                        "ArrayInit.element." + to_string(i));

                    if (rv->getType() != eleTy) {
                        try {
                            rv = type_conv(param.get(), rv, rv->getType(), eleTy, false);
                        } catch (...) {
                            throw ExceptionFactory<TypeException>(
                                    "not matched type, element type: " +
                                    type_name[rv->getType()] +
                                    ", array type: " + type_name[eleTy],
                                    param->getToken().line, param->getToken().column);
                        }
                    }

                    builder->CreateStore(rv, element_addr);
                }

                // add NULL to tail
                auto element_addr = builder->CreateGEP(
                        llvm::cast<llvm::PointerType>(array_alloca->getType()->getScalarType())->getPointerElementType(),
                        array_alloca,
                        {
                                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), 0),
                                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), element_num),
                        },
                        "ArrayInit.element.tail");
                builder->CreateStore(llvm::Constant::getNullValue(eleTy), element_addr);

                return builder->CreateLoad(arr_type, array_alloca);
            } else {
                throw ExceptionFactory<LogicException>(
                    "array initializer must be a compile-time constant",
                    this->getToken().line, this->getToken().column
                );
            }
        }
    }

    llvm::Value *Global::codeGen() {
        shared_ptr<Variable> v = static_pointer_cast<Variable>(this->var);
        string id = v->id;

        if (v->Ty.first == VOID_TY && this->expr.get() == nullptr) {
            throw ExceptionFactory<SyntaxException>(
                    "missing type of global variable '" + id + "'",
                    this->getToken().line, this->getToken().column);
        } else if (v->Ty.first != VOID_TY && this->expr.get() != nullptr) {
            Warning(
                    "type of global variable '" + id + "' will be ignored",
                    this->getToken().line, this->getToken().column
            );
        }

        auto link_type =
                this->is_export
                ? llvm::GlobalVariable::ExternalLinkage
                : llvm::GlobalVariable::PrivateLinkage;

        llvm::Constant *init = v->Ty.first == VOID_TY ? nullptr : llvm::Constant::getNullValue(v->Ty.first);

        if (this->expr != nullptr) {
            try {
                auto right = this->expr->codeGen();
                init = llvm::dyn_cast<llvm::Constant>(right);
                if (!init) {
                    throw ExceptionFactory<LogicException>(
                            "initializer element is not a compile-time constant",
                            this->getToken().line, this->getToken().column);
                }
            } catch (...) {
                throw ExceptionFactory<LogicException>(
                        "initializer element is not a compile-time constant",
                        this->getToken().line, this->getToken().column);
            }
        }

        auto g_type = init->getType();
        auto g_name = this->is_mangle ? getFunctionNameMangling(module_path_with_module_name, id) : id;

        the_module->getOrInsertGlobal(
                g_name,
                g_type,
                [&] {
                    auto gv = new llvm::GlobalVariable(
                        *the_module,
                        g_type,
                        this->is_const,
                        link_type,
                        init,
                        g_name
                    );
                    if (gv->getType()->isArrayTy()) {
                        gv->setAlignment(llvm::Align(16));
                    }
                    if (this->is_const) {
                        gv->addAttribute(llvm::Attribute::ReadOnly);
                    }
                    return gv;
                });

        return llvm::ConstantFP::getNaN(F64_TY);
    }

    llvm::Value *Grad::codeGen() {
        auto expr = this->expr;
        for (auto i: this->vars) {
            for (int j = 0; j < i.second; j++) {
                expr = derivator(expr, i.first);
            }
        }

        return expr->codeGen();
    }

    llvm::Value *If::codeGen() {
        if (!this->noCondition) {
            llvm::Value *cond = this->condition->codeGen();
            if (!cond) {
                return nullptr;
            }
            if (cond->getType() != I1_TY) {
                if (cond->getType()->isIntegerTy()) {
                    cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(cond->getType(), 0), "toBool");
                } else {
                    cond = builder->CreateFCmpUNE(cond, llvm::ConstantFP::get(cond->getType(), 0.0), "toBool");
                }
            }

            if (auto *LC = llvm::dyn_cast<llvm::ConstantInt>(cond)) {
                if (LC->isOne()) {
                    if (opt_warning) {
                        Warning(
                                "the condition is always true",
                                this->condition->getToken().line,
                                this->condition->getToken().column
                        );
                    }
                    return this->compound->codeGen();
                } else {
                    if (opt_warning) {
                        Warning(
                                "the condition is always false",
                                this->condition->getToken().line,
                                this->condition->getToken().column
                        );
                    }
                    return this->next->codeGen();
                }
            } else {
                llvm::Function *the_function = builder->GetInsertBlock()->getParent();

                bool have_merge = false;

                llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(*the_context, "if.then", the_function);
                llvm::BasicBlock *elseBB = nullptr;
                if (this->next != ASTEmpty)
                    elseBB = llvm::BasicBlock::Create(*the_context, "if.else");
                llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "if.end");

                int phi_cnt = 0;
                uint8_t phi_mask = 0x0;

                if (elseBB != nullptr) {
                    builder->CreateCondBr(cond, thenBB, elseBB);
                } else {
                    have_merge = true;
                    builder->CreateCondBr(cond, thenBB, mergeBB);
                }

                builder->SetInsertPoint(thenBB);
                symbol_table->push(thenBB);

                llvm::Value *thenv = this->compound->codeGen();
                if (!thenv) {
                    return nullptr;
                }
                symbol_table->pop();
                auto t_then = builder->GetInsertBlock()->getTerminator();
                if (!t_then) {
                    have_merge = true;
                    builder->CreateBr(mergeBB);
                }
                auto then_const = llvm::dyn_cast<llvm::Constant>(thenv);
                if (!then_const || (then_const && !then_const->isNaN())) {
                    phi_cnt++;
                    phi_mask |= 0x1;
                }
                thenBB = builder->GetInsertBlock();

                llvm::Value *elsev = nullptr;
                llvm::Instruction *t_else = nullptr;
                if (elseBB != nullptr) {
                    the_function->getBasicBlockList().push_back(elseBB);
                    builder->SetInsertPoint(elseBB);
                    symbol_table->push(elseBB);
                    elsev = this->next->codeGen();
                    if (!elsev) {
                        return nullptr;
                    }
                    symbol_table->pop();
                    t_else = builder->GetInsertBlock()->getTerminator();
                    if (!t_else) {
                        have_merge = true;
                        builder->CreateBr(mergeBB);
                    }
                    auto else_const = llvm::dyn_cast<llvm::Constant>(elsev);
                    if (!else_const || (else_const && !else_const->isNaN())) {
                        phi_cnt++;
                        phi_mask |= 0x2;
                    }
                    elseBB = builder->GetInsertBlock();
                }

                if (have_merge) {
                    the_function->getBasicBlockList().push_back(mergeBB);
                    builder->SetInsertPoint(mergeBB);
                    if (phi_cnt == 2) {
                        if (thenv->getType() != elsev->getType()) {
                            throw ExceptionFactory<TypeException>(
                                    "unmatched types of values from 'then' and 'else' block",
                                    this->token.line, this->token.column);
                        }

                        llvm::PHINode *PN = builder->CreatePHI(thenv->getType(),
                                                               2);
                        PN->addIncoming(thenv, thenBB);
                        PN->addIncoming(elsev, elseBB);

                        return PN;
                    } else if (phi_cnt == 1) {
                        throw ExceptionFactory<LogicException>(
                                "select statement must have two optional values, but "
                                + string(phi_mask & 0x1 ? "'else'" : "'then'") + " is null value",
                                this->token.line, this->token.column);
                    }
                } else {
                    delete mergeBB;
                }

                return llvm::ConstantFP::getNaN(F64_TY);
            }
        } else {
            return this->compound->codeGen();
        }
    }

    llvm::Value *LoopCtrl::codeGen() {
        if (this->type == LoopCtrl::LoopCtrlType::CTRL_BREAK) {
            auto break_to = symbol_table->getLoopExit();
            if (break_to != nullptr) {
//                builder->CreateBr(break_to);
                builder->Insert(llvm::BranchInst::Create(break_to));
            } else {
                throw ExceptionFactory<LogicException>(
                        "break must be used in a loop",
                        this->token.line, this->token.column);
            }
        } else {
            auto continue_to = symbol_table->getLoopEntry();
            if (continue_to != nullptr) {
                builder->CreateBr(continue_to);
            } else {
                throw ExceptionFactory<LogicException>(
                        "continue must be used in a loop",
                        this->token.line, this->token.column);
            }
        }
        return llvm::ConstantFP::getNaN(F64_TY);
    }

    llvm::Value *Num::codeGen() {
        //        auto t = llvm::ConstantFP::get(*the_context, llvm::APFloat((double)this->value));
        if (this->token.getType() == CHAR) {
            return llvm::ConstantInt::get(I8_TY, this->getValue().any_cast<char>());
        } else {
            if (this->getValue().type() == Float) {
                return llvm::ConstantFP::get(F32_TY,
                                             this->getValue().any_cast<float>());
            } else {
                return llvm::ConstantInt::get(I32_TY,
                                              this->getValue().any_cast<int>());
            }
        }
    }

    llvm::Value *Object::codeGen() {
        return llvm::ConstantFP::getNaN(F64_TY);
    }

    llvm::Value *UnaryOp::codeGen() {
        llvm::Value *rv = this->right->codeGen();
        if (!rv) {
            throw ExceptionFactory<LogicException>(
                    "unknown right value of unary op",
                    this->token.line, this->token.column);
        }

        llvm::Type *ty = rv->getType();
        bool float_point = rv->getType()->isFloatingPointTy();

        if (this->op.getType() == BITAND) {
            if (ty->isArrayTy()) {
                return rv;
            } else if (this->right->__AST_name == __VARIABLE_NAME) {
                return getAlloca(static_pointer_cast<Variable>(this->right).get());
            } else
                throw ExceptionFactory<MathException>(
                        "cannot get the address of unsupported type",
                        this->token.line, this->token.column);
        }

        if (this->op.getType() == STAR) {
            if (!ty->isPtrOrPtrVectorTy()) {
                throw ExceptionFactory<MathException>(
                        "type '" + type_name[ty] + "' must be pointer type",
                        this->token.line, this->token.column);
            }

            return builder->CreateLoad(ty->getPointerElementType(), rv);
        }

        if (simple_types.find(ty) == simple_types.end()) {
            throw ExceptionFactory<MathException>(
                    "unsupported type '" + type_name[ty] + "' to unary expression",
                    this->token.line, this->token.column);
        }

        if (this->op.getType() == MINUS) {
            if (float_point)
                return builder->CreateFSub(llvm::ConstantFP::get(ty, 0.0), rv, "neg");
            else
                return builder->CreateSub(llvm::ConstantInt::get(ty, 0), rv, "neg");
        } else if (this->op.getType() == PLUS) {
            return rv;
        } else if (this->op.getType() == NOT) {
            if (float_point)
                return builder->CreateFCmpUNE(rv, llvm::ConstantFP::get(ty, 0.0), "not");
            else
                return builder->CreateICmpNE(rv, llvm::ConstantInt::get(ty, 0), "not");
        } else if (this->op.getType() == BITCPL) {
            if (float_point)
                throw ExceptionFactory<MathException>(
                        "unsupported type '" + type_name[ty] + "' to bit complement expression",
                        this->token.line, this->token.column);
            else
                return builder->CreateXor(rv, llvm::ConstantInt::get(ty, -1), "complement");
        } else {
            return nullptr;
        }
    }

    llvm::Value *Sizeof::codeGen() {
        if (this->id != nullptr) {
            llvm::Value *v = getAlloca(static_pointer_cast<Variable>(this->id).get());

            if (!v) {
                throw ExceptionFactory<MissingException>(
                        "variable '" + (static_pointer_cast<Variable>(this->id))->id + "' is not defined",
                        this->token.line, this->token.column);
            }

            llvm::Type *type = v->getType()->getPointerElementType();

            if (type->isPtrOrPtrVectorTy()) {
                return llvm::ConstantInt::get(I32_TY, PTR_SIZE);
            } else {
                // non-pointer type
                return llvm::ConstantInt::get(I32_TY, type_size[type]);
            }
        }

        return llvm::ConstantInt::get(I32_TY, type_size[this->Ty.first]);
    }

    llvm::Value *String::codeGen() {
        string str = this->getValue().any_cast<string>();
        if (const_string_pool.find(str) != const_string_pool.end()) {
            return const_string_pool[str];
        } else {
            auto ptr = builder->CreateGlobalStringPtr(this->getValue().any_cast<string>(), ".str", 0, the_module);
            const_string_pool[str] = ptr;
            return ptr;
        }
    }

    llvm::Value *Variable::codeGen() {
        llvm::Constant *global_const = getGlobalConstant(this);
        if (global_const) {
            return global_const;
        }

        llvm::Value *v = getAlloca(this);

        if (!v) {
            throw ExceptionFactory<MissingException>(
                "variable '" + this->id + "' is not defined",
                this->token.line, this->token.column
            );
        }

        if (builder->GetInsertBlock()) {
            return builder->CreateLoad(v->getType()->getPointerElementType(), v, this->id.c_str());
        } else {
            throw ExceptionFactory<MissingException>(
                "cannot get the value of variable '" + this->id + "' out of function",
                this->token.line, this->token.column
            );
        }
    }

    llvm::Value *Compound::codeGen() {
        int err_count = 0;
        bool is_err = false;

        size_t cnt = 0;
        size_t child_size = this->child.size();

        llvm::Value *ret = llvm::ConstantFP::getNaN(F64_TY);

        for (shared_ptr<AST> ast: this->child) {
            try {
                llvm::BasicBlock *bb = builder->GetInsertBlock();
                if (ast->__AST_name == __COMPOUND_NAME && bb) {
                    symbol_table->push(bb);
                }

                auto value = ast->codeGen();

                if (ast->__AST_name == __COMPOUND_NAME && bb) {
                    symbol_table->pop();
                }

                cnt++;

                // value of calling function is not belonging to block
                if (ast->__AST_name != __FUNCTIONCALL_NAME) {
                    ret = value;
                }

                // terminate
                if (
                        cnt != child_size
                        && ast->__AST_name != __FUNCTIONDECL_NAME
                        && ast->__AST_name != __OBJECT_NAME
                        && ast->__AST_name != __GENERIC_NAME
                        && (
                                (
                                        builder->GetInsertBlock()
                                        && builder->GetInsertBlock()->getTerminator()
                                )
                                || (
                                        (
                                                ast->__AST_name == __BLOCKEXPR_NAME
                                                || ast->__AST_name == __IF_NAME
                                        )
                                        && llvm::dyn_cast<llvm::Constant>(ret)
                                        && !llvm::dyn_cast<llvm::Constant>(ret)->isNaN()
                                )
                        )
                        ) {
                    if (opt_warning) {
                        Warning(
                                "terminator detected, subsequent code will be ignored",
                                ast->getToken().line, ast->getToken().column);
                    }
                    break;
                }
            }
            catch (Exception e) {
                if (e.type() != __ErrReport) {
                    err_count++;
                    std::cerr << __COLOR_RED
                              << input_file_name
                              << ":" << e.line << ":" << e.column + 1 << ": "
                              << e.what()
                              << __COLOR_RESET << std::endl;
                }

                is_err = true;
            }
        }

        if (err_count != 0 || is_err) {
            throw ExceptionFactory<ErrReport>(
                    "generated " + to_string(err_count) + " errors",
                    0, 0);
        }

        return ret;
    }

    llvm::Value *Return::codeGen() {
        if (this->ret != nullptr) {
            llvm::Value *re = this->ret->codeGen();
            if (!re) {
                return nullptr;
            }

            if (re->getType() == builder->getCurrentFunctionReturnType()) {
                return builder->CreateRet(re);
            } else {
                llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
                auto ret_alloca_addr = allocaBlockEntry(the_scope, "ret", the_scope->getReturnType());
                bool state = store(this, ret_alloca_addr, re, false, "ret");

                if (!state) {
                    // rebind variable
                    if (
                            simple_types.find(re->getType()) != simple_types.end()
                            && simple_types.find(the_scope->getReturnType()) != simple_types.end()
                            ) {
                        throw ExceptionFactory<LogicException>(
                                "unmatched return type '" + type_name[re->getType()]
                                + "', excepted '" + type_name[the_scope->getReturnType()] + "'."
                                + "This return expression cause a loss of precision",
                                this->getToken().line, this->getToken().column
                        );
                    }
                    throw ExceptionFactory<LogicException>(
                            "unmatched return type '" + type_name[re->getType()]
                            + "', excepted '" + type_name[the_scope->getReturnType()] + "'."
                            + "please check return expression",
                            this->getToken().line, this->getToken().column
                    );
                }

                re = builder->CreateLoad(the_scope->getReturnType(), ret_alloca_addr);
                return builder->CreateRet(re);
            }
        } else {
            return builder->CreateRetVoid();
        }
    }

    llvm::Value *While::codeGen() {
        llvm::Function *the_function = builder->GetInsertBlock()->getParent();

        llvm::BasicBlock *headBB = llvm::BasicBlock::Create(*the_context, "loop.head", the_function);
        llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(*the_context, "loop.body");
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "loop.end");

        builder->CreateBr(headBB);

        builder->SetInsertPoint(headBB);

        symbol_table->push(headBB);
        symbol_table->setLoopExit(mergeBB);
        symbol_table->setLoopEntry(headBB);

        llvm::Value *cond = this->condition->codeGen();
        if (!cond) {
            return nullptr;
        }
        if (cond->getType() != I1_TY) {
            if (cond->getType()->isIntegerTy()) {
                cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(cond->getType(), 0), "toBool");
            } else {
                cond = builder->CreateFCmpUNE(cond, llvm::ConstantFP::get(cond->getType(), 0.0), "toBool");
            }
        }

        if (auto *LC = llvm::dyn_cast<llvm::ConstantInt>(cond)) {
            if (LC->isOne()) {
                builder->CreateBr(loopBB);
            } else {
                builder->CreateBr(mergeBB);
            }
        } else {
            builder->CreateCondBr(cond, loopBB, mergeBB);
        }

        the_function->getBasicBlockList().push_back(loopBB);
        builder->SetInsertPoint(loopBB);
        llvm::Value *body = this->compound->codeGen();
        if (!body) {
            return nullptr;
        }

        symbol_table->pop();

        auto t = builder->GetInsertBlock()->getTerminator();
        if (!t) {
            builder->CreateBr(headBB);
        }

        the_function->getBasicBlockList().push_back(mergeBB);
        builder->SetInsertPoint(mergeBB);

        if (!mergeBB->hasNPredecessorsOrMore(1)) {
            if (opt_warning) {
                Warning(
                        "endless loop",
                        this->getToken().line,
                        this->getToken().column
                );
            }
        }

        return llvm::ConstantFP::getNaN(F64_TY);
    }

    llvm::Value *NoneAST::codeGen() {
        return llvm::ConstantFP::getNaN(F64_TY);
    }

}
