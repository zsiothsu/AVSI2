mod std::io

no_mangle function print(fmt: char*, ...) -> i32

no_mangle function println(fmt: char*, ...) -> i32

no_mangle function read(fmt: char*, ...) -> i32

no_mangle generic printf {
    default: print
}

no_mangle generic scanf {
    default: read
}

