#include <string.h>

#include <ffgraph/FFKaese.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFLayoutLabel.h>
#include <ffgraph/FFTIEdgeLabel.h>
#include <ffgraph/FFTIGraphLabel.h>
#include <ffgraph/FFGraphicEdge.h>
#include <container/VOHSet.h>
#include <container/AVLSet.h>
#include <ffgraph/FFMatrix.h>

FFKaese::FFKaese () : cutMap(NULL)
{
 _setType(ffKaeseType);
 nucleus= 0;
 clusterSize= 0;
}


FFKaese::~FFKaese ()
{
 if(nucleus!=NULL) 
  delete [] nucleus;
}

void FFKaese::init(const FFGraph& g)
{ 
 FFLayoutLabel::init(g);
}

void FFKaese::thinGraph()
{
 const FFSet<FFNode>& tmpS= graph->allNodes();
 FFSet<FFNode> delNodes;
 
 for(Pix i=tmpS.first(); i!= 0; tmpS.next(i))
  {
   FFNode tmpN=tmpS(i);
   int tmpV= tmpN->getValue();
   
   if(nucleus[tmpV]!= tmpN)
    {
     BOOL borderNode= NO;
     
     const FFSet<FFEdge>& tmpSucc = tmpN->transToSucc();
     for(Pix j=tmpSucc.first(); (j!= 0) && (borderNode == NO); tmpSucc.next(j))
      {
       if(tmpSucc(j)->target()->getValue() != tmpV)
        {
         borderNode= YES;
        }
      }
   
     const FFSet<FFEdge>& tmpPred = tmpN->transFromPred();
     for(Pix j=tmpPred.first(); (j!= 0) && (borderNode == NO); tmpPred.next(j))
      {
       if(tmpPred(j)->source()->getValue() != tmpV)
        {
         borderNode = YES;
        }
      }
     if(borderNode == YES)
      {
       graph->addEdge(nucleus[tmpV], tmpN);
      }
     else
      {
       delNodes.add(tmpN);
      }
    }
  }
  
 for(Pix i=delNodes.first(); i!= 0; delNodes.next(i))
  graph->removeNode(delNodes(i));
 
 // graph->eliminateParallelEdges();
}

void FFKaese::placeNodes()
{
 FFCoord aktuPhi=0.0;
 FFCoord unit = 1.0;
 FFPoint initPos(800,0);

 // Place cluster cores on a circle
 
 int numNodes =0;
 for(int i=0; i< numClusters; i++)
  {
   if(nucleus[i] != NULL)
    {
     numNodes++;
    }
  }

 // An empty cluster set must not trigger an integer division by zero.
 unit = numNodes > 0 ? 360.0 / numNodes : 0.0;
 
 for(int i=0; i< numClusters; i++)
  {
   if(nucleus[i] != NULL)
    {
     nucleus[i]->getGraphicNode()->moveTo(rotateZ(initPos, aktuPhi));
     aktuPhi+=unit;
    }
  }
  
 // place remaining node
 const FFSet<FFNode>& tmpS= graph->allNodes();
 
 for(Pix i=tmpS.first(); i!= 0; tmpS.next(i))
  {
   FFNode tmpN=tmpS(i);
   int tmpV= tmpN->getValue();
   
   if(nucleus[tmpV]!= tmpN)
    {
     FFPoint median;
     
     const FFSet<FFEdge>& tmpSucc = tmpN->transToSucc();
     for(Pix j=tmpSucc.first(); j!= 0; tmpSucc.next(j))
      {
       FFNode tmpN2= nucleus[tmpSucc(j)->target()->getValue()];
       median += tmpN2->getGraphicNode()->getCoords();
      }
     
     const FFSet<FFEdge>& tmpPred = tmpN->transFromPred();
     for(Pix j=tmpPred.first(); j!= 0; tmpPred.next(j))
      {
       FFNode tmpN2= nucleus[tmpPred(j)->source()->getValue()];
       median += tmpN2->getGraphicNode()->getCoords();
      }
     
     median = median/tmpN->deg();
     //      median += nucleus[tmpV]->getGraphicNode()->getCoords();
     //      median = median/2.0;
     FFPoint newPos = median - nucleus[tmpV]->getGraphicNode()->getCoords();
     newPos = nucleus[tmpV]->getGraphicNode()->getCoords() + 
      (newPos/ newPos.abs2D())*50;
     tmpN->getGraphicNode()->moveTo(newPos);
    }
  }
}


