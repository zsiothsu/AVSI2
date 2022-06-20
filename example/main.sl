# package name is "com::avsi"
# "main" is the name of this module
# the module name must be the same as the file name
mod com::avsi::main

import std::io as io

import com::avsi::function_test::fun
import com::avsi::if_test::if
import com::avsi::interface_test
import com::avsi::loop_test
import com::avsi::object_test::object
import com::avsi::type_test

export no_mangle function entry() -> i32 {
    a = com::avsi::function_test::fun::foo_p_i64_r_f64(1 as i64)
    b = com::avsi::function_test::fun::foo_p_i8ptr_i32_r_i8({'a', 'b', 46 as i8}, 2)

    c = com::avsi::if_test::if::foo_iftest(0)

    com::avsi::interface_test::mangle::fun_mangled()
    com::avsi::interface_test::mangle::fun_nomangle()
    fun_nomangle()
    com::avsi::interface_test::private::function_public()
    # interface_test::private::function_private()

    # com::avsi::loop_test::for::for_test()
    # com::avsi::loop_test::while::while_test(1.23 as f64)

    com::avsi::object_test::object::obj_test()

    com::avsi::type_test::cast::cast_test()
    com::avsi::type_test::expr::expr_test()

    str = {:char:100}
    io::printStr("Hello World!\n")
    io::printStr("input: ")
    io::readStr(str)
    io::printStr("your input: ")
    io::printStr(str)
    io::printStr("\n")

    return 0
}