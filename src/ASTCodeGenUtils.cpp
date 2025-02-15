/*
 * ASTCodeGenUtils.cpp 2022
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

    map<TokenType, llvm::Type *> token_to_simple_types;

    /*******************************************************
     *                     function                        *
     *******************************************************/
    void llvm_module_fpm_init() {
        if (opt_optimize) {
            the_function_fpm->add(llvm::createReassociatePass());
            the_function_fpm->add(llvm::createGVNPass());
            the_function_fpm->add(llvm::createInstructionCombiningPass());
            the_function_fpm->add(llvm::createCFGSimplificationPass());
            the_function_fpm->add(llvm::createDeadCodeEliminationPass());
            the_function_fpm->add(llvm::createFlattenCFGPass());
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
                throw ExceptionFactory<MissingException>(
                    "variable " + var->id + " is not defined in current function. "
                    "please check the variable name, global variable is not supported in derivation",
                    var->getToken().line,
                    var->getToken().column
                );
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