void FFKaese::layout ()
{
 BOOL tmpUpdate=YES;
 if (graph->getWindow ())
  {
   graph->getWindow()->setBusy ();
   tmpUpdate = graph->getWindow()->setUpdate(NO);
  }
 
 doLayout();
 
 if (graph->getWindow ())
  {
   graph->getWindow()->unsetBusy ();
   graph->getWindow()->setUpdate(tmpUpdate);
   graph->getWindow()->adjustWindowSize();
  }
 cerr << "Ready" << endl;
}

void FFKaese::updateCuts()
{
// int tmpInner=0;
// int tmpOuter=0;
 
 for(Pix i= setA.first(); i!= 0; setA.next(i))
  {
   FFNode tmpN= setA(i);

   cutMap[tmpN]->inner=0;
   cutMap[tmpN]->outer=0;
   
   const FFSet<FFEdge>& tmpSucc = tmpN->transToSucc();
   for(Pix j=tmpSucc.first(); j!= 0; tmpSucc.next(j))
    {
     if(setA.contains(tmpSucc(j)->target()))
      cutMap[tmpN]->inner ++;
     else
      cutMap[tmpN]->outer ++;
    }
   
   const FFSet<FFEdge>& tmpPred = tmpN->transFromPred();
   for(Pix j=tmpPred.first(); j!= 0; tmpPred.next(j))
    {
     if(setA.contains(tmpPred(j)->source()))
      cutMap[tmpN]->inner ++;
     else
      cutMap[tmpN]->outer ++;
    }
   tmpN->setName(String(dec(cutMap[tmpN]->inner)) + ":" + dec(cutMap[tmpN]->outer));
  }

 for(Pix i= setB.first(); i!= 0; setB.next(i))
  {
   FFNode tmpN= setB(i);

   cutMap[tmpN]->inner=0;
   cutMap[tmpN]->outer=0;

   const FFSet<FFEdge>& tmpSucc = tmpN->transToSucc();
   for(Pix j=tmpSucc.first(); j!= 0; tmpSucc.next(j))
    {
     if(setB.contains(tmpSucc(j)->target()))
      cutMap[tmpN]->inner ++;
     else
      cutMap[tmpN]->outer ++;
    }
   
   const FFSet<FFEdge>& tmpPred = tmpN->transFromPred();
   for(Pix j=tmpPred.first(); j!= 0; tmpPred.next(j))
    {
     if(setB.contains(tmpPred(j)->source()))
      cutMap[tmpN]->inner ++;
     else
      cutMap[tmpN]->outer ++;
    }
    
   tmpN->setName(String(dec(cutMap[tmpN]->inner)) + ":" + dec(cutMap[tmpN]->outer));
  }
}
 
