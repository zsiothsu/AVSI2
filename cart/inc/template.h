/*******************************************************
 *                    Cart.toml                        *
 *******************************************************/
/**
 * template_project
 * param    %s: package name
 */
static const char *template_project =
        R"([project])" "\n"
        R"(name = "%s")" "\n"
        R"(version = "0.0.1")" "\n";

/**
 * template_path
 * param    none
 */
static const char *template_path =
        R"([path])" "\n"
        R"(#include_path = ["/usr/include/avsi"])" "\n"
        R"(#library_path = ["/usr/lib"])" "\n"
        R"(#gcc_toolchain = "")" "\n";

/**
 * template_lib
 * param    %s: building types list
 */
static const char *template_lib =
        R"([lib])" "\n"
        R"(############################################################)" "\n"
        R"(## building type. typelist:)" "\n"
        R"(## bin          Executable file)" "\n"
        R"(## staticlib    Static library. Create a lib*.a file)" "\n"
        R"(## dylib        Dynamic library. Create a lib*.so file)" "\n"
        R"(############################################################)" "\n"
        R"(type = [%s])" "\n";

/**
 * template_lib_bin
 * param    %s: target name
 */
static const char *template_lib_bin =
        R"([[lib.bin]])" "\n"
        R"(name = "%s")" "\n"
        R"(entry = "main.sl")" "\n"
        R"(ccflags = ["-O", "-W"])" "\n"
        R"(ldflags = ["-lm"])" "\n"
        R"(nostd = false)" "\n";

/**
 * template_lib_bin_removed
 * param    %s: target name
 */
static const char *template_lib_bin_removed =
        R"(#[[lib.bin]])" "\n"
        R"(#name = "%s")" "\n"
        R"(#entry = "main.sl")" "\n"
        R"(#ccflags = ["-O", "-W"])" "\n"
        R"(#ldflags = ["-lm"])" "\n"
        R"(#nostd = false)" "\n";

/**
 * template_lib
 * param    %s: library name
 */
static const char *template_lib_lib =
        R"([lib.lib])" "\n"
        R"(############################################################)" "\n"
        R"(## name will be used as library name. example name = "hello")" "\n"
        R"(## in lib.sl :)" "\n"
        R"(## mod hello)" "\n"
        R"(## and target will be libhello.a or libhello.so)" "\n"
        R"(############################################################)" "\n"
        R"(name = "%s")" "\n"
        R"(ccflags = ["-O", "-W"])" "\n"
        R"(nostd = false)" "\n";

/**
 * template_lib_lib_removed
 * param    %s: library name
 */
static const char *template_lib_lib_removed =
        R"(#[lib.lib])" "\n"
        R"(#############################################################)" "\n"
        R"(### name will be used as library name. example name = "hello")" "\n"
        R"(### in lib.sl :)" "\n"
        R"(### mod hello)" "\n"
        R"(### and target will be libhello.a or libhello.so)" "\n"
        R"(#############################################################)" "\n"
        R"(#name = "%s")" "\n"
        R"(#ccflags = ["-O", "-W"])" "\n"
        R"(#nostd = false)" "\n";

/*******************************************************
 *                     main.sl                         *
 *******************************************************/
/**
 * template_source_main
 * param    none
 */

static const char *template_source_main =
        R"(mod main)" "\n\n"
        R"(import std::io)" "\n\n"
        R"(public function main(argc: i32, argv: char**) -> i32 {)" "\n"
        R"(    std::io::println("Hello World!\n"))" "\n"
        R"(    return 0)" "\n"
        R"(})" "\n";

/*******************************************************
 *                     lib.sl                          *
 *******************************************************/
/**
 * template_source_lib
 * param    library name
 */
static const char *template_source_lib =
        R"(mod %s)" "\n\n"
        R"(public function it_works() -> i32{)" "\n"
        R"(    return 0)" "\n"
        R"(})" "\n";
