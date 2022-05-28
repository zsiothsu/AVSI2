# package name: com::avsi
# import_mod is the name of this module
mod com::avsi::import_mod

import com::avsi::export_mod as ef
import com::avsi::mod1
import com::avsi::mod3::object
import com::avsi::mod2::loop

import std::io

export no_mangle function entry() {
    a = com::avsi::export_mod::foo(1)       # absolute path
    a = export_mod::foo(1)                  # relative path
    a = ef::foo(1)                          # rename com::avsi::export_mod to ef

    a = com::avsi::mod1::sub_mod1::foo()    # module with sub-module

    export_mod::no_mangle_function()        # no mangling function
    no_mangle_function()                    # no mangling function, path could be ignored


    str = {:char:100}
    std::io::printStr("Hello World!\n")
    std::io::printStr("input: ")
    std::io::readStr(str, 50)
    std::io::printStr("your input: ")
    std::io::printStr(str)
    std::io::printStr("\n")
}