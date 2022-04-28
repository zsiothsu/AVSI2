/*
 * @Author: Chipen Hsiao
 * @Date: 2022-04-27
 * @Description: include file name and path definition
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

extern std::string compiler_exec_path;
extern std::string output_root_path;

extern std::string input_file_name;
extern std::string input_file_name_no_suffix;
extern std::string input_file_path_absolut;
extern std::string input_file_path_relative;

#endif //AVSI2_FILENAME_H
