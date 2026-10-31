/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/10/25 16:12:09 $
// $RCSfile: ffmain.cc,v $
// $Revision: 0.9 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: friedric $ $RCSfile: ffmain.cc,v $ $Date: 1995/10/25 16:12:09 $ $Revision: 0.9 $ $State: Exp $"

#include <String.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFClusterWindow.h>
#include <ffgraph/FFSubGraphLabel.h>
#include <ffgraph/FFTIGraphLabel.h>
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFSugi.h>
#include <ffgraph/FFNaiv.h>
#include <ffgraph/FFOrtho.h>
#include <ffgraph/FFSpqr.h>
#include <ffgraph/FFKaese.h>
#include <ffgraph/FFRandom.h>
#include <ffgraph/FFShow.h>
#include <vector>
#include <cerrno>
#include <cstdlib>
#include <sys/time.h>  // gettimeofday(), struct timeval

// This function is used if I want a conditional breakpoint by wrapping
// a call to this function in an if clause

// int ffBreak()
// {
//  cerr << "Breaking ... " << endl;
//  return -1;
// }

// A legacy positive integer first argument selects the empty-window mode.
// Require the *entire* argument to be numeric so filenames such as
// "2026.graph" are not mistaken for mode selectors.
static bool isPositiveIntegerArgument(const char* argument)
{
 if (!argument || !*argument) return false;
 errno = 0;
 char* end = nullptr;
 const long value = std::strtol(argument, &end, 10);
 return errno == 0 && end != argument && *end == '\0' && value > 0;
}

int main(int argc, char **argv)
{
 // Keep SIGFPE at its default disposition. IEEE-754 floating-point
 // divisions normally produce infinities/NaNs without a signal, whereas
 // ignoring an integer divide-by-zero trap can repeatedly fault.

 // Seeding random generator
 
 struct timeval tp;
 gettimeofday(&tp, NULL);

 srand48(tp.tv_usec);

 // As this is intended to be a graph editor I put layouters to all graphs
 // by default

 addDefaultGraphLabel(new FFSugi);
 addDefaultGraphLabel(new FFSpring);
 // addDefaultNodeLabel(new FFSubGraphLabel);
 addDefaultGraphLabel(new FFKaese);
 addDefaultGraphLabel(new FFRandom);
 addDefaultGraphLabel(new FFShow);
 addDefaultGraphLabel(new FFOrtho);
//  addDefaultGraphLabel(new FFSpqr);
 
 // Is always false, but needed to make the linker include the symbol :(
 // I really hate these braindead Unix linkers.
 
 if(lrand48()+1 == 0)
  addDefaultGraphLabel(new FFTIGraphLabel);
 
 // According to the name with which the application was started I will
 // open a 2D or 3D window.
 
 String appName(argv[0]);

 // Keep graph handles alive until the Tk event loop has finished. Otherwise
 // the last external graph reference can disappear before any GUI events run.
 std::vector<FFGraph> initialGraphs;

 // If arguments were supplied I try to open them as graphs.
 if((argc > 1) && !isPositiveIntegerArgument(argv[1])) {
   initialGraphs.resize(static_cast<std::size_t>(argc - 1));
   
   for(int i=1; i<argc; i++)
    {			
     FFTclBasicWindow *fenster = 0;
     
     if(appName.contains("ffgraph3D"))
      fenster=new FFTcl3DWindow;
     else 
      fenster=new FFTclWindow;
     
     // FFLayoutLabel * clustering = (FFLayoutLabel *) initialGraphs[i-1]->getGraphLabel("FFKaese");

     initialGraphs[i-1]->createGraphic(fenster);
     if(fenster != NULL)
      fenster->setFileName(argv[i]);
     
     initialGraphs[i-1]->load(String(argv[i]), YES);
     cerr << "ready loading" << endl;
     
     // clustering->layout();
     
    }
  }
 else // If no arguments were supplied, I open an empty window
  {			
   initialGraphs.emplace_back();
   if(appName.contains("ffgraph3D"))
    initialGraphs.back()->createGraphic(new FFTcl3DWindow);
   else
    initialGraphs.back()->createGraphic(new FFTclWindow);
  }

 // Process Tcl/Tk events and reclaim deleted window interpreters between
 // top-level events.
 // Tcl_DeleteInterp is unsafe inside commands that delete their own window.
 FFWindow::mainLoop();
 // Releasing the initial graph handles may itself destroy window labels.
 // Collect any interpreters queued by their destructors before exiting.
 initialGraphs.clear();
 FFWindow::collectPendingInterpreters();
}

// EOF
