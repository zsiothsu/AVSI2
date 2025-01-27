mod object_test::object

import subobject

obj S {
    a: i8
    b: f32
    c: vec[i8:100]
}

no_mangle obj NS {
    a: i8
    b: f32
    c: vec[i8:100]
}

no_mangle obj VS {
    c: vec[i32:100]
}

export function S::member_function(num: i32) -> i32 {
    this.a = num as i8
    return num
}

export function other_function(a: S, b: i32) -> i32{
    a.a = b as i8
    b
}

export function obj_test() {
    a = S(1 as i8, 1.2 as f32, {'H', 'E', 'L', 'L', 'O'})
    a.a = 2 as i8
    a.member_function(0)
    other_function(a, 10)
}