const char *farbe[]= {
"green", "red", "blue", "brown", "cyan", "magenta",
"yellow", "orange",
"snow",
"ghost white",
"GhostWhite",
"white smoke",
"WhiteSmoke",
"gainsboro",
"floral white",
"FloralWhite",
"old lace",
"OldLace",
"linen",
"antique white",
"AntiqueWhite",
"papaya whip",
"PapayaWhip",
"blanched almond",
"BlanchedAlmond",
"bisque",
"peach puff",
"PeachPuff",
"navajo white",
"NavajoWhite",
"moccasin",
"cornsilk",
"ivory",
"lemon chiffon",
"LemonChiffon",
"seashell",
"honeydew",
"mint cream",
"MintCream",
"azure",
"alice blue",
"AliceBlue",
"lavender",
"lavender blush",
"LavenderBlush",
"misty rose",
"MistyRose",
"white",
"black",
"dark slate gray",
"DarkSlateGray",
"dark slate grey",
"DarkSlateGrey",
"dim gray",
"DimGray",
"dim grey",
"DimGrey",
"slate gray",
"SlateGray",
"slate grey",
"SlateGrey",
"light slate gray",
"LightSlateGray",
"light slate grey",
"LightSlateGrey",
"gray",
"grey",
"light grey",
"LightGrey",
"light gray",
"LightGray",
"midnight blue",
"MidnightBlue",
"navy",
"navy blue",
"NavyBlue",
"cornflower blue",
"CornflowerBlue",
"dark slate blue",
"DarkSlateBlue",
"slate blue",
"SlateBlue",
"medium slate blue",
"MediumSlateBlue",
"light slate blue",
"LightSlateBlue",
"medium blue",
"MediumBlue",
"royal blue",
"RoyalBlue",
"blue",
"dodger blue",
"DodgerBlue",
"deep sky blue",
"DeepSkyBlue",
"sky blue",
"SkyBlue",
"light sky blue",
"LightSkyBlue",
"steel blue",
"SteelBlue",
"light steel blue",
"LightSteelBlue",
"light blue",
"LightBlue",
"powder blue",
"PowderBlue",
"pale turquoise",
"PaleTurquoise",
"dark turquoise",
"DarkTurquoise",
"medium turquoise",
"MediumTurquoise",
"turquoise",
"cyan",
"light cyan",
"LightCyan",
"cadet blue",
"CadetBlue",
"medium aquamarine",
"MediumAquamarine",
"aquamarine",
"dark green",
"DarkGreen",
"dark olive green",
"DarkOliveGreen",
"dark sea green",
"DarkSeaGreen",
"sea green",
"SeaGreen",
"medium sea green",
"MediumSeaGreen",
"light sea green",
"LightSeaGreen",
"pale green",
"PaleGreen",
"spring green",
"SpringGreen",
"lawn green",
"LawnGreen",
"green",
"chartreuse",
"medium spring green",
"MediumSpringGreen",
"green yellow",
"GreenYellow",
"lime green",
"LimeGreen",
"yellow green",
"YellowGreen",
"forest green",
"ForestGreen",
"olive drab",
"OliveDrab",
"dark khaki",
"DarkKhaki",
"khaki",
"pale goldenrod",
"PaleGoldenrod",
"light goldenrod yellow",
"LightGoldenrodYellow",
"light yellow",
"LightYellow",
"yellow",
"gold",
"light goldenrod",
"LightGoldenrod",
"goldenrod",
"dark goldenrod",
"DarkGoldenrod",
"rosy brown",
"RosyBrown",
"indian red",
"IndianRed",
"saddle brown",
"SaddleBrown",
"sienna",
"peru",
"burlywood",
"beige",
"wheat",
"sandy brown",
"SandyBrown",
"tan",
"chocolate",
"firebrick",
"brown",
"dark salmon",
"DarkSalmon",
"salmon",
"light salmon",
"LightSalmon",
"orange",
"dark orange",
"DarkOrange",
"coral",
"light coral",
"LightCoral",
"tomato",
"orange red",
"OrangeRed",
"red",
"hot pink",
"HotPink",
"deep pink",
"DeepPink",
"pink",
"light pink",
"LightPink",
"pale violet red",
"PaleVioletRed",
"maroon",
"medium violet red",
"MediumVioletRed",
"violet red",
"VioletRed",
"magenta",
"violet",
"plum",
"orchid",
"medium orchid",
"MediumOrchid",
"dark orchid",
"DarkOrchid",
"dark violet",
"DarkViolet",
"blue violet",
"BlueViolet",
"purple",
"medium purple",
"MediumPurple",
"thistle",
"snow1",
"snow2",
"snow3",
"snow4",
"seashell1",
"seashell2",
"seashell3",
"seashell4",
"AntiqueWhite1",
"AntiqueWhite2",
"AntiqueWhite3",
"AntiqueWhite4",
"bisque1",
"bisque2",
"bisque3",
"bisque4",
"PeachPuff1",
"PeachPuff2",
"PeachPuff3",
"PeachPuff4",
"NavajoWhite1",
"NavajoWhite2",
"NavajoWhite3",
"NavajoWhite4",
"LemonChiffon1",
"LemonChiffon2",
"LemonChiffon3",
"LemonChiffon4",
"cornsilk1",
"cornsilk2",
"cornsilk3",
"cornsilk4",
"ivory1",
"ivory2",
"ivory3",
"ivory4",
"honeydew1",
"honeydew2",
"honeydew3",
"honeydew4",
"LavenderBlush1",
"LavenderBlush2",
"LavenderBlush3",
"LavenderBlush4",
"MistyRose1",
"MistyRose2",
"MistyRose3",
"MistyRose4",
"azure1",
"azure2",
"azure3",
"azure4",
"SlateBlue1",
"SlateBlue2",
"SlateBlue3",
"SlateBlue4",
"RoyalBlue1",
"RoyalBlue2",
"RoyalBlue3",
"RoyalBlue4",
"blue1",
"blue2",
"blue3",
"blue4",
"DodgerBlue1",
"DodgerBlue2",
"DodgerBlue3",
"DodgerBlue4",
"SteelBlue1",
"SteelBlue2",
"SteelBlue3",
"SteelBlue4",
"DeepSkyBlue1",
"DeepSkyBlue2",
"DeepSkyBlue3",
"DeepSkyBlue4",
"SkyBlue1",
"SkyBlue2",
"SkyBlue3",
"SkyBlue4",
"LightSkyBlue1",
"LightSkyBlue2",
"LightSkyBlue3",
"LightSkyBlue4",
"SlateGray1",
"SlateGray2",
"SlateGray3",
"SlateGray4",
"LightSteelBlue1",
"LightSteelBlue2",
"LightSteelBlue3",
"LightSteelBlue4",
"LightBlue1",
"LightBlue2",
"LightBlue3",
"LightBlue4",
"LightCyan1",
"LightCyan2",
"LightCyan3",
"LightCyan4",
"PaleTurquoise1",
"PaleTurquoise2",
"PaleTurquoise3",
"PaleTurquoise4",
"CadetBlue1",
"CadetBlue2",
"CadetBlue3",
"CadetBlue4",
"turquoise1",
"turquoise2",
"turquoise3",
"turquoise4",
"cyan1",
"cyan2",
"cyan3",
"cyan4",
"DarkSlateGray1",
"DarkSlateGray2",
"DarkSlateGray3",
"DarkSlateGray4",
"aquamarine1",
"aquamarine2",
"aquamarine3",
"aquamarine4",
"DarkSeaGreen1",
"DarkSeaGreen2",
"DarkSeaGreen3",
"DarkSeaGreen4",
"SeaGreen1",
"SeaGreen2",
"SeaGreen3",
"SeaGreen4",
"PaleGreen1",
"PaleGreen2",
"PaleGreen3",
"PaleGreen4",
"SpringGreen1",
"SpringGreen2",
"SpringGreen3",
"SpringGreen4",
"green1",
"green2",
"green3",
"green4",
"chartreuse1",
"chartreuse2",
"chartreuse3",
"chartreuse4",
"OliveDrab1",
"OliveDrab2",
"OliveDrab3",
"OliveDrab4",
"DarkOliveGreen1",
"DarkOliveGreen2",
"DarkOliveGreen3",
"DarkOliveGreen4",
"khaki1",
"khaki2",
"khaki3",
"khaki4",
"LightGoldenrod1",
"LightGoldenrod2",
"LightGoldenrod3",
"LightGoldenrod4",
"LightYellow1",
"LightYellow2",
"LightYellow3",
"LightYellow4",
"yellow1",
"yellow2",
"yellow3",
"yellow4",
"gold1",
"gold2",
"gold3",
"gold4",
"goldenrod1",
"goldenrod2",
"goldenrod3",
"goldenrod4",
"DarkGoldenrod1",
"DarkGoldenrod2",
"DarkGoldenrod3",
"DarkGoldenrod4",
"RosyBrown1",
"RosyBrown2",
"RosyBrown3",
"RosyBrown4",
"IndianRed1",
"IndianRed2",
"IndianRed3",
"IndianRed4",
"sienna1",
"sienna2",
"sienna3",
"sienna4",
"burlywood1",
"burlywood2",
"burlywood3",
"burlywood4",
"wheat1",
"wheat2",
"wheat3",
"wheat4",
"tan1",
"tan2",
"tan3",
"tan4",
"chocolate1",
"chocolate2",
"chocolate3",
"chocolate4",
"firebrick1",
"firebrick2",
"firebrick3",
"firebrick4",
"brown1",
"brown2",
"brown3",
"brown4",
"salmon1",
"salmon2",
"salmon3",
"salmon4",
"LightSalmon1",
"LightSalmon2",
"LightSalmon3",
"LightSalmon4",
"orange1",
"orange2",
"orange3",
"orange4",
"DarkOrange1",
"DarkOrange2",
"DarkOrange3",
"DarkOrange4",
"coral1",
"coral2",
"coral3",
"coral4",
"tomato1",
"tomato2",
"tomato3",
"tomato4",
"OrangeRed1",
"OrangeRed2",
"OrangeRed3",
"OrangeRed4",
"red1",
"red2",
"red3",
"red4",
"DeepPink1",
"DeepPink2",
"DeepPink3",
"DeepPink4",
"HotPink1",
"HotPink2",
"HotPink3",
"HotPink4",
"pink1",
"pink2",
"pink3",
"pink4",
"LightPink1",
"LightPink2",
"LightPink3",
"LightPink4",
"PaleVioletRed1",
"PaleVioletRed2",
"PaleVioletRed3",
"PaleVioletRed4",
"maroon1",
"maroon2",
"maroon3",
"maroon4",
"VioletRed1",
"VioletRed2",
"VioletRed3",
"VioletRed4",
"magenta1",
"magenta2",
"magenta3",
"magenta4",
"orchid1",
"orchid2",
"orchid3",
"orchid4",
"plum1",
"plum2",
"plum3",
"plum4",
"MediumOrchid1",
"MediumOrchid2",
"MediumOrchid3",
"MediumOrchid4",
"DarkOrchid1",
"DarkOrchid2",
"DarkOrchid3",
"DarkOrchid4",
"purple1",
"purple2",
"purple3",
"purple4",
"MediumPurple1",
"MediumPurple2",
"MediumPurple3",
"MediumPurple4",
"thistle1",
"thistle2",
"thistle3",
"thistle4",
"gray0",
"grey0",
"gray1",
"grey1",
"gray2",
"grey2",
"gray3",
"grey3",
"gray4",
"grey4",
"gray5",
"grey5",
"gray6",
"grey6",
"gray7",
"grey7",
"gray8",
"grey8",
"gray9",
"grey9",
"gray10",
"grey10",
"gray11",
"grey11",
"gray12",
"grey12",
"gray13",
"grey13",
"gray14",
"grey14",
"gray15",
"grey15",
"gray16",
"grey16",
"gray17",
"grey17",
"gray18",
"grey18",
"gray19",
"grey19",
"gray20",
"grey20",
"gray21",
"grey21",
"gray22",
"grey22",
"gray23",
"grey23",
"gray24",
"grey24",
"gray25",
"grey25",
"gray26",
"grey26",
"gray27",
"grey27",
"gray28",
"grey28",
"gray29",
"grey29",
"gray30",
"grey30",
"gray31",
"grey31",
"gray32",
"grey32",
"gray33",
"grey33",
"gray34",
"grey34",
"gray35",
"grey35",
"gray36",
"grey36",
"gray37",
"grey37",
"gray38",
"grey38",
"gray39",
"grey39",
"gray40",
"grey40",
"gray41",
"grey41",
"gray42",
"grey42",
"gray43",
"grey43",
"gray44",
"grey44",
"gray45",
"grey45",
"gray46",
"grey46",
"gray47",
"grey47",
"gray48",
"grey48",
"gray49",
"grey49",
"gray50",
"grey50",
"gray51",
"grey51",
"gray52",
"grey52",
"gray53",
"grey53",
"gray54",
"grey54",
"gray55",
"grey55",
"gray56",
"grey56",
"gray57",
"grey57",
"gray58",
"grey58",
"gray59",
"grey59",
"gray60",
"grey60",
"gray61",
"grey61",
"gray62",
"grey62",
"gray63",
"grey63",
"gray64",
"grey64",
"gray65",
"grey65",
"gray66",
"grey66",
"gray67",
"grey67",
"gray68",
"grey68",
"gray69",
"grey69",
"gray70",
"grey70",
"gray71",
"grey71",
"gray72",
"grey72",
"gray73",
"grey73",
"gray74",
"grey74",
"gray75",
"grey75",
"gray76",
"grey76",
"gray77",
"grey77",
"gray78",
"grey78",
"gray79",
"grey79",
"gray80",
"grey80",
"gray81",
"grey81",
"gray82",
"grey82",
"gray83",
"grey83",
"gray84",
"grey84",
"gray85",
"grey85",
"gray86",
"grey86",
"gray87",
"grey87",
"gray88",
"grey88",
"gray89",
"grey89",
"gray90",
"grey90",
"gray91",
"grey91",
"gray92",
"grey92",
"gray93",
"grey93",
"gray94",
"grey94",
"gray95",
"grey95",
"gray96",
"grey96",
"gray97",
"grey97",
"gray98",
"grey98",
"gray99",
                "grey99",
                "gray100",
                "grey100"};


