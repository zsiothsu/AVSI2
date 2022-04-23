/*
 * @Author: Chipen Hsiao
 * @Date: 2022-03-11
 * @Description: llvm code generator
 */
#include <cstdlib>
#include <set>

#include "../inc/AST.h"
#include "../inc/SymbolTable.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

namespace AVSI {
    /*******************************************************
     *                      llvm base                      *
     *******************************************************/
//    static unique_ptr<llvm::LLVMContext> the_context = make_unique<llvm::LLVMContext>();
//    static unique_ptr<llvm::Module> the_module = make_unique<llvm::Module>("program", *the_context);
    llvm::LLVMContext *the_context = new llvm::LLVMContext();
    llvm::Module *the_module = new llvm::Module("program", *the_context);
    static unique_ptr<llvm::IRBuilder<>> builder = make_unique<llvm::IRBuilder<>>(*the_context);
    static unique_ptr<llvm::legacy::FunctionPassManager> the_fpm = make_unique<llvm::legacy::FunctionPassManager>(
            the_module);
    llvm::TargetMachine *TheTargetMachine = nullptr;

    /*******************************************************
     *               protos & definition                   *
     *******************************************************/
    llvm::Type *REAL_TY = llvm::Type::getDoubleTy(*the_context);
    llvm::Type *VOID_TY = llvm::Type::getVoidTy(*the_context);
    llvm::Type *BOOL_TY = llvm::Type::getInt1Ty(*the_context);

    SymbolTable *symbol_table = new SymbolTable();
    map<string, StructDef *> struct_types;
    map<std::string, llvm::FunctionType *> function_protos;

    set<string> simple_types = {"void", "real", "vec"};
    map<llvm::Type *, string> type_name = {
            {REAL_TY, "real"},
            {VOID_TY, "void"},
            {BOOL_TY, "bool"}
    };
    map<llvm::Type *, uint32_t> type_size = {
            {REAL_TY, 8},
            {VOID_TY, 0},
            {BOOL_TY, 1}
    };

    /*******************************************************
     *                     function                        *
     *******************************************************/
    void llvm_module_fpm_init() {
//        the_fpm->add(llvm::createReassociatePass());
//        the_fpm->add(llvm::createGVNPass());
//        the_fpm->add(llvm::createInstructionCombiningPass());
//        the_fpm->add(llvm::createCFGSimplificationPass());
//        the_fpm->add(llvm::createDeadCodeEliminationPass());
//        the_fpm->add(llvm::createFlattenCFGPass());

        the_fpm->doInitialization();
    }

    void llvm_machine_init() {
        // Initialize the target registry etc.
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        auto TargetTriple = llvm::sys::getDefaultTargetTriple();
        the_module->setTargetTriple(TargetTriple);

        std::string Error;
        auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

        // Print an error and exit if we couldn't find the requested target.
        // This generally occurs if we've forgotten to initialise the
        // TargetRegistry or we have a bogus target triple.
        if (!Target) {
            llvm::errs() << Error;
            return;
        }

        auto CPU = "generic";
        auto Features = "";

        llvm::TargetOptions opt;
        auto RM = llvm::Optional<llvm::Reloc::Model>();
        TheTargetMachine =
                Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

        the_module->setDataLayout(TheTargetMachine->createDataLayout());
    }

    void llvm_obj_output() {
        auto Filename = "a.o";
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message();
            return;
        }

        llvm::legacy::PassManager pass;
        auto FileType = llvm::CGFT_ObjectFile;

