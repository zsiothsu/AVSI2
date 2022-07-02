/*
 * Gnu.h 2022
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




#ifndef CART_GNU_H
#define CART_GNU_H

#include <vector>
#include <string>

#include "llvm/Option/ArgList.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetParser.h"
#include "llvm/Support/VirtualFileSystem.h"

#ifndef SYSTEM_PATH_DIVIDER
#ifdef __linux__
#define SYSTEM_PATH_DIVIDER "/"
#elif _WIN32
#define SYSTEM_PATH_DIVIDER "\\"
#else
#err "unsupported operating system"
#endif
#endif

namespace cart {
    using namespace llvm;
    using namespace std;


    extern vector<string> CandidateGCCInstallPaths;
    extern string GCCInstallPath;
    extern vector<string> LibraryPaths;
    extern string ELFM;
    extern bool nostd;
    extern string GCCToolchainDir;

    struct GCCVersion {
        /// The unparsed text of the version.
        std::string Text;

        /// The parsed major, minor, and patch numbers.
        int Major, Minor, Patch;

        /// The text of the parsed major, and major+minor versions.
        std::string MajorStr, MinorStr;

        /// Any textual suffix on the patch number.
        std::string PatchSuffix;

        static GCCVersion Parse(StringRef VersionText);

        bool isOlderThan(int RHSMajor, int RHSMinor, int RHSPatch,
                         StringRef RHSPatchSuffix = StringRef()) const;

        bool operator<(const GCCVersion &RHS) const {
            return isOlderThan(RHS.Major, RHS.Minor, RHS.Patch, RHS.PatchSuffix);
        }

        bool operator>(const GCCVersion &RHS) const { return RHS < *this; }

        bool operator<=(const GCCVersion &RHS) const { return !(*this > RHS); }

        bool operator>=(const GCCVersion &RHS) const { return !(*this < RHS); }
    };


    void CollectLibDirsAndTriples(
            const llvm::Triple &TargetTriple, const llvm::Triple &BiarchTriple,
            vector<string> &LibDirs,
            vector<string> &TripleAliases,
            vector<string> &BiarchLibDirs,
            vector<string> &BiarchTripleAliases);

    char const *getLDMOption(const llvm::Triple &T);

    void getLinkParam(
            vector<string> &args,
            vector<string> &ldflags,
            vector<string> &libpaths,
            vector<string> &objs,
            char const *elf_name = "./a.out");
}

#endif //CART_GNU_H
