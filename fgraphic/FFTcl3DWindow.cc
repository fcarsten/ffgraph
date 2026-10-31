/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/06 14:25:43 $
// $RCSfile: FFTcl3DWindow.cc,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: friedric $ $RCSfile: FFTcl3DWindow.cc,v $ $Date: 1996/05/06 14:25:43 $ $Revision: 1.1 $ $State: Exp $"

#include <config.h>

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <X11/Xatom.h>
}

#include <stream.h>
#include <fstream.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <string>

#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/ffError.h>

#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>

int fgraphWindowSel(ClientData, int, char *, int);

FFTcl3DWindow::FFTcl3DWindow()
{
 String tmpS;
 
 if(getenv("FF_LIBDIR") == NULL)
  tmpS=String(FF_LIBDIR);
 else
  tmpS=String(getenv("FF_LIBDIR"));
 
 tclFileName =tmpS + "/tcl/fgraph.tcl";
 if(getenv("FF_ICONDIR") == NULL)
  tmpS=String(FF_LIBDIR) + "/icons/";
 else
  tmpS=String(getenv("FF_ICONDIR"));
 iconLibDir = tmpS;
}

void FFTcl3DWindow::_savePOV(const String& datei)
{
 FFMatrix raus(ctm);
 FFPoint trans=raus.row[3];
 FFPoint camPos=viewPoint - trans;
 
 FFPoint lookAt(0,0,0);
 raus.row[3] = FFPoint(0,0,0);
 
 ofstream ofDatei(expand_path("out.pov"));
 if(!ofDatei.good())
  {
   ffError(USER_ERROR, String("Couldn't open File: ") + datei);
   return;
  }
 ofDatei << "#include \"colors.inc\"" << endl;
 ofDatei << "#include \"shapes.inc\"" << endl;
 ofDatei << "#include \"finish.inc\"" << endl;
 ofDatei << "#include \"glass.inc\""  << endl;
 ofDatei << "#include \"metals.inc\"" << endl;
 ofDatei << "#include \"stones.inc\"" << endl;
 ofDatei << "#include \"woods.inc\""  << endl << endl;


 ofDatei << "background { color Gray80 } " << endl;
 ofDatei << "camera {" << endl;
 ofDatei << " location <" << camPos.v[0]/1 << "," << -camPos.v[1]/1
         << "," << camPos.v[2]/1 << ">" << endl;
 ofDatei << " look_at <" << lookAt.v[0]/1 << "," << -lookAt.v[1]/1
         << "," << lookAt.v[2]/1 << ">\n}\n" << endl;
 
 ofDatei << "light_source { <" << camPos.v[0]/1+200 << "," 
         << -camPos.v[1]/1+500 << "," << camPos.v[2]/1+50
         << "> color White}" << endl;

 const FFSet<FFNode>& alleKnoten = graph->allNodes();
 
 for(Pix i=alleKnoten.first(); i!= 0; alleKnoten.next(i))
  {
   FFGraphicNode *gn= alleKnoten(i)->getGraphicNode();
   if(gn != NULL)
    {
     const FFPoint& tmpP = gn->getCoords() * raus;
     
     ofDatei << "sphere {" << endl;
     ofDatei << " <" << tmpP.v[0]/1 << "," << -tmpP.v[1]/1 << "," 
             << tmpP.v[2]/1 << ">, " << scale*gn->getWidth() << endl;
     ofDatei << "texture { pigment { color Yellow } }" << endl;
     ofDatei << "rotate clock*360\n}" << endl;
     
    }
  }
  
 const FFSet<FFEdge>& alleKanten = graph->allEdges();
 
 for(Pix i=alleKanten.first(); i!= 0; alleKanten.next(i))
  {
   FFEdge tmpE = alleKanten(i);
   
   FFGraphicEdge *ge= tmpE->getGraphicEdge();
   if(ge != NULL)
    {
     int numPoints = ge->anzPoints();
     
     for(int k=0; k < numPoints-1; k++)
      {
       const FFPoint& tmpS = ge->getPointAt(k) * raus;
       const FFPoint& tmpT = ge->getPointAt(k+1) * raus;
       if(tmpS != tmpT)
        {
         ofDatei << "cylinder {" << endl;
         ofDatei << " <" << tmpS.v[0]/1 << "," << -tmpS.v[1]/1 << "," 
                 << tmpS.v[2]/1 << ">, " ;
         ofDatei << " <" << tmpT.v[0]/1 << "," << -tmpT.v[1]/1 << "," 
                 << tmpT.v[2]/1 << ">, " << scale << endl;
         ofDatei << "texture { pigment { color Black } }" << endl;
         ofDatei << "rotate clock*360\n}" << endl;
         FFGraphicNode *gn= tmpE->target()->getGraphicNode();
         
         if(graph->getDirected() && (gn != NULL))
          {
           FFPoint endPunkt = gn->getCoords();
           FFPoint v(0,0,0);

           for(int l=2; (l <= numPoints) && (v == FFPoint(0,0,0)); l++)
            v= ge->getPointAt(numPoints-l) - endPunkt ;
           
           if(v != FFPoint(0,0,0))
            {
             v = v/v.abs();
             FFPoint coneEnd= (endPunkt + v*gn->getWidth()) ;
             FFPoint coneBase= (coneEnd + v*8) * raus;
             coneEnd = coneEnd * raus;
             ofDatei << "cone {< " << coneBase.v[0] << "," << -coneBase.v[1] 
                     << "," << coneBase.v[2] << ">, " << scale*4
                     << " < " << coneEnd.v[0] 
                     << "," << -coneEnd.v[1] << "," << coneEnd.v[2] 
                     << ">, " << scale << endl;
             ofDatei << "texture { pigment { color Red } }" << endl;
             ofDatei << "rotate clock*360\n}" << endl;
            }
          }
        }
      }
    }
  }
 
 system("x-povray +H480 +W640 +AA +Iout.pov &");
 
}

