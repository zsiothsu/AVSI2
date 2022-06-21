# package name is "com::avsi"
# "main" is the name of this module
# the module name must be the same as the file name
mod com::avsi::main

import std::io as io

import com::avsi::function_test::fun
import com::avsi::if_test::if
import com::avsi::interface_test as it
import com::avsi::loop_test
import com::avsi::object_test::object
import com::avsi::type_test
import submod as sub

export no_mangle function entry() -> i32 {
    a = function_test::fun::foo_p_i64_r_f64(1 as i64)
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

    type_test::cast::cast_test()
    type_test::expr::expr_test()

    submod::submod::test::test()
    sub::submod::test::test()

    str = {:char:100}
    io::printStr("Hello World!\n")
    io::printStr("input: ")
    io::readStr(str)
    io::printStr("your input: ")
    io::printStr(str)
    io::printStr("\n")

    return 0
}