mod grad_test::simple_grad
import std::math as math

export function grad_x_x_x(x: i32) -> f64 {
    y = x * x * x

    // 6x
    grad(y, x, x)
}

export function grad_complex(x: i32) -> f64 {
    t1 = 3 / x + 1
    t2 = x * x / 2
    t3 = x - 1
    y = (t1 + t2) / t3

    // (1 + 6/x^3)/(-1 + x) - (2 (-3/x^2 + x))/(-1 + x)^2 + (2 (1 + 3/x + x^2/2))/(-1 + x)^3
    grad(y, {x, 2})
}

export function grad_2d(x: i32, y: i32) -> f64 {
    z = 2 * x * x * y * y

    // 8y
    grad(z, {x, 2}, y)
}

export function grad_func(x: f32) -> f32 {
   y = math::arctan(x) + x
   grad(y, x)
}