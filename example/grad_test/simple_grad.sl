mod grad_test::simple_grad
import std::math as math
import std::io as io

function grad_x_x_x(x: i32) -> f64 {
    y = x * x * x

    // 6x
    grad(y, x, x)
}

function grad_complex(x: i32) -> f64 {
    t1 = 3 / x + 1
    t2 = x * x / 2
    t3 = x - 1
    y = (t1 + t2) / t3

    // (1 + 6/x^3)/(-1 + x) - (2 (-3/x^2 + x))/(-1 + x)^2 + (2 (1 + 3/x + x^2/2))/(-1 + x)^3
    grad(y, [x, 2])
}

function grad_2d(x: i32, y: i32) -> f64 {
    z = 2 * x * x * y * y

    // 8y
    grad(z, [x, 2], y)
}

function grad_func(x: f32) -> f32 {
   y = math::arctan(x) + x
   grad(y, x)
}

function foo(x: f32) -> f32 { x * x * x }
function foo_diff0(x: f32) -> f32 { 3 * x * x }
function foo_diff0_diff0(x: f32) -> f32 { 6 * x }
function foo_diff0_diff0_diff0(x: f32) -> f32 { 6 }

function grad_foo(x: f32) {
    y = foo(x)
    y1 = grad(y, x)
    y2 = grad(y, [x,2])
    y3 = grad(y, [x,3])

    io::println("f(x) = x ^ 3")
    io::println("f(%f) = %f", x, y)
    io::println("f'(%f) = %f", x, y1)
    io::println("f''(%f) = %f", x, y2)
    io::println("f'''(%f) = %f", x, y3)
}


