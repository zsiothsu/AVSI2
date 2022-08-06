mod object_test::object

import subobject

obj S {
    a: i8
    b: f32
    c: vec[i8;100]
    d: subobject::subS
}

export function S::member_function(a: i32) -> i32 {
  this.a = a as i8
  return a
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

    c_len = sizeof(s.c) / sizeof(typename i8)
}