void FFKaese::doLayout()
{
 eventStatus tmpStatus= graph->getEventStatus();
 graph->setEventStatus(EVENT_DISABLED);
 
 const FFSet<FFNode>& tmpS= graph->allNodes();
 int numNodes = graph->getNumNodes();
 int maxDeg = 0;

 for(Pix j=tmpS.first(); j!=0; tmpS.next(j))
  {
   if(tmpS(j)->deg() > maxDeg)
    maxDeg = tmpS(j)->deg();
  }
 
 int *histogram= new int[maxDeg+1];

 for(int i=0; i <= maxDeg; i++)
  histogram[i]=0;

 for(Pix j=tmpS.first(); j!=0; tmpS.next(j))
  {
   histogram[tmpS(j)->deg()] ++;
  }

 for(int i=0; i <= maxDeg; i++)
  if(histogram[i] != 0)
   cout << "Degree: " << i << ", Number : " << histogram[i] << endl;

 int nintyPercent = (numNodes * 9)/10;
 int limit = 0;
 
 {
  int localCount =0;
 
  while( abs(nintyPercent - localCount) >= 
         abs(nintyPercent - (localCount + histogram[limit])) ||
         //  while( (localCount + histogram[limit] < nintyPercent) ||
         (numNodes > localCount + 64))
   localCount += histogram[limit ++];
   
  if(limit > maxDeg)
   limit --;
 }

 // We have to jump back over empty node degrees
 
 while((histogram[limit] == 0) &&
       (limit>0))
  limit--;
  
 numClusters = 0;
 for(Pix j=tmpS.first(); j!=0; tmpS.next(j))
  {
   if(tmpS(j)->deg() >= limit)
    {
     numClusters++;
    }
  }
 
 //  numClusters = MIN(64, (int)rint( (float)numNodes / 10.0));
 
 DLList<FFNode> *clusterList =  new DLList<FFNode>[numClusters];
  
 FFNode *topTen = new FFNode[numClusters];
 if(nucleus != NULL)
  delete [] nucleus;
 
 nucleus = new FFNode[numClusters];
 
 for(int i=0; i< numClusters; i++)
  {
   topTen[i] = 0;
   nucleus[i] = 0;
  }
 
 cerr << "Building " << numClusters << " clusters." << endl;
 
 {
  int localCount=0;
  
  for( Pix iter = tmpS.first(); iter!=0; tmpS.next(iter))
   {
    FFNode tmpN= tmpS(iter);
    tmpN->setValue(-1);
    tmpN->getGraphicNode()->setColor("black");
    
    int aktuDeg = tmpN->deg();
    if(aktuDeg >= limit)
     {
      topTen[localCount++] = tmpN;
     }
   }
 }
 
 
 for(int i=0; i< numClusters; i++)
  {
   if(topTen[i] != NULL)
    {
     topTen[i]->setValue(i);
     topTen[i]->getGraphicNode()->setColor(farbe[i]);
//      cerr << topTen[i]->getName() << " is nucleus with color " << farbe[i] <<  endl;
     nucleus[i] = topTen[i];
    }
  }
 
 // FFSet<FFEdge> delEdges;

 while(maxDeg != 0)
  {
   int newMaxDeg = 0;
   
   // Searching for neighbours of top-ten
   cerr << "Max deg: " << maxDeg << "       \r";
   
   for(int k=0; k< numClusters; k++)
    {
     if(topTen[k] != NULL)
      {
       int aktuDeg= topTen[k]->deg();
       // cerr << "Investigating degree of " << topTen[k]->getName() << " ... ";
       
       if(aktuDeg >= maxDeg)
        {
         FFNode tmpN = topTen[k];
         topTen[k]= 0;
         // cerr << " ok" << endl << "Getting neighbours" << endl;
         
         const FFSet<FFEdge>& tmpSucc = tmpN->transToSucc();
         for(Pix j=tmpSucc.first(); j!= 0; tmpSucc.next(j))
          {
           FFNode tmpT = tmpSucc(j)->target();
           
           if(tmpT->getValue() == -1)
            {
             tmpT->setValue(k);
             tmpT->getGraphicNode()->setColor(farbe[k]);
             // cerr << "  " << tmpT->getName() << endl;
             clusterList[k].append(tmpT);
             if(tmpT->deg() >= newMaxDeg)
              newMaxDeg = tmpT->deg();
            }
          }
         
         const FFSet<FFEdge>& tmpPred = tmpN->transFromPred();
         for(Pix j=tmpPred.first(); j!= 0; tmpPred.next(j))
          {
           FFNode tmpS = tmpPred(j)->source();
           
           if(tmpS->getValue() == -1)
            {
             tmpS->setValue(k);
             tmpS->getGraphicNode()->setColor(farbe[k]);
             // cerr << "  " << tmpS->getName() << endl;
             
             clusterList[k].append(tmpS);
             if(tmpS->deg() >= newMaxDeg)
              newMaxDeg = tmpS->deg();
            }
          }
        }
       else
        {
         // cerr << " no " << endl;
        }
      }
     if(graph->getWindow())
      graph->getWindow()->update();
    }
   
   for(int k=0; k< numClusters; k++)
    {
     // cerr << "Finding new top ten" << endl;
     
     if(topTen[k] == NULL)
      {
       Pix maxElement = 0;
       
       for (Pix j=clusterList[k].first(); j!= 0; clusterList[k].next(j))
        {
         if((maxElement == NULL) || 
            (clusterList[k](maxElement)->deg() < clusterList[k](j)->deg()))
          maxElement = j;
        }
       if(maxElement != NULL)
        {
         FFNode tmpN = clusterList[k](maxElement);
         clusterList[k].del(maxElement);
         topTen[k] = tmpN;
         if(tmpN->deg() >= newMaxDeg)
          newMaxDeg = tmpN->deg();
         // cerr << "Found " << tmpN->getName() << endl;
        }
      }
    }
   maxDeg = newMaxDeg;
  }
 
 if(clusterSize != NULL)
  delete [] clusterSize;
 
 clusterSize = new int[numClusters+1];

 for(int i=0; i<=numClusters; i++)
  clusterSize[i]=0;
 
 for( Pix iter = tmpS.first(); iter!=0; tmpS.next(iter))
  {
   FFNode tmpN= tmpS(iter);
   clusterSize[tmpN->getValue()]++;
  }

 for(int i=0; i<=numClusters; i++)
  if(clusterSize[i] != 0)
   cout << "Cluster " << farbe[i] << " has size " << clusterSize[i] << endl;

 cout << endl << "Doing local opitmization" << endl << endl;
 
 doLocalOptimize(); // uses ClusterSize

 if(graph->getWindow())
  graph->getWindow()->update();

//   thinGraph();
 graph->setEventStatus(tmpStatus);
 //  graph->getWindow()->update();

//  placeNodes();
//  graph->getWindow()->update();
  
 for(int i=0; i<=numClusters; i++)
  clusterSize[i]=0;
 
 for( Pix iter = tmpS.first(); iter!=0; tmpS.next(iter))
  {
   FFNode tmpN= tmpS(iter);
   clusterSize[tmpN->getValue()]++;
  }

 for(int i=0; i<=numClusters; i++)
  if(clusterSize[i] != 0)
   cout << "Cluster " << farbe[i] << " has size " << clusterSize[i] << endl;

 const FFSet<FFEdge>& tmpSE = graph->allEdges();
 
 int numCuts =0;
 for(Pix i=tmpSE.first(); i!= 0; tmpSE.next(i))
  {
   FFEdge tmpE= tmpSE(i);
   if(tmpE->target()->getValue() != 
      tmpE->source()->getValue())
    {
     numCuts++;
     tmpE->setWeight(0.5);
    }
   else
     tmpE->setWeight(3.0);
    
  }
 
 cout << endl << "Number of cuts: " << numCuts << endl;
 
 delete [] clusterSize;
 clusterSize = 0;
 delete [] histogram;
 delete [] clusterList;
 delete [] topTen;
}