        if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
            llvm::errs() << "TheTargetMachine can't emit a file of this type";
            return;
        }

        pass.run(*the_module);
        dest.flush();

        llvm::outs() << "Wrote " << Filename << "\n";
    }

    void llvm_asm_output() {
        auto Filename = "a.s";
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message();
            return;
        }

        llvm::legacy::PassManager pass;
        auto FileType = llvm::CGFT_AssemblyFile;

        if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
            llvm::errs() << "TheTargetMachine can't emit a file of this type";
            return;
        }

        pass.run(*the_module);
        dest.flush();

        llvm::outs() << "Wrote " << Filename << "\n";
    }

    void llvm_module_printIR() {
        auto Filename = "a.ll";
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);
        the_module->print(dest, nullptr);
        llvm::outs() << "Wrote " << Filename << "\n";
    }

    void debug_type(llvm::Value *v) {
        if (!v) return;
        v->getType()->print(llvm::outs());
        cout << endl;
    }

    void debug_type(llvm::Type *v) {
        v->print(llvm::outs());
        cout << endl;
    }

    /*******************************************************
     *                    IR generator                     *
     *******************************************************/
    llvm::PointerType *getArrayBasicTypePointer(llvm::PointerType *Ty) {
        llvm::Type *basic = Ty->getPointerElementType();
        while (basic->isArrayTy()) basic = basic->getArrayElementType();
        return basic->getPointerTo();
    }

    bool isTheSameBasicType(llvm::PointerType *l, llvm::PointerType *r) {
        llvm::Type *basicl = l->getPointerElementType();
        llvm::Type *basicr = r->getPointerElementType();

        while (basicl->isArrayTy()) basicl = basicl->getArrayElementType();
        while (basicr->isArrayTy()) basicr = basicr->getArrayElementType();

        return basicl == basicr;
    }

    llvm::AllocaInst *allocaBlockEntry(llvm::Function *fun, string name, llvm::Type *Ty) {
        llvm::IRBuilder<> blockEntry(
                &fun->getEntryBlock(),
                fun->getEntryBlock().begin()
        );

        return blockEntry.CreateAlloca(
                Ty,
                0,
                name.c_str()
        );
    }

    llvm::Value *getAlloca(Variable *var) {
        llvm::Value *v = symbol_table->find(var->id);

        if (!v) {
            v = the_module->getGlobalVariable(var->id);
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
                        true)
                );
                if (i.first == Variable::ARRAY) {
                    llvm::Value *index = i.second->codeGen();
                    index = builder->CreateFPToSI(index, llvm::Type::getInt32Ty((*the_context)));
                    offset_list.push_back(index);

                    try {
                        v = builder->CreateGEP(
                                llvm::cast<llvm::PointerType>(v->getType()->getScalarType())->getElementType(),
                                v,
                                offset_list,
                                var->id
                        );
                    } catch (...) {
                        throw ExceptionFactory(
                                __TypeException,
                                "index out of range or not an array",
                                i.second->getToken().line, i.second->getToken().column
                        );
                    }
                } else {
                    string member_name = ((Variable *) (i.second))->id;
                    auto current_ty = v->getType()->getPointerElementType();

                    bool find_flag = false;
                    for (auto iter: struct_types) {
                        if (iter.second->Ty == current_ty) {
                            auto index = iter.second->members[member_name];
                            offset_list.push_back(llvm::ConstantInt::get(
                                    llvm::Type::getInt32Ty(*the_context),
                                    index,
                                    true)
                            );
                            find_flag = true;
                            break;
                        }
                    }

                    if (find_flag) {
                        v = builder->CreateGEP(
                                llvm::cast<llvm::PointerType>(v->getType()->getScalarType())->getElementType(),
                                v,
                                offset_list,
                                var->id
                        );
                    } else {
                        throw ExceptionFactory(
                                __MissingException,
                                "unrecognized member '" + member_name + "'",
                                i.second->getToken().line, i.second->getToken().column
                        );
                    }
                }
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


    llvm::Value *AST::codeGen() {
        return llvm::Constant::getNullValue(REAL_TY);
    }

    llvm::Value *Assign::codeGen() {
        llvm::Value *rv = this->right->codeGen();
        llvm::Type *Ty = rv->getType();


        if (Ty == VOID_TY) {
            throw ExceptionFactory(
                    __LogicException,
                    "cannot assign void to variable",
                    this->getToken().line, this->getToken().column
            );
        }

        string lname = ((Variable *) this->left)->id;

        auto pre_allocated = getAlloca((Variable *) this->left);
        llvm::Type *pre_allocated_type = pre_allocated ? pre_allocated->getType()->getPointerElementType() : nullptr;

        if (pre_allocated != nullptr && pre_allocated_type->isPtrOrPtrVectorTy() &&
            Ty->isPtrOrPtrVectorTy() &&
            isTheSameBasicType((llvm::PointerType *) pre_allocated_type, (llvm::PointerType *) Ty)) {
            /*
             * An array pointer assigned to another pointer.
             * But in AVSI, high-dimensional array is not an
             * array of pointer of low-dimensional array,
             * so the role of assign is copy right array to left
             */
            llvm::Value *dest = builder->CreateLoad(pre_allocated->getType()->getPointerElementType(), pre_allocated,
                                                    "dest.addr");
            builder->CreateMemCpy(dest, llvm::MaybeAlign(), rv, llvm::MaybeAlign(),
                                  type_size[dest->getType()->getPointerElementType()]);
        } else if (pre_allocated != nullptr && pre_allocated_type == Ty) {
            builder->CreateStore(rv, pre_allocated);
        } else if (pre_allocated != nullptr && Ty->isArrayTy() &&
                   pre_allocated_type == Ty->getPointerTo()) {
            /*
             * There is only one case where Ty is an array type is rv is an array initializer
             * that return a raw array.
             * Assign a pointer of raw array to left
             */
            builder->CreateStore(getLoadStorePointerOperand(rv), pre_allocated);
        } else if (((Variable *) this->left)->offset.empty()) {
            llvm::Function *the_scope = builder->GetInsertBlock()->getParent();

            llvm::Type *to_type = nullptr;
            string to_name = lname;

            // if right value is an array initializer
            if (Ty->isArrayTy()) {
                to_type = Ty->getPointerTo();
                rv = getLoadStorePointerOperand(rv);
                to_name = lname + ".addr";

                if (((Variable *) this->left)->Ty.first->isArrayTy() &&
                    isTheSameBasicType((llvm::PointerType *) to_type,
                                       (llvm::PointerType *) ((Variable *) this->left)->Ty.first->getPointerTo())) {
                    /*
                     * if excepted array size less than or equal right,
                     * create a pointer to right value
                     */
                    if (type_size[((Variable *) this->left)->Ty.first] <= type_size[Ty]) {
                        to_type = ((Variable *) this->left)->Ty.first->getPointerTo();
                        rv = builder->CreatePointerCast(rv, to_type);
                        to_name = lname + ".cast";
                        llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, to_name, to_type);
                        symbol_table->insert(lname, new_var_alloca);
                        builder->CreateStore(rv, new_var_alloca);
                        goto assign_end;
                    }
                    /*
                     * if excepted array size greate than offered
                     * create a new array of size excepted
                     */
                    else {
                        to_type = ((Variable *) this->left)->Ty.first->getPointerTo();

                        // new array init
                        llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, "ArrayInit.cast.begin",
                                                                            ((Variable *) this->left)->Ty.first);
                        builder->CreateMemSet(
                                new_var_alloca,
                                llvm::ConstantInt::get(
                                        llvm::Type::getInt8Ty(*the_context),
                                        0
                                ),
                                type_size[((Variable *) this->left)->Ty.first],
                                llvm::MaybeAlign()
                        );
                        llvm::AllocaInst *ptr = allocaBlockEntry(the_scope, "ArrayInit.Ptr", to_type);
                        // copy array
                        builder->CreateMemCpy(new_var_alloca, llvm::MaybeAlign(), rv, llvm::MaybeAlign(),
                                              type_size[Ty]);
                        goto assign_end;
                    }

                } else {
                    if (((Variable *) this->left)->Ty.first != VOID_TY) {
                        Warning(
                                __Warning,
                                "failed to cast type '" +
                                type_name[to_type] +
                                "' to '"
                                + type_name[((Variable *) this->left)->Ty.first->getPointerTo()] +
                                "', the left type will be ignored ",
                                this->getToken().line, this->getToken().column
                        );
                    }
                    goto assign_begin;
                }
            } else {
                if (Ty->isPtrOrPtrVectorTy() &&
                    ((Variable *) this->left)->Ty.first->isArrayTy() &&
                    isTheSameBasicType((llvm::PointerType *) Ty,
                                       (llvm::PointerType *) ((Variable *) this->left)->Ty.first->getPointerTo())) {
                    to_type = ((Variable *) this->left)->Ty.first->getPointerTo();
                    rv = builder->CreatePointerCast(rv, to_type);
                    to_name = lname + ".cast";
                } else {
                    to_type = Ty;
                }
                goto assign_begin;
            }

            assign_begin:
            llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, to_name, to_type);
            symbol_table->insert(lname, new_var_alloca);
            builder->CreateStore(rv, new_var_alloca);
        } else {
            throw ExceptionFactory(
                    __LogicException,
                    "not matched type, left: " + \
                    type_name[pre_allocated->getType()->getPointerElementType()] + \
                    ", right: " + type_name[Ty],
                    this->getToken().line, this->getToken().column
            );
        }
        assign_end:
        return llvm::Constant::getNullValue(REAL_TY);
    }

    llvm::Value *BinOp::codeGen() {
        llvm::Value *lv = this->left->codeGen();
        llvm::Value *rv = this->right->codeGen();

        if (!lv || !rv) {
            return nullptr;
        }

        llvm::Value *cmp_value_boolean;
        llvm::Value *lv_bool;
        llvm::Value *rv_bool;
        switch (this->op.getType()) {
            case PLUS:
                return builder->CreateFAdd(lv, rv, "addTmp");
            case MINUS:
                return builder->CreateFSub(lv, rv, "subTmp");
            case STAR:
                return builder->CreateFMul(lv, rv, "mulTmp");
            case SLASH:
                return builder->CreateFDiv(lv, rv, "divTmp");
            case EQ:
                cmp_value_boolean = builder->CreateFCmpUEQ(lv, rv, "cmpEQTmp");
                return builder->CreateUIToFP(cmp_value_boolean, REAL_TY, "boolTmp");
            case NE:
                cmp_value_boolean = builder->CreateFCmpUNE(lv, rv, "cmpNETmp");
                return builder->CreateUIToFP(cmp_value_boolean, REAL_TY, "boolTmp");
            case GT:
                cmp_value_boolean = builder->CreateFCmpUGT(lv, rv, "cmpGTTmp");
                return builder->CreateUIToFP(cmp_value_boolean, REAL_TY, "boolTmp");
            case LT:
                cmp_value_boolean = builder->CreateFCmpULT(lv, rv, "cmpLTTmp");
                return builder->CreateUIToFP(cmp_value_boolean, REAL_TY, "boolTmp");
            case GE:
                cmp_value_boolean = builder->CreateFCmpUGE(lv, rv, "cmpGETmp");
                return builder->CreateUIToFP(cmp_value_boolean, REAL_TY, "boolTmp");
            case LE:
                cmp_value_boolean = builder->CreateFCmpULE(lv, rv, "cmpLETmp");
                return builder->CreateUIToFP(cmp_value_boolean, REAL_TY, "boolTmp");
            case OR:
                lv_bool = builder->CreateFPToUI(lv, BOOL_TY, "boolTmp");
                rv_bool = builder->CreateFPToUI(rv, BOOL_TY, "boolTmp");
                cmp_value_boolean = builder->CreateOr(lv_bool, rv_bool, "boolOrTmp");
//                cmp_value_boolean = builder->CreateOr(lv, rv, "boolOrTmp");
                return builder->CreateUIToFP(cmp_value_boolean, REAL_TY, "boolTmp");
            case AND:
                lv_bool = builder->CreateFPToUI(lv, BOOL_TY, "boolTmp");
                rv_bool = builder->CreateFPToUI(rv, BOOL_TY, "boolTmp");
                cmp_value_boolean = builder->CreateAnd(lv_bool, rv_bool, "boolAndTmp");
//                cmp_value_boolean = builder->CreateAnd(lv, rv, "boolAndTmp");
                return builder->CreateUIToFP(cmp_value_boolean, REAL_TY, "boolTmp");
            default:
                return nullptr;
        }
    }

    llvm::Value *Boolean::codeGen() {
        if (this->value == true) {
            return llvm::ConstantFP::get(REAL_TY, 1.0);
        } else {
            return llvm::ConstantFP::get(REAL_TY, 0.0);
        }
    }

    llvm::Value *For::codeGen() {
        llvm::Value *start = this->initList->codeGen();
        if (!start) {
            return nullptr;
        }

        llvm::Function *the_function = builder->GetInsertBlock()->getParent();

        llvm::BasicBlock *headBB = llvm::BasicBlock::Create(*the_context, "loop.head", the_function);
        llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(*the_context, "loop.body", the_function);
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "loop.end", the_function);

        builder->CreateBr(headBB);

        the_function->getBasicBlockList().push_back(headBB);
        builder->SetInsertPoint(headBB);

        symbol_table->push(headBB);
        llvm::Value *cond = this->condition->codeGen();
        if (!cond) {
            return nullptr;
        }

        cond = builder->CreateFCmpONE(cond, llvm::ConstantFP::get(*the_context, llvm::APFloat(0.0)));
        llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
        llvm::IRBuilder<> blockEntry(
                &the_scope->getEntryBlock(),
                the_scope->getEntryBlock().begin()
        );
        llvm::AllocaInst *new_var_alloca = blockEntry.CreateAlloca(
                BOOL_TY,
                0,
                "ifCondAlloca"
        );
        builder->CreateStore(cond, new_var_alloca);

        cond = builder->CreateLoad(BOOL_TY, new_var_alloca);
        builder->CreateCondBr(cond, loopBB, mergeBB);
        headBB = builder->GetInsertBlock();

        the_function->getBasicBlockList().push_back(loopBB);
        builder->SetInsertPoint(loopBB);

        llvm::Value *body = this->compound->codeGen();
        if (!body) {
            return nullptr;
        }
        llvm::Value *adjust = this->adjustment->codeGen();
        if (!adjust) {
            return nullptr;
        }
        symbol_table->pop();

        auto t = builder->GetInsertBlock()->getTerminator();
        if (!t) {
            builder->CreateBr(headBB);
        }
        loopBB = builder->GetInsertBlock();

        the_function->getBasicBlockList().push_back(mergeBB);
        builder->SetInsertPoint(mergeBB);

        return llvm::Constant::getNullValue(REAL_TY);
    }

    llvm::Value *FunctionDecl::codeGen() {
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

        llvm::FunctionType *FT = llvm::FunctionType::get(
                this->retTy.first->isArrayTy()
                ? getArrayBasicTypePointer(this->retTy.first->getPointerTo())
                : this->retTy.first,
                Tys,
                false
        );

        if (the_module->getFunction(this->id) == nullptr) {
            // check is re-definition
            auto funType = function_protos.find(this->id);
            if (funType != function_protos.end() && (
                    (funType->second != FT) || (the_module->getFunction(this->id) != nullptr)
            )) {
                throw ExceptionFactory(
                        __LogicException,
                        "function redefined there",
                        this->token.line, this->token.column
                );
            }

            // create function
            llvm::Function *the_function = llvm::Function::Create(
                    FT,
                    llvm::Function::ExternalLinkage,
                    this->id,
                    the_module
            );
            function_protos[this->id] = FT;

            uint8_t param_index = 0;
            for (auto &arg: the_function->args()) {
                Variable *var = ((Param *)
                        this->paramList)->paramList[param_index++];
                arg.setName(var->id);
            }
        }

        if (this->compound != nullptr) {
            llvm::Function *the_function = the_module->getFunction(this->id);

            // create body
            llvm::BasicBlock *BB = llvm::BasicBlock::Create(*the_context, "entry", the_function);
            symbol_table->push(BB);
            builder->SetInsertPoint(BB);

            // initialize param
            for (auto &arg: the_function->args()) {
                llvm::AllocaInst *alloca = allocaBlockEntry(the_function, arg.getName().str(), arg.getType());
                builder->CreateStore(&arg, alloca);
                symbol_table->insert(arg.getName().str(), alloca);
            }

            if (this->compound->codeGen()) {
                auto t = builder->GetInsertBlock()->getTerminator();
                if (!t && this->retTy.second != "void") builder->CreateRet(llvm::ConstantFP::get(REAL_TY, 0.0));
                symbol_table->pop();
                llvm::verifyFunction(*the_function, &llvm::errs());
                the_fpm->run(*the_function);
                return the_function;
            }
        }

        return nullptr;
    }

    llvm::Value *FunctionCall::codeGen() {
        llvm::Function *fun = the_module->getFunction(this->id);

        if (!fun) {
            throw ExceptionFactory(
                    __MissingException,
                    "function '" + this->id + "' is not declared",
                    this->getToken().line, this->getToken().column
            );
        }

        if (fun->arg_size() != this->paramList.size()) {
            // if function has declared, check parameters' size
            throw ExceptionFactory(
                    __LogicException,
                    "candidate function not viable: requires " + \
                to_string(fun->arg_size()) + \
                " arguments, but " + \
                to_string(this->paramList.size()) + \
                " were provided",
                    this->token.line, this->token.column
            );
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
                                       (llvm::PointerType *) callee_type)
                    ) {
                v = builder->CreatePointerCast(v, callee_type);
            } else if (callee_type != caller_type) {
                throw ExceptionFactory(
                        __LogicException,
                        "unmatched type, provided: " + \
                            type_name[caller_type] + \
                            ", excepted: " + type_name[callee_arg_iter->getType()],
                        this->getToken().line, this->getToken().column
                );
            }

            args.push_back(v);
            callee_arg_iter++;
        }

        return builder->CreateCall(fun, args, "callLocal");
    }

    llvm::Value *StructInit::codeGen() {
        llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
        llvm::StructType *Ty = struct_types[STRUCT(this->id)]->Ty;

        uint32_t param_num = this->paramList.size();
        uint32_t member_num = Ty->getNumElements();

        // check number of parameters
        if (param_num > member_num) {
            throw ExceptionFactory(
                    __LogicException,
                    "candidate function not viable: requires " + \
                    to_string(member_num) + \
                    " or less arguments, but " + \
                    to_string(param_num) + \
                    " were provided",
                    this->token.line, this->token.column
            );
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
                    id + ".member." + to_string(i)
            );

            if (member_addr->getType()->getPointerElementType() != rv->getType()) {
                throw ExceptionFactory(
                        __LogicException,
                        "not matched type, left: " + \
                        type_name[member_addr->getType()->getPointerElementType()] + \
                        ", right: " + type_name[rv->getType()],
                        this->getToken().line, this->getToken().column
                );
            }

            builder->CreateStore(rv, member_addr);
        }

        return builder->CreateLoad(Ty, new_var_alloca);
    }

    llvm::Value *ArrayInit::codeGen() {
        llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
        uint32_t element_num = this->num;

        if (element_num == 0) {
            // create vec[real;8] for default
            auto Ty = llvm::ArrayType::get(REAL_TY, 0);
            type_name[Ty] = "vec[" + type_name[REAL_TY] + ";" + to_string(0) + "]";
            type_size[Ty] = 0;
            llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, "ArrayInitTemp", Ty);
            return builder->CreateLoad(Ty, new_var_alloca);
        }

        if (this->Ty.first != VOID_TY) {
            auto Ty = llvm::ArrayType::get(this->Ty.first, element_num);
            type_name[Ty] = "vec[" + type_name[this->Ty.first] + ";" + to_string(element_num) + "]";
            type_name[Ty->getPointerTo()] = "vec[" + type_name[this->Ty.first] + ";" + to_string(element_num) + "]*";
            type_size[Ty] = type_size[this->Ty.first] * element_num;
            llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, "ArrayInitTemp", Ty);
            return builder->CreateLoad(Ty, new_var_alloca);
        }

        llvm::Value *head_rv = this->paramList[0]->codeGen();
        auto eleTy = head_rv->getType();
        auto Ty = llvm::ArrayType::get(eleTy, element_num);
        type_name[Ty] = "vec[" + type_name[eleTy] + ";" + to_string(element_num) + "]";
        type_name[Ty->getPointerTo()] = "vec[" + type_name[eleTy] + ";" + to_string(element_num) + "]*";
        type_size[Ty] = type_size[eleTy] * element_num;
        // initialize array
        llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, "ArrayInit.begin", Ty);

        // store first element
        auto first_element_addr = builder->CreateGEP(
                llvm::cast<llvm::PointerType>(new_var_alloca->getType()->getScalarType())->getElementType(),
                new_var_alloca,
                {
                        llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), 0),
                        llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), 0),
                },
                "ArrayInit.element." + to_string(0)
        );
        builder->CreateStore(head_rv, first_element_addr);

        // initialize other elements
        for (int i = 1; i < element_num; i++) {
            AST *param = this->paramList[i];
            llvm::Value *rv = param->codeGen();
            auto element_addr = builder->CreateGEP(
                    llvm::cast<llvm::PointerType>(new_var_alloca->getType()->getScalarType())->getElementType(),
                    new_var_alloca,
                    {
                            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), 0),
                            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*the_context), i),
                    },
                    "ArrayInit.element." + to_string(i)
            );

            if (rv->getType() != eleTy) {
                throw ExceptionFactory(
                        __LogicException,
                        "not matched type, element type: " + \
                        type_name[rv->getType()] + \
                        ", array type: " + type_name[eleTy],
                        this->getToken().line, this->getToken().column
                );
            }

            builder->CreateStore(rv, element_addr);
        }

        return builder->CreateLoad(Ty, new_var_alloca);
    }

    llvm::Value *Global::codeGen() {
        auto *v = (Variable *) this->var;
        string name = v->id;

        if (v->Ty.first == VOID_TY) {
            throw ExceptionFactory(
                    __LogicException,
                    "missing type of global variable '" + name + "'",
                    this->getToken().column, this->getToken().column
            );
        }

        the_module->getOrInsertGlobal(name, v->Ty.first);
        return llvm::Constant::getNullValue(REAL_TY);
    }

    llvm::Value *If::codeGen() {
        if (!this->noCondition) {
            llvm::Value *cond = this->condition->codeGen();
            if (!cond) {
                return nullptr;
            }

            /*
                store cont to local variable
                I don't know why I must add these code... but it will cause error if remove allco
                For example:
                    if [ 0.0 ] then
                    fi
                1. IR code generated without alloca: (error)
                    br i1 ture, label %if.then, label %if.else
                2. IR code generated without alloca: (pass)
                    %ifCondAlloca = alloca double, align 8
                    store i1 true, double* %ifCondAlloca, align 1
                    %0 = load i1, double* %ifCondAlloca, align 1
                    br i1 %0, label %if.then, label %if.else
            */
            cond = builder->CreateFCmpUNE(cond, llvm::ConstantFP::get(*the_context, llvm::APFloat(0.0)), "ifCond");
            llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
            llvm::IRBuilder<> blockEntry(
                    &the_scope->getEntryBlock(),
                    the_scope->getEntryBlock().begin()
            );
            llvm::AllocaInst *new_var_alloca = blockEntry.CreateAlloca(
                    BOOL_TY,
                    0,
                    "ifCondAlloca"
            );
            builder->CreateStore(cond, new_var_alloca);

            llvm::Function *the_function = builder->GetInsertBlock()->getParent();

            llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(*the_context, "if.then", the_function);
            llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(*the_context, "if.else", the_function);
            llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "if.end", the_function);
            uint8_t non_ret_block_then = 0;
            uint8_t non_ret_block_else = 0;

            cond = builder->CreateLoad(BOOL_TY, new_var_alloca);
            builder->CreateCondBr(cond, thenBB, elseBB);

            the_function->getBasicBlockList().push_back(thenBB);
            builder->SetInsertPoint(thenBB);
            symbol_table->push(thenBB);
            llvm::Value *thenv = this->compound->codeGen();
            if (!thenv) {
                return nullptr;
            }
            symbol_table->pop();
            auto t = builder->GetInsertBlock()->getTerminator();
            if (!t) {
                builder->CreateBr(mergeBB);
                non_ret_block_then = 1;
            }
            thenBB = builder->GetInsertBlock();

            the_function->getBasicBlockList().push_back(elseBB);
            builder->SetInsertPoint(elseBB);
            symbol_table->push(elseBB);
            llvm::Value *elsev = this->next->codeGen();
            if (!elsev) {
                return nullptr;
            }
            symbol_table->pop();
            t = builder->GetInsertBlock()->getTerminator();
            if (!t) {
                builder->CreateBr(mergeBB);
                non_ret_block_else = 1;
            }
            elseBB = builder->GetInsertBlock();

            the_function->getBasicBlockList().push_back(mergeBB);
            builder->SetInsertPoint(mergeBB);

            llvm::PHINode *PN = builder->CreatePHI(REAL_TY,
                                                   non_ret_block_then + non_ret_block_else, "ifTmp");
            if (non_ret_block_then) PN->addIncoming(thenv, thenBB);
            if (non_ret_block_else) PN->addIncoming(elsev, elseBB);

            return PN;
        } else {
            return this->compound->codeGen();
        }
    }

    llvm::Value *Num::codeGen() {
//        auto t = llvm::ConstantFP::get(*the_context, llvm::APFloat((double)this->value));
        auto t = llvm::ConstantFP::get(REAL_TY,
                                       (double) this->getValue().any_cast<double>());
        return t;
    }

    llvm::Value *Object::codeGen() {
        return llvm::Constant::getNullValue(REAL_TY);
    }

    llvm::Value *UnaryOp::codeGen() {
        llvm::Value *rv = this->right->codeGen();
        if (!rv) {
            throw ExceptionFactory(
                    __LogicException,
                    "unknown right value of unary op",
                    this->token.line, this->token.column
            );
        }

        if (this->op.getType() == MINUS) {
            return builder->CreateFSub(
                    llvm::ConstantFP::get(REAL_TY, 0.0),
                    rv,
                    "unaryAddTmp"
            );
        } else if (this->op.getType() == PLUS) {
            return builder->CreateFAdd(
                    llvm::ConstantFP::get(REAL_TY, 0.0),
                    rv,
                    "unarySubTmp"
            );
        } else if (this->op.getType() == NOT) {
            llvm::Value *rv_bool = builder->CreateFPToUI(rv, BOOL_TY, "boolTmp");
            llvm::Value *v = builder->CreateNot(rv_bool, "unaryNotTmp");
            return builder->CreateUIToFP(v, REAL_TY, "boolTmp");
        } else {
            return nullptr;
        }
    }

    llvm::Value *Variable::codeGen() {
        llvm::Value *v = getAlloca(this);

        // if v is an array pointer, return pointer directly
        if (v->getType()->getPointerElementType()->isArrayTy()) {
            return v;
        }

        return builder->CreateLoad(v->getType()->getPointerElementType(), v, this->id.c_str());
    }


    llvm::Value *Compound::codeGen() {
        for (AST *ast: this->child) {
            ast->codeGen();
        }

        return llvm::Constant::getNullValue(REAL_TY);
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

        llvm::Value *cond = this->condition->codeGen();
        if (!cond) {
            return nullptr;
        }
        cond = builder->CreateFCmpONE(cond, llvm::ConstantFP::get(*the_context, llvm::APFloat(0.0)));
        llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
        llvm::IRBuilder<> blockEntry(
                &the_scope->getEntryBlock(),
                the_scope->getEntryBlock().begin()
        );
        llvm::AllocaInst *new_var_alloca = blockEntry.CreateAlloca(
                BOOL_TY,
                0,
                "ifCondAlloca"
        );
        builder->CreateStore(cond, new_var_alloca);
        cond = builder->CreateLoad(BOOL_TY, new_var_alloca);
        builder->CreateCondBr(cond, loopBB, mergeBB);

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

        return llvm::Constant::getNullValue(REAL_TY);
    }

    llvm::Value *NoneAST::codeGen() {
        return llvm::Constant::getNullValue(REAL_TY);
    }
}