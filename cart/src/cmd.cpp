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
#include <vector>
#include <set>
#include <string>
#include <filesystem>
#include <fstream>
#include <dirent.h>
#include <cstdlib>
#include <unistd.h>
#include <wait.h>
#include <queue>
#include "../inc/template.h"

#define __COLOR_RESET "\033[0m"
#define __COLOR_RED "\033[31m"
#define __COLOR_GREEN "\033[32m"
#define __COLOR_YELLOW "\033[33m"

extern bool opt_help;
extern bool opt_verbose;
extern bool opt_lib;
extern bool opt_bin;
extern bool opt_bins;

const static char *help_new =
        R"(cart new)"
        "\n"
        R"(Create a new Project)"
        "\n\n"
        R"(USAGE:)"
        "\n"
        R"(cart new [OPTIONS] <name>)"
        "\n\n"
        R"(SHORT OPTIONS:)"
        "\n"
        R"(    -h      --help           Print this help message)"
        "\n"
        R"(    -v      --verbose        Use verbose output)"
        "\n\n"
        R"(LONG OPTIONS:)"
        "\n"
        R"(            --lib            Use a library template.)"
        "\n"
        R"(            --bin            Use a binary template.)"
        "\n\n"
        R"(ARGS:)"
        "\n"
        R"(    <name>                   Project name)"
        "\n";

const static char *help_build =
        R"(cart build)"
        "\n"
        R"(Build the current project)"
        "\n\n"
        R"(USAGE:)"
        "\n"
        R"(cart build [OPTIONS])"
        "\n\n"
        R"(SHORT OPTIONS:)"
        "\n"
        R"(    -h      --help           Print this help message)"
        "\n"
        R"(    -v      --verbose        Use verbose output)"
        "\n\n"
        R"(LONG OPTIONS:)"
        "\n"
        R"(            --lib            Build only this package's library)"
        "\n"
        R"(            --bin <name>...  Build only the specified binary)"
        "\n"
        R"(            --bins           Build all binaries)"
        "\n";

const static char *help_clean =
        R"(cart clean)"
        "\n"
        R"(Clean all built objects)"
        "\n";

/*******************************************************
 *                      hasher                         *
 *******************************************************/
typedef std::uint64_t hash_t;
constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;

constexpr hash_t hasher(char const *str, hash_t last_value = basis) {
    return *str ? hasher(str + 1, (*str ^ last_value) * prime) : last_value;
}

namespace cart {
    using namespace std;

    string PROJECT_NAME;
    vector<string> INCLUDE_PATH;
    vector<string> LIBRARY_PATH;

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

    void build_objs(
            string entry_file,
            string name,
            toml::Array *ccflags,
            toml::Array *ldflags,
            bool nostd) {
        vector<string> avsiargs = {
                "avsi",
                entry_file,
                "--package-name",
                name};

        for (auto i: INCLUDE_PATH) {
            avsiargs.push_back("-I");
            avsiargs.push_back(i);
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

            if (opt_verbose) {
                for (auto i: args) {
                    cout << i << " ";
                }
                cout << endl
                     << endl;
            }

            args.push_back((char const *) 0);
            execvp("avsi", (char *const *) args.data());
            exit(-1);
        }
    }

