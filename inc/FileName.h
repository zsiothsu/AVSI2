/*
 * FileName.h 2022
 *
 * include file name and path definition
 *
 * LLVM IR code generator
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

#ifndef AVSI2_FILENAME_H
#define AVSI2_FILENAME_H

#include <string>

#ifndef SYSTEM_PATH_DIVIDER
#ifdef __linux__
#define SYSTEM_PATH_DIVIDER "/"
#elif _WIN32
#define SYSTEM_PATH_DIVIDER "\\"
#else
#err "unsupported operating system"
#endif
#endif

#define MODULE_INIT_NAME    "__init__"

extern std::string compiler_command_line;

extern std::string compiler_exec_path;
extern std::string output_root_path;

extern std::string input_file_name;
extern std::string input_file_name_no_suffix;
extern std::string input_file_path_absolut;
extern std::string input_file_path_relative;

#endif //AVSI2_FILENAME_H
