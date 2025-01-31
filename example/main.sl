// the module name must be the same as the file name
mod main

import std::io as io

import function_test::fun as ft
import if_test::if
import interface_test as it
import loop_test
import object_test::object
import type_test
import root::submod as sub
import grad_test

export no_mangle function main(argc: i32, argv: char**) -> i32 {
    std::io::println("\n=============== test begin ===============")

    /*
     * multi-line comment
     */

    a = function_test::fun::foo_p_i64_r_f64(1 as i64)
    // call with absulote path
    a = root::function_test::fun::foo_p_i64_r_f64(1 as i64)
    // call with alias
    a = ft::foo_p_i64_r_f64(1 as i64)

    b = function_test::fun::foo_p_i8ptr_i32_r_i8({'a', 'b', 46 as i8}, 2)

    c = if_test::if::foo_iftest(0 as i8)

    it::mangle::fun_mangled()
    it::mangle::fun_nomangle()
    fun_nomangle()
    interface_test::private::function_public()
    // interface_test::private::function_private()

    loop_test::for::for_test()
    loop_test::while::while_test(5)

    object_test::object::obj_test()
    size = sizeof(typename object_test::object::S)
    size = sizeof(typename NS)
    s = root::object_test::object::S(1 as i8, 1.2 as f32, {'H', 'E', 'L', 'L', 'O'})
    s.a = s.member_function(1) as i8
    object_test::object::S::member_function(&s, 1)
    t = VS({1,2})

    type_test::cast::cast_test()
    type_test::expr::expr_test()

    submod::submod::test::test()
    sub::submod::test::test()

    type_test::global::global_init()
    io::println("%d", type_test::global::global_variable)
    type_test::global::global_variable = 5678
    io::println("%d", type_test::global::global_variable)

    io::println("generic fun i32")
    io::println("%d", ft::generic_fun(1 as i32))

    io::println("generic fun i64")
    io::println("%d", ft::generic_fun(1 as i64))

    io::println("generic fun default")
    io::println("%lf", ft::generic_fun(1 as i8))

    io::print("f(x) = x^3\nf''(6): ")
    io::println("%.6lf", grad_test::simple_grad::grad_x_x_x(6))

    io::print("f(x) = ((3 / x + 1) + (x * x / 2)) / (x - 1)\nf''(6): ")
    io::println("%.6lf", grad_test::simple_grad::grad_complex(6))

    io::print("f(x, y) = 2x^2y^2\nd/dx^2dy(f(2,2)) ")
    io::println("%.6lf", grad_test::simple_grad::grad_2d(2, 2))

    io::print("f(x) = arctan(x) + x\nf'(0): ")
    io::println("%.6lf", grad_test::simple_grad::grad_func(0))

    io::println("\n================ test end ================")
    io::println("num args: %d", argc)

    for (i = 0; i < argc; i = i + 1) do
        std::io::println("%s", argv[i])
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

    io::println(banner)

    str = {:char:100}
    io::print("\nHello World!\ninput:")
    io::read("%s", str)
    io::println("your input: %s", str)

    return 0
}
