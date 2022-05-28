mod com::avsi::export_mod

export function private_function() {
    return
}

export function foo(x: real) -> real{
    private_function()
    return 0
}

export no_mangle function no_mangle_function() {
    return
}

no_mangle function private_no_mangle_function() {
    return
}