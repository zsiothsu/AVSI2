/*
 * Gnu.cpp 2022
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
 *
 * NOTICE: Some code references the llvm project
 *
 * //===--- Gnu.cpp - Gnu Tool and ToolChain Implementations -------*- C++ -*-===//
 * //
 * // Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * // See https://llvm.org/LICENSE.txt for license information.
 * // SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 * //
 * //===----------------------------------------------------------------------===//
 */

#include "../inc/Gnu.h"

#include <iostream>
#include <filesystem>
#include <set>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/Host.h>

/* Directory where gcc is installed. */
#define GCC_INSTALL_PREFIX ""

extern bool opt_verbose;

namespace cart {
    using namespace llvm;
    using namespace std;

    vector<string> CandidateGCCInstallPaths;
    string GCCInstallPath;
    string GCCLibraryPath;
    vector<string> LibraryPaths;
    string ELFM;
    bool nostd;
    string GCCToolchainDir;
    GCCVersion Version;

    GCCVersion GCCVersion::Parse(StringRef VersionText) {
        const GCCVersion BadVersion = {VersionText.str(), -1, -1, -1, "", "", ""};
        std::pair<StringRef, StringRef> First = VersionText.split('.');
        std::pair<StringRef, StringRef> Second = First.second.split('.');

        GCCVersion GoodVersion = {VersionText.str(), -1, -1, -1, "", "", ""};
        if (First.first.getAsInteger(10, GoodVersion.Major) || GoodVersion.Major < 0)
            return BadVersion;
        GoodVersion.MajorStr = First.first.str();
        if (First.second.empty())
            return GoodVersion;
        StringRef MinorStr = Second.first;
        if (Second.second.empty()) {
            if (size_t EndNumber = MinorStr.find_first_not_of("0123456789")) {
                GoodVersion.PatchSuffix = std::string(MinorStr.substr(EndNumber));
                MinorStr = MinorStr.slice(0, EndNumber);
            }
        }
        if (MinorStr.getAsInteger(10, GoodVersion.Minor) || GoodVersion.Minor < 0)
            return BadVersion;
        GoodVersion.MinorStr = MinorStr.str();

        // First look for a number prefix and parse that if present. Otherwise just
        // stash the entire patch string in the suffix, and leave the number
        // unspecified. This covers versions strings such as:
        //   5        (handled above)
        //   4.4
        //   4.4-patched
        //   4.4.0
        //   4.4.x
        //   4.4.2-rc4
        //   4.4.x-patched
        // And retains any patch number it finds.
        StringRef PatchText = Second.second;
        if (!PatchText.empty()) {
            if (size_t EndNumber = PatchText.find_first_not_of("0123456789")) {
                // Try to parse the number and any suffix.
                if (PatchText.slice(0, EndNumber).getAsInteger(10, GoodVersion.Patch) ||
                    GoodVersion.Patch < 0)
                    return BadVersion;
                GoodVersion.PatchSuffix = std::string(PatchText.substr(EndNumber));
            }
        }

        return GoodVersion;
    }

    /// Less-than for GCCVersion, implementing a Strict Weak Ordering.
    bool GCCVersion::isOlderThan(int RHSMajor, int RHSMinor,
                                 int RHSPatch,
                                 StringRef RHSPatchSuffix) const {
        if (Major != RHSMajor)
            return Major < RHSMajor;
        if (Minor != RHSMinor)
            return Minor < RHSMinor;
        if (Patch != RHSPatch) {
            // Note that versions without a specified patch sort higher than those with
            // a patch.
            if (RHSPatch == -1)
                return true;
            if (Patch == -1)
                return false;

            // Otherwise just sort on the patch itself.
            return Patch < RHSPatch;
        }
        if (PatchSuffix != RHSPatchSuffix) {
            // Sort empty suffixes higher.
            if (RHSPatchSuffix.empty())
                return true;
            if (PatchSuffix.empty())
                return false;

            // Provide a lexicographic sort to make this a total ordering.
            return PatchSuffix < RHSPatchSuffix;
        }

        // The versions are equal.
        return false;
    }

