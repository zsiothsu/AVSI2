/*
 * @Author: Chipen Hsiao
 * @Date: 2020-05-01
 * @LastEditTime: 2020-06-02 15:46:03
 * @Description: entry for interpreter
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
        {0, 0, 0,                             0}
};

static bool opt_ir = false;
static bool opt_asm = false;
static bool opt_module = false;
bool opt_reliance = false;
static bool opt_help = false;

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
    "    -h             --help      Display available options.\n";

    printf("%s", msg.c_str());
}

void getOption(int argc, char **argv) {
    while ((opt = getopt_long(argc, argv, "lSmro:h", long_options, &loidx)) != -1) {
        if (opt == 0) {
            opt = lopt;
        }
        filesystem::path path = output_root_path;
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
    input_file_path_relative = p.parent_path().string();
    p = filesystem::absolute(p);
    input_file_path_absolut = p.parent_path().string();
    compiler_exec_path = filesystem::current_path().string();

    if (input_file_path_relative.find(compiler_exec_path)) {
        input_file_path_relative =
                "./" +
                input_file_path_relative.substr(
                        0, input_file_path_relative.find(compiler_exec_path)
                );
    }

    if (output_root_path.empty()) output_root_path = compiler_exec_path;

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

    try {
        llvm_global_context_reset();
        llvm_machine_init();
        llvm_module_fpm_init();

        Lexer *lexer = new Lexer(&file);
        Parser *parser = new Parser(lexer);
        AST *tree = parser->parse();
        if (opt_reliance) return 0;
        tree->codeGen();

        std::cerr << __COLOR_RESET
                  << std::endl
                  << input_file_name
                  << ":"
                  << "generate " << err_count << " errors,"
                  << warn_count << " warnings"
                  << __COLOR_RESET << std::endl;

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
        return 1;
    }
    return 0;
}
