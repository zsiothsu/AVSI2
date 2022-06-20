mod com::avsi::type_test::expr

export function expr_test() {
    a = 1
    b = 2
    c = a + b - 312
    d = c as f32
    e = c / 313.4234 + 2.5e6 * 0.2
}