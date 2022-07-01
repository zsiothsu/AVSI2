mod loop_test::while

import std::io

export function while_test(x: f64) -> f64{
    a = x
    i = 0
    x = 1

    while [ i < a ] do
        x = x * 2
        if [ a  -lt 128 ] then
            std::io::printReal(x as real)
            std::io::printStr(" ")
        else
            break
        fi
        i = i + 1
    done

    std::io::printStr("\n")

    return 0
}