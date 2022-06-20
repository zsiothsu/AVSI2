mod com::avsi::if_test::if

export function foo_iftest(a: i32) -> bool {
    b = 1
    c = true
    ret = false
    if [ c && b == 1] then
        if [ a == 1 ] then
            ret = true
        else
            ret = false
        fi
    fi

    return ret
}