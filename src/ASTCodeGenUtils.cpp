/*
 * ASTCodeGenUtils.cpp 2022
 *
 * utils for code generator
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
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/Utils/ValueMapper.h>

extern bool opt_ir;
extern bool opt_module;
extern bool opt_reliance;
extern bool opt_verbose;
extern bool opt_warning;
extern bool opt_optimize;

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
    extern llvm::legacy::FunctionPassManager *the_fpm;
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

    map<TokenType, llvm::Type *> token_to_simple_types;

    /*******************************************************
     *                     function                        *
     *******************************************************/
    void llvm_module_fpm_init() {
        if (opt_optimize) {
            the_fpm->add(llvm::createReassociatePass());
            the_fpm->add(llvm::createGVNPass());
            the_fpm->add(llvm::createInstructionCombiningPass());
            the_fpm->add(llvm::createCFGSimplificationPass());
            the_fpm->add(llvm::createDeadCodeEliminationPass());
            the_fpm->add(llvm::createFlattenCFGPass());
        }

        the_fpm->doInitialization();
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
        auto path_size = module_path.size();
        bool is_absolute_module_path = true;
        if (path.size() < path_size) {
            is_absolute_module_path = false;
        } else {
            /**
             * if module_path is the prefix of import module path,
             * the provided path will be considered an absolute path
             */
            for (int i = 0; i < path_size; i++) {
                if (module_path[i] != path[i]) {
                    is_absolute_module_path = false;
                    break;
                }
            }
        }

        // create alias
        if (is_absolute_module_path) {
            string unresolved_path_absulote = getpathListToUnresolved(path);
            unresolved_path_absulote += (unresolved_path_absulote.empty() ? "" : "::") + mod;

            // remove the module_path prefix so it can be treated as a relative path
            path.erase(path.begin(), path.begin() + path_size);

            string unresolved_path_relative = getpathListToUnresolved(path);
            unresolved_path_relative += (unresolved_path_relative.empty() ? "" : "::") + mod;

            // create alias: relative -> absolute
            //               renamed -> absolute
            module_name_alias[unresolved_path_relative] = unresolved_path_absulote;
            module_name_alias[unresolved_path_absulote] = unresolved_path_absulote;
            if (!as.empty())
                module_name_alias[as] = unresolved_path_absulote;
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
                const uint16_t arg_size = 9;
                char const *args[arg_size] = {
                        program.c_str(),
                        module_source_file_system_path.c_str(),
                        "-o",
                        output_root_path.c_str(),
                        "-m",
                        (char *) 0,
                        (char *) 0,
                        (char *) 0,
                        (char *) 0};
                int index = 5;
                if (opt_verbose)
                    args[index++] = "-v";
                if (opt_warning)
                    args[index++] = "-W";
                if (opt_optimize)
                    args[index++] = "-O";

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
                    for (int i = 4; i < arg_size - 1; i++) {
                        if (args[i] != 0) {
                            clog << args[i] << " ";
                        }
                    }
                    clog << endl;
                }

                execvp(compiler_command_line.c_str(), (char *const *) args);
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
        auto ptr = ir.release();

        if (ptr == nullptr) {
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
            the_module->getOrInsertGlobal(glb->getName(), glb->getValueType());
            global_iter++;
        }

        auto md_iter = ptr->getNamedMDList().begin();
        while (md_iter != ptr->getNamedMDList().end()) {
            auto md = &(*md_iter);
            auto new_md = the_module->getOrInsertNamedMetadata(md->getName());
            for (auto node: md->operands()) {
                new_md->addOperand(node);
            }
            md_iter++;
        }

        auto struct_iter = ptr->getIdentifiedStructTypes();
        for (auto i: struct_iter) {
            ::size_t size = i->getNumElements();
            string id = string(i->getName());
            auto md = ptr->getOrInsertNamedMetadata("struct." + id);

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
    }

    /**
     * @description:    reset compiler
     * @return:         none
     */
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
                {F64_TY,   "f64"},
                {F32_TY,   "f32"},
                {I128_TY,  "i128"},
                {I64_TY,   "i64"},
                {I32_TY,   "i32"},
                {I16_TY,   "i16"},
                {I8_TY,    "i8"},
                {I1_TY,    "bool"},
                {VOID_TY,  "void"},
                {ISIZE_TY, "isize"}
        };
        type_size = {
                {F64_TY,  8},
                {F32_TY,  4},
                {I128_TY, 16},
                {I64_TY,  8},
                {I32_TY,  4},
                {I16_TY,  2},
                {I8_TY,   1},
                {I1_TY,   1},
                {VOID_TY, 0},
                {ISIZE_TY, PTR_SIZE},
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
        auto RM = llvm::Optional<llvm::Reloc::Model>();
        TheTargetMachine =
                Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

        the_module->setDataLayout(TheTargetMachine->createDataLayout());
    }

    void llvm_emit_obj() {
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

        if (opt_verbose)
            llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";

        dest.close();
    }

    void llvm_emit_asm() {
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

        if (opt_verbose)
            llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";
    }

    void llvm_emit_bitcode() {
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename = input_file_name_no_suffix == MODULE_INIT_NAME ? dir.string() : input_file_name_no_suffix;

        auto Filename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER +
                string(file_basename) + ".bc";
        llvm_create_dir(filesystem::path(Filename).parent_path());

        /*
         * It is important to CLONE the module.
         * Since the optimization may change the module.
         * And generating bitcode directly may cause segmentation fault.
         */
        llvm::ValueToValueMapTy vmt;
        auto clone = llvm::CloneModule(*the_module, vmt).release();

        std::error_code EC;
        llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);
        llvm::WriteBitcodeToFile(*clone, dest);

        dest.flush();

        if (opt_verbose) llvm::outs() << "Wrote " << filesystem::absolute(filesystem::path(Filename)) << "\n";

        dest.close();
    }

    void llvm_emit_ir() {
        std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
        string file_basename = input_file_name_no_suffix == MODULE_INIT_NAME ? dir.string() : input_file_name_no_suffix;

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
}
