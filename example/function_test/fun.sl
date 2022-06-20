mod com::avsi::function_test::fun

export function foo_p_i64_r_f64(x: i64) -> f64{
    ret = x as f64
    return ret
}

export function foo_p_i8ptr_i32_r_i8(x: vec[i8;0], y: i32) -> i8 {
    return x[y]
}