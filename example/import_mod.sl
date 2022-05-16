mod com::avsi::import_mod

import com::avsi::export_mod        # absolute path
import export_mod                   # relative path

export function entry() {
    a = export_mod::foo(1)
    a = com::avsi::export_mod::foo(1)
}