    void GetPaths(const llvm::Triple &TargetTriple) {
        llvm::Triple BiarchVariantTriple = TargetTriple.isArch32Bit()
                                           ? TargetTriple.get64BitArchVariant()
                                           : TargetTriple.get32BitArchVariant();
        // The library directories which may contain GCC installations.
        vector<string> CandidateLibDirs, CandidateBiarchLibDirs;
        // The compatible GCC triples for this particular architecture.
        vector<string> CandidateTripleAliases;
        vector<string> CandidateBiarchTripleAliases;
        CollectLibDirsAndTriples(TargetTriple, BiarchVariantTriple, CandidateLibDirs,
                                 CandidateTripleAliases, CandidateBiarchLibDirs,
                                 CandidateBiarchTripleAliases);

        filesystem::path install_dir_root = GCCToolchainDir;
        if (!filesystem::exists(install_dir_root)) {
            cout << "cart: unknown gcc_toolchain directory" << endl;
            exit(-1);
        }

        if (opt_verbose)
            cout << endl;

        for (auto libdirs: CandidateLibDirs) {
            filesystem::path gcc_dir =
                    install_dir_root.string() + SYSTEM_PATH_DIVIDER + libdirs + SYSTEM_PATH_DIVIDER + "gcc";

            GCCVersion max_version = GCCVersion::Parse("0.0.0");
            filesystem::path select_path = "";
            if (filesystem::exists(gcc_dir)) {
                for (auto triple_alias: CandidateTripleAliases) {
                    filesystem::path gcc_dir_with_triple = gcc_dir.string() + SYSTEM_PATH_DIVIDER + triple_alias;

                    if (filesystem::exists(gcc_dir_with_triple)) {
                        for (auto &LI: filesystem::directory_iterator(gcc_dir_with_triple)) {
                            filesystem::path gcc_dir_with_version = LI.path();

                            if (opt_verbose)
                                cout << "Found gcc: " << gcc_dir_with_version << endl;

                            auto version_string = basename(gcc_dir_with_version.c_str());

                            auto version = GCCVersion::Parse(version_string);
                            if (version > max_version) {
                                select_path = gcc_dir_with_version;
                            }
                        }
                    }
                }
                CandidateGCCInstallPaths.push_back(select_path.string());
            }
        }

        for (auto gcc_paths: CandidateGCCInstallPaths) {
            if ((gcc_paths.find("lib64") != string::npos) ^ TargetTriple.isArch32Bit()) {
                GCCInstallPath = gcc_paths;

                if (opt_verbose)
                    cout << "Select gcc: " << gcc_paths << endl << endl;

                break;
            }
        }

        auto target_lib = TargetTriple.isArch32Bit() ? "lib" : "lib64";
        GCCLibraryPath =
                GCCInstallPath
                + SYSTEM_PATH_DIVIDER + ".."
                + SYSTEM_PATH_DIVIDER + ".."
                + SYSTEM_PATH_DIVIDER + ".."
                + SYSTEM_PATH_DIVIDER + ".."
                + SYSTEM_PATH_DIVIDER + target_lib;


        LibraryPaths.push_back(GCCInstallPath);
        LibraryPaths.push_back(GCCLibraryPath);
        LibraryPaths.push_back("/usr/lib");
        LibraryPaths.push_back("/lib");

        if (TargetTriple.isArch64Bit()) {
            LibraryPaths.push_back("/usr/lib64");
            LibraryPaths.push_back("/lib64");
        }
    }

    void CollectLibDirsAndTriples(
            const llvm::Triple &TargetTriple, const llvm::Triple &BiarchTriple,
            vector<string> &LibDirs,
            vector<string> &TripleAliases,
            vector<string> &BiarchLibDirs,
            vector<string> &BiarchTripleAliases) {
        /*
         * This project currently only supports the x86 series of architectures
         */
        static const char *const X86_64LibDirs[] = {"/lib64", "/lib"};
        static const char *const X86_64Triples[] = {
                "x86_64-linux-gnu", "x86_64-unknown-linux-gnu",
                "x86_64-pc-linux-gnu", "x86_64-redhat-linux6E",
                "x86_64-redhat-linux", "x86_64-suse-linux",
                "x86_64-manbo-linux-gnu", "x86_64-linux-gnu",
                "x86_64-slackware-linux", "x86_64-unknown-linux",
                "x86_64-amazon-linux"};
        static const char *const X32Triples[] = {"x86_64-linux-gnux32",
                                                 "x86_64-pc-linux-gnux32"};
        static const char *const X32LibDirs[] = {"/libx32", "/lib"};
        static const char *const X86LibDirs[] = {"/lib32", "/lib"};
        static const char *const X86Triples[] = {
                "i586-linux-gnu",
                "i686-linux-gnu",
                "i686-pc-linux-gnu",
                "i386-redhat-linux6E",
                "i686-redhat-linux",
                "i386-redhat-linux",
                "i586-suse-linux",
                "i686-montavista-linux",
                "i686-gnu",
        };

        switch (TargetTriple.getArch()) {
            case llvm::Triple::x86_64:
                if (TargetTriple.isX32()) {
                    LibDirs.insert(end(LibDirs), begin(X32LibDirs), end(X32LibDirs));
                    TripleAliases.insert(end(TripleAliases), begin(X32Triples), end(X32Triples));
                    BiarchLibDirs.insert(end(TripleAliases), begin(X86_64LibDirs), end(X86_64LibDirs));
                    BiarchTripleAliases.insert(end(BiarchTripleAliases), begin(X86_64Triples), end(X86_64Triples));
                } else {
                    LibDirs.insert(end(BiarchTripleAliases), begin(X86_64LibDirs), end(X86_64LibDirs));
                    TripleAliases.insert(end(BiarchTripleAliases), begin(X86_64Triples), end(X86_64Triples));
                    BiarchLibDirs.insert(end(BiarchLibDirs), begin(X32LibDirs), end(X32LibDirs));
                    BiarchTripleAliases.insert(end(BiarchTripleAliases), begin(X32Triples), end(X32Triples));
                }
                BiarchLibDirs.insert(end(BiarchLibDirs), begin(X86LibDirs), end(X86LibDirs));
                BiarchTripleAliases.insert(end(BiarchTripleAliases), begin(X86Triples), end(X86Triples));
                break;
            case llvm::Triple::x86:
                LibDirs.insert(end(LibDirs), begin(X86LibDirs), end(X86LibDirs));
                // MCU toolchain is 32 bit only and its triple alias is TargetTriple
                // itself, which will be inserted below.
                if (!TargetTriple.isOSIAMCU()) {
                    TripleAliases.insert(end(TripleAliases), begin(X86Triples), end(X86Triples));
                    BiarchLibDirs.insert(end(BiarchLibDirs), begin(X86_64LibDirs), end(X86_64LibDirs));
                    BiarchTripleAliases.insert(end(BiarchLibDirs), begin(X86_64Triples), end(X86_64Triples));
                    BiarchLibDirs.insert(end(BiarchLibDirs), begin(X32LibDirs), end(X32LibDirs));
                    BiarchTripleAliases.insert(end(BiarchTripleAliases), begin(X32Triples), end(X32Triples));
                }
                break;
            default:
                // By default, just rely on the standard lib directories and the original
                // triple.
                break;
        }
        // Always append the drivers target triple to the end, in case it doesn't
        // match any of our aliases.
        if(std::find(begin(TripleAliases), end(TripleAliases), TargetTriple.str()) == end(TripleAliases))
            TripleAliases.push_back(TargetTriple.str());
        // Also include the multiarch variant if it's different.
        if (TargetTriple.str() != BiarchTriple.str())
            BiarchTripleAliases.push_back(BiarchTriple.str());
    }

