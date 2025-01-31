mod loop_test::for

import std::io

function for_test() {
    for (i = 0; i -lt 10; i = i + 1) do
        a = 'a' + i as i8
        if [ a < 'e' ] then
            std::io::print("%c", a)
        else
            continue
        fi
    done
    std::io::println("")
}