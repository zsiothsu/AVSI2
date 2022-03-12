/*
 * @Author: Chipen Hsiao
 * @Date: 2022-03-11
 * @Description: llvm code generator
 */
#include <cstdlib>

#include "../inc/AST.h"
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
    static llvm::LLVMContext *the_context = new llvm::LLVMContext();
    static llvm::Module *the_module = new llvm::Module("program", *the_context);
    static unique_ptr<llvm::IRBuilder<>> builder = make_unique<llvm::IRBuilder<>>(*the_context);
    static unique_ptr<llvm::legacy::FunctionPassManager> the_fpm = make_unique<llvm::legacy::FunctionPassManager>(
            the_module);
    map<string, llvm::AllocaInst *> named_values;
    map<std::string, llvm::FunctionType *> function_protos;
    llvm::TargetMachine *TheTargetMachine = nullptr;

    void llvm_module_fpm_init() {
//        the_fpm->add(llvm::createReassociatePass());
//        the_fpm->add(llvm::createGVNPass());
//        the_fpm->add(llvm::createInstructionCombiningPass());
//        the_fpm->add(llvm::createCFGSimplificationPass());

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

        llvm::FunctionType *FT = llvm::FunctionType::get(
                llvm::Type::getDoubleTy(*the_context),
                vector<llvm::Type *>(),
                false
        );

        llvm::Function *main = llvm::Function::Create(
                FT,
                llvm::Function::ExternalLinkage,
                "entry",
                the_module
        );
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

    void llvm_module_printIR() {
        the_module->print(llvm::errs(), nullptr);
    }

    /*******************************************************
     *                    IR generator                     *
     *******************************************************/
    llvm::AllocaInst *allocaBlockEntry(llvm::Function *fun, llvm::StringRef name) {
        llvm::IRBuilder<> blockEntry(
                &fun->getEntryBlock(),
                fun->getEntryBlock().begin()
        );

        return blockEntry.CreateAlloca(
                llvm::Type::getDoubleTy(*the_context),
                0,
                name
        );
    }

    llvm::Value *Assign::codeGen() {
        llvm::Value *rv = this->right->codeGen();
        string lname = ((Variable *) this->left)->id;
        if (named_values.find(lname) != named_values.end()) {
            return builder->CreateStore(rv, named_values[lname]);
        } else {
            llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
            llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, lname);
            named_values[lname] = new_var_alloca;
            return builder->CreateStore(rv, new_var_alloca);
        }
    }

    llvm::Value *BinOp::codeGen() {
        llvm::Value *lv = this->left->codeGen();
        llvm::Value *rv = this->right->codeGen();

        if (!lv || !rv) {
            return nullptr;
        }

        llvm::Value *cmp_value_boolean;
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
                cmp_value_boolean = builder->CreateFCmpOEQ(lv, rv, "cmpEQTmp");
                return builder->CreateUIToFP(cmp_value_boolean, llvm::Type::getDoubleTy(*the_context), "boolTmp");
            case NE:
                cmp_value_boolean = builder->CreateFCmpONE(lv, rv, "cmpNETmp");
                return builder->CreateUIToFP(cmp_value_boolean, llvm::Type::getDoubleTy(*the_context), "boolTmp");
            case GT:
                cmp_value_boolean = builder->CreateFCmpOGT(lv, rv, "cmpGTTmp");
                return builder->CreateUIToFP(cmp_value_boolean, llvm::Type::getDoubleTy(*the_context), "boolTmp");
            case LT:
                cmp_value_boolean = builder->CreateFCmpOLT(lv, rv, "cmpLTTmp");
                return builder->CreateUIToFP(cmp_value_boolean, llvm::Type::getDoubleTy(*the_context), "boolTmp");
            case GE:
                cmp_value_boolean = builder->CreateFCmpOGE(lv, rv, "cmpGETmp");
                return builder->CreateUIToFP(cmp_value_boolean, llvm::Type::getDoubleTy(*the_context), "boolTmp");
            case LE:
                cmp_value_boolean = builder->CreateFCmpOLE(lv, rv, "cmpLETmp");
                return builder->CreateUIToFP(cmp_value_boolean, llvm::Type::getDoubleTy(*the_context), "boolTmp");
            case OR:
                //TODO: double to bool
                cmp_value_boolean = builder->CreateOr(lv, rv, "boolOrTmp");
                return builder->CreateUIToFP(cmp_value_boolean, llvm::Type::getDoubleTy(*the_context), "boolTmp");
            case AND:
                cmp_value_boolean = builder->CreateAnd(lv, rv, "boolAndTmp");
                return builder->CreateUIToFP(cmp_value_boolean, llvm::Type::getDoubleTy(*the_context), "boolTmp");
            default:
                return nullptr;
        }
    }

    llvm::Value *Boolean::codeGen() {
        if (this->value == true) {
            return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*the_context), 1.0);
        } else {
            return llvm::ConstantFP::get(llvm::Type::getDoubleTy(*the_context), 0.0);
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

        llvm::Value *cond = this->condition->codeGen();
        if (!cond) {
            return nullptr;
        }
        cond = builder->CreateFCmpONE(cond, llvm::ConstantFP::get(*the_context, llvm::APFloat(0.0)));
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
        builder->CreateBr(headBB);
        loopBB = builder->GetInsertBlock();

        the_function->getBasicBlockList().push_back(mergeBB);
        builder->SetInsertPoint(mergeBB);

        return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*the_context));
    }

    llvm::Value *FunctionDecl::codeGen() {
        // create function type
        std::vector<llvm::Type *> Doubles(
                ((Param *) this->paramList)->paramList.size(),
                llvm::Type::getDoubleTy(*the_context)
        );
        llvm::FunctionType *FT = llvm::FunctionType::get(
                llvm::Type::getDoubleTy(*the_context),
                Doubles,
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
                Variable *var = ((Param *) this->paramList)->paramList[param_index++];
                arg.setName(var->id);
            }
        }

        if (this->compound != nullptr) {
            llvm::Function *the_function = the_module->getFunction(this->id);

            // create body
            llvm::BasicBlock *BB = llvm::BasicBlock::Create(*the_context, "entry", the_function);
            builder->SetInsertPoint(BB);

            // initialize param
            named_values.clear();
            for (auto &arg: the_function->args()) {
                llvm::AllocaInst *alloca = allocaBlockEntry(the_function, arg.getName());
                builder->CreateStore(&arg, alloca);
                named_values[string(arg.getName())] = alloca;
            }

            if (this->compound->codeGen()) {
                builder->CreateRet(llvm::ConstantFP::get(llvm::Type::getDoubleTy(*the_context), 0.0));
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
                    "undefined reference '" + this->id + "'",
                    this->token.line, this->token.column
            );
        }

        if (fun->arg_size() != this->paramList.size()) {
            throw ExceptionFactory(
                    __LogicException,
                    "candidate function not viable: requires " + \
                to_string(fun->arg_size()) + \
                "arguments, but " + \
                to_string(this->paramList.size()) + \
                " were provided",
                    this->token.line, this->token.column
            );
        }

        vector<llvm::Value *> args;
