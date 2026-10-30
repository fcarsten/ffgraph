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
#include <signal.h>
#include <sys/time.h>  // gettimeofday(), struct timeval

// This function is used if I want a conditional breakpoint by wrapping
// a call to this function in an if clause

// int ffBreak()
// {
//  cerr << "Breaking ... " << endl;
//  return -1;
// }

main(int argc, char **argv)
{
 // Floating point exeptions suck. Especially division by zero.
 // What is +-infinity and NAN defined for in IEEE if not for handling
 // this kind of stuff?

 signal(SIGFPE, SIG_IGN);

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

 // If arguments were supplied I try to open them as graphs.

 if((argc > 1) && (atoi(argv[1]) <= 0)) {      		
   FFGraph tmpG[argc];
   
   for(int i=1; i<argc; i++)
    {			
     FFTclBasicWindow *fenster = 0;
     
     if(appName.contains("ffgraph3D"))
      fenster=new FFTcl3DWindow;
     else 
      fenster=new FFTclWindow;
     
     // FFLayoutLabel * clustering = (FFLayoutLabel *) tmpG[i-1]->getGraphLabel("FFKaese");

     tmpG[i-1]->createGraphic(fenster);
     if(fenster != NULL)
      fenster->setFileName(argv[i]);
     
     tmpG[i-1]->load(String(argv[i]), YES);
     cerr << "ready loading" << endl;
     
     // clustering->layout();
     
    }
  }
 else // If no arguments were supplied, I open an empty window
  {			
   FFGraph tmpG;
   if(appName.contains("ffgraph3D"))
    tmpG->createGraphic(new FFTcl3DWindow);
   else 
    tmpG->createGraphic(new FFTclWindow);
  }

 // Now I start the main event loop by passing control to the Tcl/Tk library
 // This function never returns.

 Tk_MainLoop();  
}

// EOF
