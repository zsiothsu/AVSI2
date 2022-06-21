mod com::avsi::loop_test::for

import std::io

export function for_test() {
    for (i = 0; i -lt 10; i = i + 1) do
        a = 'a' + i as i8
        if [ a < 'e' ] then
            std::io::printStr({a})
        else
            continue
        fi
    done
    std::io::printStr("\n")
}