void FFKaese::doLocalOptimize()
{
 const FFSet<FFNode>& tmpS= graph->allNodes();
 BOOL didChange = YES;

 // To guarantee Termination we make it more unlikely that a node
 // changes the cluster if it already has done so
 
 AVLMap<FFNode, int> numOfSwitches(0);
 
 while(didChange == YES)
  {
   didChange = NO;
   
   for(Pix i=tmpS.first(); i!=0; tmpS.next(i))
    {
     FFNode tmpN= tmpS(i);
     int tmpI = mostCommonNeighbours(tmpN, numOfSwitches[tmpN]);
     
     if(tmpN->getValue() != tmpI)
      {
       didChange = YES;
       int oldValue = tmpN->getValue();
       numOfSwitches[tmpN] = numOfSwitches[tmpN] +1;
//        else
//         {
//          cerr << "Switching node " << tmpN->getName() << " from cluster " 
//               << farbe[oldValue] << " to cluster "
//               << farbe[tmpI] << endl;
       clusterSize[oldValue] --;
       clusterSize[tmpI] ++;
       
       tmpN->setValue(tmpI);
       tmpN->getGraphicNode()->setColor(farbe[tmpI]);
       //        }
       
       if(tmpN == nucleus[oldValue])
        {
         if(clusterSize[oldValue] == 0)
          nucleus[oldValue]= 0;
         else
          {
           const FFSet<FFNode>& tmpS= graph->allNodes();
           for(Pix i=tmpS.first(); i!= 0; tmpS.next(i))
            {
             if(tmpS(i)->getValue() == oldValue)
              {
               nucleus[oldValue] = tmpS(i);
               break;
              }
            }
          }
         //         mergeClusters(tmpI, oldValue);
        }
       if(graph->getWindow())
        graph->getWindow()->update();
      }
    }
  }
}

