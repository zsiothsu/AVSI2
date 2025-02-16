# 0 "/home/chipen/code/AVSI2/example/loop_test/.///for.sl"
# 0 "<組み込み>"
# 0 "<コマンドライン>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<コマンドライン>" 2
# 1 "/home/chipen/code/AVSI2/example/loop_test/.///for.sl"
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
