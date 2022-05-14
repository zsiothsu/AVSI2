/*
 * @Author: Chipen Hsiao
 * @Date: 2022-03-11
 * @Description: llvm code generator
 */
#include <cstdlib>
#include <set>
#include <cstdint>

#include "../inc/AST.h"
#include "../inc/SymbolTable.h"
#include "../inc/FileName.h"
#include <filesystem>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>

extern bool opt_reliance;

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
    llvm::Type *REAL_TY;
    llvm::Type *CHAR_TY;
    llvm::Type *VOID_TY;
    llvm::Type *BOOL_TY;

    AST *ASTEmpty = new NoneAST();
    AST *ASTEmptyNotEnd = new NoneAST();

    SymbolTable *symbol_table;
    map<string, StructDef *> struct_types;
    map<std::string, llvm::FunctionType *> function_protos;

    set<llvm::Type *> simple_types;
    map<llvm::Type *, uint8_t> simple_types_map;

    map<llvm::Type *, string> type_name;
    map<llvm::Type *, uint32_t> type_size;

    map<string, string> module_name_alias;

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

    void llvm_import_module(vector<string> path, string mod, int line, int col) {
        // import module can use absolute path or relative path
        // resolve path to locate module
        int path_size = module_path.size();
        bool is_absolute_module_path = true;
        if (path.size() < path_size) {
            is_absolute_module_path = false;
        } else {
            for (int i = 0; i < path_size; i++) {
                if (module_path[i] != path[i]) {
                    is_absolute_module_path = false;
                    break;
                }
            }
        }

        int search_begin_index = 0;
        if (is_absolute_module_path) {
            search_begin_index = path_size;
        }

        // get bc file path in file system
        string module_file_system_path;
        if (is_absolute_module_path) {
            module_file_system_path = output_root_path + SYSTEM_PATH_DIVIDER;

            string unresolved_path = getpathListToUnresolved(path);
            unresolved_path += (unresolved_path.empty() ? "" : "::") + mod;
            module_name_alias[unresolved_path] = unresolved_path;
        } else {
            module_file_system_path =
                    output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER;

            // create alias: relative -> absolute
            string unresolved_path = getpathListToUnresolved(path);
            unresolved_path += (unresolved_path.empty() ? "" : "::") + mod;
            string unresolved_path_absolute = getpathListToUnresolved(module_path);
            unresolved_path_absolute += "::" + unresolved_path;
            module_name_alias[unresolved_path] = unresolved_path_absolute;
        }
        for (int i = search_begin_index; i < path.size(); i++) {
            module_file_system_path += SYSTEM_PATH_DIVIDER + path[i];
        }
        module_file_system_path += SYSTEM_PATH_DIVIDER + mod;
        if (std::filesystem::is_directory(std::filesystem::path(module_file_system_path))) {
            module_file_system_path += SYSTEM_PATH_DIVIDER + mod + ".bc";
        } else {
            module_file_system_path += ".bc";
        }
        std::filesystem::path bcfile = module_file_system_path;

        // generate .r for Makefile
        if (opt_reliance) {
            auto OFilename =
                    output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER
                    + string(input_file_name_no_suffix) + ".o";

            auto BCFilename =
                    output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER
                    + string(input_file_name_no_suffix) + ".bc";

            auto RFilename =
                    output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER
                    + string(input_file_name_no_suffix) + ".r";

            llvm_create_dir(filesystem::path(RFilename).parent_path());
            std::error_code EC;
            llvm::raw_fd_ostream dest(RFilename, EC, llvm::sys::fs::OF_None);

            dest << OFilename << " " << BCFilename << ":" << bcfile.string() << "\n";
            return;
        }

        if (!std::filesystem::exists(bcfile)) {
            string unparsed_name;
            for (string i: path) {
                unparsed_name += i + "::";
            }
            unparsed_name += mod;

            throw ExceptionFactory(
                    __MissingException,
                    "module " + unparsed_name + " is not found",
                    line, col
            );
        }

        // read bc file to memory buffer
        auto mbuf = llvm::MemoryBuffer::getFile(bcfile.c_str());
        if (!mbuf) {
            if (!std::filesystem::exists(bcfile)) {
                string unparsed_name;
                for (string i: path) {
                    unparsed_name += i + "::";
                }
                unparsed_name += mod;

                throw ExceptionFactory(
                        __MissingException,
                        "error occurred when reading module " + unparsed_name,
                        line, col
                );
            }
        }

        // create and import module
        llvm::SMDiagnostic smd = llvm::SMDiagnostic();
        auto ir = llvm::parseIR(*mbuf.get(), smd, *the_context);
        auto ptr = ir.release();

        // import symbols
        // import function
        auto function_iter = ptr->getFunctionList().begin();
        while (function_iter != ptr->getFunctionList().end()) {
            auto fun = &(*function_iter);
            the_module->getOrInsertFunction(fun->getName(), fun->getFunctionType());
            function_iter++;
        }

        // import global
        auto global_iter = ptr->getGlobalList().begin();
        while (global_iter != ptr->getGlobalList().end()) {
            auto glb = &(*global_iter);
            the_module->getOrInsertFunction(glb->getName(), glb->getValueType());
            global_iter++;
        }

        auto md_iter = ptr->getNamedMDList().begin();
        while (md_iter != ptr->getNamedMDList().end()) {
            auto md = &(*md_iter);
            the_module->getOrInsertNamedMetadata(md->getName());
            md_iter++;
        }
    }

    void llvm_global_context_reset() {
        // reset context and module
        delete TheTargetMachine;
        delete the_fpm;
        delete builder;
        delete the_module;
        delete the_context;

        the_context = new llvm::LLVMContext();
        the_module = new llvm::Module("program", *the_context);
        builder = new llvm::IRBuilder<>(*the_context);
        the_fpm = new llvm::legacy::FunctionPassManager(the_module);
        TheTargetMachine = nullptr;

        // reset symbols
        delete symbol_table;
        REAL_TY = llvm::Type::getDoubleTy(*the_context);
        CHAR_TY = llvm::Type::getInt8Ty(*the_context);
        VOID_TY = llvm::Type::getVoidTy(*the_context);
        BOOL_TY = llvm::Type::getInt1Ty(*the_context);

        symbol_table = new SymbolTable();
        struct_types.clear();
        function_protos.clear();
        simple_types.clear();
        simple_types_map.clear();
        type_name.clear();
        type_size.clear();

        simple_types = {REAL_TY,
                        CHAR_TY,
                        BOOL_TY};
        simple_types_map = {
                {REAL_TY, 0x04},
                {CHAR_TY, 0x02},
                {BOOL_TY, 0x01}
        };
        type_name = {
                {REAL_TY, "real"},
                {CHAR_TY, "char"},
                {VOID_TY, "void"},
                {BOOL_TY, "bool"},
        };
        type_size = {
                {REAL_TY, 8},
                {CHAR_TY, 1},
                {VOID_TY, 0},
                {BOOL_TY, 1},
        };

        module_name = "";
        the_module->setModuleIdentifier(input_file_name_no_suffix);
        the_module->setSourceFileName(input_file_name);

        global_insert_point = builder->GetInsertBlock();
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
        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER
                + string(input_file_name_no_suffix) + ".o";
        llvm_create_dir(filesystem::path(Filename).parent_path());
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

        cout << Filename << endl;

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
        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER
                + string(input_file_name_no_suffix) + ".s";
        llvm_create_dir(filesystem::path(Filename).parent_path());
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

    void llvm_module_output() {
        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER
                + string(input_file_name_no_suffix) + ".bc";
        llvm_create_dir(filesystem::path(Filename).parent_path());
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

        cout << Filename << endl;

        llvm::WriteBitcodeToFile(*the_module, dest);

        dest.flush();
        llvm::outs() << "Wrote " << Filename << "\n";
    }

    void llvm_module_printIR() {
        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER
                + string(input_file_name_no_suffix) + ".ll";
        llvm_create_dir(filesystem::path(Filename).parent_path());
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
        if (!v) return;
        v->print(llvm::outs());
        cout << endl;
    }

    void llvm_create_dir(string dir) {
        std::filesystem::path d = dir;
        if (!filesystem::exists(d.parent_path())) {
            llvm_create_dir(d.parent_path());
        }
        filesystem::create_directory(d);
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
                    getFunctionNameMangling(getpathUnresolvedToList(mod_path), var->id)
            );
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
                    index = builder->CreateFPToSI(index, MACHINE_WIDTH_TY);
                    offset_list.push_back(index);

                    if (!v->getType()->getPointerElementType()->isArrayTy()) {
                        // for raw pointer
                        index = builder->CreateMul(index,
                                                   llvm::ConstantInt::get(
                                                           MACHINE_WIDTH_TY,
                                                           v->getType()->getPointerElementType()->isPtrOrPtrVectorTy()
                                                           ? PTR_SIZE
                                                           : type_size[v->getType()->getPointerElementType()]
                                                   )
                        );

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
                                    var->id
                            );
                        } catch (...) {
                            throw ExceptionFactory(
                                    __TypeException,
                                    "index out of range or not an array",
                                    i.second->getToken().line, i.second->getToken().column
                            );
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

    llvm::Value *store(AST *ast, llvm::Value *l_alloca_addr, llvm::Value *r_value, bool assignment = true,
                       string l_base_name = "member") {
        llvm::Type *r_type = r_value->getType();
        Assign *assigin_ast = (Assign *) ast;

        if (r_type == VOID_TY) {
            throw ExceptionFactory(
                    __LogicException,
                    "cannot assign void to variable",
                    ast->getToken().line, ast->getToken().column
            );
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
            llvm::Value *l_ptr = builder->CreateLoad(l_alloca_addr->getType()->getPointerElementType(), l_alloca_addr,
                                                     "dest.addr");
            auto &r_ptr = r_value;

            uint32_t size = 0;
            if (l_type->getPointerElementType()->isArrayTy() && r_type->getPointerElementType()->isArrayTy()) {
                size = min(type_size[l_ptr->getType()->getPointerElementType()],
                           type_size[r_ptr->getType()->getPointerElementType()]);
                builder->CreateMemCpy(l_ptr, llvm::MaybeAlign(), r_ptr, llvm::MaybeAlign(),
                                      size);
            } else if (l_type->getPointerElementType()->isArrayTy()) {
                size = type_size[l_ptr->getType()->getPointerElementType()];
                builder->CreateMemCpy(l_ptr, llvm::MaybeAlign(), r_ptr, llvm::MaybeAlign(),
                                      size);
            } else {
                r_ptr = builder->CreatePointerCast(r_ptr, l_ptr->getType(), "ptr.cast");
                builder->CreateStore(r_ptr, l_alloca_addr);
            }
        } else if (
                l_alloca_addr != nullptr &&
                l_type == r_type
                ) {
            builder->CreateStore(r_value, l_alloca_addr);
        } else if (assignment && ((Variable *) assigin_ast->left)->offset.empty()) {
            llvm::Function *the_scope = builder->GetInsertBlock()->getParent();

            llvm::Type *cast_to_type = nullptr;
            string cast_to_name = l_base_name;

            auto l_excepted_type = ((Variable *) assigin_ast->left)->Ty.first;

            // if right value is an array initializer or another array
            if (
                    r_type->isPtrOrPtrVectorTy() &&
                    r_type->getPointerElementType()->isArrayTy()
                    ) {
                cast_to_type = r_type;
                cast_to_name = l_base_name + ".addr";

                if (
                        l_excepted_type->isPtrOrPtrVectorTy() &&
                        l_excepted_type->getPointerElementType()->isArrayTy() &&
                        isTheSameBasicType((llvm::PointerType *) r_type,
                                           (llvm::PointerType *) l_excepted_type)
                        ) {

                    if (
                            type_size[l_excepted_type->getPointerElementType()]
                            <= type_size[r_type->getPointerElementType()]
                            ) {
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
                                        0
                                ),
                                type_size[l_excepted_type->getPointerElementType()],
                                llvm::MaybeAlign()
                        );

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
                                    __Warning,
                                    "failed to cast type '" +
                                    type_name[cast_to_type] +
                                    "' to '"
                                    + type_name[l_excepted_type] +
                                    "', the left type will be ignored ",
                                    assigin_ast->getToken().line, assigin_ast->getToken().column
                            );
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
                                           (llvm::PointerType *) l_excepted_type)
                        ) {
                    // if right is a normal pointer
                    cast_to_type = l_excepted_type;
                    r_value = builder->CreatePointerCast(r_value, cast_to_type);
                    cast_to_name = l_base_name + ".cast.ptr2array";
                } else {
                    cast_to_type = r_type;
                    if (l_excepted_type != VOID_TY) {
                        Warning(
                                __Warning,
                                "failed to cast type '" +
                                type_name[cast_to_type] +
                                "' to '"
                                + type_name[l_excepted_type] +
                                "', the left type will be ignored ",
                                assigin_ast->getToken().line, assigin_ast->getToken().column
                        );
                    }
                }
                goto assign_begin;
            }

            assign_begin:
            llvm::AllocaInst *new_var_alloca = allocaBlockEntry(the_scope, cast_to_name, cast_to_type);
            builder->CreateStore(r_value, new_var_alloca);
            symbol_table->insert(l_base_name, new_var_alloca);
        } else {
            throw ExceptionFactory(
                    __LogicException,
                    "not matched type, left: " + \
                    type_name[l_alloca_addr->getType()->getPointerElementType()] + \
                    ", right: " + type_name[r_type],
                    ast->getToken().line, ast->getToken().column
            );
        }
        assign_end:
        return llvm::Constant::getNullValue(REAL_TY);
    }

    llvm::Value *Assign::codeGen() {
        llvm::Value *r_value = this->right->codeGen();

        string l_base_name = ((Variable *) this->left)->id;
        auto l_alloca_addr = getAlloca((Variable *) this->left);

        return store(this, l_alloca_addr, r_value, true, l_base_name);
    }

    llvm::Value *BinOp::codeGen() {
        auto token_type = this->getToken().getType();

        if (token_type != AND && token_type != OR) {
            llvm::Value *lv = this->left->codeGen();
            llvm::Value *rv = this->right->codeGen();

            if (!lv || !rv) {
                return nullptr;
            }

            llvm::Type *l_type = lv->getType();
            llvm::Type *r_type = rv->getType();

            if (
                    simple_types.find(l_type) == simple_types.end() ||
                    simple_types.find(r_type) == simple_types.end()
                    ) {
                return nullptr;
            }

            int result_type_bitmap = simple_types_map[l_type] | simple_types_map[r_type];
            bool float_point = false;
            if (result_type_bitmap >= 0x04) float_point = true;

            llvm::Value *cmp_value_boolean;
            // llvm::Value *lv_bool = nullptr;
            // llvm::Value *rv_bool = nullptr;

            llvm::Value *lv_real = nullptr;
            llvm::Value *rv_real = nullptr;

            if (float_point) {
                lv_real = l_type->isDoubleTy() ? lv : builder->CreateSIToFP(lv, REAL_TY);
                rv_real = r_type->isDoubleTy() ? rv : builder->CreateSIToFP(rv, REAL_TY);
            }

            switch (this->op.getType()) {
                case PLUS:
                    if (float_point) return builder->CreateFAdd(lv_real, rv_real, "addTmp");
                    else return builder->CreateAdd(lv, rv, "addTmp");
                case MINUS:
                    if (float_point) return builder->CreateFSub(lv_real, rv_real, "subTmp");
                    else return builder->CreateSub(lv, rv, "subTmp");
                case STAR:
                    if (float_point) return builder->CreateFMul(lv_real, rv_real, "mulTmp");
                    else if (float_point) return builder->CreateMul(lv, rv, "mulTmp");
                case SLASH:
                    lv_real = l_type->isDoubleTy() ? lv : builder->CreateSIToFP(lv, REAL_TY);
                    rv_real = r_type->isDoubleTy() ? rv : builder->CreateSIToFP(rv, REAL_TY);
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
                        cmp_value_boolean = builder->CreateFCmpOGT(lv_real, rv_real, "cmpLTTmp");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSGT(lv, rv, "cmpLTTmp");
                    }
                    return cmp_value_boolean;
                case GE:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOGT(lv_real, rv_real, "cmpGETmp");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSGT(lv, rv, "cmpGETmp");
                    }
                    return cmp_value_boolean;
                case LE:
                    if (float_point) {
                        cmp_value_boolean = builder->CreateFCmpOGT(lv_real, rv_real, "cmpLETmp");
                    } else {
                        cmp_value_boolean = builder->CreateICmpSGT(lv, rv, "cmpLETmp");
                    }
                    return cmp_value_boolean;
//                case OR:
//                    if (float_point) {
//                        lv_bool = builder->CreateFCmpUNE(lv_real, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
//                        rv_bool = builder->CreateFCmpUNE(rv_real, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
//                    } else {
//                        lv_bool = builder->CreateICmpNE(lv, llvm::ConstantInt::get(lv->getType(), 0), "toBool");
//                        rv_bool = builder->CreateICmpNE(rv, llvm::ConstantInt::get(lv->getType(), 0), "toBool");
//                    }
//                    return builder->CreateOr(lv_bool, rv_bool, "boolOrTmp");
//                case AND:
//                    if (float_point) {
//                        lv_bool = builder->CreateFCmpUNE(lv_real, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
//                        rv_bool = builder->CreateFCmpUNE(rv_real, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
//                    } else {
//                        lv_bool = builder->CreateICmpNE(lv, llvm::ConstantInt::get(lv->getType(), 0), "toBool");
//                        rv_bool = builder->CreateICmpNE(rv, llvm::ConstantInt::get(lv->getType(), 0), "toBool");
//                    }
//                    return builder->CreateAnd(lv_bool, rv_bool, "boolAndTmp");
                default:
                    return nullptr;
            }
        } else {
            if (token_type == AND) {
                auto l = this->left->codeGen();
                if (!l) {
                    return nullptr;
                }
                auto l_phi_path = builder->GetInsertBlock();

                auto the_function = builder->GetInsertBlock()->getParent();
                auto Positive = llvm::BasicBlock::Create(*the_context, "land.lhs.true.rhs.head", the_function);
                auto Negative = llvm::BasicBlock::Create(*the_context, "land.end", the_function);

                if (l->getType() != BOOL_TY) {
                    if (l->getType()->isIntegerTy()) {
                        l = builder->CreateICmpNE(l, llvm::ConstantInt::get(l->getType(), 0), "toBool");
                    } else {
                        l = builder->CreateFCmpUNE(l, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
                    }
                }
                builder->CreateCondBr(l, Positive, Negative);

                the_function->getBasicBlockList().push_back(Positive);
                builder->SetInsertPoint(Positive);
                auto r = this->right->codeGen();
                auto r_phi_path = builder->GetInsertBlock();
                if (r->getType() != BOOL_TY) {
                    if (r->getType()->isIntegerTy()) {
                        r = builder->CreateICmpNE(r, llvm::ConstantInt::get(r->getType(), 0), "toBool");
                    } else {
                        r = builder->CreateFCmpUNE(r, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
                    }
                }
                builder->CreateBr(Negative);

                the_function->getBasicBlockList().push_back(Negative);
                builder->SetInsertPoint(Negative);

                llvm::PHINode *PN = builder->CreatePHI(BOOL_TY,
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

                if (l->getType() != BOOL_TY) {
                    if (l->getType()->isIntegerTy()) {
                        l = builder->CreateICmpNE(l, llvm::ConstantInt::get(l->getType(), 0), "toBool");
                    } else {
                        l = builder->CreateFCmpUNE(l, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
                    }
                }
                builder->CreateCondBr(l, Positive, Negative);

                the_function->getBasicBlockList().push_back(Negative);
                builder->SetInsertPoint(Negative);
                auto r = this->right->codeGen();
                auto r_phi_path = builder->GetInsertBlock();
                if (r->getType() != BOOL_TY) {
                    if (r->getType()->isIntegerTy()) {
                        r = builder->CreateICmpNE(r, llvm::ConstantInt::get(r->getType(), 0), "toBool");
                    } else {
                        r = builder->CreateFCmpUNE(r, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
                    }
                }
                builder->CreateBr(Positive);

                the_function->getBasicBlockList().push_back(Positive);
                builder->SetInsertPoint(Positive);

                llvm::PHINode *PN = builder->CreatePHI(BOOL_TY,
                                                       2);
                PN->addIncoming(l, l_phi_path);
                PN->addIncoming(r, r_phi_path);

                return PN;
            }
        }
    }

    llvm::Value *Boolean::codeGen() {
        if (this->value == true) {
            return llvm::ConstantInt::get(BOOL_TY, 1);
        } else {
            return llvm::ConstantInt::get(BOOL_TY, 0);
        }
    }

    llvm::Value *For::codeGen() {
        llvm::Function *the_function = builder->GetInsertBlock()->getParent();

        llvm::BasicBlock *headBB = llvm::BasicBlock::Create(*the_context, "loop.head", the_function);
        llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(*the_context, "loop.body", the_function);
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "loop.end", the_function);

        // not region of headBB, but symbol table should be pushed before initial list
        symbol_table->push(headBB);
        symbol_table->setLoopExit(mergeBB);
        symbol_table->setLoopEntry(loopBB);

        llvm::Value *start = this->initList->codeGen();
        if (!(((Compound * )(this->initList))->child.empty()) && (!start)) {
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

            if (cond->getType() != BOOL_TY) {
                if (cond->getType()->isIntegerTy()) {
                    cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(cond->getType(), 0), "toBool");
                } else {
                    cond = builder->CreateFCmpUNE(cond, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
                }
            }

            builder->CreateCondBr(cond, loopBB, mergeBB);
            headBB = builder->GetInsertBlock();
        } else {
            builder->CreateBr(loopBB);
        }

        the_function->getBasicBlockList().push_back(loopBB);
        builder->SetInsertPoint(loopBB);

        llvm::Value *body = this->compound->codeGen();
        if (!(((Compound * )(this->compound))->child.empty()) && (!body)) {
            return nullptr;
        }

        if (!builder->GetInsertBlock()->getTerminator()) {
            llvm::Value *adjust = this->adjustment->codeGen();
            if (!(((Compound * )(this->adjustment))->child.empty()) && (!adjust)) {
                return nullptr;
            }
            symbol_table->pop();

            auto t = builder->GetInsertBlock()->getTerminator();
            if (!t) {
                builder->CreateBr(headBB);
            }
        }
        loopBB = builder->GetInsertBlock();

        the_function->getBasicBlockList().push_back(mergeBB);
        builder->SetInsertPoint(mergeBB);

        return llvm::Constant::getNullValue(REAL_TY);
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

        llvm::FunctionType *FT = llvm::FunctionType::get(
                this->retTy.first->isArrayTy()
                ? getArrayBasicTypePointer(this->retTy.first->getPointerTo())
                : this->retTy.first,
                Tys,
                false
        );

        if (the_module->getFunction(this->id) == nullptr) {
            builder->ClearInsertionPoint();
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

            auto link_type =
                    this->is_export
                    ? llvm::Function::ExternalLinkage
                    : llvm::Function::PrivateLinkage;


            // create function
            string func_name =
                    this->id == ENTRY_NAME
                    ? this->id
                    : NAME_MANGLING(this->id);

            llvm::Function *the_function = llvm::Function::Create(
                    FT,
                    link_type,
                    func_name,
                    the_module
            );
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
            string func_name =
                    this->id == ENTRY_NAME
                    ? this->id
                    : NAME_MANGLING(this->id);

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
                auto t = builder->GetInsertBlock()->getTerminator();
                if (!t && this->retTy.first == VOID_TY) {
                    builder->CreateRetVoid();
                } else if (!t) {
                    builder->CreateRet(llvm::ConstantFP::get(REAL_TY, 0.0));
                }
                symbol_table->pop();
                llvm::verifyFunction(*the_function, &llvm::errs());
                the_fpm->run(*the_function);
                if (last_BB != nullptr) {
                    builder->SetInsertPoint(last_BB, last_pt);
                }
                return the_function;
            }
        }
        if (last_BB != nullptr) {
            builder->SetInsertPoint(last_BB, last_pt);
        }
        return nullptr;
    }

    llvm::Value *FunctionCall::codeGen() {
        string mod_path = getpathListToUnresolved(this->getToken().getModInfo());
        mod_path = module_name_alias[mod_path];
        llvm::Function *fun = the_module->getFunction(
                getFunctionNameMangling(getpathUnresolvedToList(mod_path), this->id)
        );

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
            } else if (
                    caller_type->isStructTy() &&
                    caller_type->isStructTy() &&
                    ((llvm::StructType *) caller_type)->isLayoutIdentical((llvm::StructType *) callee_type)
                    ) {
                v = getLoadStorePointerOperand(v);
                v = builder->CreatePointerCast(v, callee_type->getPointerTo());
                v = builder->CreateLoad(callee_type, v);
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

            store(this, member_addr, rv, false, id + ".member." + to_string(i));
        }

        return builder->CreateLoad(Ty, new_var_alloca);
    }

    llvm::Value *ArrayInit::codeGen() {
        llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
        uint32_t element_num = this->num;

        if (element_num == 0) {
            llvm::Type *contain_type = this->Ty.first;
            if (this->Ty.first == VOID_TY) {
                contain_type = REAL_TY;
            }
            llvm::Type *ptr_type = contain_type->getPointerTo();
            type_name[ptr_type] = type_name[contain_type] + "*";
            type_size[ptr_type] = PTR_SIZE;
            llvm::AllocaInst *ptr_alloca = allocaBlockEntry(the_scope, "ArrayInitTemp", contain_type);
            return ptr_alloca;
        }

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
                            0
                    ),
                    type_size[arr_type],
                    llvm::MaybeAlign()
            );

            return array_alloca;
        }

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
                "ArrayInit.element." + to_string(0)
        );
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

        return array_alloca;
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
                }
        );

        return llvm::Constant::getNullValue(REAL_TY);
    }

    llvm::Value *If::codeGen() {
        if (!this->noCondition) {
            llvm::Value *cond = this->condition->codeGen();
            if (!cond) {
                return nullptr;
            }
            if (cond->getType() != BOOL_TY) {
                if (cond->getType()->isIntegerTy()) {
                    cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(cond->getType(), 0), "toBool");
                } else {
                    cond = builder->CreateFCmpUNE(cond, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
                }
            }

            llvm::Function *the_function = builder->GetInsertBlock()->getParent();

            llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(*the_context, "if.then", the_function);
            llvm::BasicBlock *elseBB = nullptr;
            if (this->next != ASTEmpty) elseBB = llvm::BasicBlock::Create(*the_context, "if.else", the_function);
            llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*the_context, "if.end", the_function);

            if (elseBB != nullptr) {
                builder->CreateCondBr(cond, thenBB, elseBB);
            } else {
                builder->CreateCondBr(cond, thenBB, mergeBB);
            }

            the_function->getBasicBlockList().push_back(thenBB);
            builder->SetInsertPoint(thenBB);
            symbol_table->push(thenBB);
            llvm::Value *thenv = this->compound->codeGen();
            if (((Compound *) this->compound)->child.empty()) {
                builder->CreateBr(mergeBB);
            } else if (!thenv) {
                return nullptr;
            }
            symbol_table->pop();
            auto t = builder->GetInsertBlock()->getTerminator();
            if (!t) {
                builder->CreateBr(mergeBB);
            }
            thenBB = builder->GetInsertBlock();

            llvm::Value *elsev;
            if (elseBB != nullptr) {
                the_function->getBasicBlockList().push_back(elseBB);
                builder->SetInsertPoint(elseBB);
                symbol_table->push(elseBB);
                elsev = this->next->codeGen();
                if (((Compound *) this->compound)->child.empty()) {
                    builder->CreateBr(mergeBB);
                } else if (!elsev) {
                    return nullptr;
                }
                symbol_table->pop();
                t = builder->GetInsertBlock()->getTerminator();
                if (!t) {
                    builder->CreateBr(mergeBB);
                }
                elseBB = builder->GetInsertBlock();
            }

            the_function->getBasicBlockList().push_back(mergeBB);
            builder->SetInsertPoint(mergeBB);

            return llvm::Constant::getNullValue(REAL_TY);
        } else {
            return this->compound->codeGen();
        }
    }

    llvm::Value *LoopCtrl::codeGen() {
        if (this->type == LoopCtrl::LoopCtrlType::CTRL_BREAK) {
            auto break_to = symbol_table->getLoopExit();
            if (break_to != nullptr) {
                builder->CreateBr(break_to);
            } else {
                throw ExceptionFactory(
                        __LogicException,
                        "break must be used in a loop",
                        this->token.line, this->token.column
                );
            }
        } else {
            auto continue_to = symbol_table->getLoopEntry();
            if (continue_to != nullptr) {
                builder->CreateBr(continue_to);
            } else {
                throw ExceptionFactory(
                        __LogicException,
                        "continue must be used in a loop",
                        this->token.line, this->token.column
                );
            }
        }
        return llvm::Constant::getNullValue(REAL_TY);
    }

    llvm::Value *Num::codeGen() {
//        auto t = llvm::ConstantFP::get(*the_context, llvm::APFloat((double)this->value));
        if (this->token.getType() == CHAR) {
            return llvm::ConstantInt::get(CHAR_TY, this->getValue().any_cast<char>());
        } else {
            return llvm::ConstantFP::get(REAL_TY,
                                         this->getValue().any_cast<double>());
        }
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
            llvm::Value *rv_bool = builder->CreateFCmpUNE(rv, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
            return builder->CreateNot(rv_bool, "unaryNotTmp");
        } else {
            return nullptr;
        }
    }

    llvm::Value *Sizeof::codeGen() {
        if (this->id != nullptr) {
            llvm::Value *v = getAlloca((Variable *) this->id);

            if (!v) {
                throw ExceptionFactory(
                        __MissingException,
                        "variable '" + ((Variable *) (this->id))->id + "' is not defined",
                        this->token.line, this->token.column
                );
            }

            llvm::Type *type = v->getType()->getPointerElementType();

            if (type->isPtrOrPtrVectorTy()) {
                if (type->getPointerElementType()->isArrayTy()) {
                    return llvm::ConstantFP::get(REAL_TY, type_size[type->getPointerElementType()]);
                } else {
                    return llvm::ConstantFP::get(REAL_TY, PTR_SIZE);
                }
            } else {
                return llvm::ConstantFP::get(REAL_TY, type_size[type]);
            }
        }

        return llvm::ConstantFP::get(REAL_TY, type_size[this->Ty.first]);
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
            throw ExceptionFactory(
                    __MissingException,
                    "variable '" + this->id + "' is not defined",
                    this->token.line, this->token.column
            );
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
                if (builder->GetInsertBlock() && builder->GetInsertBlock()->getTerminator()) {
                    if (cnt != child_size) {
                        Warning(
                                __Warning,
                                "terminator detected, subsequent code will be ignored",
                                ast->getToken().line, ast->getToken().column
                        );
                    }
                    break;
                }
            } catch (Exception e) {
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
            throw ExceptionFactory(
                    __ErrReport,
                    "generated " + to_string(err_count) + " errors",
                    0, 0
            );
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
        symbol_table->setLoopExit(mergeBB);
        symbol_table->setLoopEntry(loopBB);

        llvm::Value *cond = this->condition->codeGen();
        if (!cond) {
            return nullptr;
        }
        if (cond->getType() != BOOL_TY) {
            if (cond->getType()->isIntegerTy()) {
                cond = builder->CreateICmpNE(cond, llvm::ConstantInt::get(cond->getType(), 0), "toBool");
            } else {
                cond = builder->CreateFCmpUNE(cond, llvm::ConstantFP::get(REAL_TY, 0.0), "toBool");
            }
        }
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
