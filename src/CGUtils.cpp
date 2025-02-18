/*
 * CGUtils.cpp 2025
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

#include "Exception.h"
#include "Token.h"
#include "SymbolTable.h"

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
    extern vector<string> module_path_with_module_name;

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

    extern map<string, llvm::Constant *> const_string_pool;

    /*******************************************************
     *                     function                        *
     *******************************************************/
    void llvm_module_fpm_init() {
        if (opt_optimize) {
            the_function_fpm->add(llvm::createReassociatePass());
            the_function_fpm->add(llvm::createConstantHoistingPass());
            the_function_fpm->add(llvm::createInstructionCombiningPass());
            the_function_fpm->add(llvm::createReassociatePass());
            the_function_fpm->add(llvm::createCFGSimplificationPass());
            the_function_fpm->add(llvm::createDeadCodeEliminationPass());
            the_function_fpm->add(llvm::createFlattenCFGPass());
            the_function_fpm->add(llvm::createGVNPass());
            the_function_fpm->add(llvm::createLICMPass());
            the_function_fpm->add(llvm::createLoopSinkPass());
            the_function_fpm->add(llvm::createLoopInstSimplifyPass());
            the_function_fpm->add(llvm::createLoopFlattenPass());
            the_function_fpm->add(llvm::createLoopRotatePass());
            the_function_fpm->add(llvm::createLoopIdiomPass());
            the_function_fpm->add(llvm::createLoopPredicationPass());
            the_function_fpm->add(llvm::createLoopVersioningLICMPass());
            the_function_fpm->add(llvm::createLoopDistributePass());
            the_function_fpm->add(llvm::createLoopFusePass());
            the_function_fpm->add(llvm::createGVNHoistPass());
            the_function_fpm->add(llvm::createSpeculativeExecutionPass());
            the_function_fpm->add(llvm::createIndVarSimplifyPass());


            the_function_fpm->add(llvm::createReassociatePass());
            the_function_fpm->add(llvm::createConstantHoistingPass());
            the_function_fpm->add(llvm::createInstructionCombiningPass());
            the_function_fpm->add(llvm::createDeadStoreEliminationPass());
            the_function_fpm->add(llvm::createAggressiveDCEPass());
            the_function_fpm->add(llvm::createAggressiveInstCombinerPass()); 
        }
        the_function_fpm->doInitialization();

        the_module_fpm->add(llvm::createFunctionInliningPass());
        the_module_fpm->add(llvm::createAlwaysInlinerLegacyPass());
    }

    /**
     * @description:    import specific module
     * @param:          path: path to module, both absolute and relative
     *                        are available
     * @param:          mod: module name
     * @param:          line: line of "import" keyword
     * @param:          col: column of "import" keyword
     * @param:          as: rename the module
     * @return:         none
     */
    void llvm_import_module(vector<string> path, string mod, int line, int col, string as) {
        // to check absolute or relative path
        auto module_path_size = module_path.size();
        bool is_absolute_module_path = true;
        bool is_std = false;

        if (!path.empty() && path[0] == "std") {
            is_std = true;
        }

        if(!path.empty() && path[0] == "root") {
            path.erase(path.begin());
            path.insert(path.begin(), package_path.begin(), package_path.end());
        } else {
            is_absolute_module_path = false;
        }

        // create alias
        if (is_absolute_module_path) {
            string unresolved_path_absulote = getpathListToUnresolved(path);
            unresolved_path_absulote += (unresolved_path_absulote.empty() ? "" : "::") + mod;

            // remove the module_path prefix so it can be treated as a relative path
            path.erase(path.begin(), path.begin() + module_path_size);

            string unresolved_path_relative = getpathListToUnresolved(path);
            unresolved_path_relative += (unresolved_path_relative.empty() ? "" : "::") + mod;

            // create alias: relative -> absolute
            //               renamed -> absolute
            module_name_alias[unresolved_path_relative] = unresolved_path_absulote;
            module_name_alias[unresolved_path_absulote] = unresolved_path_absulote;
            if (!as.empty())
                module_name_alias[as] = unresolved_path_absulote;
        } else if (is_std) {
            string unresolved_path_relative = getpathListToUnresolved(path);
            unresolved_path_relative += (unresolved_path_relative.empty() ? "" : "::") + mod;
            module_name_alias[unresolved_path_relative] = unresolved_path_relative;
            if (!as.empty())
                module_name_alias[as] = unresolved_path_relative;
        } else {
            string unresolved_path_relative = getpathListToUnresolved(path);
            unresolved_path_relative += (unresolved_path_relative.empty() ? "" : "::") + mod;
            string unresolved_path_absolute = getpathListToUnresolved(module_path);
            unresolved_path_absolute += (unresolved_path_absolute.empty() ? "" : "::") + unresolved_path_relative;

            module_name_alias[unresolved_path_relative] = unresolved_path_absolute;
            module_name_alias[unresolved_path_absolute] = unresolved_path_absolute;
            if (!as.empty())
                module_name_alias[as] = unresolved_path_absolute;
        }

        // get bc file and source file in file system
        string module_file_system_path;
        string module_source_file_system_path;
        module_file_system_path =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER;
        module_source_file_system_path =
                compiler_exec_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER;

        // create file path
        for (int i = 0; i < path.size(); i++) {
            module_file_system_path += SYSTEM_PATH_DIVIDER + path[i];
            module_source_file_system_path += SYSTEM_PATH_DIVIDER + path[i];
        }
        module_file_system_path += SYSTEM_PATH_DIVIDER + mod;
        module_source_file_system_path += SYSTEM_PATH_DIVIDER + mod;

        // add file extension.
        // if file is not exist, the name generated will be wrong
        // so create a backup
        string module_file_system_path_backup = module_file_system_path;
        if (std::filesystem::is_directory(std::filesystem::path(module_file_system_path))) {
            module_file_system_path += SYSTEM_PATH_DIVIDER + mod + ".bc";
        } else {
            module_file_system_path += ".bc";
        }
        if (std::filesystem::is_directory(std::filesystem::path(module_source_file_system_path))) {
            module_source_file_system_path += SYSTEM_PATH_DIVIDER + string(MODULE_INIT_NAME) + ".sl";
        } else {
            module_source_file_system_path += ".sl";
        }
        std::filesystem::path bcfile = module_file_system_path;
        std::filesystem::path sourcefile = module_source_file_system_path;

        // if bc file is not exist or source file changed,
        // compile files recursively
        if (
                std::filesystem::exists(sourcefile) && (!std::filesystem::exists(bcfile) ||
                                                        last_write_time(sourcefile) > last_write_time(bcfile) ||
                                                        sourcefile.filename().stem() == MODULE_INIT_NAME)) {
            pid_t pid = fork();
            int status = 0;
            if (pid == -1) {
                throw ExceptionFactory<SysErrException>(
                        "unable to fork program. paused",
                        line, col);
            } else if (pid > 1) {
                wait(&status);

                if (std::filesystem::is_directory(std::filesystem::path(module_file_system_path_backup))) {
                    module_file_system_path = module_file_system_path_backup + SYSTEM_PATH_DIVIDER + mod + ".bc";
                } else {
                    module_file_system_path = module_file_system_path_backup + ".bc";
                }
                bcfile = module_file_system_path;
            } else {
                string program = compiler_command_line;
                vector<const char*> args =
                {
                        program.c_str(),
                        module_source_file_system_path.c_str(),
                        "-o",
                        output_root_path.c_str(),
                        "-m"
                };

                if (opt_verbose) args.push_back("-v");
                if (opt_warning) args.push_back("-W");
                if (opt_optimize) args.push_back("-O");
                if (opt_ir) args.push_back("-l");

                if (opt_pic) args.push_back("-fpic");

                // include path
                for(int i = 1; i < include_path.size(); i++) {
                    args.push_back("-I");
                    args.push_back(include_path[i].c_str());
                }

                // package name
                string p;
                if(!package_path.empty()) {
                    bool first_flag = true;
                    for(auto i : package_path) {
                        if(!first_flag) {
                            p.append(".");
                        }
                        first_flag = false;
                        p.append(i);
                    }
                    args.push_back("--package-name");
                    args.push_back(p.c_str());
                }

                args.push_back((char const *)0);

                clog << "importing "
                     << __COLOR_GREEN
                     << getpathListToUnresolved(module_path)
                     << "::"
                     << (path.empty() ? "" : (getpathListToUnresolved(path) + "::"))
                     << mod
                     << __COLOR_RESET << " by " << __COLOR_GREEN
                     << getpathListToUnresolved(module_path)
                     << ((input_file_name_no_suffix == MODULE_INIT_NAME) ? "" : "::" + module_name_nopath)
                     << __COLOR_RESET << endl;
                if (opt_verbose) {
                    clog << args[0] << " "
                         << filesystem::absolute(sourcefile) << " "
                         << args[2] << " "
                         << filesystem::absolute(filesystem::path(output_root_path)) << " ";
                    for (int i = 4; i < args.size(); i++) {
                        if (args[i] != 0) {
                            clog << args[i] << " ";
                        }
                    }
                    clog << endl;
                }

                execvp(compiler_command_line.c_str(), (char * const *)args.data());
                exit(-1);
            }
        } else if (!std::filesystem::exists(bcfile)) {
            // try to search include path
            for (string p: include_path) {
                module_file_system_path = p;
                for (int i = 0; i < path.size(); i++) {
                    module_file_system_path += SYSTEM_PATH_DIVIDER + path[i];
                }
                module_file_system_path += SYSTEM_PATH_DIVIDER + mod;
                if (std::filesystem::is_directory(std::filesystem::path(module_file_system_path))) {
                    module_file_system_path += SYSTEM_PATH_DIVIDER + mod + ".bc";
                } else {
                    module_file_system_path += ".bc";
                }
                bcfile = module_file_system_path;

                if (std::filesystem::exists(bcfile))
                    break;
            }
        }

        if (!std::filesystem::exists(bcfile)) {
            string unparsed_name;
            for (string i: path) {
                unparsed_name += i + "::";
            }
            unparsed_name += mod;

            std::cerr << __COLOR_RED
                      << input_file_name
                      << ":" << line << ":" << col + 1 << ": "
                      << __MissingException << ": "
                      << "module "
                      << unparsed_name
                      << " is not found"
                      << __COLOR_RESET << std::endl;

            cout << "missing: " << bcfile << endl;
            exit(-1);
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

                throw ExceptionFactory<MissingException>(
                        "error occurred when reading module " + unparsed_name,
                        line, col);
            }
        }

        // create and import module
        llvm::SMDiagnostic smd = llvm::SMDiagnostic();
        auto ir = llvm::parseIR(*mbuf.get(), smd, *the_context);
        auto imported_module = ir.release();

        if (imported_module == nullptr) {
            string unparsed_name;
            for (string i: path) {
                unparsed_name += i + "::";
            }
            unparsed_name += mod;

            smd.print(unparsed_name.c_str(), llvm::errs());
            cout << smd.getLineContents().str() << " " << smd.getMessage().str() << endl;
            throw ExceptionFactory<IRErrException>(
                    "failed to import module " + unparsed_name,
                    line, col);
        }

        // import symbols
        // import function
        auto function_iter = imported_module->getFunctionList().begin();
        while (function_iter != imported_module->getFunctionList().end()) {
            auto fun = &(*function_iter);
            the_module->getOrInsertFunction(fun->getName(), fun->getFunctionType(), fun->getAttributes());
            function_iter++;
        }

        // import global
        auto global_iter = imported_module->getGlobalList().begin();
        while (global_iter != imported_module->getGlobalList().end()) {
            auto glb = &(*global_iter);
            the_module->getOrInsertGlobal(glb->getName(), glb->getValueType());
            global_iter++;
        }

        // import named metadata
        auto md_iter = imported_module->getNamedMDList().begin();
        while (md_iter != imported_module->getNamedMDList().end()) {
            auto md = &(*md_iter);
            if (the_module->getNamedMetadata(md->getName())) {
                md_iter++;
                continue;
            }
            auto new_md = the_module->getOrInsertNamedMetadata(md->getName());
            for (auto node: md->operands()) {
                new_md->addOperand(node);
            }
            md_iter++;
        }

        // import struct
        auto struct_iter = imported_module->getIdentifiedStructTypes();
        for (auto i: struct_iter) {
            ::size_t size = i->getNumElements();
            string id = string(i->getName());
            auto md = imported_module->getOrInsertNamedMetadata("struct." + id);

            StructDef *sd = new StructDef(i);
            for (int j = 0; j < size; j++) {
                auto member_name = llvm::dyn_cast<llvm::MDString>(
                        llvm::dyn_cast<llvm::MDNode>(md->getOperand(j))->getOperand(0)
                )->getString();
                sd->members[string(member_name)] = j;
            }
            auto struct_type_size = llvm::dyn_cast<llvm::MDString>(
                    llvm::dyn_cast<llvm::MDNode>(md->getOperand(size))->getOperand(0)
            )->getString();
            type_size[i] = stoi(string(struct_type_size));
            auto struct_type_name = llvm::dyn_cast<llvm::MDString>(
                    llvm::dyn_cast<llvm::MDNode>(md->getOperand(size + 1))->getOperand(0)
            )->getString();
            type_name[i] = struct_type_name;
            struct_types[id] = sd;
        }

        // import gereric
        for (auto i = imported_module->named_metadata_begin() ; i != imported_module->named_metadata_end(); i++) {
            GenericDef *gd = new GenericDef();
            if(i->getName().find("generic.") != string::npos) {
                int idx = atoi(
                    llvm::dyn_cast<llvm::MDString>(
                            llvm::dyn_cast<llvm::MDNode>(i->getOperand(0))->getOperand(0)
                    )->getString().str().c_str()
                );

                gd->idx = idx;

                for (int j = 1; j < i->getNumOperands(); j++) {
                    auto node = llvm::dyn_cast<llvm::MDNode>(i->getOperand(j));
                    auto mapped_func_name = llvm::dyn_cast<llvm::MDString>(node->getOperand(0))->getString().str();
                    auto ty_name = llvm::dyn_cast<llvm::MDString>(node->getOperand(1))->getString().str();
                    gd->function_map[ty_name] = mapped_func_name;
                }
            }

            generic_function[i->getName().substr(8).str()] = gd;
        }
    }

    /**
     * @description:    reset compiler
     * @return:         none
     */
    void llvm_global_context_reset() {
        // reset context and module
        delete TheTargetMachine;
        delete the_function_fpm;
        delete the_module_fpm;
        delete builder;
        delete the_module;
        delete the_context;

        the_context = new llvm::LLVMContext();
        the_module = new llvm::Module("program", *the_context);
        builder = new llvm::IRBuilder<>(*the_context);
        the_function_fpm = new llvm::legacy::FunctionPassManager(the_module);
        the_module_fpm = new llvm::legacy::PassManager();
        TheTargetMachine = nullptr;

        // reset symbols
        delete symbol_table;
        F64_TY = llvm::Type::getDoubleTy(*the_context);
        F32_TY = llvm::Type::getFloatTy(*the_context);
        I128_TY = llvm::Type::getInt128Ty(*the_context);
        I64_TY = llvm::Type::getInt64Ty(*the_context);
        I32_TY = llvm::Type::getInt32Ty(*the_context);
        I16_TY = llvm::Type::getInt16Ty(*the_context);
        I8_TY = llvm::Type::getInt8Ty(*the_context);
        I1_TY = llvm::Type::getInt1Ty(*the_context);
        VOID_TY = llvm::Type::getVoidTy(*the_context);
        ISIZE_TY = MACHINE_WIDTH_TY;


        symbol_table = new SymbolTable();
        struct_types.clear();
        function_protos.clear();
        simple_types.clear();
        simple_types_map.clear();
        type_name.clear();
        type_size.clear();

        simple_types = {
                F64_TY, F32_TY,
                I128_TY, I64_TY,
                I32_TY, I16_TY,
                I8_TY, I1_TY,
                VOID_TY, ISIZE_TY,
        };

        simple_types_map = {
                {F64_TY,  (uint8_t) (0x1 << 7)},
                {F32_TY,  (uint8_t) (0x1 << 6)},
                {I128_TY, (uint8_t) (0x1 << 5)},
                {I64_TY,  (uint8_t) (0x1 << 4)},
                {I32_TY,  (uint8_t) (0x1 << 3)},
                {I16_TY,  (uint8_t) (0x1 << 2)},
                {I8_TY,   (uint8_t) (0x1 << 1)},
                {I1_TY,   (uint8_t) 0x1},
                {ISIZE_TY, PTR_SIZE == 8 ? (uint8_t) (0x1 << 4) : (uint8_t) (0x1 << 3)}
        };
        type_name = {
                {F64_TY, "f64"}, {F64_TY->getPointerTo(), "f64*"},
                {F32_TY, "f32"}, {F32_TY->getPointerTo(), "f32*"},
                {I128_TY, "i128"}, {I128_TY->getPointerTo(), "i128*"},
                {I64_TY, "i64"}, {I64_TY->getPointerTo(), "i64*"},
                {I32_TY, "i32"}, {I32_TY->getPointerTo(), "i32*"},
                {I16_TY, "i16"}, {I16_TY->getPointerTo(), "i16*"},
                {I8_TY, "i8"}, {I8_TY->getPointerTo(), "i8*"},
                {I1_TY, "bool"}, {I1_TY->getPointerTo(), "bool*"},
                {VOID_TY, "void"},
                {ISIZE_TY, "isize"}, {ISIZE_TY->getPointerTo(), "isize*"},
                {nullptr, "default"}
        };
        
        type_size = {
                {F64_TY, 8}, {F64_TY->getPointerTo(), 1},
                {F32_TY, 4}, {F32_TY->getPointerTo(), 1},
                {I128_TY, 16}, {I128_TY->getPointerTo(), 1},
                {I64_TY, 8}, {I64_TY->getPointerTo(), 1},
                {I32_TY, 4}, {I32_TY->getPointerTo(), 1},
                {I16_TY, 2}, {I16_TY->getPointerTo(), 1},
                {I8_TY, 1}, {I8_TY->getPointerTo(), 1},
                {I1_TY, 1}, {I1_TY->getPointerTo(), 1},
                {VOID_TY, 0},
                {ISIZE_TY, PTR_SIZE}, {ISIZE_TY->getPointerTo(), 1},
        };

        token_to_simple_types = {
                {F64,   F64_TY},
                {F32,   F32_TY},
                {I128,  I128_TY},
                {I64,   I64_TY},
                {I32,   I32_TY},
                {I16,   I16_TY},
                {I8,    I8_TY},
                {BOOL,  I1_TY},
                {ISIZE, ISIZE_TY},
                {VOID,  VOID_TY}
        };

        module_name = "";
        the_module->setModuleIdentifier(input_file_name_no_suffix);
        the_module->setSourceFileName(input_file_name);

        global_insert_point = builder->GetInsertBlock();
    }

    /**
     * @description:    initializer for compiler, get information of target
     * @return:         none
     */
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
        auto RM = llvm::Optional<llvm::Reloc::Model>(opt_pic ? llvm::Reloc::PIC_ : llvm::Reloc::Static);
        TheTargetMachine =
                Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

        the_module->setDataLayout(TheTargetMachine->createDataLayout());
    }

    void llvm_emit_obj() {
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename;
        if (input_file_name_no_suffix == MODULE_INIT_NAME)
            file_basename = dir.string();
        else if (input_file_name_no_suffix == MODULE_LIB_NAME)
            file_basename = module_name_nopath;
        else
            file_basename = input_file_name_no_suffix;

        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER +
                string(file_basename) + ".o";
        llvm_create_dir(filesystem::path(Filename).parent_path());
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

        if (EC) {
            llvm::errs() << "Could not open file: " << EC.message();
            return;
        }

        llvm::ValueToValueMapTy vmt;
        auto clone = llvm::CloneModule(*the_module, vmt).release();

        llvm::legacy::PassManager pass;
        auto FileType = llvm::CGFT_ObjectFile;

        if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
            llvm::errs() << "TheTargetMachine can't emit a file of this type";
            return;
        }

        pass.run(*clone);
        dest.flush();

        if (opt_verbose)
            llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";

        dest.close();
    }

    void llvm_emit_asm() {
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename;
        if (input_file_name_no_suffix == MODULE_INIT_NAME)
            file_basename = dir.string();
        else if (input_file_name_no_suffix == MODULE_LIB_NAME)
            file_basename = module_name_nopath;
        else
            file_basename = input_file_name_no_suffix;

        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER +
                string(file_basename) + ".s";
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

        if (opt_verbose)
            llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";
    }

    void llvm_emit_bitcode() {
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename;
        if (input_file_name_no_suffix == MODULE_INIT_NAME)
            file_basename = dir.string();
        else if (input_file_name_no_suffix == MODULE_LIB_NAME)
            file_basename = module_name_nopath;
        else
            file_basename = input_file_name_no_suffix;

        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER +
                string(file_basename) + ".bc";
        llvm_create_dir(filesystem::path(Filename).parent_path());

        /*
         * It is important to CLONE the module.
         * Generating bitcode directly may cause segmentation fault.
         */
        llvm::ValueToValueMapTy vmt;
        auto clone = llvm::CloneModule(*the_module, vmt).release();

        vector<string> remove_list;
        for (auto &fun: clone->functions()) {
            if (fun.hasExternalLinkage()) {
                fun.deleteBody();
            } else {
                remove_list.emplace_back(fun.getName());
            }
        }

        for (const auto &fun_name: remove_list) {
            auto fun = clone->getFunction(fun_name);
            fun->replaceAllUsesWith(llvm::UndefValue::get((llvm::Type *) fun->getType()));
            fun->eraseFromParent();
        }

        remove_list.clear();
        for (auto &glb: clone->globals()) {
            if (glb.getLinkage() == llvm::GlobalValue::LinkageTypes::PrivateLinkage) {
                remove_list.emplace_back(string(glb.getName()));
            }
        }

        for (const auto &glb_name: remove_list) {
            auto glb = clone->getGlobalVariable(glb_name, true);
            glb->replaceAllUsesWith(llvm::UndefValue::get((llvm::Type *) glb->getType()));
            glb->eraseFromParent();
        }


        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);
        llvm::WriteBitcodeToFile(*clone, dest);

        dest.flush();

        if (opt_verbose) llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";

        dest.close();
    }

    void llvm_emit_ir() {
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename;
        if (input_file_name_no_suffix == MODULE_INIT_NAME)
            file_basename = dir.string();
        else if (input_file_name_no_suffix == MODULE_LIB_NAME)
            file_basename = module_name_nopath;
        else
            file_basename = input_file_name_no_suffix;

        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER +
                string(file_basename) + ".ll";
        llvm_create_dir(filesystem::path(Filename).parent_path());
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);
        the_module->print(dest, nullptr);
        if (opt_verbose)
            llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";
        dest.close();
    }

    string llvm_emit_cpp() {
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename;
        if (input_file_name_no_suffix == MODULE_INIT_NAME)
            file_basename = dir.string();
        else if (input_file_name_no_suffix == MODULE_LIB_NAME)
            file_basename = module_name_nopath;
        else
            file_basename = input_file_name_no_suffix;

        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER +
                string(file_basename) + ".i";
        llvm_create_dir(filesystem::path(Filename).parent_path());

        string param = input_file_name_raw + " -o " + Filename;
        FILE* p = popen(("cpp " + param).c_str(), "r");
        if (!p) {
            throw ExceptionFactory<SysErrException>(
                    "failed to run cpp",
                    0, 0);
        }

        char buf[1024];
        string result;
        while (fgets(buf, 1024, p)) {
            result += buf;
        }

        cout << result;

        int status = pclose(p);
        if (status != 0) {
            throw ExceptionFactory<SysErrException>(
                "some error while preprocessing", 0, 0
            );
        } else {
            return Filename;
        }
    }

    void llvm_run_optimization() {
        the_module_fpm->run(*the_module);
    }

    void debug_type(llvm::Value *v) {
        if (!v)
            return;
        v->getType()->print(llvm::outs());
        cout << endl;
    }

    void debug_type(llvm::Type *v) {
        if (!v)
            return;
        v->print(llvm::outs());
        cout << endl;
    }

    /**
     * @description:    create folders recursively
     * @param:          dir: folder's name
     * @return:         none
     */
    void llvm_create_dir(string dir) {
        std::filesystem::path d = dir;
        if (!filesystem::exists(d.parent_path())) {
            llvm_create_dir(d.parent_path());
        }
        filesystem::create_directory(d);
    }

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

    uint64_t registerType(llvm::Type *Ty) {
        if (Ty == nullptr) {
            return 0;
        }

        llvm::Constant *s = llvm::ConstantInt::get(I64_TY, the_module->getDataLayout().getTypeAllocSize(Ty));
        auto size = llvm::dyn_cast<llvm::ConstantInt>(s)->getLimitedValue();
        if (type_size.find(Ty) == type_size.end()) {
            type_size[Ty] = size;
        }
        if (type_name.find(Ty) == type_name.end()) {
            if (Ty->isArrayTy()) {
                string name = "arr[";
                registerType(Ty->getArrayElementType());
                name += type_name[Ty->getArrayElementType()];
                name += ";" + to_string(Ty->getArrayNumElements()) + "]";
                type_name[Ty] = name;
            } else if (Ty->isVectorTy()) {
                string name = "vec[";
                registerType(Ty->getScalarType());
                name += type_name[Ty->getScalarType()];
                name += ";" + to_string(((llvm::VectorType*)Ty)->getElementCount().getKnownMinValue()) + "]";
                type_name[Ty] = name;
            } else if (Ty->isPtrOrPtrVectorTy()) {
                string name;
                registerType(Ty->getPointerElementType());
                name += type_name[Ty->getPointerElementType()] + "*";
                type_name[Ty] = name;
            } else if (Ty->isStructTy()) {
                string name = "AnonymousObj{";
                int ele_num = Ty->getStructNumElements();
                bool first_flag = true;
                for (int i = 0; i < ele_num; i++) {
                    if (!first_flag) {
                        name += ",";
                    }
                    first_flag = false;
                    auto ty = Ty->getStructElementType(i);
                    registerType(ty);
                    name += type_name[ty];
                }
                name += "}";
                type_name[Ty] = name;
            } else {
                type_name[Ty] = "unnamedType";
            }
        }

        return size;
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
     * @description:    get offset address of variable
     * @param:          base: base address
     * @return:         a pointer to target address
     */
    llvm::Value *getOffset(llvm::Value *base, shared_ptr<AST> offset) {        
        auto current_ty = base->getType()->getPointerElementType();
        vector<llvm::Value *> offset_list;
        offset_list.push_back(llvm::ConstantInt::get(I32_TY, 0));
        if (current_ty->isArrayTy() || current_ty->isVectorTy()) {
            // array
            llvm::Value *index = offset->codeGen();

            if (index->getType()->isFloatingPointTy()) {
                index = builder->CreateFPToSI(index, I32_TY);
            } else if (index->getType() != I32_TY) {
                index = builder->CreateSExtOrTrunc(index, I32_TY);
            }
            offset_list.push_back(index);

            base = builder->CreateInBoundsGEP(
                    base->getType()->getScalarType()->getPointerElementType(),
                    base,
                    offset_list,
                    "idx_arr");
        } else if (
                current_ty->isStructTy()
                || (
                        current_ty->isPtrOrPtrVectorTy()
                        && current_ty->getPointerElementType()->isStructTy()
                )
                ) {
            // structure
            if (offset->__AST_name == __VARIABLE_NAME) {
                string member_name = static_pointer_cast<Variable>(offset)->id;
                bool find_flag = false;

                auto struct_ty = current_ty;
                if (current_ty->isPtrOrPtrVectorTy()) struct_ty = current_ty->getPointerElementType();

                for (auto iter: struct_types) {
                    if (iter.second->Ty == struct_ty) {
                        if (iter.second->members.find(member_name) == iter.second->members.end()) {
                            break;
                        }

                        auto index = iter.second->members[member_name];
                        offset_list.push_back(llvm::ConstantInt::get(
                                I32_TY,
                                index,
                                true));
                        find_flag = true;
                        break;
                    }
                }

                if (current_ty->isPtrOrPtrVectorTy()) {
                    base = builder->CreateLoad(current_ty, base, "struct");
                }

                if (find_flag) {
                    base = builder->CreateGEP(
                            llvm::cast<llvm::PointerType>(base->getType()->getScalarType())->getPointerElementType(),
                            base,
                            offset_list,
                            "idx_obj");
                } else {
                    throw ExceptionFactory<MissingException>(
                            "unrecognized member '" + member_name + "'",
                            offset->getToken().line, offset->getToken().column);
                }
            } else if (offset->__AST_name == __NUM_NAME) {
                offset_list.push_back(llvm::ConstantInt::get(
                        I32_TY,
                        static_pointer_cast<Num>(offset)->getValue().any_cast<int>(),
                        true));
                base = builder->CreateGEP(
                        llvm::cast<llvm::PointerType>(base->getType()->getScalarType())->getPointerElementType(),
                        base,
                        offset_list,
                        "idx_obj");
            }
        } else if (current_ty->isPtrOrPtrVectorTy()) {
            // raw pointer
            llvm::Value *index = offset->codeGen();

            if (index->getType()->isFloatingPointTy()) {
                index = builder->CreateFPToSI(index, MACHINE_WIDTH_TY);
            } else {
                index = builder->CreateSExtOrTrunc(index, MACHINE_WIDTH_TY);
            }

            index = builder->CreateMul(
                    index,
                    llvm::ConstantInt::get(
                            MACHINE_WIDTH_TY,
                            current_ty->getPointerElementType()->isPtrOrPtrVectorTy()
                            ? PTR_SIZE
                            : type_size[current_ty->getPointerElementType()]
                    )

            );

            base = builder->CreateLoad(current_ty, base);
            base = builder->CreatePtrToInt(base, ISIZE_TY, "idx_ptr_conv");
            base = builder->CreateAdd(base, index, "idx_ptr_conv");
            base = builder->CreateIntToPtr(base, current_ty, "idx_ptr_conv");
        } else {
            throw ExceptionFactory<TypeException>(
                    "subscripted value is not an array, pointer, or structure",
                    offset->getToken().line, offset->getToken().column);
        }

        return base;
    }

    llvm::Constant *getGlobalConstant(Variable *var) {
        auto modinfo = var->getToken().getModInfo();
        llvm::GlobalVariable *v;
        if (modinfo.empty()) {
            // function in currnet module
            v = the_module->getGlobalVariable(
                    getFunctionNameMangling(module_path_with_module_name, var->id)
            );

            if (!v) {
                v = the_module->getGlobalVariable(var->id);
            }
        } else if (modinfo[0] == "root") {
            // absolute path
            vector<string> p;
            modinfo.erase(modinfo.begin());
            p.insert(p.end(), package_path.begin(), package_path.end());
            p.insert(p.end(), modinfo.begin(), modinfo.end());
            v = the_module->getGlobalVariable(
                    getFunctionNameMangling(p, var->id));
        } else {
            // may be relative path
            auto fun_path = module_path;
            fun_path.insert(fun_path.end(), modinfo.begin(), modinfo.end());
            v = the_module->getGlobalVariable(
                    getFunctionNameMangling(fun_path, var->id));

            // may be alias
            if (!v) {
                string head = modinfo[0];
                if (module_name_alias.find(head) != module_name_alias.end()) {
                    auto path_cut = modinfo;
                    path_cut.erase(path_cut.begin());
                    head = module_name_alias[head];
                    auto head_to_origin = getpathUnresolvedToList(head);
                    for (auto i: path_cut) {
                        head_to_origin.push_back(i);
                    }
                    v = the_module->getGlobalVariable(
                            getFunctionNameMangling(head_to_origin, var->id));
                }
            }
        }   

        if (v) {
            if (v->hasAttribute(llvm::Attribute::ReadOnly)) {
                auto init = v->getInitializer();
                if (init && (llvm::dyn_cast<llvm::ConstantFP>(init)  || llvm::dyn_cast<llvm::ConstantInt>(init))) {
                    return init;
                } else {
                    return nullptr;
                }
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }

    /**
     * @description:    get address of variable
     * @param:          var: variable AST
     * @return:         a pointer to target address
     */
    llvm::Value *getAlloca(Variable *var, bool *is_const) {
        // remember that all value token from symbol table is address
        llvm::Value *v = symbol_table->find(var->id);
        auto modinfo = var->getToken().getModInfo();

        if (!v) {
            if (modinfo.empty()) {
                // function in currnet module
                v = the_module->getGlobalVariable(
                        getFunctionNameMangling(module_path_with_module_name, var->id)
                );

                if (!v) {
                    v = the_module->getGlobalVariable(var->id);
                }
            } else if (modinfo[0] == "root") {
                // absolute path
                vector<string> p;
                modinfo.erase(modinfo.begin());
                p.insert(p.end(), package_path.begin(), package_path.end());
                p.insert(p.end(), modinfo.begin(), modinfo.end());
                v = the_module->getGlobalVariable(
                        getFunctionNameMangling(p, var->id));
            } else {
                // may be relative path
                auto fun_path = module_path;
                fun_path.insert(fun_path.end(), modinfo.begin(), modinfo.end());
                v = the_module->getGlobalVariable(
                        getFunctionNameMangling(fun_path, var->id));

                // may be alias
                if (!v) {
                    string head = modinfo[0];
                    if (module_name_alias.find(head) != module_name_alias.end()) {
                        auto path_cut = modinfo;
                        path_cut.erase(path_cut.begin());
                        head = module_name_alias[head];
                        auto head_to_origin = getpathUnresolvedToList(head);
                        for (auto i: path_cut) {
                            head_to_origin.push_back(i);
                        }
                        v = the_module->getGlobalVariable(
                                getFunctionNameMangling(head_to_origin, var->id));
                    }
                }
            }

            if (v && is_const) {
                *is_const = llvm::dyn_cast<llvm::GlobalVariable>(v)->hasAttribute(llvm::Attribute::ReadOnly);
            }
        }

        if (v && !var->offset.empty()) {
            for (auto i: var->offset) {
                if (llvm::dyn_cast<llvm::Constant>(v) && llvm::dyn_cast<llvm::Constant>(v)->isNaN()) {
                    throw ExceptionFactory<LogicException>(
                            "cannot get element of null type",
                            i.second->getToken().line, i.second->getToken().column);
                }

                if (i.first == Variable::offsetType::MEMBER || i.first == Variable::offsetType::ARRAY) {
                    v = getOffset(v, i.second);
                } else {
                    auto left_ty = v->getType()->getPointerElementType();
                    if (
                            left_ty->isStructTy()
                            || (
                                    left_ty->isPtrOrPtrVectorTy()
                                    && left_ty->getPointerElementType()->isStructTy()
                            )
                            ) {
                        // get struct path to locate function
                        auto struct_name = type_name[left_ty->isPtrOrPtrVectorTy() ? left_ty->getPointerElementType()
                                                                                   : left_ty];
                        auto index = struct_name.find('{');
                        string part = struct_name.substr(0, index);
                        auto struct_path = getpathUnresolvedToList(part);
                        shared_ptr<FunctionCall> function_call_ast = static_pointer_cast<FunctionCall>(i.second);
                        auto old_function_token = function_call_ast->getToken();
                        auto old_param_list = function_call_ast->paramList;

                        // add param "this"
                        llvm::Value *param_this = v;
                        if (left_ty->isPtrOrPtrVectorTy()) {
                            param_this = builder->CreateLoad(left_ty, v, "this");
                        }

                        // create a new function call
                        Token casted_function_token = Token(ID, function_call_ast->id, old_function_token.line,
                                                            old_function_token.column);
                        casted_function_token.setModInfo(struct_path);
                        FunctionCall *new_function = new FunctionCall(function_call_ast->id,
                                                                      function_call_ast->paramList,
                                                                      casted_function_token);
                        new_function->param_this = param_this;
                        auto ret = new_function->codeGen();

                        // store return value
                        if (
                                !llvm::dyn_cast<llvm::Constant>(ret)
                                || (
                                        llvm::dyn_cast<llvm::Constant>(ret)
                                        && !llvm::dyn_cast<llvm::Constant>(ret)->isNaN()
                                )) {
                            llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
                            auto addr = allocaBlockEntry(the_scope, "call", ret->getType());
                            builder->CreateStore(ret, addr);
                            v = addr;
                        } else {
                            v = llvm::ConstantFP::getNaN(F64_TY);;
                        }
                    }
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
     * @return:         true: variable changed
     *                  false: rebind
     * @note:           new variable won't be created if the function is called by statement
     *                  that isn't assignment statement.
     *                  store "a = b" is allowed only in the following cases:
     *                  2. a and b are both basic number types
     *                  3. a except an array pointer and b is an array. a will point to be in
     *                     this case
     *                  4. a except an array and b is an array pointer.
     */
    bool
    store(
        AST *ast, llvm::Value *l_alloca_addr, 
        llvm::Value *r_value, bool assignment,
        string l_base_name
    ) {
        llvm::Value *r_alloca_addr = llvm::getLoadStorePointerOperand(r_value);
        llvm::Type *r_type = r_value->getType();

        llvm::Type *l_alloca_content_type = l_alloca_addr ? l_alloca_addr->getType()->getPointerElementType() : nullptr;
        llvm::Type *l_except_type =
                assignment
                ? (static_pointer_cast<Variable>(((Assign*)ast)->left))->Ty.first
                : nullptr;

        bool l_is_single_value;
        bool l_except_type_is_offered;

        if (!assignment) {
            l_is_single_value = true;
            l_except_type_is_offered = false;
        } else {
            l_is_single_value = (static_pointer_cast<Variable>(((Assign*)ast)->left))->offset.empty();
            l_except_type_is_offered = (static_pointer_cast<Variable>(((Assign*)ast)->left))->Ty.second != "none";
        }

        if (r_type == VOID_TY) {
            throw ExceptionFactory<TypeException>(
                    "cannot assign void to variable",
                    ast->getToken().line, ast->getToken().column);
        }

        // default : rebind variable
        llvm::Type *store_type = r_type;
        bool create_new_space = true;

        /**
         * function definition
         */
        auto assign_err = [&](llvm::Type *l, llvm::Type *r) -> void {
            registerType(l);
            registerType(r);

            throw ExceptionFactory<TypeException>(
                    "failed to store value, except type '" +
                    type_name[l] +
                    "', offered '" + type_name[r] + "'",
                    ast->getToken().line, ast->getToken().column);
        };

        /* v -> ptr -> symtab */
        auto assign = [&](llvm::Value *v, llvm::Value *ptr, llvm::Type *Ty) -> bool {
            llvm::AllocaInst *addr = (llvm::AllocaInst *) ptr;
            if (create_new_space) {
                llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
                addr = allocaBlockEntry(the_scope, l_base_name, Ty);
            }

            if (addr == nullptr) {
                throw ExceptionFactory<SysErrException>(
                        "failed to store value",
                        ast->getToken().line, ast->getToken().column);
            }

            builder->CreateStore(v, addr);
            if (create_new_space) {
                if (l_is_single_value && assignment) {
                    symbol_table->insert(l_base_name, addr, true);
                } else {
                    if (l_alloca_content_type) registerType(l_alloca_content_type);
                    registerType(v->getType());

                    throw ExceptionFactory<SysErrException>(
                            "failed to store value, left: " +
                            (l_alloca_content_type ? type_name[l_alloca_content_type] : string("null")) + " right: " +
                            type_name[v->getType()],
                            ast->getToken().line, ast->getToken().column);
                }
            }

            return !create_new_space;
        };

        /* ptr -> symtab */
        auto bind = [&](llvm::Value *ptr) -> void {
            if (l_is_single_value && assignment) {
                llvm::AllocaInst *addr = (llvm::AllocaInst *) ptr;
                symbol_table->insert(l_base_name, addr, true);
            }
        };

        auto memcp = [&](llvm::Value *l_addr, llvm::Value *r_addr, llvm::Type *l_type, llvm::Type *r_type) -> bool {
            llvm::AllocaInst *addr = (llvm::AllocaInst *) l_addr;
            if (create_new_space) {
                llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
                addr = allocaBlockEntry(the_scope, l_base_name, l_type);
            }

            if (addr == nullptr) {
                throw ExceptionFactory<SysErrException>(
                        "failed to store value",
                        ast->getToken().line, ast->getToken().column);
            }

            if (type_size.find(l_type) != type_size.end()) registerType(l_type);
            if (type_size.find(r_type) != type_size.end()) registerType(r_type);

            auto size = min(type_size[l_type], type_size[r_type]);
            builder->CreateMemCpy(l_addr, llvm::MaybeAlign(), r_addr, llvm::MaybeAlign(), size);

            if (create_new_space) {
                if (l_is_single_value && assignment) {
                    symbol_table->insert(l_base_name, addr, true);
                } else {
                    if (l_alloca_content_type) registerType(l_alloca_content_type);
                    registerType(r_addr->getType()->getPointerElementType());

                    throw ExceptionFactory<SysErrException>(
                            "failed to store value, left: " +
                            (l_alloca_content_type ? type_name[l_alloca_content_type] : string("null")) + " right: " +
                            type_name[r_addr->getType()->getPointerElementType()],
                            ast->getToken().line, ast->getToken().column);
                }
            }

            return !create_new_space;
        };

        if (simple_types.find(r_type) != simple_types.end()) {
            if (l_alloca_content_type && simple_types.find(l_alloca_content_type) != simple_types.end() &&
                l_alloca_content_type != VOID_TY) {
                store_type = l_alloca_content_type;
                create_new_space = false;
            }
            if (l_except_type_is_offered && l_except_type && l_except_type != l_alloca_content_type &&
                simple_types.find(l_except_type) != simple_types.end() &&
                l_except_type != VOID_TY) {
                store_type = l_except_type;
                create_new_space = true;
            } else if (l_except_type_is_offered && simple_types.find(l_except_type) == simple_types.end()) {
                assign_err(l_except_type, r_type);
            }

            bool is_l_fp = store_type->isFloatingPointTy();
            bool is_r_fp = r_type->isFloatingPointTy();
            if (simple_types_map[r_type] <= simple_types_map[store_type]) {
                if (is_r_fp && is_l_fp) {
                    r_value = builder->CreateFPExt(r_value, store_type, "conv.fp.ext");
                } else if (!is_r_fp && is_l_fp) {
                    if (r_value->getType() == I1_TY) {
                        r_value = builder->CreateUIToFP(r_value, store_type, "conv.zi.fp");
                    } else {
                        r_value = builder->CreateSIToFP(r_value, store_type, "conv.si.fp");
                    }
                } else {
                    if (r_value->getType() == I1_TY) {
                        r_value = builder->CreateZExt(r_value, store_type, "conv.zi.ext");
                    } else {
                        r_value = builder->CreateSExt(r_value, store_type, "conv.si.ext");
                    }
                }
            } else {
                if (is_r_fp && is_l_fp) {
                    r_value = builder->CreateFPTrunc(r_value, store_type, "conv.fp.trunc");
                } else if (is_r_fp && !is_l_fp) {
                    r_value = builder->CreateFPToSI(r_value, store_type, "conv.fp.si");
                } else {
                    r_value = builder->CreateTrunc(r_value, store_type, "conv.si.trunc");
                }
                Warning(
                    "implicit conversion from '" + type_name[r_type] + "' to '" + type_name[store_type] + "'",
                    ast->getToken().line,
                    ast->getToken().column
                );
            }

            return assign(r_value, create_new_space ? nullptr : l_alloca_addr, store_type);
        } else if (r_type->isArrayTy()) {
            llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
            if (!r_alloca_addr) {
                r_alloca_addr = allocaBlockEntry(the_scope, "arr.init", r_type);
                builder->CreateStore(r_value, r_alloca_addr);
            }

            if (!l_except_type_is_offered) {
                if (!l_alloca_addr) {
                    bind(r_alloca_addr);
                    return false;
                } else {
                    if (l_alloca_content_type->isArrayTy() && l_alloca_content_type->getArrayElementType() == r_type->getArrayElementType()) {
                        bind(r_alloca_addr);
                        return false;
                    } else if (l_alloca_content_type->isVectorTy() && l_alloca_content_type->getScalarType() == r_type->getArrayElementType()) {
                        create_new_space = false;
                        return memcp(l_alloca_addr, r_alloca_addr, l_alloca_content_type, r_type);
                    } else {
                        bind(r_alloca_addr);
                        return false;
                    }
                }
            } else {
                if (!l_alloca_addr) {
                    if (l_except_type->isVectorTy() && l_except_type->getScalarType() == r_type->getArrayElementType()) {
                        create_new_space = true;
                        return memcp(nullptr, r_alloca_addr, l_except_type, r_type);
                    } else if (l_except_type->isArrayTy() && l_except_type->getArrayElementType() == r_type->getArrayElementType()) {
                        bind(r_alloca_addr);
                        return false;
                    } else {
                        assign_err(l_except_type, r_type);
                    }
                } else {
                    if (l_except_type == l_alloca_content_type) {
                        bind(r_alloca_addr);
                        return false;
                    } else {
                        if (l_except_type->isVectorTy() && l_except_type->getScalarType() == r_type->getArrayElementType()) {
                            create_new_space = true;
                            return memcp(nullptr, r_alloca_addr, l_except_type, r_type);
                        } else if (l_except_type->isArrayTy() && l_except_type->getArrayElementType() == r_type->getArrayElementType()) {
                            bind(r_alloca_addr);
                            return false;
                        } else {
                            assign_err(l_except_type, r_type);
                        }
                    }
                }
            }
        } else if (r_type->isVectorTy()) {
            llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
            if (!r_alloca_addr) {
                r_alloca_addr = allocaBlockEntry(the_scope, "vec.init", r_type);
                builder->CreateStore(r_value, r_alloca_addr);
            }

            if (!l_except_type_is_offered) {
                if (!l_alloca_addr) {
                    bind(r_alloca_addr);
                    return false;
                } else {
                    if (l_alloca_content_type->isVectorTy() && l_alloca_content_type->getScalarType() == r_type->getArrayElementType()) {
                        bind(r_alloca_addr);
                        return false;
                    } else if (l_alloca_content_type->isArrayTy() && l_alloca_content_type->getArrayElementType() == r_type->getArrayElementType()) {
                        create_new_space = false;
                        return memcp(l_alloca_addr, r_alloca_addr, l_alloca_content_type, r_type);
                    } else {
                        bind(r_alloca_addr);
                        return false;
                    }
                }
            } else {
                if (!l_alloca_addr) {
                    if (l_except_type->isArrayTy() && l_except_type->getArrayElementType() == r_type->getArrayElementType()) {
                        create_new_space = true;
                        return memcp(nullptr, r_alloca_addr, l_except_type, r_type);
                    } else if (l_except_type->isVectorTy() && l_except_type->getScalarType() == r_type->getArrayElementType()) {
                        bind(r_alloca_addr);
                        return false;
                    } else {
                        assign_err(l_except_type, r_type);
                    }
                } else {
                    if (l_except_type == l_alloca_content_type) {
                        bind(r_alloca_addr);
                        return false;
                    } else {
                        if (l_except_type->isArrayTy() && l_except_type->getArrayElementType() == r_type->getArrayElementType()) {
                            create_new_space = true;
                            return memcp(nullptr, r_alloca_addr, l_except_type, r_type);
                        } else if (l_except_type->isVectorTy() && l_except_type->getScalarType() == r_type->getArrayElementType()) {
                            bind(r_alloca_addr);
                            return false;
                        } else {
                            assign_err(l_except_type, r_type);
                        }
                    }
                }
            }
        } else if (r_type->isPtrOrPtrVectorTy()) {
            if (l_alloca_content_type && l_alloca_content_type == r_type) {
                store_type = r_type;
                create_new_space = false;
                return assign(r_value, l_alloca_addr, store_type);
            } else if (l_except_type_is_offered && l_except_type != r_type) {
                assign_err(l_except_type, r_type);
            } else {
                store_type = r_type;
                create_new_space = true;
                return assign(r_value, nullptr, store_type);
            }
        } else if (r_type->isStructTy()) {
            if (l_alloca_content_type && l_alloca_content_type->isStructTy() &&
                ((llvm::StructType *) l_alloca_content_type)->isLayoutIdentical((llvm::StructType *) r_type)) {
                store_type = l_alloca_content_type;
                create_new_space = false;
                r_value = builder->CreateBitCast(r_value, l_alloca_content_type, "conv.bitcast");
                return assign(r_value, l_alloca_addr, store_type);
            } else if (l_except_type_is_offered && l_except_type->isStructTy() &&
                       ((llvm::StructType *) l_except_type)->isLayoutIdentical((llvm::StructType *) r_type)) {
                store_type = l_except_type;
                create_new_space = true;
                r_value = builder->CreateBitCast(r_value, l_alloca_content_type, "conv.bitcast");
                return assign(r_value, nullptr, store_type);
            } else if (l_except_type_is_offered && l_except_type != r_type) {
                assign_err(l_except_type, r_type);
            } else {
                store_type = r_type;
                create_new_space = true;
                return assign(r_value, nullptr, store_type);
            }
        }

        return false;
    }

    llvm::Value *type_conv(AST *ast, llvm::Value *v, llvm::Type *vtype, llvm::Type *etype, bool AllowPtrConv) {
        if (vtype == etype) return v;

        if (etype == VOID_TY) {
            throw ExceptionFactory<TypeException>(
                    "void type is not allowed",
                    ast->getToken().line, ast->getToken().column);
        }

        bool is_v_simple = simple_types.find(vtype) != simple_types.end();
        bool is_e_simple = simple_types.find(etype) != simple_types.end();

        if (is_v_simple && is_e_simple) {
            bool is_v_fp = vtype->isFloatingPointTy();
            bool is_e_fp = etype->isFloatingPointTy();
            if (simple_types_map[vtype] < simple_types_map[etype]) {
                if (is_v_fp && is_e_fp) {
                    return builder->CreateFPExt(v, etype, "conv.fp.ext");
                } else if (!is_v_fp && is_e_fp) {
                    if (v->getType() == I1_TY) {
                        return builder->CreateUIToFP(v, etype, "conv.zi.fp");
                    } else {
                        return builder->CreateSIToFP(v, etype, "conv.si.fp");
                    }
                } else {
                    if (v->getType() == I1_TY) {
                        return builder->CreateZExt(v, etype, "conv.zi.ext");
                    } else {
                        return builder->CreateSExt(v, etype, "conv.si.ext");
                    }
                }
            } else {
                if (is_v_fp && is_e_fp) {
                    return builder->CreateFPTrunc(v, etype, "conv.fp.trunc");
                } else if (is_v_fp && !is_e_fp) {
                    return builder->CreateFPToSI(v, etype, "conv.fp.si");
                } else {
                    return builder->CreateTrunc(v, etype, "conv.si.trunc");
                }
            }
        } else if (!(is_v_simple ^ is_e_simple)) {
            bool is_v_ptr = vtype->isPtrOrPtrVectorTy();
            bool is_e_ptr = etype->isPtrOrPtrVectorTy();
            bool is_v_arr = vtype->isArrayTy();
            bool is_e_arr = etype->isArrayTy();

            if (is_v_ptr && is_e_ptr && AllowPtrConv) {
                return builder->CreatePointerBitCastOrAddrSpaceCast(v, etype, "conv.ptr");
            } else if (is_v_ptr && is_e_arr) {
                throw ExceptionFactory<SyntaxException>(
                        "undefined C-like cast '" + type_name[vtype] + "' to '" + type_name[etype] +
                        "'. cast from pointer to array is not allowed",
                        ast->getToken().line, ast->getToken().column);
            } else if (is_v_arr && is_e_ptr) {
                auto addr = getLoadStorePointerOperand(v);
                if (addr) {
                    addr = builder->CreatePointerCast(addr, etype);
                    return addr;
                } else {
                    llvm::Function *the_scope = builder->GetInsertBlock()->getParent();
                    addr = allocaBlockEntry(the_scope, "cond", vtype);
                    builder->CreateStore(v, addr);
                    addr = builder->CreatePointerCast(addr, etype);
                    return addr;
                }

            }

            throw ExceptionFactory<TypeException>(
                    "undefined cast '" + type_name[vtype] + "' to '" + type_name[etype] + "'",
                    ast->getToken().line, ast->getToken().column);
        } else {
            bool is_v_ptr = vtype->isPtrOrPtrVectorTy();
            bool is_e_ptr = etype->isPtrOrPtrVectorTy();

            if (is_e_ptr) {
                return builder->CreateIntToPtr(v, etype
                );
            } else if (is_v_ptr) {
                return builder->CreatePtrToInt(v, etype
                );
            }

            throw ExceptionFactory<TypeException>(
                    "undefined cast '" + type_name[vtype] + "' to '" + type_name[etype] + "'",
                    ast->getToken().line, ast->getToken().column);
        }
    }
}
