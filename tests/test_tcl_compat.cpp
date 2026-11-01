#include "test_check.h"
#include <cstring>
#include <tcl.h>
#include <ffgraph/FFTclCompat.h>

int main() {
    Tcl_Interp *interp = Tcl_CreateInterp();
    FF_CHECK(interp != nullptr);
    const char *dangerous = "name with spaces; [exit] {x} $y";
    FF_CHECK(FF_TclEvalObjv(interp, {"set", "ffgraph_test_value", dangerous}) == TCL_OK);
    const char *value = Tcl_GetVar(interp, "ffgraph_test_value", TCL_GLOBAL_ONLY);
    FF_CHECK(value != nullptr);
    FF_CHECK(std::strcmp(value, dangerous) == 0);
    FF_CHECK(FF_TclEvalObjv(interp, {"list", "first", dangerous, "last"}) == TCL_OK);
    Tcl_Obj *result = Tcl_GetObjResult(interp);
    int count = 0;
    FF_CHECK(Tcl_ListObjLength(interp, result, &count) == TCL_OK);
    FF_CHECK(count == 3);
    Tcl_Obj *item = nullptr;
    FF_CHECK(Tcl_ListObjIndex(interp, result, 1, &item) == TCL_OK);
    FF_CHECK(item != nullptr && std::strcmp(Tcl_GetString(item), dangerous) == 0);
    Tcl_DeleteInterp(interp);
    return 0;
}
