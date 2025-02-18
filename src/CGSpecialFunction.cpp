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

    map<string, function<llvm::Value*(FunctionCall*)>> special_function_list = {
        {"min", SF_min},
        {"max", SF_max},
    };

    /*******************************************************
     *                     function                        *
     *******************************************************/
    /**
     * @description:    convert binary operation operands to the same type
     * @param           ast: the function call AST
     * @param           lv: the left operand
     * @param           rv: the right operand
     * @return:         true: convert to floating point
     *                  false: convert to integer
     */
    bool SF_convert_binop_operand(FunctionCall *ast, llvm::Value *lv, llvm::Value *rv) {
        if (!lv || !rv) {
            throw ExceptionFactory<IRErrException>(
                "left or right operand must have a value",
                ast->getToken().line, ast->getToken().column
            );
        }

        auto lv_const = llvm::dyn_cast<llvm::Constant>(lv);
        auto rv_const = llvm::dyn_cast<llvm::Constant>(rv);

        if (lv_const && lv_const->isNaN()) {
            throw ExceptionFactory<IRErrException>(
                "left operand must have a value",
                ast->getToken().line, ast->getToken().column
            );
        }
        if (rv_const && rv_const->isNaN()) {
            throw ExceptionFactory<IRErrException>(
                "right operand must have a value",
                ast->getToken().line, ast->getToken().column
            );
        }

        llvm::Type *l_type = lv->getType();
        llvm::Type *r_type = rv->getType();

        if (simple_types.find(l_type) == simple_types.end()) {
            throw ExceptionFactory<MathException>(
                "unsupported left type '" + type_name[l_type] + "' and '" + type_name[r_type] + "' to binary expression",
                ast->getToken().line, ast->getToken().column
            );
        }

        if (simple_types.find(r_type) == simple_types.end()) {
            throw ExceptionFactory<MathException>(
                "unsupported right type '" + type_name[r_type] + "' and '" + type_name[r_type] + "' to binary expression",
                ast->getToken().line, ast->getToken().column
            );
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

        if (float_point) {
            lv = l_type->isFloatingPointTy()
                    ? builder->CreateFPExt(lv, cast_to, "conv.fp.ext")
                    : (
                      l_type == I1_TY
                      ? builder->CreateUIToFP(lv, cast_to, "conv.zi.fp")
                      : builder->CreateSIToFP(lv, cast_to, "conv.si.fp")
                    );
            rv = r_type->isFloatingPointTy()
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

        return float_point;
    }

    bool SF_is_builtin(FunctionCall *ast) {
        if (!ast->getToken().getModInfo().empty()) {
            return false;
        }

        if (special_function_list.find(ast->id) != special_function_list.end()) {
            return true;
        }

        return false;
    }

    llvm::Value *SF_builtin(FunctionCall *ast) {
        if (!SF_is_builtin(ast)) {
            throw ExceptionFactory<SysErrException>(
                "function " + ast->id + " is not a builtin function",
                ast->getToken().line,
                ast->getToken().column
            );
        }

        return special_function_list[ast->id](ast);
    }

    llvm::Value *SF_min(FunctionCall *ast) {
        if (ast->paramList.size() != 2) {
            throw ExceptionFactory<SysErrException>(
                "function min requires 2 arguments",
                ast->getToken().line,
                ast->getToken().column
            );
        }

        auto lv = ast->paramList[0]->codeGen();
        auto rv = ast->paramList[1]->codeGen();

        auto float_point = SF_convert_binop_operand(ast, lv, rv);
        
        llvm::Value *cmp_value_boolean;
        if (float_point) {
            cmp_value_boolean = builder->CreateFCmpOLT(lv, rv, "min.lt");
        } else {
            cmp_value_boolean = builder->CreateICmpSLT(lv, rv, "min.lt");
        }        

        if (auto *LC = llvm::dyn_cast<llvm::ConstantInt>(cmp_value_boolean)) {
            if (LC->isOne()) {
                return lv;
            } else if (LC->isZero()){
                return rv;
            } else {
                throw ExceptionFactory<SysErrException>(
                    "failed to compare values",
                    ast->getToken().line,
                    ast->getToken().column
                );
            }
        } else {
            llvm::Function *the_function = builder->GetInsertBlock()->getParent();
            llvm::BasicBlock *leftBB = llvm::BasicBlock::Create(*the_context, "min.left", the_function);
            llvm::BasicBlock *rightBB = llvm::BasicBlock::Create(*the_context, "min.right", the_function);
            llvm::BasicBlock *ansBB = llvm::BasicBlock::Create(*the_context, "min.ans", the_function);

            builder->CreateCondBr(cmp_value_boolean, leftBB, rightBB);

            builder->SetInsertPoint(leftBB);
            builder->CreateBr(ansBB);
            builder->SetInsertPoint(rightBB);
            builder->CreateBr(ansBB);
            builder->SetInsertPoint(ansBB);

            llvm::PHINode *PN = builder->CreatePHI(lv->getType(),2);
            PN->addIncoming(lv, leftBB);
            PN->addIncoming(rv, rightBB);

            return PN;
        }
    }

    llvm::Value *SF_max(FunctionCall *ast) {
        if (ast->paramList.size() != 2) {
            throw ExceptionFactory<SysErrException>(
                "function max requires 2 arguments",
                ast->getToken().line,
                ast->getToken().column
            );
        }

        auto lv = ast->paramList[0]->codeGen();
        auto rv = ast->paramList[1]->codeGen();

        auto float_point = SF_convert_binop_operand(ast, lv, rv);
        
        llvm::Value *cmp_value_boolean;
        if (float_point) {
            cmp_value_boolean = builder->CreateFCmpOGT(lv, rv, "max.gt");
        } else {
            cmp_value_boolean = builder->CreateICmpSGT(lv, rv, "max.gt");
        }        

        if (auto *LC = llvm::dyn_cast<llvm::ConstantInt>(cmp_value_boolean)) {
            if (LC->isOne()) {
                return lv;
            } else if (LC->isZero()){
                return rv;
            } else {
                throw ExceptionFactory<SysErrException>(
                    "failed to compare values",
                    ast->getToken().line,
                    ast->getToken().column
                );
            }
        } else {
            llvm::Function *the_function = builder->GetInsertBlock()->getParent();
            llvm::BasicBlock *leftBB = llvm::BasicBlock::Create(*the_context, "max.left", the_function);
            llvm::BasicBlock *rightBB = llvm::BasicBlock::Create(*the_context, "max.right", the_function);
            llvm::BasicBlock *ansBB = llvm::BasicBlock::Create(*the_context, "max.ans", the_function);

            builder->CreateCondBr(cmp_value_boolean, leftBB, rightBB);

            builder->SetInsertPoint(leftBB);
            builder->CreateBr(ansBB);
            builder->SetInsertPoint(rightBB);
            builder->CreateBr(ansBB);
            builder->SetInsertPoint(ansBB);

            llvm::PHINode *PN = builder->CreatePHI(lv->getType(),2);
            PN->addIncoming(lv, leftBB);
            PN->addIncoming(rv, rightBB);

            return PN;
        }   
    }
}