    void exec_new(int argc, char **argv, int ind) {
        filesystem::path current_dir = filesystem::current_path();

        if (opt_help) {
            cout << help_new << endl;
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

        if (package_name.size() > 250) {
            cout << "cart: package name is too long (max 250)" << endl;
            exit(-1);
        }

        // create configure file
        filesystem::path cfg = project_dir.string() + SYSTEM_PATH_DIVIDER + "Cart.toml";
        filesystem::path main_file = project_dir.string() + SYSTEM_PATH_DIVIDER + "main.sl";
        filesystem::path lib_file = project_dir.string() + SYSTEM_PATH_DIVIDER + "lib.sl";

        std::ofstream cfg_stream(cfg, ios::out);
        if (cfg_stream.is_open()) {
            char out[1765];

            // [project]
            sprintf(out, template_project, package_name.c_str());
            cfg_stream << out << endl;

            // [path]
            cfg_stream << template_path << endl;

            // [lib]
            vector<string> building_types_list;
            string building_types;
            if ((!opt_bin && !opt_lib) || opt_bin) {
                building_types_list.push_back(R"("bin")");
            }
            if (opt_lib) {
                building_types_list.push_back(R"("staticlib")");
                building_types_list.push_back(R"("dylib")");
            }
            building_types.append(building_types_list[0]);
            for (auto i = building_types_list.begin() + 1; i != building_types_list.end(); i++) {
                building_types.append(",");
                building_types.append(*i);
            }
            sprintf(out, template_lib, building_types.c_str());
            cfg_stream << out << endl;

            // [[lib.bin]]
            if ((!opt_bin && !opt_lib) || opt_bin) {
                sprintf(out, template_lib_bin, package_name.c_str());
            } else {
                sprintf(out, template_lib_bin_removed, package_name.c_str());
            }
            cfg_stream << out << endl;

            // [lib.lib]
            if (opt_lib) {
                sprintf(out, template_lib_lib, package_name.c_str());
            } else {
                sprintf(out, template_lib_lib_removed, package_name.c_str());
            }
            cfg_stream << out << endl;

            cfg_stream.close();
        } else {
            cout << "cart: cannot create files" << endl;
            exit(-1);
        }

        if ((!opt_bin && !opt_lib) || opt_bin) {
            std::ofstream main_stream(main_file, ios::out);
            if (main_stream.is_open()) {
                main_stream << template_source_main << endl;
            } else {
                cout << "cart: cannot create files" << endl;
                exit(-1);
            }
        }

        if (opt_lib) {
            std::ofstream lib_stream(lib_file, ios::out);
            if (lib_stream.is_open()) {
                char out[500];
                sprintf(out, template_source_lib, package_name.c_str());
                lib_stream << out << endl;
            } else {
                cout << "cart: cannot create files" << endl;
                exit(-1);
            }
        }
    }

    void exec_build(int argc, char **argv, int ind) {
        if (opt_help) {
            cout << help_build << endl;
            exit(0);
        }

        auto table_configure = toml::parseFile("./Cart.toml");

        if (!table_configure.table) {
            cout << "cart: current directory isn't a project" << endl;
            exit(-1);
        }

        auto table_project = table_configure.table->getTable("project");
        if (!table_project) {
            cout << "cart: missing [project] in Cart.toml" << endl;
            exit(-1);
        }

        auto [ok_project_name, name] = table_project->getString("name");
        if (!ok_project_name) {
            cout << "cart: missing project name" << endl;
            exit(-1);
        }
        PROJECT_NAME = name;

        auto paths = table_configure.table->getTable("path");
        unique_ptr<toml::Array> include_path = nullptr;
        unique_ptr<toml::Array> library_path = nullptr;
        if (paths) {
            include_path = paths->getArray("include_path");
            library_path = paths->getArray("library_path");
            auto [ok_gcc, gcc_dir] = paths->getString("gcc_toolchain");
            if (ok_gcc) {
                GCCToolchainDir = gcc_dir;
            } else {
                GCCToolchainDir = "/usr";
            }
        }
        if (include_path) {
            for (int i = 0;; i++) {
                auto inc_path = include_path->getString(i);
                if (!inc_path.first)
                    break;
                INCLUDE_PATH.push_back(inc_path.second);
            }
        }
        if (library_path) {
            for (int i = 0;; i++) {
                auto lib_path = library_path->getString(i);
                if (!lib_path.first)
                    break;
                LIBRARY_PATH.push_back(lib_path.second);
            }
        }

        auto table_lib = table_configure.table->getTable("lib");
        if (!table_lib) {
            cout << "cart: missing [lib] in Cart.toml" << endl;
            exit(-1);
        }

        auto building_types = table_lib->getArray("type");
        //                bin    slib   dylib
        bool types[3] = {false, false, false};
        if (building_types) {
            for (int i = 0;; i++) {
                auto type = building_types->getString(i);
                if (!type.first)
                    break;
                if (type.second == "bin")
                    types[0] = true;
                else if (type.second == "staticlib")
                    types[1] = true;
                else if (type.second == "dylib")
                    types[2] = true;
                else {
                    cout << "cart: unknowed building type: " << type.second << endl;
                    exit(-1);
                }
            }
        } else {
            cout << "cart: missing \"type\" at [lib] in Cart.toml" << endl;
            exit(-1);
        }

        if (!opt_bin && !opt_lib) {
            if (types[0]) {
                if (ind >= argc)
                    build_bin(table_configure.table);
                else
                    build_bin(table_configure.table, argc, argv, ind);
            }
        } else {
            if (opt_lib) {
                if (types[1])
                    build_lib(table_configure.table, false);
                if (types[2])
                    build_lib(table_configure.table, true);
            }
            if (opt_bin) {
                if (types[0]) {
                    if (ind < argc)
                        build_bin(table_configure.table, argc, argv, ind);
                    else {
                        cout << "cart: target name must be offered" << endl;
                        exit(-1);
                    }
                }
            } else if(opt_bins) {
                if (types[0])
                    build_bin(table_configure.table);
            }
        }
    }

    void exec_clean(int argc, char **argv, int ind) {
        if (opt_help) {
            cout << help_clean << endl;
            exit(0);
        }

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
        if (filesystem::exists(file))
            filesystem::remove(file);
    }

    void build_bin(
            shared_ptr<toml::Table> table,
            int argc,
            char **argv,
            int ind) {
        auto table_lib = table->getTable("lib");
        set<string> targets;

        if (argc != -1) {
            while (ind < argc) {
                string target = argv[ind++];
                targets.insert(target);
            }
        }

        auto link = [&](
                string name,
                toml::Array *ldflags,
                bool nostd) -> void {
            vector<string> ldargs;
            vector<string> ldf;
            vector<string> libpaths;
            vector<string> objs;

            searchObjs(objs);

            for (auto i: LIBRARY_PATH) {
                libpaths.push_back(i);
            }

            if (ldflags) {
                for (int i = 0;; i++) {
                    auto flag = ldflags->getString(i);
                    if (!flag.first)
                        break;
                    ldf.push_back(flag.second.c_str());
                }
            }

            getLinkParam(ldargs, ldf, libpaths, objs, nostd, name.c_str());

            // avsi build
            auto ld = fork();
            int status = 0;
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
                args.push_back((char const *) 0);

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
        };

        auto tables_bin = table_lib->getArray("bin")->getTableVector().release();
        if (!tables_bin) {
            cout << "cart: missing [[lib.bin]]" << endl;
            exit(-1);
        }

        for (auto table_bin: *tables_bin) {
            auto name = table_bin.getString("name");
            if (!name.first) {
                cout << "cart: missing name at [[lib.bin]]" << endl;
                exit(-1);
            }
            auto entry = table_bin.getString("entry");
            if (!entry.first) {
                cout << "cart: missing name at [[lib.bin]]" << endl;
                exit(-1);
            }
            auto nostd = table_bin.getBool("nostd");
            if (!nostd.first) {
                nostd.second = false;
            }

            if (!entry.first) {
                cout << "cart: missing name at [[lib.bin]]" << endl;
                exit(-1);
            }
            auto ccflags = table_bin.getArray("ccflags").release();
            auto ldflags = table_bin.getArray("ldflags").release();

            if (argc != -1) {
                if (targets.find(name.second) != targets.end()) {
                    build_objs(entry.second, name.second, ccflags, ldflags, nostd.second);
                    link(name.second, ldflags, nostd.second);
                }
            } else {
                build_objs(entry.second, name.second, ccflags, ldflags, nostd.second);
                link(name.second, ldflags, nostd.second);
            }
        }
    }

    void build_lib(shared_ptr<toml::Table> table, bool dynamic) {
        auto table_lib = table->getTable("lib");
        auto table_lib_lib = table_lib->getTable("lib");

        auto name = table_lib_lib->getString("name");
        if (!name.first) {
            cout << "cart: missing name at [[lib.bin]]" << endl;
            exit(-1);
        }
        auto nostd = table_lib_lib->getBool("nostd");
        if (!nostd.first) {
            nostd.second = false;
        }
        string entry = "lib.sl";

        auto ccflags = table_lib_lib->getArray("ccflags").release();
        auto ldflags = table_lib_lib->getArray("ldflags").release();

        build_objs(entry, name.second, ccflags, ldflags, nostd.second);

        vector<string> arargs;
        vector<string> arf;
        vector<string> objs;

        searchObjs(objs);

        arargs.push_back("-r");
        arargs.push_back("build" SYSTEM_PATH_DIVIDER "lib" + name.second + ".a");

        for (auto i: objs) {
            arargs.push_back(i);
        }

        auto ar = fork();
        int status = 0;
        if (ar == -1) {
            cout << "cart: failed to call ar" << endl;
            exit(-1);
        } else if (ar > 0) {
            int stat = wait(&status);
            if (stat == -1) {
                cout << "cart: failed to call ar" << endl;
                exit(-1);
            }
            if (status != 0) {
                cout << "cart: failed to generate static library. code: " << status << endl;
                exit(-1);
            }
        } else {
            vector<char const *> args;

            cout << __COLOR_GREEN "generate static library" __COLOR_RESET << endl;

            args.push_back("ar");

            for (auto &i: arargs) {
                args.push_back(i.c_str());
            }
            args.push_back((char const *) 0);

            if (opt_verbose) {
                cout << "ar ";
                for (auto i: arargs) {
                    cout << i << " ";
                }
                cout << endl;
            }

            execvp("ar", (char *const *) args.data());
            exit(-1);
        }
    }
}