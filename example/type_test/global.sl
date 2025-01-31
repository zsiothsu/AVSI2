mod type_test::global

global global_variable: i32
global unused_global_variable: i32

function global_init() {
    global_variable = 1234
}