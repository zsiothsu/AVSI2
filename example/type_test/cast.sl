mod type_test::cast

export function cast_test() {
    a = 1
    b = 1.5
    c = b as i64
    d:f32 = b as f64
    e = b as i128
    f: bool = true

    arr = {:i32:2}
    i8_ptr = arr as vec[i8:0]
    addr = i8_ptr as i64 + 4
    i8_ptr = addr as vec[i32:0]
    void_ptr = arr as vec[void:0]

    x: vec[i32:3] = {1, 2, 3}
    y = x
    y:vec[i32:3] = x
}