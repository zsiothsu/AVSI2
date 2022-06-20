/*
 * ASTCodeGen.cpp 2022
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
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Bitcode/BitcodeWriter.h>

#define UNUSED(x) ((void)(x))

extern bool opt_reliance;
extern bool opt_verbose;
extern bool opt_warning;

namespace AVSI {
    /*******************************************************
     *                      llvm base                      *
     *******************************************************/
    string module_name;
    vector<string> module_path = vector<string>();

    llvm::LLVMContext *the_context;
    llvm::Module *the_module;
    llvm::IRBuilder<> *builder;
    llvm::legacy::FunctionPassManager *the_fpm;
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

    AST *ASTEmpty = new NoneAST();
    AST *ASTEmptyNotEnd = new NoneAST();

    // global stack based symbol table
    SymbolTable *symbol_table;
    // store structure definitions. including members' name and type.
    map<string, StructDef *> struct_types;
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

    /*******************************************************
     *                    IR generator                     *
     *******************************************************/
    /**
    * @description:    get basic type of an array
    * @param:          Ty: pointer type
    * @return:         the pointer type point to basic type
    * @example:        offer a pointer type like [[real x 3] x 3]* to the
     *                 function will return real*
    */
    llvm::PointerType *getArrayBasicTypePointer(llvm::PointerType *Ty) {
        llvm::Type *basic = Ty->getPointerElementType();
        while (basic->isArrayTy())
            basic = basic->getArrayElementType();
        return basic->getPointerTo();
    }

    /**
     * @description:    compare the basic type between two arrays
     * @param:          l: pointer type
     * @param:          r: pointer type
     * @return:         bool
     * @example:        compare [[real x 3] x 3]* and  [real x 3]* will
     *                  get true for they have the smae basic type "real"
     */
    bool isTheSameBasicType(llvm::PointerType *l, llvm::PointerType *r) {
        llvm::Type *basicl = l->getPointerElementType();
        llvm::Type *basicr = r->getPointerElementType();

        while (basicl->isArrayTy())
            basicl = basicl->getArrayElementType();
        while (basicr->isArrayTy())
            basicr = basicr->getArrayElementType();

        return basicl == basicr;
    }

    /**
     * @description:    insert an alloca instruction at the head of function
     * @param:          fun: the function variable defined in
     * @param:          name: name of variable
     * @param:          type: variable type
     * @return:         a pointer point to the space allocated
     */
    llvm::AllocaInst *allocaBlockEntry(llvm::Function *fun, string name, llvm::Type *Ty) {
        llvm::IRBuilder<> blockEntry(
                &fun->getEntryBlock(),
                fun->getEntryBlock().begin());

        return blockEntry.CreateAlloca(
                Ty,
                0,
                name.c_str());
    }

    /**
     * @description:    get address of variable
     * @param:          var: variable AST
     * @return:         a pointer to target address
     */
    llvm::Value *getAlloca(Variable *var) {
        /* get address of variable
         * -------------------------------------------------------------------
         *  variable                processing                  result
         * -------------------------------------------------------------------
         *  a:real                  &real                       real *
         *  b:vec[real;2]           &(vec[real;2]*)             real* *
         *  b[0]                    &(*&(vec[real;2]*)))[0]     real *
         *  c:vec[vec[real;2];0]    the same as b
         *  d = {:vec[real;2];0}    the same as b
         */
        llvm::Value *v = symbol_table->find(var->id);

        string mod_path = getpathListToUnresolved(var->getToken().getModInfo());
        mod_path = module_name_alias[mod_path];

        if (!v) {
            v = the_module->getGlobalVariable(
                    getFunctionNameMangling(getpathUnresolvedToList(mod_path), var->id));
        }

        if (v && !var->offset.empty()) {
            /*
             * for the variable of vector type is a pointer,
             * offset is not effective in pointer, but that it point to
             * so get the address point to first
             */
            for (auto i: var->offset) {
                if (v->getType()->getPointerElementType()->isPtrOrPtrVectorTy()) {
                    v = builder->CreateLoad(v->getType()->getPointerElementType(), v);
                }
                vector<llvm::Value *> offset_list;
                offset_list.push_back(llvm::ConstantInt::get(
                        llvm::Type::getInt32Ty(*the_context),
                        0,
                        true));
                if (i.first == Variable::ARRAY) {
                    llvm::Value *index = i.second->codeGen();

                    if (index->getType()->isFloatingPointTy()) {
                        index = builder->CreateFPToSI(index, MACHINE_WIDTH_TY);
                    } else {
                        index = builder->CreateSExtOrTrunc(index, MACHINE_WIDTH_TY);
                    }
                    offset_list.push_back(index);

                    if (!v->getType()->getPointerElementType()->isArrayTy()) {
                        // for raw pointer
                        index = builder->CreateMul(index,
                                                   llvm::ConstantInt::get(
                                                           MACHINE_WIDTH_TY,
                                                           v->getType()->getPointerElementType()->isPtrOrPtrVectorTy()
                                                           ? PTR_SIZE
                                                           : type_size[v->getType()->getPointerElementType()]));

                        llvm::Type *v_oldTy = v->getType();
                        v = builder->CreatePtrToInt(v, MACHINE_WIDTH_TY);
                        v = builder->CreateAdd(v, index);
                        v = builder->CreateIntToPtr(v, v_oldTy);
                    } else {
                        try {
                            v = builder->CreateGEP(
                                    llvm::cast<llvm::PointerType>(v->getType()->getScalarType())->getElementType(),
                                    v,
                                    offset_list,
                                    var->id);
                        }
                        catch (...) {
                            throw ExceptionFactory<TypeException>(
                                    "index out of range or not an array",
                                    i.second->getToken().line, i.second->getToken().column);
                        }
                    }
                } else {
                    string member_name = ((Variable *) (i.second))->id;
                    auto current_ty = v->getType()->getPointerElementType();

                    bool find_flag = false;
                    for (auto iter: struct_types) {
                        if (iter.second->Ty == current_ty) {
                            if (iter.second->members.find(member_name) == iter.second->members.end()) {
                                break;
                            }

                            auto index = iter.second->members[member_name];
                            offset_list.push_back(llvm::ConstantInt::get(
                                    llvm::Type::getInt32Ty(*the_context),
                                    index,
                                    true));
                            find_flag = true;
                            break;
                        }
                    }

                    if (find_flag) {
                        v = builder->CreateGEP(
                                llvm::cast<llvm::PointerType>(v->getType()->getScalarType())->getElementType(),
                                v,
                                offset_list,
                                var->id);
                    } else {
                        throw ExceptionFactory<MissingException>(
                                "unrecognized member '" + member_name + "'",
                                i.second->getToken().line, i.second->getToken().column);
                    }
                }

                /**
                 * for nest array, the result of GEP instruction is just the raw space
                 * of array. we need to create a pointer point to the space.
                 */
                if (v->getType()->getPointerElementType()->isArrayTy()) {
                    llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
                    auto alloca = allocaBlockEntry(the_scope, "array.ptr.addr", v->getType());
                    builder->CreateStore(v, alloca);
                    v = alloca;
                }
            }
        }

        // v is a pointer to target address
        return v;
    }

    /**
     * @description:    perform a stored action, including automatic type conversion
     * @param:          ast: AST of assignment, just for debug message
     * @param:          l_alloca_addr: space to be assigned. new space will be created if
     *                                 l_alloca_addr is nullptr
     * @param:          r_value: value
     * @param:          assignment: a label used to distinguish assignment(true)
     *                              statements and struct initialization(false)
     * @param:          l_base_name: space name. if l_alloca_addr is nullptr, new space
     *                               will be named by l_base_name
     * @return:         none
     */
    llvm::Value *
    store(AST *ast, llvm::Value *l_alloca_addr, llvm::Value *r_value, bool assignment = true,
          string l_base_name = "member") {
        llvm::Type *r_type = r_value->getType();
        Assign *assigin_ast = (Assign *) ast;

        if (r_type == VOID_TY) {
            throw ExceptionFactory<LogicException>(
                    "cannot assign void to variable",
                    ast->getToken().line, ast->getToken().column);
        }

        llvm::Type *l_type = l_alloca_addr ? l_alloca_addr->getType()->getPointerElementType() : nullptr;

        if (
                l_alloca_addr != nullptr &&
                l_type->isPtrOrPtrVectorTy() &&
                r_type->isPtrOrPtrVectorTy() &&
                (l_type->getPointerElementType()->isArrayTy() || r_type->getPointerElementType()->isArrayTy()) &&
                isTheSameBasicType((llvm::PointerType *) l_type, (llvm::PointerType *) r_type)) {
            /*
             * real* = real*
             * obj* = obj*
             *
             * An array pointer assigned to another pointer.
             * But in AVSI, high-dimensional array is not an
             * array of pointer of low-dimensional array,
             * so the role of assign is copy right array to left
             */
            auto &r_ptr = r_value;

            uint32_t size = 0;
            if (l_type->getPointerElementType()->isArrayTy() && r_type->getPointerElementType()->isArrayTy()) {
                llvm::Value *l_ptr = builder->CreateLoad(l_alloca_addr->getType()->getPointerElementType(),
                                                         l_alloca_addr,
                                                         "dest.addr");
                size = min(type_size[l_type->getPointerElementType()],
                           type_size[l_type->getPointerElementType()]);
                builder->CreateMemCpy(l_ptr, llvm::MaybeAlign(), r_ptr, llvm::MaybeAlign(),
                                      size);
            } else if (l_type->getPointerElementType()->isArrayTy()) {
                llvm::Value *l_ptr = builder->CreateLoad(l_alloca_addr->getType()->getPointerElementType(),
                                                         l_alloca_addr,
                                                         "dest.addr");
                size = type_size[l_type->getPointerElementType()];
                builder->CreateMemCpy(l_ptr, llvm::MaybeAlign(), r_ptr, llvm::MaybeAlign(),
                                      size);
            } else {
                r_ptr = builder->CreatePointerCast(r_ptr, l_type, "ptr.cast");
                builder->CreateStore(r_ptr, l_alloca_addr);
            }
        } else if (
                l_alloca_addr != nullptr &&
                l_type == r_type) {
            builder->CreateStore(r_value, l_alloca_addr);
        } else if (l_alloca_addr != nullptr &&
                   simple_types.find(l_type) != simple_types.end() &&
                   simple_types.find(r_type) != simple_types.end()) {
            bool is_l_fp = l_type->isFloatingPointTy();
            bool is_r_fp = r_type->isFloatingPointTy();
            if (simple_types_map[r_type] < simple_types_map[l_type]) {
                if (is_r_fp && is_l_fp) {
                    r_value = builder->CreateFPExt(r_value, l_type, "conv");
                } else if (!is_r_fp && is_l_fp) {
                    r_value = builder->CreateSIToFP(r_value, l_type, "conv");
                } else {
                    r_value = builder->CreateSExt(r_value, l_type, "conv");
                }
            } else {
                if (is_r_fp && is_l_fp) {
                    r_value = builder->CreateFPTrunc(r_value, l_type, "conv");
                } else if (is_r_fp && !is_l_fp) {
                    r_value = builder->CreateFPToSI(r_value, l_type, "conv");
                } else {
                    r_value = builder->CreateTrunc(r_value, l_type, "conv");
                }
            }
            builder->CreateStore(r_value, l_alloca_addr);
        } else if (assignment && ((Variable *) assigin_ast->left)->offset.empty()) {
            llvm::Function *the_scope = builder->GetInsertBlock()->getParent();

            llvm::Type *cast_to_type = r_type;
            string cast_to_name = l_base_name;
            auto l_excepted_type = ((Variable *) assigin_ast->left)->Ty.first;

            // if right value is an array initializer or another array
            if (
                    r_type->isPtrOrPtrVectorTy() &&
                    r_type->getPointerElementType()->isArrayTy()) {
                cast_to_type = r_type;
                cast_to_name = l_base_name + ".addr";

                if (
                        l_excepted_type->isPtrOrPtrVectorTy() &&
                        l_excepted_type->getPointerElementType()->isArrayTy() &&
                        isTheSameBasicType((llvm::PointerType *) r_type,
                                           (llvm::PointerType *) l_excepted_type)) {

                    if (
                            type_size[l_excepted_type->getPointerElementType()] <=
                            type_size[r_type->getPointerElementType()]) {
                        /*
                         * if excepted array size less than or equal right,
                         * create a pointer to right value
                         */
                        cast_to_name = l_base_name + ".cast.array2array";
                        cast_to_type = l_excepted_type;
                        r_value = builder->CreatePointerCast(r_value, cast_to_type);

                        llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, cast_to_name, cast_to_type);
                        builder->CreateStore(r_value, new_var_alloca);
                        symbol_table->insert(l_base_name, new_var_alloca);
                        goto assign_end;
                    } else {
                        /*
                         * if excepted array size greate than offered
                         * create a new array of size excepted
                         */
                        cast_to_type = l_excepted_type;

                        // new array init
                        llvm::AllocaInst *new_array_alloca = allocaBlockEntry(the_scope, "ArrayInit.cast.begin",
                                                                              l_excepted_type->getPointerElementType());
                        builder->CreateMemSet(
                                new_array_alloca,
                                llvm::ConstantInt::get(
                                        llvm::Type::getInt8Ty(*the_context),
                                        0),
                                type_size[l_excepted_type->getPointerElementType()],
                                llvm::MaybeAlign());

                        llvm::AllocaInst *new_ptr = allocaBlockEntry(the_scope, l_base_name + ".ArrayInit.Ptr",
                                                                     cast_to_type);
                        builder->CreateStore(new_array_alloca, new_ptr);

                        // copy array
                        builder->CreateMemCpy(new_array_alloca, llvm::MaybeAlign(), r_value, llvm::MaybeAlign(),
                                              type_size[r_type->getPointerElementType()]);

                        symbol_table->insert(l_base_name, new_ptr);
                        goto assign_end;
                    }
                } else {
                    if (l_excepted_type != VOID_TY) {
                        if (l_excepted_type->isPtrOrPtrVectorTy() &&
                            isTheSameBasicType((llvm::PointerType *) r_type, (llvm::PointerType *) l_excepted_type)) {
                            cast_to_type = l_excepted_type;
                            cast_to_name = l_base_name + ".cast.array2ptr";
                            r_value = builder->CreatePointerCast(r_value, l_excepted_type);
                        } else {
                            Warning(
                                    "failed to cast type '" +
                                    type_name[cast_to_type] +
                                    "' to '" + type_name[l_excepted_type] +
                                    "', the left type will be ignored ",
                                    assigin_ast->getToken().line, assigin_ast->getToken().column);
                        }
                    }
                    goto assign_begin;
                }
            } else {
                if (
                        r_type->isPtrOrPtrVectorTy() &&
                        l_excepted_type->isPtrOrPtrVectorTy() &&
                        l_excepted_type->getPointerElementType()->isArrayTy() &&
                        isTheSameBasicType((llvm::PointerType *) r_type,
                                           (llvm::PointerType *) l_excepted_type)) {
                    // if right is a normal pointer
                    cast_to_type = l_excepted_type;
                    r_value = builder->CreatePointerCast(r_value, cast_to_type);
                    cast_to_name = l_base_name + ".cast.ptr2array";
                } else {
                    if (
                            simple_types.find(l_excepted_type) != simple_types.end() &&
                            simple_types.find(r_type) != simple_types.end() &&
                            l_excepted_type != VOID_TY
                            ) {
                        cast_to_type = l_excepted_type;
                        bool is_l_fp = l_excepted_type->isFloatingPointTy();
                        bool is_r_fp = r_type->isFloatingPointTy();
                        if (simple_types_map[r_type] < simple_types_map[l_excepted_type]) {
                            if (is_r_fp && is_l_fp) {
                                r_value = builder->CreateFPExt(r_value, l_excepted_type, "conv");
                            } else if (!is_r_fp && is_l_fp) {
                                r_value = builder->CreateSIToFP(r_value, l_excepted_type, "conv");
                            } else {
                                r_value = builder->CreateSExt(r_value, l_excepted_type, "conv");
                            }
                        } else {
                            if (is_r_fp && is_l_fp) {
                                r_value = builder->CreateFPTrunc(r_value, l_excepted_type, "conv");
                            } else if (is_r_fp && !is_l_fp) {
                                r_value = builder->CreateFPToSI(r_value, l_excepted_type, "conv");
                            } else {
                                r_value = builder->CreateTrunc(r_value, l_excepted_type, "conv");
                            }
                        }
                    } else {
                        cast_to_type = r_type;
                        if(((Variable *) assigin_ast->left)->Ty.second != "none") {
                            Warning(
                                "failed to cast type '" +
                                type_name[cast_to_type] +
                                "' to '" + type_name[l_excepted_type] +
                                "', the left type will be ignored ",
                                assigin_ast->getToken().line, assigin_ast->getToken().column);
                        }
                    }
                }
                goto assign_begin;
            }

            assign_begin:
            llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, cast_to_name, cast_to_type);
            builder->CreateStore(r_value, new_var_alloca);
            symbol_table->insert(l_base_name, new_var_alloca);
        } else {
            throw ExceptionFactory<LogicException>(
                    "not matched type, left: " +
                    type_name[l_alloca_addr->getType()->getPointerElementType()] +
                    ", right: " + type_name[r_type],
                    ast->getToken().line, ast->getToken().column);
        }
        assign_end:
        return llvm::Constant::getNullValue(F64_TY);
    }

    llvm::Value *AST::codeGen() {
        return llvm::Constant::getNullValue(F64_TY);
    }

    llvm::Value *Assign::codeGen() {
        llvm::Value *r_value = this->right->codeGen();

        string
                l_base_name = ((Variable *)
                this->left)->id;
        auto l_alloca_addr = getAlloca((Variable *)
                                               this->left);

        return store(this, l_alloca_addr, r_value, true, l_base_name);
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

            llvm::Type *l_type = lv->getType();
            llvm::Type *r_type = rv->getType();

            if (
                    simple_types.find(l_type) == simple_types.end() ||
                    simple_types.find(r_type) == simple_types.end()) {
                return nullptr;
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
                          ? builder->CreateFPExt(lv, cast_to, "conv")
                          : builder->CreateSIToFP(lv, cast_to, "conv");
                rv_real = r_type->isFloatingPointTy()
                          ? builder->CreateFPExt(rv, cast_to, "conv")
                          : builder->CreateSIToFP(rv, cast_to, "conv");
            } else {
                lv = builder->CreateSExt(lv, cast_to, "conv");
                rv = builder->CreateSExt(rv, cast_to, "conv");
            }

            switch (this->op.getType()) {
                case PLUS:
                    if (float_point)
                        return builder->CreateFAdd(lv_real, rv_real, "addTmp");
                    else
                        return builder->CreateAdd(lv, rv, "addTmp");
                case MINUS:
                    if (float_point)
                        return builder->CreateFSub(lv_real, rv_real, "subTmp");
                    else
                        return builder->CreateSub(lv, rv, "subTmp");
                case STAR:
                    if (float_point)
                        return builder->CreateFMul(lv_real, rv_real, "mulTmp");
                    else if (float_point)
                        return builder->CreateMul(lv, rv, "mulTmp");
                case SLASH:
                    lv_real = l_type->isDoubleTy() ? lv : builder->CreateSIToFP(lv, F64_TY);
                    rv_real = r_type->isDoubleTy() ? rv : builder->CreateSIToFP(rv, F64_TY);
                    return builder->CreateFDiv(lv_real, rv_real, "divTmp");
                case EQ:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOEQ(lv_real, rv_real, "cmpEQTmp");
                    } else {
                        cmp_value_boolean = builder->CreateICmpEQ(lv, rv, "cmpEQTmp");
                    }
                    return cmp_value_boolean;
                case NE:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpONE(lv_real, rv_real, "cmpNETmp");
                    } else {
                        cmp_value_boolean = builder->CreateICmpNE(lv, rv, "cmpNETmp");
                    }
                    return cmp_value_boolean;
                case GT:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOGT(lv_real, rv_real, "cmpGTTmp");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSGT(lv, rv, "cmpGTTmp");
                    }
                    return cmp_value_boolean;
                case LT:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOLT(lv_real, rv_real, "cmpLTTmp");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSLT(lv, rv, "cmpLTTmp");
                    }
                    return cmp_value_boolean;
                case GE:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOGE(lv_real, rv_real, "cmpGETmp");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSGE(lv, rv, "cmpGETmp");
                    }
                    return cmp_value_boolean;
                case LE:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOLE(lv_real, rv_real, "cmpLETmp");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSLE(lv, rv, "cmpLETmp");
                    }
                    return cmp_value_boolean;
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
                auto l_phi_path = builder->GetInsertBlock();

                auto the_function = builder->GetInsertBlock()->getParent();
                auto Positive = llvm::BasicBlock::Create(*the_context, "land.lhs.true.rhs.head", the_function);
                auto Negative = llvm::BasicBlock::Create(*the_context, "land.end", the_function);

                if (l->getType() != I1_TY) {
                    if (l->getType()->isIntegerTy()) {
                        l = builder->CreateICmpNE(l, llvm::ConstantInt::get(l->getType(), 0), "toBool");
                    } else {
                        l = builder->CreateFCmpUNE(l, llvm::ConstantFP::get(F64_TY, 0.0), "toBool");
                    }
                }
                builder->CreateCondBr(l, Positive, Negative);

                the_function->getBasicBlockList().push_back(Positive);
                builder->SetInsertPoint(Positive);
                auto r = this->right->codeGen();
                auto r_phi_path = builder->GetInsertBlock();
                if (r->getType() != I1_TY) {
                    if (r->getType()->isIntegerTy()) {
                        r = builder->CreateICmpNE(r, llvm::ConstantInt::get(r->getType(), 0), "toBool");
                    } else {
                        r = builder->CreateFCmpUNE(r, llvm::ConstantFP::get(F64_TY, 0.0), "toBool");
                    }
                }
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
                auto l_phi_path = builder->GetInsertBlock();

                auto the_function = builder->GetInsertBlock()->getParent();
                auto Negative = llvm::BasicBlock::Create(*the_context, "lor.lhs.false.rhs.head", the_function);
                auto Positive = llvm::BasicBlock::Create(*the_context, "lor.end", the_function);

                if (l->getType() != I1_TY) {
                    if (l->getType()->isIntegerTy()) {
                        l = builder->CreateICmpNE(l, llvm::ConstantInt::get(l->getType(), 0), "toBool");
                    } else {
                        l = builder->CreateFCmpUNE(l, llvm::ConstantFP::get(F64_TY, 0.0), "toBool");
                    }
                }
                builder->CreateCondBr(l, Positive, Negative);

                the_function->getBasicBlockList().push_back(Negative);
                builder->SetInsertPoint(Negative);
                auto r = this->right->codeGen();
                auto r_phi_path = builder->GetInsertBlock();
                if (r->getType() != I1_TY) {
                    if (r->getType()->isIntegerTy()) {
                        r = builder->CreateICmpNE(r, llvm::ConstantInt::get(r->getType(), 0), "toBool");
                    } else {
                        r = builder->CreateFCmpUNE(r, llvm::ConstantFP::get(F64_TY, 0.0), "toBool");
                    }
                }
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
        llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(*the_context, "loop.body", the_function);
        llvm::BasicBlock *adjBB = llvm::BasicBlock::Create(*the_context, "loop.adjust", the_function);
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "loop.end", the_function);

        // not region of headBB, but symbol table should be pushed before initial list
        symbol_table->push(headBB);
        symbol_table->setLoopExit(mergeBB);
        symbol_table->setLoopEntry(adjBB);

        llvm::Value *start = this->initList->codeGen();
        if (!(((Compound *) (this->initList))->child.empty()) && (!start)) {
            return nullptr;
        }

        builder->CreateBr(headBB);

        the_function->getBasicBlockList().push_back(headBB);
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
        if (!(((Compound *) (this->compound))->child.empty()) && (!body)) {
            return nullptr;
        }
        symbol_table->pop();

        if (!builder->GetInsertBlock()->getTerminator()) {
            builder->CreateBr(adjBB);
        }

        the_function->getBasicBlockList().push_back(adjBB);
        builder->SetInsertPoint(adjBB);

        llvm::Value *adjust = this->adjustment->codeGen();
        if (!(((Compound *) (this->adjustment))->child.empty()) && (!adjust)) {
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

        return llvm::Constant::getNullValue(F64_TY);
    }

    llvm::Value *FunctionDecl::codeGen() {
        llvm::BasicBlock *last_BB = nullptr;
        llvm::BasicBlock::iterator last_pt;
        if (builder->GetInsertBlock() && builder->GetInsertBlock()->getParent() != nullptr) {
            last_BB = builder->GetInsertBlock();
            last_pt = builder->GetInsertPoint();
        }

        // create function parameters' type
        std::vector<llvm::Type *> Tys;
        for (Variable *i: ((Param *) (this->paramList))->paramList) {
            /* Passing array pointers between functions
             *
             * To be able to call external functions. Passing
             * array type by C ABI.
             *
             * e.g. [3 x double]* will be represented as double*
             * in function declaration
             */
            if (i->Ty.first->isArrayTy()) {
                Tys.push_back(getArrayBasicTypePointer(i->Ty.first->getPointerTo()));
            } else {
                Tys.push_back(i->Ty.first);
            }
        }

        llvm::Type *retTy = this->retTy.first->isArrayTy()
                            ? getArrayBasicTypePointer(this->retTy.first->getPointerTo())
                            : this->retTy.first;

        if (this->id == ENTRY_NAME || this->id == "_start") {
            retTy = llvm::Type::getInt32Ty(*the_context);
        }

        llvm::FunctionType *FT = llvm::FunctionType::get(
                retTy,
                Tys,
                false);

        string func_name =
                (this->id == ENTRY_NAME || !this->is_mangle)
                ? this->id
                : NAME_MANGLING(this->id);

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

            uint8_t param_index = 0;
            for (auto &arg: the_function->args()) {
                Variable *var = ((Param *)
                        this->paramList)->paramList[param_index++];
                arg.setName(var->id);
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
                llvm::AllocaInst *alloca = allocaBlockEntry(the_function, arg.getName().str(), arg.getType());
                builder->CreateStore(&arg, alloca);
                symbol_table->insert(arg.getName().str(), alloca);
            }

            if (this->compound->codeGen()) {
                auto endbb = builder->GetInsertBlock();
                auto t = endbb->getTerminator();
                if (!endbb->isEntryBlock() && !endbb->hasNPredecessorsOrMore(1)) {
                    endbb->eraseFromParent();
                } else if (!t) {
                    if (the_function->getReturnType() == VOID_TY) {
                        builder->CreateRetVoid();
                    } else if (the_function->getReturnType()->isFloatingPointTy()) {
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
                the_fpm->run(*the_function);
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
        string mod_path = getpathListToUnresolved(this->getToken().getModInfo());
        if (module_name_alias.find(mod_path) != module_name_alias.end()) {
            mod_path = module_name_alias[mod_path];
        }

        // get function with mangled name. if module get null function, try with non-mangled name again
        llvm::Function *fun = the_module->getFunction(
                getFunctionNameMangling(getpathUnresolvedToList(mod_path), this->id));
        if (!fun) fun = the_module->getFunction(this->id);

        if (!fun) {
            throw ExceptionFactory<MissingException>(
                    "function '" + this->id + "' is not declared",
                    this->getToken().line, this->getToken().column);
        }

        if (fun->arg_size() != this->paramList.size()) {
            // if function has declared, check parameters' size
            throw ExceptionFactory<LogicException>(
                    "candidate function not viable: requires " +
                    to_string(fun->arg_size()) +
                    " arguments, but " +
                    to_string(this->paramList.size()) +
                    " were provided",
                    this->token.line, this->token.column);
        }

        vector<llvm::Value *> args;
        auto callee_arg_iter = fun->args().begin();
        for (int i = 0; i < this->paramList.size(); i++) {
            AST *arg = paramList[i];
            llvm::Value *v = arg->codeGen();
            if (!v) {
                return nullptr;
            }

            llvm::Type *callee_type = callee_arg_iter->getType();
            llvm::Type *caller_type = v->getType();

            if (
                    caller_type->isPtrOrPtrVectorTy() &&
                    callee_type->isPtrOrPtrVectorTy() &&
                    isTheSameBasicType((llvm::PointerType *) caller_type,
                                       (llvm::PointerType *) callee_type)) {
                v = builder->CreatePointerCast(v, callee_type);
            } else if (
                    caller_type->isStructTy() &&
                    caller_type->isStructTy() &&
                    ((llvm::StructType *) caller_type)->isLayoutIdentical((llvm::StructType *) callee_type)) {
                v = getLoadStorePointerOperand(v);
                v = builder->CreatePointerCast(v, callee_type->getPointerTo());
                v = builder->CreateLoad(callee_type, v);
            } else if (callee_type != caller_type) {
                throw ExceptionFactory<LogicException>(
                        "unmatched type, provided: " +
                        type_name[caller_type] +
                        ", excepted: " + type_name[callee_arg_iter->getType()],
                        this->getToken().line, this->getToken().column);
            }

            args.push_back(v);
            callee_arg_iter++;
        }

        if (fun->getFunctionType()->getReturnType() != VOID_TY) {
            return builder->CreateCall(fun, args, "callLocal");
        } else {
            return builder->CreateCall(fun, args);
        }
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
        llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, this->id + "InitTemp", Ty);
        for (int i = 0; i < param_num; i++) {
            AST *param = this->paramList[i];
            llvm::Value *rv = param->codeGen();

            auto member_addr = builder->CreateGEP(
                    llvm::cast<llvm::PointerType>(new_var_alloca->getType()->getScalarType())->getElementType(),
                    new_var_alloca,
                    {
                            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), 0),
                            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), i),
                    },
                    id + ".member." + to_string(i));

            store(this, member_addr, rv, false, id + ".member." + to_string(i));
        }

        return builder->CreateLoad(Ty, new_var_alloca);
    }

    llvm::Value *TypeTrans::codeGen() {
        auto v = this->factor->codeGen();

        auto vtype = v->getType();
        auto etype = this->Ty.first;

        if (vtype == etype) return v;

        if (etype == VOID_TY) {
            throw ExceptionFactory<LogicException>(
                    "void type is not allowed",
                    this->getToken().column, this->getToken().column);
        }

        bool is_v_simple = simple_types.find(vtype) != simple_types.end();
        bool is_e_simple = simple_types.find(etype) != simple_types.end();

        if (is_v_simple && is_e_simple) {
            bool is_v_fp = vtype->isFloatingPointTy();
            bool is_e_fp = etype->isFloatingPointTy();
            if (simple_types_map[vtype] < simple_types_map[etype]) {
                if (is_v_fp && is_e_fp) {
                    return builder->CreateFPExt(v, etype, "conv");
                } else if (!is_v_fp && is_e_fp) {
                    return builder->CreateSIToFP(v, etype, "conv");
                } else {
                    return builder->CreateSExt(v, etype, "conv");
                }
            } else {
                if (is_v_fp && is_e_fp) {
                    return builder->CreateFPTrunc(v, etype, "conv");
                } else if (is_v_fp && !is_e_fp) {
                    return builder->CreateFPToSI(v, etype, "conv");
                } else {
                    return builder->CreateTrunc(v, etype, "conv");
                }
            }
        } else if (!(is_v_simple ^ is_e_simple)) {
            bool is_v_ptr = vtype->isPtrOrPtrVectorTy();
            bool is_e_ptr = etype->isPtrOrPtrVectorTy();

            if (is_v_ptr && is_e_ptr) {
                return builder->CreatePointerBitCastOrAddrSpaceCast(v, etype, "conv");
            } else {
                throw ExceptionFactory<LogicException>(
                        "undefined cast '" + type_name[vtype] + "' to '" + type_name[etype] + "'",
                        this->getToken().column, this->getToken().column);
            }
        } else {
            bool is_v_ptr = vtype->isPtrOrPtrVectorTy();
            bool is_e_ptr = etype->isPtrOrPtrVectorTy();

            if (is_e_ptr) {
                return builder->CreateIntToPtr(v, etype);
            } else if (is_v_ptr) {
                return builder->CreatePtrToInt(v, etype);
            }

            throw ExceptionFactory<LogicException>(
                    "undefined cast '" + type_name[vtype] + "' to '" + type_name[etype] + "'",
                    this->getToken().column, this->getToken().column);
        }
    }

    llvm::Value *ArrayInit::codeGen() {
        llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
        uint32_t element_num = this->num;

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
            llvm::AllocaInst *ptr_alloca = allocaBlockEntry(the_scope, "ArrayInitTemp", contain_type);
            return ptr_alloca;
        }

        if (this->paramList.size() == 0 && this->Ty.first == VOID_TY) {
            throw ExceptionFactory<LogicException>(
                    "array with void type is not allowed",
                    this->getToken().column, this->getToken().column);
        }

        /**
         * create array by type, return a array initialized by 0.
         */
        if (this->Ty.first != VOID_TY) {
            auto arr_type = llvm::ArrayType::get(this->Ty.first, element_num);
            type_name[arr_type] = "vec[" + type_name[this->Ty.first] + ";" + to_string(element_num) + "]";
            type_name[arr_type->getPointerTo()] =
                    "vec[" + type_name[this->Ty.first] + ";" + to_string(element_num) + "]*";
            type_size[arr_type] = type_size[this->Ty.first] * element_num;
            type_size[arr_type->getPointerTo()] = PTR_SIZE;
            llvm::AllocaInst *array_alloca = allocaBlockEntry(the_scope, "ArrayInit.by_type", arr_type);
            builder->CreateMemSet(
                    array_alloca,
                    llvm::ConstantInt::get(
                            llvm::Type::getInt8Ty(*the_context),
                            0),
                    type_size[arr_type],
                    llvm::MaybeAlign());

            return array_alloca;
        }

        bool is_const_array = true;
        bool is_char_array = true;

        // check type of array initializer
        for (auto i: this->paramList) {
            if (i->__AST_name != __NUM_NAME) {
                is_const_array = false;
                break;
            }
            if (((Num *) i)->getToken().getType() != CHAR) {
                is_char_array = false;
            }
        }

        if (is_const_array) {
            // create a constant data array for constant array
            llvm::Constant *arr = nullptr;
            llvm::Type *tname;
            int tsize;
            if (is_char_array) {
                string str;
                for (auto i: this->paramList) {
                    str += ((Num *) i)->getToken().getValue().any_cast<char>();
                }
                arr = llvm::ConstantDataArray::getString(*the_context, str);
                tname = I8_TY;
                tsize = type_size[I8_TY];
            } else {
                vector<double> data;
                for (auto i: this->paramList) {
                    data.push_back(((Num *) i)->getToken().getValue().any_cast<double>());
                }
                arr = llvm::ConstantDataArray::get(*the_context, data);
                tname = F64_TY;
                tsize = type_size[F64_TY];
            }

            llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
            auto addr_alloca = allocaBlockEntry(the_scope, "const.array", arr->getType());
            builder->CreateStore(arr, addr_alloca);

            type_name[arr->getType()] = "vec[" + type_name[tname] + ";" + to_string(element_num) + "]";
            type_name[arr->getType()->getPointerTo()] = "vec[" + type_name[tname] + ";" + to_string(element_num) + "]*";
            type_size[arr->getType()] = tsize * element_num;

            return addr_alloca;
        } else {
            // else initialize an array by store action

            llvm::Value *head_rv = this->paramList[0]->codeGen();
            auto eleTy = head_rv->getType();
            auto arr_type = llvm::ArrayType::get(eleTy, element_num);

            type_name[arr_type] = "vec[" + type_name[eleTy] + ";" + to_string(element_num) + "]";
            type_name[arr_type->getPointerTo()] = "vec[" + type_name[eleTy] + ";" + to_string(element_num) + "]*";
            type_size[arr_type] = (eleTy->isPointerTy() ? PTR_SIZE : type_size[eleTy]) * element_num;
            // initialize array
            llvm::AllocaInst *array_alloca = allocaBlockEntry(the_scope, "ArrayInit.begin", arr_type);

            // store first element
            auto first_element_addr = builder->CreateGEP(
                    llvm::cast<llvm::PointerType>(array_alloca->getType()->getScalarType())->getElementType(),
                    array_alloca,
                    {
                            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), 0),
                            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), 0),
                    },
                    "ArrayInit.element." + to_string(0));
            builder->CreateStore(head_rv, first_element_addr);

            // initialize other elements
            for (int i = 1; i < element_num; i++) {
                AST *param = this->paramList[i];
                llvm::Value *rv = param->codeGen();
                auto element_addr = builder->CreateGEP(
                        llvm::cast<llvm::PointerType>(array_alloca->getType()->getScalarType())->getElementType(),
                        array_alloca,
                        {
                                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), 0),
                                llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), i),
                        },
                        "ArrayInit.element." + to_string(i));

                if (rv->getType() != eleTy) {
                    throw ExceptionFactory<LogicException>(
                            "not matched type, element type: " +
                            type_name[rv->getType()] +
                            ", array type: " + type_name[eleTy],
                            this->getToken().line, this->getToken().column);
                }

                builder->CreateStore(rv, element_addr);
            }
            return array_alloca;
        }
    }

    llvm::Value *Global::codeGen() {
        auto *v = (Variable *)
                this->var;
        string name = v->id;

        if (v->Ty.first == VOID_TY) {
            throw ExceptionFactory<LogicException>(
                    "missing type of global variable '" + name + "'",
                    this->getToken().column, this->getToken().column);
        }

        auto link_type =
                this->is_export
                ? llvm::GlobalVariable::ExternalLinkage
                : llvm::GlobalVariable::PrivateLinkage;

        the_module->getOrInsertGlobal(
                NAME_MANGLING(name), v->Ty.first,
                [v, link_type, name] {
                    return new llvm::GlobalVariable(
                            *the_module,
                            v->Ty.first,
                            false,
                            link_type,
                            llvm::Constant::getNullValue(v->Ty.first),
                            NAME_MANGLING(name));
                });

        return llvm::Constant::getNullValue(F64_TY);
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

                llvm::BasicBlock *loop_mergeBB = symbol_table->getLoopExit();

                bool have_merge = false;

                llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(*the_context, "if.then", the_function,
                                                                    loop_mergeBB);
                llvm::BasicBlock *elseBB = nullptr;
                if (this->next != ASTEmpty)
                    elseBB = llvm::BasicBlock::Create(*the_context, "if.else", the_function, loop_mergeBB);
                llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "if.end", the_function,
                                                                     loop_mergeBB);

                if (elseBB != nullptr) {
                    builder->CreateCondBr(cond, thenBB, elseBB);
                } else {
                    have_merge = true;
                    builder->CreateCondBr(cond, thenBB, mergeBB);
                }

                the_function->getBasicBlockList().push_back(thenBB);
                builder->SetInsertPoint(thenBB);
                symbol_table->push(thenBB);

                // // I don't know why the instruction must be placed here.
                // // Removing it will result in segmentation fault
                //  auto ph = builder->CreateICmpNE(cond, llvm::ConstantInt::get(cond->getType(), 0), "placeholder");
                //  UNUSED(ph);
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
                thenBB = builder->GetInsertBlock();

                llvm::Value *elsev;
                llvm::Instruction *t_else = nullptr;
                if (elseBB != nullptr) {
                    the_function->getBasicBlockList().push_back(elseBB);
                    builder->SetInsertPoint(elseBB);
                    symbol_table->push(elseBB);
                    //  ph = builder->CreateICmpNE(cond, llvm::ConstantInt::get(cond->getType(), 0), "placeholder");
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
                    elseBB = builder->GetInsertBlock();
                }

                if (have_merge) {
                    the_function->getBasicBlockList().push_back(mergeBB);
                    builder->SetInsertPoint(mergeBB);
                } else {
                    mergeBB->eraseFromParent();
                }

                return llvm::Constant::getNullValue(F64_TY);
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
        return llvm::Constant::getNullValue(F64_TY);
    }

    llvm::Value *Num::codeGen() {
        //        auto t = llvm::ConstantFP::get(*the_context, llvm::APFloat((double)this->value));
        if (this->token.getType() == CHAR) {
            return llvm::ConstantInt::get(I8_TY, this->getValue().any_cast<char>());
        } else {
            if (this->getValue().type() == Float) {
                return llvm::ConstantFP::get(F32_TY,
                                             this->getValue().any_cast<double>());
            } else {
                return llvm::ConstantInt::get(I32_TY,
                                              this->getValue().any_cast<int>());
            }
        }
    }

    llvm::Value *Object::codeGen() {
        return llvm::Constant::getNullValue(F64_TY);
    }

    llvm::Value *UnaryOp::codeGen() {
        llvm::Value *rv = this->right->codeGen();
        if (!rv) {
            throw ExceptionFactory<LogicException>(
                    "unknown right value of unary op",
                    this->token.line, this->token.column);
        }

        if (this->op.getType() == MINUS) {
            return builder->CreateFSub(
                    llvm::ConstantFP::get(F64_TY, 0.0),
                    rv,
                    "unaryAddTmp");
        } else if (this->op.getType() == PLUS) {
            return builder->CreateFAdd(
                    llvm::ConstantFP::get(F64_TY, 0.0),
                    rv,
                    "unarySubTmp");
        } else if (this->op.getType() == NOT) {
            llvm::Value *rv_bool = builder->CreateFCmpUNE(rv, llvm::ConstantFP::get(F64_TY, 0.0), "toBool");
            return builder->CreateNot(rv_bool, "unaryNotTmp");
        } else {
            return nullptr;
        }
    }

    llvm::Value *Sizeof::codeGen() {
        if (this->id != nullptr) {
            llvm::Value *v = getAlloca((Variable *)
                                               this->id);

            if (!v) {
                throw ExceptionFactory<MissingException>(
                        "variable '" + ((Variable *) (this->id))->id + "' is not defined",
                        this->token.line, this->token.column);
            }

            llvm::Type *type = v->getType()->getPointerElementType();

            if (type->isPtrOrPtrVectorTy()) {
                if (type->getPointerElementType()->isArrayTy()) {
                    // array type
                    return llvm::ConstantInt::get(I32_TY, type_size[type->getPointerElementType()]);
                } else {
                    // raw pointer
                    return llvm::ConstantInt::get(I32_TY, PTR_SIZE);
                }
            } else {
                // non-pointer type
                return llvm::ConstantInt::get(I32_TY, type_size[type]);
            }
        }

        return llvm::ConstantInt::get(I32_TY, type_size[this->Ty.first]);
    }

    llvm::Value *String::codeGen() {
        llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
        auto value = llvm::ConstantDataArray::getString(*the_context, this->getValue().any_cast<string>());
        llvm::AllocaInst *ptr_alloca = allocaBlockEntry(the_scope, "String", value->getType());
        builder->CreateStore(value, ptr_alloca);

        llvm::Type *Ty = value->getType();
        type_name[Ty] = "vec[char;" + to_string(this->getValue().any_cast<string>().length() + 1) + "]";
        type_name[Ty->getPointerTo()] = type_name[Ty] + "*";
        type_size[Ty] = this->getValue().any_cast<string>().length() + 1;
        return ptr_alloca;
    }

    llvm::Value *Variable::codeGen() {
        llvm::Value *v = getAlloca(this);

        if (!v) {
            throw ExceptionFactory<MissingException>(
                    "variable '" + this->id + "' is not defined",
                    this->token.line, this->token.column);
        }

        // if v is an array pointer, return pointer directly
        if (v->getType()->getPointerElementType()->isArrayTy()) {
            return v;
        }

        return builder->CreateLoad(v->getType()->getPointerElementType(), v, this->id.c_str());
    }

    llvm::Value *Compound::codeGen() {
        int err_count = 0;
        bool is_err = false;

        size_t cnt = 0;
        size_t child_size = this->child.size();
        for (AST *ast: this->child) {
            try {
                ast->codeGen();
                cnt++;
                if (
                        cnt != child_size &&
                        ast->__AST_name != __FUNCTIONDECL_NAME &&
                        builder->GetInsertBlock() &&
                        builder->GetInsertBlock()->getTerminator()) {
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

        return llvm::Constant::getNullValue(F64_TY);
    }

    llvm::Value *Return::codeGen() {
        if (this->ret != nullptr) {
            llvm::Value *re = this->ret->codeGen();
            if (!re) {
                return nullptr;
            }

            if (re->getType()->isArrayTy()) {
                re = getLoadStorePointerOperand(re);
            }

            llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
            auto ret_alloca_addr = allocaBlockEntry(the_scope, "ret", the_scope->getReturnType());
            store(this, ret_alloca_addr, re, true, "ret");
            re = builder->CreateLoad(the_scope->getReturnType(), ret_alloca_addr);
            return builder->CreateRet(re);
        } else {
            return builder->CreateRetVoid();
        }
    }

    llvm::Value *While::codeGen() {
        llvm::Function *the_function = builder->GetInsertBlock()->getParent();

        llvm::BasicBlock *headBB = llvm::BasicBlock::Create(*the_context, "loop.head", the_function);
        llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(*the_context, "loop.body", the_function);
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "loop.end", the_function);

        builder->CreateBr(headBB);

        the_function->getBasicBlockList().push_back(headBB);
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

        if (!mergeBB->getUniquePredecessor()) {
            if (opt_warning) {
                Warning(
                        "endless loop",
                        this->getToken().line,
                        this->getToken().column
                );
            }
        }

        return llvm::Constant::getNullValue(F64_TY);
    }

    llvm::Value *NoneAST::codeGen() {
        return llvm::Constant::getNullValue(F64_TY);
    }
}
