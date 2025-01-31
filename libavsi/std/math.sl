mod std::math

export no_mangle function _sin(x: f64) -> f64

export no_mangle function _cos(x: f64) -> f64
export no_mangle function _tan(x: f64) -> f64
export no_mangle function _cot(x: f64) -> f64
export no_mangle function _sec(x: f64) -> f64
export no_mangle function _csc(x: f64) -> f64
export no_mangle function _log(x: f64) -> f64
export no_mangle function _exp(x: f64) -> f64
export no_mangle function _sqrt(x: f64) -> f64
export no_mangle function _arcsin(x: f64) -> f64
export no_mangle function _arccos(x: f64) -> f64
export no_mangle function _arctan(x: f64) -> f64
export no_mangle function _arccot(x: f64) -> f64

no_mangle generic sin {
    default: _sin
}

no_mangle generic cos {
    default: _cos
}

no_mangle generic tan {
    default: _tan
}

no_mangle generic cot {
    default: _cot
}

no_mangle generic sec {
    default: _sec
}

no_mangle generic csc {
    default: _csc
}

no_mangle generic log {
    default: _log
}

no_mangle generic exp {
    default: _exp
}

no_mangle generic sqrt {
    default: _sqrt
}

no_mangle generic arcsin {
    default: _arcsin
}

no_mangle generic arccos {
    default: _arccos
}

no_mangle generic arctan {
    default: _arctan
}

no_mangle generic arccot {
    default: _arccot
}
