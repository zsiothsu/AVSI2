/*
 * main.cpp 2022
 *
 * entry for interpreter
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
#include <unistd.h>
#include <memory>
#include <getopt.h>
#include <filesystem>
#include <bits/getopt_ext.h>
#include <llvm/Support/FileSystem.h>

#include "./inc/Parser.h"
#include "./inc/FileName.h"

#define AVSI_VERSION_MAJOR      0
#define AVSI_VERSION_MINOR      1
#define AVSI_VERSION_PATCH      1
#define AVSI_VERSION_STRING     AVSI_VERSION_MAJOR.AVSI_VERSION_MINOR.AVSI_VERSION_PATCH

#define _MACRO_TO_STR(m)        #m
#define MACRO_TO_STR(m)         _MACRO_TO_STR(m)

using namespace std;
using namespace AVSI;

/*******************************************************
 *                 options handler                     *
 *******************************************************/
extern int optind, opterr, optopt;
static int opt = 0, lopt = 0, loidx = 0;

static struct option long_options[] = {
        {"ir", no_argument, NULL, 'l'},
        {"asm", no_argument, NULL, 'S'},
        {"module", no_argument, NULL, 'm'},
        {"reliance", no_argument, NULL, 'r'},
        {"output", required_argument, NULL, 'o'},
        {"help", no_argument, NULL, 'h'},
        {"verbose", no_argument, NULL, 'v'},
        {"include", required_argument, NULL, 'I'},
        {"warning", no_argument, NULL, 'W'},
        {"optimize", no_argument, NULL, 'O'},
        {"package-name", required_argument, NULL, 100},
        {0, 0, 0, 0}
};

bool opt_ir = false;
static bool opt_asm = false;
bool opt_module = false;
static bool opt_help = false;
bool opt_reliance = false;
bool opt_verbose = false;
bool opt_warning = false;
bool opt_optimize = false;

void printHelp(void) {
    string version = \
    "avsi "  MACRO_TO_STR(AVSI_VERSION_STRING) " based on llvm " LLVM_VERSION_STRING;

    string msg = \
    "usage:\n"
    "    avsi [options] file\n"
    "options:\n"
    "    -l             --ir        Generate .ll LLVM IR.\n"
    "    -S             --asm       Generate .s assembly file.\n"
    "    -m             --module    Generate .bc module file. \n"
    "    -r             --reliance  Generate .r reliance file for Makefile.\n"
    "    -o <dir>       --output    Output to <dir>.\n"
    "    -h             --help      Display available options.\n"
    "    -v             --verbose   Display more details during building.\n"
    "    -I             --include   Add include path.\n"
    "    -W             --warning   Show all warnings.\n"
    "    -O             --optimize  Optimize code\n\n"
    "long options:\n"
    "    --package-name <name>      Set package name split by '.', e.g.  std.io.file\n";

    printf("%s\n\n%s", version.c_str(), msg.c_str());
}

void getOption(int argc, char **argv) {
    while ((opt = getopt_long(argc, argv, "lSmro:hvI:WO", long_options, &loidx)) != -1) {
        if (opt == 0) {
            opt = lopt;
        }
        filesystem::path path = output_root_path;
        filesystem::path t;
        string arg_string;
        switch (opt) {
            case 'l':
                opt_ir = true;
                break;
            case 'S':
                opt_asm = true;
                break;
            case 'm':
                opt_module = true;
                break;
            case 'r':
                opt_reliance = true;
                break;
            case 'o':
                output_root_path = string(optarg);
                path = output_root_path;
                output_root_path = filesystem::absolute(path).string();
                break;
            case 'h':
                opt_help = true;
                break;
            case 'v':
                opt_verbose = true;
                break;
            case 'I':
                t = filesystem::path(optarg);
                include_path.push_back(filesystem::absolute(t).string());
                break;
            case 'W':
                opt_warning = true;
                break;
            case 'O':
                opt_optimize = true;
                break;
            case 100:
                if(!package_path.empty()) {
                    cout << "redefined package name" << endl;
                    exit(-1);
                }

                arg_string.clear();
                arg_string = optarg;
                ::size_t index;
                while((index = arg_string.find('.')) != std::string::npos) {
                    string p = arg_string.substr(0,index);
                    package_path.push_back(p);
                    arg_string.erase(0, index + 1);
                }
                package_path.push_back(string(arg_string));
                break;
            default:
                printf("error: unsupported option");
                break;
        }
    }
}

