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

using namespace std;
using namespace AVSI;

/*******************************************************
 *                 options handler                     *
 *******************************************************/
extern int optind, opterr, optopt;
//extern char *optarg;
static int opt = 0, lopt = 0, loidx = 0;

static struct option long_options[] = {
        {"ir",       no_argument,       NULL, 'l'},
        {"asm",      no_argument,       NULL, 'S'},
        {"module",   no_argument,       NULL, 'm'},
        {"reliance", no_argument,       NULL, 'r'},
        {"output",   required_argument, NULL, 'o'},
        {"help",     no_argument,       NULL, 'h'},
        {"verbose",  no_argument,       NULL, 'v'},
        {"include", required_argument,  NULL, 'I'},
        {0, 0, 0,                             0}
};

static bool opt_ir = false;
static bool opt_asm = false;
static bool opt_module = false;
bool opt_reliance = false;
static bool opt_help = false;
bool opt_verbose = false;

void printHelp(void) {
    string msg = \
    "usage:\n"
    "    avsi [options] file\n"
    "options:\n"
    "    -l             --ir        Generate .ll LLVM IR.\n"
    "    -S             --asm       Generate .s assembly file.\n"
    "    -m             --module    Generate .bc module file. \n"
    "    -r             --reliance  Generate .r reliance file for Makefile.\n"
    "    -o <dir>       --output    output to <dir>.\n"
    "    -h             --help      Display available options.\n"
    "    -v             --verbose   Display more details during building.\n"
    "    -I             --include   Add include path\n";

    printf("%s", msg.c_str());
}

void getOption(int argc, char **argv) {
    while ((opt = getopt_long(argc, argv, "lSmro:hvI:", long_options, &loidx)) != -1) {
        if (opt == 0) {
            opt = lopt;
        }
        filesystem::path path = output_root_path;
        filesystem::path t;
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
    std::filesystem::path tmp_file =
            output_root_path + SYSTEM_PATH_DIVIDER + input_file_path_relative + SYSTEM_PATH_DIVIDER +
            string(file_basename) + ".lock";
    if(std::filesystem::exists(tmp_file)) {
        std::cerr << __COLOR_RED
                  << input_file_name
                  << ": File lock detected\n"
                  << "please check dependencies between modules, circular dependencies are prohibited."
                  << __COLOR_RESET
                  << endl;
        std::filesystem::remove(tmp_file);
        return -1;
    } else {
        ofstream os(tmp_file, ios::app);
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

        if(err_count + warn_count != 0) {
            std::cerr << __COLOR_RESET
                  << std::endl
                  << input_file_name
                  << ":"
                  << "generate " << err_count << " errors,"
                  << warn_count << " warnings"
                  << __COLOR_RESET << std::endl;
        }

        if (opt_ir) llvm_module_printIR();
        if (opt_asm) llvm_asm_output();
        if (opt_module) llvm_module_output();
        if (!(opt_ir || opt_asm))llvm_obj_output();
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
        std::filesystem::remove(tmp_file);
        return 1;
    }

    // remove lock
    std::filesystem::remove(tmp_file);
    return 0;
}
