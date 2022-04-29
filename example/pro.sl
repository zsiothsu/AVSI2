mod root::pro

import root::mod3

obj a{
    a: real
    b: real
}

obj vecStruct {
    a: vec[real; 2]
    b: vec[vec[real;2];2]
}

function test1() -> real {
    a = {123, 456}
    b = vecStruct(a)
    b.b[0] = a
    b.b[1] = a
    c = {7890, 12}
    d = {c,c}

    e = sizeof(b.b)
    f = sizeof(d)
}

export function entry() {
    d = a()
    e = root::mod3::foo(d)
}