mod type_test::expr

export function expr_test() {
    x = 1
    y = 2
    z = +x
    z = ~x
    z = -x
    z = !x
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

export function select_test() -> i32{
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

export function block_expr_test() -> i32{
    b = 22.4
    b as i32
}