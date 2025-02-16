# 0 "/home/chipen/code/AVSI2/example/loop_test/.///while.sl"
# 0 "<組み込み>"
# 0 "<コマンドライン>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<コマンドライン>" 2
# 1 "/home/chipen/code/AVSI2/example/loop_test/.///while.sl"
mod loop_test::while

import std::io

function while_test(x: f64) -> f64 {
    a = x
    i = 0
    x = 1

    while [ i < a ] do
        x = x * 2
        if [ a -lt 128 ] then
            std::io::print("%d ",x)
        else
            break
        fi
        i = i + 1
    done

    std::io::println("")

    return 0
}
