mod type_test::cast

export function cast_test() {
    a = 1
    b = 1.5
    c = b as i64
    d:f32 = b as f64
    e = b as i128
    f: bool = true

    test_arr = {:i32:2}
    i8_ptr = test_arr as arr[i8:0]
    addr = i8_ptr as i64 + 4
    i8_ptr = addr as arr[i32:0]
    void_ptr = test_arr as arr[void:0]

    x: arr[i32:3] = {1, 2, 3}
    y = x
    y:arr[i32:3] = x
}