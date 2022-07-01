# the module name must be the same as the file name
mod main

import std::io as io

import function_test::fun as ft
import if_test::if
import interface_test as it
import loop_test
import object_test::object
import type_test
import root::submod as sub

export no_mangle function main(argc: i32, argv: vec[vec[char;0];0]) -> i32 {
    std::io::printStr("\n=============== test begin ===============\n")

    # call with relative path
    a = function_test::fun::foo_p_i64_r_f64(1 as i64)
    # call with absulote path
    a = root::function_test::fun::foo_p_i64_r_f64(1 as i64)
    # call with alias
    a = ft::foo_p_i64_r_f64(1 as i64)

    b = function_test::fun::foo_p_i8ptr_i32_r_i8({'a', 'b', 46 as i8}, 2)

    c = if_test::if::foo_iftest(0 as i8)

    it::mangle::fun_mangled()
    it::mangle::fun_nomangle()
    fun_nomangle()
    interface_test::private::function_public()
    # interface_test::private::function_private()

    loop_test::for::for_test()
    loop_test::while::while_test(5)

    object_test::object::obj_test()
    size = sizeof(typename object_test::object::S)
    size = sizeof(typename NS)
    s = root::object_test::object::S(1 as i8, 1.2 as f32, {'H', 'E', 'L', 'L', 'O'})
    s = s.a
    t = VS({1,2})

    type_test::cast::cast_test()
    type_test::expr::expr_test()

    submod::submod::test::test()
    sub::submod::test::test()

    type_test::global::global_init()
    io::printReal(type_test::global::global_variable)
    io::printStr("\n")
    type_test::global::global_variable = 5678
    io::printReal(type_test::global::global_variable)
    std::io::printStr("\n================ test end ================\n")
    std::io::printStr("num args: ")
    std::io::printReal(argc as real)
    std::io::printStr("\n")

    for (i = 0; i < argc; i = i + 1) do
        std::io::printStr(argv[i])
        std::io::printStr("\n")
    done


    banner = 
    "\n\033[1;5;34m"
    "      __      _______ _____ \n"
    "     /\\ \\    / / ____|_   _|\n"
    "    /  \\ \\  / / (___   | |  \n"
    "   / /\\ \\ \\/ / \\___ \\  | |  \n"
    "  / ____ \\  /  ____) |_| |_ \n"
    " /_/    \\_\\/  |_____/|_____|\n"
    "\033[0m"

    io::printStr(banner)

    str = {:char:100}
    io::printStr("\nHello World!\n")
    io::printStr("input: ")
    io::readStr(str)
    io::printStr("your input: ")
    io::printStr(str)
    io::printStr("\n")

    return 0
}
