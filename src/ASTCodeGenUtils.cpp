/*
 * ASTCodeGenUtils.cpp 2022
 *
 * MIT License
 *
 * Copyright (c) 2022 Chipen Hsiaoman
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
extern bool opt_verbose;

namespace AVSI {
    using namespace std;

    /*******************************************************
     *                      llvm base                      *
     *******************************************************/
    extern string module_name;
    extern vector<string> module_path;

    extern llvm::LLVMContext *the_context;
    extern llvm::Module *the_module;
    extern llvm::IRBuilder<> *builder;
    extern llvm::legacy::FunctionPassManager *the_fpm;
    extern llvm::TargetMachine *TheTargetMachine;
    
    extern llvm::BasicBlock *global_insert_point;

    /*******************************************************
     *               protos & definition                   *
     *******************************************************/
    extern llvm::Type *REAL_TY;
    extern llvm::Type *CHAR_TY;
    extern llvm::Type *VOID_TY;
    extern llvm::Type *BOOL_TY;

    extern AST *ASTEmpty;
    extern AST *ASTEmptyNotEnd;

    extern SymbolTable *symbol_table;

    extern map<string, StructDef *> struct_types;
    extern map<std::string, llvm::FunctionType *> function_protos;
    extern set<llvm::Type *> simple_types;
    extern map<llvm::Type *, uint8_t> simple_types_map;

    extern map<llvm::Type *, string> type_name;
    extern map<llvm::Type *, uint32_t> type_size;

    extern map<string, string> module_name_alias;

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
        the_fpm->doFinalization();
    }

    void llvm_import_module(vector<string> path, string mod, int line, int col, string as) {
        // import module can use absolute path or relative path
        // resolve path to locate module
        if (input_file_name_no_suffix == MODULE_INIT_NAME) {
            std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
            module_path.push_back(dir);
        }

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

        // get bc file path in file system
        string module_file_system_path;
        string module_source_file_system_path;
        if (is_absolute_module_path) {
            module_file_system_path =
                    output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER;
            module_source_file_system_path =
                    compiler_exec_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER;

            string unresolved_path = getpathListToUnresolved(path);
            unresolved_path += (unresolved_path.empty() ? "" : "::") + mod;
            module_name_alias[unresolved_path] = unresolved_path;

            path.erase(path.begin(), path.begin() + path_size);
            string unresolved_path_relative = getpathListToUnresolved(path);
            unresolved_path_relative += (unresolved_path_relative.empty() ? "" : "::") + mod;
            module_name_alias[unresolved_path_relative] = unresolved_path;
            if(!as.empty()) module_name_alias[as] = unresolved_path;
        } else {
            module_file_system_path =
                    output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER;
            module_source_file_system_path =
                    compiler_exec_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER;

            // create alias: relative -> absolute
            string unresolved_path = getpathListToUnresolved(path);
            unresolved_path += (unresolved_path.empty() ? "" : "::") + mod;
            string unresolved_path_absolute = getpathListToUnresolved(module_path);
            unresolved_path_absolute += "::" + unresolved_path;
            module_name_alias[unresolved_path] = unresolved_path_absolute;
            module_name_alias[unresolved_path_absolute] = unresolved_path_absolute;
            if(!as.empty()) module_name_alias[as] = unresolved_path_absolute;
        }
        for (int i = 0; i < path.size(); i++) {
            module_file_system_path += SYSTEM_PATH_DIVIDER + path[i];
            module_source_file_system_path += SYSTEM_PATH_DIVIDER + path[i];
        }
        module_file_system_path += SYSTEM_PATH_DIVIDER + mod;
        module_source_file_system_path += SYSTEM_PATH_DIVIDER + mod;
        string module_file_system_path_not_gen = module_file_system_path;
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
        if (
                !std::filesystem::exists(bcfile) ||
                (
                        std::filesystem::exists(sourcefile) &&
                        last_write_time(sourcefile) > last_write_time(bcfile)
                ) ||
                sourcefile.filename().stem() == MODULE_INIT_NAME
                ) {
            pid_t pid = fork();
            int status = 0;
            if (pid == -1) {
                throw ExceptionFactory<SysErrException>(
                        "unable to fork program. paused",
                        line, col);
            } else if (pid > 1) {
                wait(&status);

                if (std::filesystem::is_directory(std::filesystem::path(module_file_system_path_not_gen))) {
                    module_file_system_path = module_file_system_path_not_gen + SYSTEM_PATH_DIVIDER + mod + ".bc";
                } else {
                    module_file_system_path = module_file_system_path_not_gen + ".bc";
                }
                bcfile = module_file_system_path;
            } else {
                string program = compiler_command_line;
                char const *args[7] = {
                        program.c_str(),
                        module_source_file_system_path.c_str(),
                        "-o",
                        output_root_path.c_str(),
                        "-m",
                        (char *) 0,
                        (char *) 0
                };
                if (opt_verbose) args[5] = "-v";

                clog << "importing "
                     << __COLOR_GREEN
                     << getpathListToUnresolved(module_path)
                     << "::"
                     << (path.empty() ? "" : (getpathListToUnresolved(path) + "::"))
                     << mod
                     << __COLOR_RESET << " by " << __COLOR_GREEN
                     << getpathListToUnresolved(module_path)
                     << "::"
                     << module_name
                     << __COLOR_RESET << endl;
                if (opt_verbose) {
                    clog << args[0] << " "
                         << filesystem::absolute(sourcefile) << " "
                         << args[2] << " "
                         << filesystem::absolute(filesystem::path(output_root_path)) << " "
                         << args[4] << " "
                         << args[5] << endl;
                }

                execve(compiler_command_line.c_str(), (char *const *) args, nullptr);
                exit(-1);
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

            cout << bcfile << endl;
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
                {BOOL_TY, 0x01}};
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
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename = input_file_name_no_suffix == MODULE_INIT_NAME ? dir.string() : input_file_name_no_suffix;

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

        llvm::legacy::PassManager pass;
        auto FileType = llvm::CGFT_ObjectFile;

        if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
            llvm::errs() << "TheTargetMachine can't emit a file of this type";
            return;
        }

        pass.run(*the_module);
        dest.flush();

        if (opt_verbose) llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";
    }

    void llvm_asm_output() {
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename = input_file_name_no_suffix == MODULE_INIT_NAME ? dir.string() : input_file_name_no_suffix;

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

        if (opt_verbose) llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";
    }

    void llvm_module_output() {
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename = input_file_name_no_suffix == MODULE_INIT_NAME ? dir.string() : input_file_name_no_suffix;

        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER +
                string(file_basename) + ".bc";
        llvm_create_dir(filesystem::path(Filename).parent_path());
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

        llvm::WriteBitcodeToFile(*the_module, dest);

        dest.flush();

        FILE *file = fopen(Filename.c_str(), "r");
        fflush(file);

        if (opt_verbose) llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";
    }

    void llvm_module_printIR() {
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename = input_file_name_no_suffix == MODULE_INIT_NAME ? dir.string() : input_file_name_no_suffix;

        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER +
                string(file_basename) + ".ll";
        llvm_create_dir(filesystem::path(Filename).parent_path());
        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);
        the_module->print(dest, nullptr);
        if (opt_verbose) llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";
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

    void llvm_create_dir(string dir) {
        std::filesystem::path d = dir;
        if (!filesystem::exists(d.parent_path())) {
            llvm_create_dir(d.parent_path());
        }
        filesystem::create_directory(d);
    }
}
