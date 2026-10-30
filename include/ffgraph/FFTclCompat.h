#ifndef FFGRAPH_FFTCLCOMPAT_H
#define FFGRAPH_FFTCLCOMPAT_H

#include <tcl.h>
#include <initializer_list>
#include <vector>

// Evaluate a Tcl command as an argument vector. Unlike Tcl_VarEval, each
// string is one argument and is never reparsed as Tcl source.
inline int FF_TclEvalObjv(Tcl_Interp *interp,
                          std::initializer_list<const char *> arguments)
{
    std::vector<Tcl_Obj *> objects;
    objects.reserve(arguments.size());
    for (const char *argument : arguments) {
        Tcl_Obj *object = Tcl_NewStringObj(argument ? argument : "", -1);
        Tcl_IncrRefCount(object);
        objects.push_back(object);
    }
    const int result = Tcl_EvalObjv(interp, static_cast<int>(objects.size()),
                                    objects.data(), TCL_EVAL_GLOBAL);
    for (Tcl_Obj *object : objects) {
        Tcl_DecrRefCount(object);
    }
    return result;
}

#endif
