mod com::avsi::loop_test::while

export function while_test(x: f64) -> f64{
    a = x

    while [ true ] do
        a = a * 2
        if [ a  -lt 1024 ] then
            continue
        else
            a = 0
        fi
    done
}