int FFKaese::mostCommonNeighbours(FFNode knoten, int numOfSwitches)
{
 float *neighbours = new float[numClusters];
 int maxN =0;
 
 for(int i=0; i<numClusters; i++)
  neighbours[i]=0;

 const FFSet<FFEdge>& tmpSucc = knoten->transToSucc();
 for(Pix j=tmpSucc.first(); j!= 0; tmpSucc.next(j))
  {
   FFNode tmpT = tmpSucc(j)->target();
   neighbours[tmpT->getValue()]++;
  }
         
 const FFSet<FFEdge>& tmpPred = knoten->transFromPred();
 for(Pix j=tmpPred.first(); j!= 0; tmpPred.next(j))
  {
   FFNode tmpS = tmpPred(j)->source();
   neighbours[tmpS->getValue()]++;
  }
 
 int tmpVal = knoten->getValue();
 maxN = tmpVal;
 int sollWert = graph->getNumNodes()/numClusters;

#define abweichung(xxx) abs(sollWert - clusterSize[xxx])
 
 if(neighbours[tmpVal] > 1)
  neighbours[tmpVal] += 1;
 
 if(clusterSize[tmpVal] < sollWert/10)
  neighbours[tmpVal]=0;
 
 // neighbours[tmpVal] += numOfSwitches;
 
 for(int i=0; i<numClusters; i++)
  if((i != tmpVal) && (clusterSize[i] > sollWert/10) &&
     (neighbours[i] > 0.0f))
  {
   // These counts are non-negative. Compare products instead of ratios
   // so zero neighbour counts never create NaNs or infinities.
   if( (neighbours[maxN] <= 0.75f * neighbours[i]) ||
       ( (neighbours[maxN] > 0.0f) &&
         (neighbours[i] >= 0.75f * neighbours[maxN]) &&
         (abweichung(i) + numOfSwitches < abweichung(maxN))))
    maxN = i;
  }
 
 delete [] neighbours;
 return maxN;
}

void FFKaese::mergeClusters(int c1, int c2)
{
 const FFSet<FFNode>& tmpS= graph->allNodes();
 
 for(Pix i=tmpS.first(); i!= 0; tmpS.next(i))
  {
   FFNode tmpN=tmpS(i);
   if(tmpN->getValue() == c2)
    {
     tmpN->setValue(c1);
     tmpN->getGraphicNode()->setColor(farbe[c1]);
    }
  }
}

