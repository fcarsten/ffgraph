// Compile/run smoke test for FFTclCompat.h without requiring Tcl development
// packages. The stub captures the exact argument vector passed to Tcl_EvalObjv.
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>
#include <tcl.h>

static std::vector<std::string> captured;
static int capturedFlags = 0;
static int liveObjects = 0;
extern "C" Tcl_Obj *Tcl_NewStringObj(const char *s, int len) {
    Tcl_Obj *o = new Tcl_Obj;
    std::string *copy = new std::string(s, len < 0 ? std::strlen(s) : static_cast<size_t>(len));
    o->bytes = copy->c_str();
    o->refs = 0;
    o->backing = copy;
    ++liveObjects;
    return o;
}
extern "C" void Tcl_IncrRefCount(Tcl_Obj *o) { ++o->refs; }
extern "C" void Tcl_DecrRefCount(Tcl_Obj *o) {
    if (--o->refs == 0) {
        delete static_cast<std::string *>(o->backing);
        --liveObjects;
        delete o;
    }
}
extern "C" int Tcl_EvalObjv(Tcl_Interp *, int objc, Tcl_Obj *const objv[], int flags) {
    captured.clear();
    for (int i = 0; i < objc; ++i) captured.emplace_back(objv[i]->bytes);
    capturedFlags = flags;
    return TCL_OK;
}
#include <ffgraph/FFTclCompat.h>

int main() {
    Tcl_Interp interp{};
    const char *dangerous = "name with spaces; [exit] {x} $y";
    assert(FF_TclEvalObjv(&interp, {"setSelected", dangerous, ".canvas"}) == TCL_OK);
    assert(captured.size() == 3);
    assert(captured[0] == "setSelected");
    assert(captured[1] == dangerous);
    assert(captured[2] == ".canvas");
    assert(capturedFlags == TCL_EVAL_GLOBAL);
    assert(liveObjects == 0);
}
