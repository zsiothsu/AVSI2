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
#include <bits/getopt_ext.h>

#include "./inc/Parser.h"

using namespace std;
using namespace AVSI;

/*******************************************************
 *                 options handler                     *
 *******************************************************/
extern int optind, opterr, optopt;
extern char *optarg;
static int opt = 0, lopt = 0, loidx = 0;
extern char *file_name;

static struct option long_options[] = {
        {"ir",   no_argument, &lopt, 1},
        {"asm",  no_argument, &lopt, 2},
        {"help", no_argument, &lopt, 3},
        {0, 0,                0,     0}
};

static bool opt_ir = false;
static bool opt_asm = false;
static bool opt_help = false;

void printHelp(void) {
    string msg = \
    "usage:\n"
    "    avsi [options] file\n"
    "options:\n"
    "    -l     --ir        Generate LLVM IR.\n"
    "    -S     --asm       Generate assembly file.\n"
    "    -h     --help      Display available options.\n";

    printf("%s", msg.c_str());
}

void getOption(int argc, char **argv) {
    while ((opt = getopt_long(argc, argv, "lSh", long_options, &loidx)) != -1) {
        if (opt == 0) {
            opt = lopt;
        }
        switch (opt) {
            case 'l':
            case 1:
                opt_ir = true;
                break;
            case 'S':
            case 2:
                opt_asm = true;
                break;
            case 'h':
            case 3:
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
    file_name = fileName;
    ifstream file;
    file.open(fileName, ios::in);
    if (!file.is_open()) {
        cout << "AVSI: can't open file '" + string(fileName) + "'" << endl;
        return -1;
    }

    Lexer *lexer = new Lexer(&file);
    Parser *parser = new Parser(lexer);
    try {
        AST *tree = parser->parse();

        llvm_machine_init();
        llvm_module_fpm_init();
        tree->codeGen();
        if (opt_ir) llvm_module_printIR();
        if (opt_asm) llvm_asm_output();
        if (!(opt_ir || opt_asm))llvm_obj_output();
    }

    catch (Exception &e) {
        std::cerr << __COLOR_RED
                  << basename(fileName)
                  << ":" <<  e.line  << ":" << e.column + 1 << ": "
                  << e.what()
                  << __COLOR_RESET << std::endl;
        return 1;
    }
    return 0;
}