/*******************************************************
 *                  program entry                      *
 *******************************************************/
int main(int argc, char **argv) {
    compiler_command_line = string(argv[0]);

    // add default search path
    include_path.emplace_back("/usr/include/avsi");

    getOption(argc, argv);

    if (opt_help) {
        printHelp();
        return 0;
    }

    if (optind >= argc) {
        cout << "AVSI: no input file" << endl;
        return -1;
    }

    char *fileName = argv[optind];
    input_file_name = basename(fileName);
    input_file_name_no_suffix = input_file_name.substr(0, input_file_name.find('.'));

    ifstream file;
    file.open(fileName, ios::in);
    if (!file.is_open()) {
        cout << "AVSI: can't open file '" + string(fileName) + "'" << endl;
        return -1;
    }

    filesystem::path p = fileName;
    p = filesystem::absolute(p);
    input_file_path_relative = filesystem::absolute(p.parent_path()).string();
    input_file_path_absolut = p.parent_path().string();
    compiler_exec_path = filesystem::absolute(filesystem::current_path()).string();

    if (input_file_path_relative.find(compiler_exec_path) != string::npos) {
        input_file_path_relative =
                "./" +
                input_file_path_relative.substr(
                        input_file_path_relative.find(compiler_exec_path) + compiler_exec_path.size()
                );
    }

    if (output_root_path.empty()) output_root_path = compiler_exec_path + SYSTEM_PATH_DIVIDER + "build";

    if (opt_reliance) {
        auto RFilename =
                output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER
                + string(input_file_name_no_suffix) + ".r";

        llvm_create_dir(filesystem::path(RFilename).parent_path());
        std::error_code EC;
        llvm::raw_fd_ostream dest(RFilename, EC, llvm::sys::fs::OF_None);
        dest << "\n";
    }

    extern uint16_t err_count;
    extern uint16_t warn_count;

    // create lock file
    std::filesystem::path dir = filesystem::path(input_file_path_absolut).filename();
    string file_basename = input_file_name_no_suffix == MODULE_INIT_NAME ? dir.string() : input_file_name_no_suffix;
    std::filesystem::path lock_file =
            output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER +
            string(file_basename) + ".lock";
    if (std::filesystem::exists(lock_file)) {
        std::cerr << __COLOR_RED
                  << input_file_name
                  << ": File lock detected\n"
                  << "please check dependencies between modules, circular dependencies are prohibited."
                  << __COLOR_RESET
                  << endl;
        std::filesystem::remove(lock_file);
        return -1;
    } else {
        ofstream os(lock_file, ios::app);
        os << " ";
        os.close();
    }

    try {
        llvm_global_context_reset();
        llvm_machine_init();
        llvm_module_fpm_init();

        Lexer *lexer = new Lexer(&file);
        Parser *parser = new Parser(lexer);
        AST *tree = parser->parse();
        if (opt_reliance) return 0;
        tree->codeGen();

        if (err_count + warn_count != 0) {
            std::cerr << __COLOR_RESET
                      << std::endl
                      << input_file_name
                      << ":"
                      << "generate " << err_count << " errors,"
                      << warn_count << " warnings"
                      << __COLOR_RESET << std::endl;
        }

        if (opt_ir) llvm_emit_ir();
        if (opt_asm) llvm_emit_asm();
        if (opt_module) llvm_emit_bitcode();
        if (!(opt_ir || opt_asm))llvm_emit_obj();
    } catch (Exception &e) {
        if (e.type() == __ErrReport) {
            std::cerr << __COLOR_RED
                      << std::endl
                      << input_file_name
                      << ":"
                      << "generate " << err_count << " errors,"
                      << warn_count << " warnings"
                      << __COLOR_RESET << std::endl;
        }
        std::filesystem::remove(lock_file);
        return 1;
    }

    // remove lock
    std::filesystem::remove(lock_file);
    return 0;
}
