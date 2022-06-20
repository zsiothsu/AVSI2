mod com::avsi::mod3::object

obj Object {
    num: f64
    list: vec[f64;100]
    f64_ptr: vec[f64;0]
    str: vec[char;20]
}

export function entry_() {
    a = Object()

    a.num = 10
    a.list = {1,2,3,4}
    a.f64_ptr = {1,2,3,4,5}
    a.str = {'H','e','l','l','o'}
    a.str = "Hello"

    for(i = 0; i < sizeof(a.list) / sizeof(typename f64); i = i + 1) do
        b = a.list[i]
    done
}