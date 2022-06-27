mod com::avsi::object_test::object

import com::avsi::object_test::subobject

obj S {
    a: i8
    b: f32
    c: vec[i8;100]
    d: subobject::subS
}

no_mangle obj NS {
    a: i8
    b: f32
    c: vec[i8;100]
}

no_mangle obj VS {
    c: vec[i32;100]
}

export function obj_test() {
    s = S(1 as i8, 1.2 as f32, {'H', 'E', 'L', 'L', 'O'})

    s.a = 2 as i8
    s.b = s.a as f32
    s.c[0] = 'h'

    size = sizeof(s)
    c_len = sizeof(s.c) / sizeof(typename i8)
}