    char const *getLDMOption(const llvm::Triple &T) {
        switch (T.getArch()) {
            case llvm::Triple::x86:
                if (T.isOSIAMCU())
                    return "elf_iamcu";
                return "elf_i386";
            case llvm::Triple::x86_64:
                if (T.isX32())
                    return "elf32_x86_64";
                return "elf_x86_64";
            default:
                return nullptr;
        }
    }

    void getLinkParam(
            vector<string> &args,
            vector<string> &ldflags,
            vector<string> &libpaths,
            vector<string> &objs,
            char const *elf_name) {
        const string TargetTriple = llvm::sys::getDefaultTargetTriple();
        const llvm::Triple Triple = llvm::Triple(TargetTriple);

        GetPaths(Triple);

        // -m option
        if (const char *LDMOption = getLDMOption(Triple)) {
            args.push_back("-m" + string(getLDMOption(Triple)));
        } else {
            cout << "cart: unknown target architecture" << endl;
            exit(-1);
        }

        // -o option
        args.push_back("-o");
        args.push_back(elf_name);

        if (!nostd) {
            auto target_lib = Triple.isArch32Bit() ? "lib" : "lib64";
            args.push_back("-dynamic-linker");
            args.push_back(string("/") + target_lib + string("/") + "ld-linux-x86-64.so.2");
        }

        if (!nostd) {
            auto crt1 = GCCLibraryPath + SYSTEM_PATH_DIVIDER + "crt1.o";
            auto crti = GCCLibraryPath + SYSTEM_PATH_DIVIDER + "crti.o";
            auto crtbegin = GCCInstallPath + SYSTEM_PATH_DIVIDER + "crtbegin.o";

            args.push_back(crt1.c_str());
            args.push_back(crti.c_str());
            args.push_back(crtbegin.c_str());
        }

        for (auto lib_path: LibraryPaths) {
            args.push_back("-L");
            args.push_back(lib_path.c_str());
        }

        for (auto libp: libpaths) {
            args.push_back(libp);
        }

        for (auto obj: objs) {
            args.push_back(obj);
        }

        for (auto ldf: ldflags) {
            args.push_back(ldf);
        }

        if (!nostd) {
            args.push_back("-lavsi");
            args.push_back("-lgcc");
            args.push_back("--as-needed");
            args.push_back("-lgcc_s");
            args.push_back("--no-as-needed");
            args.push_back("-lc");
            args.push_back("-lgcc");
            args.push_back("--as-needed");
            args.push_back("-lgcc_s");
            args.push_back("--no-as-needed");
        }

        if (!nostd) {
            auto crtend = GCCInstallPath + SYSTEM_PATH_DIVIDER + "crtend.o";
            auto crtn = GCCLibraryPath + SYSTEM_PATH_DIVIDER + "crtn.o";

            args.push_back(crtend.c_str());
            args.push_back(crtn.c_str());
        }
    }
}