//        for (AST *arg: this->paramList) {
//            llvm::Value *v = arg->codeGen();
//            if (!v) {
//                return nullptr;
//            }
//
//            args.push_back(v);
//        }
        for (int i = 0; i < this->paramList.size(); i++) {
            AST *arg = paramList[i];
            llvm::Value *v = arg->codeGen();
            if (!v) {
                return nullptr;
            }

            args.push_back(v);
        }

        return builder->CreateCall(fun, args, "callTmp");
    }

    llvm::Value *Global::codeGen() {
        string name = ((Variable *) this->var)->id;
        the_module->getOrInsertGlobal(name, builder->getBFloatTy());
        return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*the_context));
    }

    llvm::Value *If::codeGen() {
        if (!this->noCondition) {
            llvm::Value *cond = this->condition->codeGen();
            if (!cond) {
                return nullptr;
            }
            cond = builder->CreateFCmpONE(cond, llvm::ConstantFP::get(*the_context, llvm::APFloat(0.0)));

            llvm::Function *the_function = builder->GetInsertBlock()->getParent();

            llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(*the_context, "if.then", the_function);
            llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(*the_context, "if.else", the_function);
            llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "if.end", the_function);

            builder->CreateCondBr(cond, thenBB, elseBB);

            the_function->getBasicBlockList().push_back(thenBB);
            builder->SetInsertPoint(thenBB);
            llvm::Value *thenv = nullptr;
            thenv = this->compound->codeGen();
            if (!thenv) {
                return nullptr;
            }
            builder->CreateBr(mergeBB);
            thenBB = builder->GetInsertBlock();

            the_function->getBasicBlockList().push_back(elseBB);
            builder->SetInsertPoint(elseBB);
            llvm::Value *elsev = nullptr;
//        if(this->compound != nullptr) {
            elsev = this->next->codeGen();
            if (!elsev) {
                return nullptr;
            }
            builder->CreateBr(mergeBB);
            elseBB = builder->GetInsertBlock();
//        }

            the_function->getBasicBlockList().push_back(mergeBB);
            builder->SetInsertPoint(mergeBB);
            llvm::PHINode *PN = builder->CreatePHI(llvm::Type::getDoubleTy(*the_context), 2, "ifTmp");
            PN->addIncoming(thenv, thenBB);
            PN->addIncoming(elsev, elseBB);

            return PN;
        } else {
            return this->compound->codeGen();
        }
    }

    llvm::Value *Num::codeGen() {
//        auto t = llvm::ConstantFP::get(*the_context, llvm::APFloat((double)this->value));
        auto t = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*the_context), this->getValue().any_cast<double>());
        return t;
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
                    llvm::ConstantFP::get(*the_context, llvm::APFloat(0.0f)),
                    rv,
                    "unaryAddTmp"
            );
        } else if (this->op.getType() == PLUS) {
            return builder->CreateFAdd(
                    llvm::ConstantFP::get(*the_context, llvm::APFloat(0.0f)),
                    rv,
                    "unarySubTmp"
            );
        } else if (this->op.getType() == NOT) {
            llvm::Value *v = builder->CreateNot(rv, "unaryNotTmp");
            return builder->CreateUIToFP(v, llvm::Type::getDoubleTy(*the_context), "boolTmp");
        } else {
            return nullptr;
        }
    }

    llvm::Value *Variable::codeGen() {
        llvm::Value *v = named_values[this->id];

        if (!v) {
            v = the_module->getGlobalVariable(this->id);
            if (!v) {
                throw ExceptionFactory(
                        __LogicException,
                        "variable '" + this->id + "' is not defined",
                        this->token.line, this->token.column
                );
            }
            return v;
        }

        return builder->CreateLoad(v, this->id.c_str());
    }

    llvm::Value *Compound::codeGen() {
        for (AST *ast: this->child) {
            ast->codeGen();
        }

        return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*the_context));
    }

    llvm::Value *Return::codeGen() {
        if (this->ret != nullptr) {
            llvm::Value *re = this->ret->codeGen();
            if (!re) {
                return nullptr;
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

        llvm::Value *cond = this->condition->codeGen();
        if (!cond) {
            return nullptr;
        }
        cond = builder->CreateFCmpONE(cond, llvm::ConstantFP::get(*the_context, llvm::APFloat(0.0)));
        builder->CreateCondBr(cond, loopBB, mergeBB);

        the_function->getBasicBlockList().push_back(loopBB);
        builder->SetInsertPoint(loopBB);
        llvm::Value *body = this->compound->codeGen();
        if (!body) {
            return nullptr;
        }
        builder->CreateBr(headBB);

        the_function->getBasicBlockList().push_back(mergeBB);
        builder->SetInsertPoint(mergeBB);

        return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*the_context));
    }
}