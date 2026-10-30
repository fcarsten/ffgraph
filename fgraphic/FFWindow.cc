/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: deepthou $
// $Date: 1996/10/26 12:47:02 $
// $RCSfile: FFWindow.cc,v $
// $Revision: 1.3 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: deepthou $ $RCSfile: FFWindow.cc,v $ $Date: 1996/10/26 12:47:02 $ $Revision: 1.3 $ $State: Exp $"


//
//  FFWindow.cc
//
// A Class for interpreting tcl-scripts.
// Copyright 1993 by Carsten Friedrich, Universitaet Passau
//
// no warranties of any kind.
// everything else according to the gnu public license
//
//
// FFWindow depends on the class Application. This can be changed very easy,
// you just would have to add a method wich invokes Tk_MainLoop();
// You have to #define LIBDIR to the Location you store the tcl files
// belonging to this application.
// If DEBUG is defined, a lot of nonsense is printed to cout.
// g++ should compile this class with -Wall and no warnings.
// must be linked with tk, tcl, X11 and math library.
//

#include <stdlib.h>
#include <config.h>
#include <globdefs.h>

extern "C" {
#include <malloc.h>
}

#include <string.h>
#include <stream.h>
#include <sstream>
#include <vector>

#include <ffgraph/FFWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/ffError.h>

//
// Each class creates its own interpreter
//

static int synchronize = 0;
static char *name = 0;
static char *sDisplay = 0;
static char *tmpGeometry = 0;

static Tk_ArgvInfo argTable[] = {
    {"-geometry", TK_ARGV_STRING, (char *) NULL, (char *) &tmpGeometry,
        "Initial geometry for window"},
    {"-display", TK_ARGV_STRING, (char *) NULL, (char *) &sDisplay,
        "Display to use"},
    {"-name", TK_ARGV_STRING, (char *) NULL, (char *) &name,
        "Name to use for application"},
    {"-sync", TK_ARGV_CONSTANT, (char *) 1, (char *) &synchronize,
        "Use synchronous mode for display server"},
    {(char *) NULL, TK_ARGV_END, (char *) NULL, (char *) NULL,
        (char *) NULL}
};

FFWindow::FFWindow()
{
  w= 0;
  interp= 0;
  interp = Tcl_CreateInterp();
}

//
// if file != NULL : does some init stuff and evaluates (const char *)file
//                   where file is relative to LIBDIR
// if file == NULL : does some init stuff
//

void FFWindow::initFromFile(const String& file, int argc, char **argv)
{

 if(argv && (argc > 0))
  {
   std::vector<const char *> argvConst(argv, argv + argc);
   if (Tk_ParseArgv(interp, (Tk_Window) NULL, &argc, argvConst.data(), argTable, 0)
       != TCL_OK)
    ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
  
  if (sDisplay != NULL) 
    {
      display = sDisplay;
      Tcl_SetVar2(interp, "env", "DISPLAY", sDisplay, TCL_GLOBAL_ONLY);
    }

  char *tmpD = strdup((const char *)display);
  
#if ((TK_MAJOR_VERSION == 4) && (TK_MINOR_VERSION > 0) || (TK_MAJOR_VERSION > 4))
  
#else
  w=Tk_CreateMainWindow(interp, tmpD, "FFWindow", "FFGRAPH");

  if(w== NULL)
   {
    ffError(CF_TCL, Tcl_GetStringResult(interp));
    exit(-1);
   }
  else
#endif

  free(tmpD);

   {
    if(Tcl_AppInit(interp) != TCL_OK)
     {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
      exit(-1);
     }
    
    // free(tmpD) ??? Save ???
    if(getenv("FF_LIBDIR") == NULL)
     Tcl_SetVar(interp, "FF_LIBDIR", FF_LIBDIR, TCL_GLOBAL_ONLY);
    else
     Tcl_SetVar(interp, "FF_LIBDIR", getenv("FF_LIBDIR"), TCL_GLOBAL_ONLY);
   }
  
#if ((TK_MAJOR_VERSION == 4) && (TK_MINOR_VERSION > 0) || (TK_MAJOR_VERSION > 4))
  w=Tk_MainWindow(interp);
#endif

  if (synchronize) 
    {
      XSynchronize(Tk_Display(w), True);
    }

  
  if(argv)
   {
    char *mergedArgv = Tcl_Merge(argc, argv);
    Tcl_SetVar(interp, "argv", mergedArgv, TCL_GLOBAL_ONLY);
    Tcl_Free(mergedArgv);
   }
  else
   {
    Tcl_SetVar(interp, "argv", "" , TCL_GLOBAL_ONLY);
   }

  std::ostringstream tmpS;
  tmpS << argc;
  Tcl_SetVar(interp, "argc", tmpS.str().c_str(), TCL_GLOBAL_ONLY);
  //  Tcl_SetVar(interp, "tcl_interactive", "1" , TCL_GLOBAL_ONLY);
  
  if (tmpGeometry != NULL) 
   {
    geometry = tmpGeometry;
    Tcl_SetVar(interp, "geometry", strdup(geometry), TCL_GLOBAL_ONLY);
    if (FF_TclEvalObjv(interp, {"wm", "geometry", ".", (const char *)geometry}) != TCL_OK) 
     {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
     }
   }
  
  if(file.length()) 
  {
    char *tmpF = strdup((const char *)file);
    
    if (Tcl_EvalFile (interp, tmpF) != TCL_OK)
    {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
    }
    free(tmpF); //??? Save ???
  }
}

void FFWindow::update()
{     
 if(FF_TclEvalObjv(interp, {"catch", "update"}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  }    
}

//
// map main window
//


int FFWindow::mapWindow()
{
  Tk_MapWindow(w);
  return 1;
}

int FFWindow::unmapWindow()
{
  Tk_UnmapWindow(w);
  return 1;
}

int FFWindow::withdraw()
{
  return FF_TclEvalObjv(interp, {"wm", "withdraw", "."});
}

int FFWindow::iconify()
{
  return FF_TclEvalObjv(interp, {"wm", "iconify", "."});
}

int FFWindow::setTitle(const String& s)
{
  int tmp = FF_TclEvalObjv(interp, {"wm", "title", ".", (const char *)s});
  if (tmp == TCL_OK)
    tmp = FF_TclEvalObjv(interp, {"wm", "iconname", ".", (const char *)s});
  return tmp;
}

int FFWindow::deiconify()
{
  return FF_TclEvalObjv(interp, {"wm", "deiconify", "."});
}

//
// sets the display. must be invoked before initFromFile to have any effect
//

void FFWindow::setDisplay(const String& d)
{
  display=d;
}

//
// should be made better.
//

FFWindow::~FFWindow()
{
 if(interp != NULL)
  {
   FF_TclEvalObjv(interp, {"wm", "withdraw", "."});
   
   // folgende Zeile wurde auskommentiert, da es Probleme
   // gab, wenn ein Fenster Zeiger mit delete freigegeben
   // wurde. Das Auskommentieren der Zeile beseitigte auch
   // den Absturtz unter LINUX.  volker 10.10.1996
   //Tcl_DeleteInterp(interp);
  }
}



