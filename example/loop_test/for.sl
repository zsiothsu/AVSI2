mod com::avsi::loop_test::for

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