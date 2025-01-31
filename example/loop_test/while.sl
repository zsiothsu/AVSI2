mod loop_test::while

import std::io

export function while_test(x: f64) -> f64{
    a = x
    i = 0
    x = 1

    while [ i < a ] do
        x = x * 2
        if [ a  -lt 128 ] then
            print("%d ",x)
        else
            break
        fi
        i = i + 1
    done

    std::io::println("")

    return 0
}