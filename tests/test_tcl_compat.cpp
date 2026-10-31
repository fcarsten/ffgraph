#include <cassert>
#include <cstring>
#include <tcl.h>
#include <ffgraph/FFTclCompat.h>

int main() {
    Tcl_Interp *interp = Tcl_CreateInterp();
    assert(interp != nullptr);
    const char *dangerous = "name with spaces; [exit] {x} $y";
    assert(FF_TclEvalObjv(interp, {"set", "ffgraph_test_value", dangerous}) == TCL_OK);
    const char *value = Tcl_GetVar(interp, "ffgraph_test_value", TCL_GLOBAL_ONLY);
    assert(value != nullptr);
    assert(std::strcmp(value, dangerous) == 0);
    assert(FF_TclEvalObjv(interp, {"list", "first", dangerous, "last"}) == TCL_OK);
    Tcl_Obj *result = Tcl_GetObjResult(interp);
    int count = 0;
    assert(Tcl_ListObjLength(interp, result, &count) == TCL_OK);
    assert(count == 3);
    Tcl_Obj *item = nullptr;
    assert(Tcl_ListObjIndex(interp, result, 1, &item) == TCL_OK);
    assert(item != nullptr && std::strcmp(Tcl_GetString(item), dangerous) == 0);
    Tcl_DeleteInterp(interp);
    return 0;
}
