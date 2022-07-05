/*
 * cmd.cpp 2022
 *
 * cart command
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

#include "../inc/cmd.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <dirent.h>
#include <cstdlib>
#include <unistd.h>
#include <wait.h>
#include <queue>
#include "../inc/tomlcpp.hpp"

#define __COLOR_RESET       "\033[0m"
#define __COLOR_RED         "\033[31m"
#define __COLOR_GREEN       "\033[32m"
#define __COLOR_YELLOW      "\033[33m"

extern bool opt_verbose;

namespace cart {
    using namespace std;

    void searchObjs(vector<string> &objs) {
        filesystem::path current_dir = filesystem::current_path();
        filesystem::path build_path = current_dir.string() + SYSTEM_PATH_DIVIDER + "build";

        queue<filesystem::path> searchq;
        searchq.push(build_path);
        while (!searchq.empty()) {
            auto current = searchq.front();
            searchq.pop();
            for (auto &content: filesystem::directory_iterator(current)) {
                auto p = content.path();
                if (filesystem::is_directory(p)) {
                    searchq.push(p);
                } else {
                    StringRef base = basename(p.c_str());
                    if (base.endswith(".o")) {
                        objs.push_back(p.c_str());
                    }
                }
            }
        }
    }

    void exec_new(int argc, char **argv, int ind) {
        filesystem::path current_dir = filesystem::current_path();

        if (opt_help) {
            cout \
 << "cart new\n\n"
 << "USAGE:\n"
 << "cart new <name> Create a project with specified name"
 << endl;
            exit(0);
        }

        if (ind >= argc) {
            cout << "cart: missing package name" << endl;
            exit(-1);
        }

        // create directory
        string package_name = argv[ind++];
        filesystem::path project_dir = current_dir.string() + SYSTEM_PATH_DIVIDER + package_name;
        filesystem::create_directory(project_dir);

        // create configure file
        filesystem::path cfg = project_dir.string() + SYSTEM_PATH_DIVIDER + "Cart.toml";
        filesystem::path main_file = project_dir.string() + SYSTEM_PATH_DIVIDER + "main.sl";

        std::ofstream cfg_stream(cfg, ios::out);
        if (cfg_stream.is_open()) {
            cfg_stream
                    << "[project]" << endl
                    << "name = \"" << package_name << "\"" << endl
                    << R"(version = "0.0.1")" << endl
                    << R"(entry = "main.sl")" << endl
                    << R"(ccflags = ["-O", "-W"])" << endl
                    << R"(ldflags = [])" << endl
                    << R"(target = "a.out")" << endl
                    << R"(nostd = false)" << endl
                    << endl
                    << R"([path])" << endl
                    << R"(#include_path = ["/usr/include/avsi"])" << endl
                    << R"(#library_path = ["/usr/lib"])" << endl
                    << R"(#gcc_toolchain = "")";
            cfg_stream.close();
        } else {
            cout << "cart: cannot create files" << endl;
            exit(-1);
        }

        std::ofstream main_stream(main_file, ios::out);
        if (main_stream.is_open()) {
            main_stream
                    << R"(mod main)" << endl
                    << endl
                    << R"(import std::io)" << endl
                    << endl
                    << R"(export function main(argc: i32, argv: vec[vec[char;0];0]) -> i32 {)" << endl
                    << R"(    std::io::printStr("Hello World!\n"))" << endl
                    << R"(    return 0)" << endl
                    << R"(})" << endl;
        } else {
            cout << "cart: cannot create files" << endl;
            exit(-1);
        }
    }

    void exec_build(int argc, char **argv, int ind) {
        auto configure = toml::parseFile("./Cart.toml");

        if (!configure.table) {
            cout << "cart: current directory isn't a project" << endl;
            exit(-1);
        }

        auto project = configure.table->getTable("project");
        if (!project) {
            cout << "cart: missing [project] in Cart.toml" << endl;
            exit(-1);
        }

        auto [ok_name, name] = project->getString("name");
        auto [ok_entry, entry_file] = project->getString("entry");
        auto [ok_target, target_file] = project->getString("target");
        auto [ok_nostd, _nostd] = project->getBool("nostd");
        auto ccflags = project->getArray("ccflags").release();
        auto ldflags = project->getArray("ldflags").release();

        if (!ok_name) {
            cout << "cart: missing project name" << endl;
            exit(-1);
        }
        if (!ok_entry) {
            cout << "cart: missing project entry file name" << endl;
            exit(-1);
        }
        if (!ok_nostd) {
            nostd = true;
        } else {
            nostd = _nostd;
        }

        if (!ok_target) {
            target_file = "a.out";
        }

        auto paths = configure.table->getTable("path");
        toml::Array *include_path = nullptr;
        toml::Array *library_path = nullptr;
        if (paths) {
            include_path = paths->getArray("include_path").release();
            library_path = paths->getArray("library_path").release();
            auto [ok_gcc, gcc_dir] = paths->getString("gcc_toolchain");
            if (ok_gcc) {
                GCCToolchainDir = gcc_dir;
            } else {
                GCCToolchainDir = "/usr";
            }
        }

        vector<string> avsiargs = {
                "avsi",
                entry_file,
                "--package-name",
                name
        };

        pair<bool, string> inc_path;
        if (include_path) {
            for (int i = 0;; i++) {
                inc_path = include_path->getString(i);
                if (!inc_path.first)
                    break;
                avsiargs.push_back(inc_path.second);
            }
        }

        pair<bool, string> flag;
        if (ccflags) {
            for (int i = 0;; i++) {
                flag = ccflags->getString(i);
                if (!flag.first)
                    break;
                avsiargs.push_back(flag.second);
            }
        }

        // avsi build
        auto avsi = fork();
        int status = 0;
        if (avsi == -1) {
            cout << "cart: failed to call avsi" << endl;
            exit(-1);
        } else if (avsi > 0) {
            int stat = wait(&status);
            if (stat == -1) {
                cout << "cart: failed to call avsi" << endl;
                exit(-1);
            }
            if (status != 0) {
                cout << "cart: failed to build project" << endl;
                exit(-1);
            }
        } else {
            cout << __COLOR_GREEN "compile source files" __COLOR_RESET << endl;

            vector<char const *> args;
            for (auto &i: avsiargs) {
                args.push_back(i.c_str());
            }

            if(opt_verbose) {
                for (auto i: args) {
                    cout << i << " ";
                }
                cout << endl << endl;
            }

            args.push_back((char const *) 0);
            execvp("avsi", (char *const *) args.data());
            exit(-1);
        }

        vector<string> ldargs;
        vector<string> ldf;
        vector<string> libpaths;
        vector<string> objs;

        searchObjs(objs);

        if (library_path) {
            for (int i = 0;; i++) {
                auto lib_path = library_path->getString(i);
                if (!lib_path.first)
                    break;
                libpaths.push_back(lib_path.second.c_str());
            }
        }

        if (ldflags) {
            for (int i = 0;; i++) {
                auto flag = ldflags->getString(i);
                if (!flag.first)
                    break;
                ldf.push_back(flag.second.c_str());
            }
        }

        getLinkParam(ldargs, ldf, libpaths, objs, target_file.c_str());

        // avsi build
        auto ld = fork();
        status = 0;
        if (ld == -1) {
            cout << "cart: failed to call ld" << endl;
            exit(-1);
        } else if (ld > 0) {
            int stat = wait(&status);
            if (stat == -1) {
                cout << "cart: failed to call ld" << endl;
                exit(-1);
            }
            if (status != 0) {
                cout << "cart: failed to link project. code: " << status << endl;
                exit(-1);
            }
        } else {
            vector<char const *> args;

            cout << __COLOR_GREEN "link objs" __COLOR_RESET << endl;

            args.push_back("ld");

            for (auto &i: ldargs) {
                args.push_back(i.c_str());
            }
            args.push_back((char const *)0);

            if (opt_verbose) {
                cout << "ld ";
                for (auto i: ldargs) {
                    cout << i << " ";
                }
                cout << endl;
            }

            execvp("ld", (char *const *) args.data());
            exit(-1);
        }
    }

    void exec_clean(int argc, char **argv, int ind) {
        filesystem::path current_dir = filesystem::current_path();
        filesystem::path build_path = current_dir.string() + SYSTEM_PATH_DIVIDER + "build";
        filesystem::remove_all(build_path);

        auto configure = toml::parseFile("./Cart.toml");

        if (!configure.table) {
            cout << "cart: current directory isn't a project" << endl;
            exit(-1);
        }

        auto project = configure.table->getTable("project");
        if (!project) {
            cout << "cart: missing [project] in Cart.toml" << endl;
            exit(-1);
        }

        auto [ok_target, target_file] = project->getString("target");
        if (!ok_target) {
            target_file = "a.out";
        }

        filesystem::path file = target_file;
        if(filesystem::exists(file))
            filesystem::remove(file);
    }
}