/*
 * main.cpp 2022
 *
 * entry for cart
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

#include <iostream>
#include <filesystem>
#include <getopt.h>
#include <bits/getopt_ext.h>
#include <string>

#include "./inc/cmd.h"

using namespace std;

/*******************************************************
 *                      hasher                         *
 *******************************************************/
typedef std::uint64_t hash_t;
constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;

constexpr hash_t hasher(char const *str, hash_t last_value = basis) {
    return *str ? hasher(str + 1, (*str ^ last_value) * prime) : last_value;
}

/*******************************************************
 *                 options handler                     *
 *******************************************************/
extern int optind, opterr, optopt;
static int opt = 0, lopt = 0, loidx = 0;

static struct option long_options[] = {
        {"help",    no_argument, NULL, 'h'},
        {"verbose", no_argument, NULL, 'v'},
        {0, 0, 0,                      0}
};

bool opt_help = false;
bool opt_verbose = false;

void printHelp(void) {
    string version = "cart " "0.0.1";
    string msg = \
            "USAGE:\n"
            "cart [OPTIONS] [SUBCOMMAND]]\n\n"
            "OPTIONS:\n"
            "    -h     --help      Print this help message\n"
            "    -v     --verbose   Use verbose output\n\n"
            "SUBCOMMAND:\n"
            "   new     Create a new project\n"
            "   build   Compile current project\n"
            "   clean   Clean building\n";

    printf("%s\n\n%s", version.c_str(), msg.c_str());
}

void getOption(int argc, char **argv) {
    while ((opt = getopt_long(argc, argv, "hv", long_options, &loidx)) != -1) {
        if (opt == 0) {
            opt = lopt;
        }
        string arg_string;
        switch (opt) {
            case 'h':
                opt_help = true;
                break;
            case 'v':
                opt_verbose = true;
                break;
            default:
                printf("error: unsupported option");
                break;
        }
    }
}

int main(int argc, char **argv) {
    getOption(argc, argv);

    if (optind >= argc && opt_help) {
        printHelp();
        return 0;
    }

    char const *cmd = argv[optind++];

    switch (hasher(cmd)) {
        case hasher("new"):
            cart::exec_new(argc, argv, optind);
            break;
        case hasher("build"):
            cart::exec_build(argc, argv, optind);
            break;
        case hasher("clean"):
            cart::exec_clean(argc, argv, optind);
            break;
        default:
            cout << "cart: unsupported command" << endl;
            break;
    }

    return 0;
}