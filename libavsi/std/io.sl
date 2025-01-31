mod std::io

export no_mangle function print(fmt: char*, ...) -> i32

export no_mangle function println(fmt: char*, ...) -> i32

export no_mangle function read(fmt: char*, ...) -> i32