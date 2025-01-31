mod if_test::if

import std::io

function foo_iftest(a: i32) -> bool {
    b = 1
    c = true
    ret = false
    if [ c && b == 1] then
        if [ a == 1 ] then
            ret = true
            std::io::println("true")
        else
            ret = false
            std::io::println("false")
        fi
    fi

    select = if [c == true] then 1 else 2 fi

    return ret
}