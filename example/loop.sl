mod com::avsi::loop

export function while_test(x: real) -> real{
    a = x

    while [ true ] do
        a = a * 2
        if [ a  -lt 1024 ] then
            continue
        else
            break
        fi
    done
}

export function for_test() {
    for (i = 0; i -lt 10; i = i + 1) do
        a = i
        if [ a -eq 5 ] then
            break
        else
            continue
        fi
    done
}

export function entry() {
    r = while_test(10)
    for_test()
}