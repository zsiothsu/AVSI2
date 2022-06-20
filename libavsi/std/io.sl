mod std::io

export no_mangle function printStr(s: vec[char;0])

export no_mangle function printReal(num: real)

export no_mangle function readNum() -> real

export no_mangle function readStr(str: vec[char;0]) -> i32