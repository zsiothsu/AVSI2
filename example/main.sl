# package name is "com::avsi"
# "main" is the name of this module
# the module name must be the same as the file name
mod com::avsi::main

import com::avsi::export_mod as ef
import com::avsi::mod1
import com::avsi::mod3::object
import com::avsi::mod2::loop

import std::io as io

export function cast_test() {
    a = 1
    b = 1.5
    c = b as i64
    d:f32 = b as f64
    e = b as i128
    f: bool = true

    arr = {:i32:2}
    i8_ptr = arr as vec[i8;0]
    addr = i8_ptr as i64 + 4
    i8_ptr = addr as vec[i32;0]
    void_ptr = arr as vec[void;0]
}

export no_mangle function entry() {
    a = com::avsi::export_mod::foo(1 as f64)       # absolute path
    a = export_mod::foo(1 as f64)                  # relative path
    a = ef::foo(1 as f64)                          # rename com::avsi::export_mod to ef

    a = com::avsi::mod1::sub_mod1::foo()    # module with sub-module

    export_mod::no_mangle_function()        # no mangling function
    no_mangle_function()                    # no mangling function, path could be ignored


    str = {:char:100}
    io::printStr("Hello World!\n")
    io::printStr("input: ")
    io::readStr(str, 50 as f64)
    io::printStr("your input: ")
    io::printStr(str)
    io::printStr("\n")
}