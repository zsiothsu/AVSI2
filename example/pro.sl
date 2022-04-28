mod root::pro

import root::mod1::mod1
import root::mod2::mod2
import root::mod3

obj struct{
    a:real
    b: vec[real;2]
    c: vec[real;0]
}

export function entry() {
    c = root::mod1::mod1::foo()
}