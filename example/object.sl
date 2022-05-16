mod com::avsi::object

obj Object {
    num: real
    list: vec[real;100]
    real_ptr: vec[real;0]
    str: vec[char;20]
}

export function entry() {
    a = Object()

    a.num = 10
    a.list = {1,2,3,4}
    a.real_ptr = {1,2,3,4,5}
    a.str = {'H','e','l','l','o'}
    a.str = "Hello"

    for(i = 0; i < sizeof(a.list) / sizeof(typename real); i = i + 1) do
        b = a.list[i]
    done
}