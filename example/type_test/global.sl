mod com::avsi::type_test::global

export global global_variable: i32
export global unused_global_variable: i32

export function global_init() {
    global_variable = 1234
}