mod function_test::fun

export function foo_p_i64_r_f64(x: i64) -> f64{
    ret = x as f64
    return ret
}

export function foo_p_i8ptr_i32_r_i8(x: arr[i8:0], y: i32) -> i8 {
    return x[y]
}

export function fun_i32(x: i32) -> i32 {
    return x + 1
}

export function fun_i64(x: i64) -> i64 {
    return x + 2
}

export function fun_default(x: f32) -> f32 {
    return x + 3
}

generic generic_fun <0> {
    i32 : fun_i32,
    i64 : fun_i64,
    default: fun_default
}