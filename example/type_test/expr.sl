mod type_test::expr

function expr_test() {
    x = 1
    y = 2
    z = +x
    z = ~x
    z = -x
    z = !x
    z = &x
    z = *z
    z = x * y
    z = x / y
    z = x % y
    z = x + y
    z = x - y
    z = x << y
    z = x >> y
    z = x >>> y
    z = x == y
    z = x != y
    z = x & y
    z = x | y
    z = x && y
    z = x || y

    a = 1
    b = 2
    c = a + b - 312
    d = c as f32
    e = c / 313.4234 + 2.5e6 * 0.2
}

function select_test() -> i32{
    b = 1
    ret = 0

    b = if true && false
    then
        if true && false then
            1 + 1
        else
            2 * 2
        fi
    else
        3 - 3
    fi

    ret + 20
}

function block_expr_test() -> i32{
    b = 22.4
    b as i32
}

function arr_test_1(test_arr: arr[i32:0]) {
    b = &test_arr
    c = b[0]
}

function arr_test_2() {
    a = {1 as i32, 2,3,4}
    b = &a
    c = b[0]
}