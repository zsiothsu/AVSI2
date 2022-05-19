mod com::avsi::import_mod

import com::avsi::export_mod
import com::avsi::mod1
import com::avsi::mod3::object
import com::avsi::mod2::loop

export function entry() {
    # a = export_mod::foo(1)
    # a = com::avsi::export_mod::foo(1)
    # a = com::avsi::mod1::sub_mod1::foo()
}