int FFTcl3DWindow::recall(Tcl_Interp *i, int c, const char **v)
{
 // cerr << "ping" << endl;
 String befehl(v[1]);
 
 if(befehl == "exportPOV")
  {
   _savePOV(String("out.pov"));
  }
 else
  return FFTclWindow::recall(i,c,v);
  
 return TCL_OK;
}

struct hideStruct
{
 FFGraphicNode *n;
 std::string s;
 std::string l;
 FFCoord z;
};

void FFTcl3DWindow::_hideLines()
{
 int laenge=nodeMap.length();
 FFPoint tmpP;
 
 if(laenge<2)
  return;
 
 std::vector<hideStruct> liste;
 liste.reserve(laenge);
 int k=0;
 
 for(Pix i= nodeMap.first(); i!=0; nodeMap.next(i))
  {
   if(nodeMap.contents(i))
    {
     liste.emplace_back();
     liste[k].n = nodeMap.contents(i);
     tmpP = proj(liste[k].n->getCoords(),
                 liste[k].n->getRange());
     liste[k].z = tmpP.v[2];
     liste[k].s = liste[k].n->getId().c_str();
     liste[k].l = liste[k].n->getLabelId().c_str();
     k++;
    }
  }
 
 std::sort(liste.begin(), liste.end(),
           [](const hideStruct& a, const hideStruct& b) { return a.z < b.z; });
 if (liste.empty()) return; 
 
 FF_TclEvalObjv(interp, {(const char *)canvas, "raise", liste[0].s.c_str(), "all"});
 FF_TclEvalObjv(interp, {(const char *)canvas, "raise", liste[0].l.c_str(), liste[0].s.c_str()});
 
 for(int j=1; j< k; j++)
  {
   FF_TclEvalObjv(interp, {(const char *)canvas, "raise", liste[j].s.c_str(), liste[j-1].l.c_str()});
   FF_TclEvalObjv(interp, {(const char *)canvas, "raise", liste[j].l.c_str(), liste[j].s.c_str()});
  } 		
}
