mod com::avsi::if_test::if

import std::io

export function foo_iftest(a: i32) -> bool {
    b = 1
    c = true
    ret = false
    if [ c && b == 1] then
        if [ a == 1 ] then
            ret = true
            std::io::printStr("true\n")
        else
            ret = false
            std::io::printStr("false\n")
        fi
    fi

    return ret
}