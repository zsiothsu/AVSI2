mod root::pro

import root::mod3

obj a{
    a: real
    b: real
}

export function entry() {
    d = a()
    e = root::mod3::foo(d)
}