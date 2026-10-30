#include <string.h>

#include <ffgraph/FFOrtho.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFLayoutLabel.h>
#include <ffgraph/FFTIEdgeLabel.h>
#include <ffgraph/FFOrthoInsp.h>
#include <ffgraph/FFTIGraphLabel.h>
#include <ffgraph/FFGraphicEdge.h>
#include <container/VOHSet.h>
#include <container/AVLSet.h>

#define FF_RAND 5

#define FF_WIN_UPD(a);   if (graph->getWindow ()) \
{\
 graph->getWindow()->update();\
 sleep(a);\
}

FFOrtho::FFOrtho ()
{
 xOffSet = ORTHO_X_OFFSET;
 yOffSet = ORTHO_Y_OFFSET;
 nodes = 0;
 count = 0;
 anzX = 0;
 anzY = 0;
 threeD = NO;
 anim = YES;
 doSwapping = NO;
 doSwapBadies = NO;
 doEdgeAdjustment = YES;
 initPlacement = "adjust";
 
 _setType(ffOrthoType);
}

void FFOrtho::init(const FFGraph& g)
{ 
 FFLayoutLabel::init(g);
 inspector= new FFOrthoInsp(this);
}

FFCoord distSum(const FFNode node, const FFPoint& pos, 
                const FFNode partner = NULL,
                const FFPoint& partnerPos = FFPoint(0,0))
{
 FFGraphicNode *gn = node->getGraphicNode();
 
 if(gn == NULL)
  return HUGE_VAL;
 
 const FFSet<FFEdge>& edgeTo = node->transToSucc();
 const FFSet<FFEdge>& edgeFrom = node->transFromPred();
 FFCoord dist= 0.0;
 
 for(Pix i = edgeTo.first(); i!= 0; edgeTo.next(i))
  {
   FFNode tmpN = edgeTo(i)->target();
   if(tmpN != node) // Reflexive Kanten nicht mitzaehlen !
    {
     if(tmpN != partner)
      {
       FFGraphicNode *tmpGN = tmpN->getGraphicNode();
       
       if(tmpN != NULL)
        {
         dist += (pos - tmpGN->getCoords()).absabs2D();
         // dist += pos.brieftraeger(tmpGN->getCoords());
        }
      }
     else
      {
       dist += (pos - partnerPos).absabs2D();
       // dist += pos.brieftraeger(partnerPos);
      }
    }
  }
 
 for(Pix i = edgeFrom.first(); i!= 0; edgeFrom.next(i))
  {
   FFNode tmpN = edgeFrom(i)->source();
   if(tmpN != node) // Reflexive Kanten nicht mitzaehlen !
    {
     if(tmpN != partner)
      {
       FFGraphicNode *tmpGN = tmpN->getGraphicNode();
       
       if(tmpN != NULL)
        {
         dist += (pos - tmpGN->getCoords()).absabs2D();
         // dist += pos.brieftraeger(tmpGN->getCoords());
        }
      }
     else
      {
       dist += (pos - partnerPos).absabs2D();
       // dist += pos.brieftraeger(partnerPos);
      }
    }
  }

 if(dist == 0.0)
  return HUGE_VAL;
 

 // Tie Breaker
 // 
 // Ein freies Feld nach rechts oben wird immer genommen
 //
 
 if(partner == NULL) 
  if(pos.v[0] > gn->getCoords().v[0])
   dist --;
  else if(pos.v[1] > gn->getCoords().v[1])
   dist --;
 
 return dist;
}

inline FFCoord distSum(const FFNode node)
{
 FFGraphicNode *gn= node->getGraphicNode();
 if(gn != NULL)
  return(distSum(node, gn->getCoords()));
 return HUGE_VAL;
}

int ffMinArray(const FFCoord tmpA[], int num)
{
 if(num > 0)
  {
   int result = 0;
   for(int k=1; k< num; k++)
    if(tmpA[k] < tmpA[result])
     result= k;

   return result;
  }
 else
  return -1;
}

// Rueckgabewert: Eine prinzipiell gute Position, deren Swappartner
//                aber zu teuer ist. Ziel -> neue Suche in dessen
//                Umgebung.

int FFOrtho::testWunschPos(FFNode knoten, FFCoord schwellWert, int bad)
{
 int myPos = knoten->getValue();
 FFPoint istP = knoten->getGraphicNode()->getCoords();
 FFCoord oldValue1 = knoten->getCoord();
 
 int result = -1;

#if(FF_WSIZE >= 5)
 wunschPos[1] = wunschPos[0] - anzX;     // ber Wunschposition
 wunschPos[2] = wunschPos[0] - 1;        // links von Wunschposition
 wunschPos[3] = wunschPos[0] + 1;        // rechts von Wunschposition
 wunschPos[4] = wunschPos[0] + anzX;     // unter Wunschposition
#endif

#if(FF_WSIZE >= 9)
 wunschPos[5] = wunschPos[1] - 1;   // Jetzt mit der 3x3 Matrix um
 wunschPos[6] = wunschPos[1] + 1;   // den Punkt
 wunschPos[7] = wunschPos[4] - 1;
 wunschPos[8] = wunschPos[4] + 1;
#endif
  
 for(int k=0; k< FF_WSIZE ; k++)
  {
   
   if((wunschPos[k] <= count) && (wunschPos[k] >= 0) && 
      (wunschPos[k] != myPos) && (wunschPos[k] != bad))
    { 
     if (nodes[wunschPos[k]] != NULL)
      {
       wunschP[k] = nodes[wunschPos[k]]->getGraphicNode()->getCoords();
       
       FFCoord newValue1 = distSum(knoten, wunschP[k], nodes[wunschPos[k]], 
                                   istP);
       
       FFCoord oldValue2 = nodes[wunschPos[k]]->getCoord();
       
       FFCoord newValue2 = distSum(nodes[wunschPos[k]], istP, knoten, 
                                   wunschP[k]);
       distArray[k] = (newValue1 + newValue2) -(oldValue1 + oldValue2);
       
       if((distArray[k] >= 0) && (k>0) &&
          (((distSum(knoten, wunschP[k])+xOffSet) - oldValue1) < -schwellWert))
        {
         //        cerr << "Fand einen potentielle neue WunschPosition: " << k 
         //             << " mit alt: " << oldValue1 << ", neu: " 
         //             << distSum(knoten, wunschP[k])+xOffSet << endl;
         result = k;
        }
      }
     else
      {
       wunschP[k]= FFPoint(((int)(wunschPos[k] % anzX)) * xOffSet,
                           ((int)(wunschPos[k] / anzX)) * yOffSet);
       FFCoord newValue1 = distSum(knoten, wunschP[k]);
       distArray[k]= newValue1 - oldValue1;
      }
    }
   else
    distArray[k]= HUGE_VAL;
  }
 return result;
}

void _updateDists(FFNode node)
{
 node->setCoord(distSum(node));
 const FFSet<FFEdge>& edgeS1 = node->transToSucc();
 
 for(Pix j=edgeS1.first(); j!= 0; edgeS1.next(j))
  {
   FFNode tmpN=edgeS1(j)->target();
   tmpN->setCoord(distSum(tmpN));
  }
 
 const FFSet<FFEdge>& edgeS2 = node->transFromPred();
 for(Pix j=edgeS2.first(); j!= 0; edgeS2.next(j))
  {
   FFNode tmpN=edgeS2(j)->source();
   tmpN->setCoord(distSum(tmpN));
  }
}

//
// eins: Erster Knoten
// zwei: Zweiter Knoten
// dest1: Ziel fuer Knoten 1, falls am Platz von Knoten 2 kein Knoten existiert
//
// Rueckgabe: YES wenn wirklich eine Verbesserung eingtreten ist. Da beim
//            abstand links - oben bevorzugt wird terminiert der Algorithmus
//            sonst nicht.
//

BOOL FFOrtho::_doSwap(int eins, int zwei, FFPoint dest1)
{
 FFNode node1 = nodes[eins];
 FFNode node2 = nodes[zwei];

 FFGraphicNode *tmpGN1 = node1->getGraphicNode();
 const FFPoint& p1= tmpGN1->getCoords();
 
 if(node2 != NULL)
  {
   FFGraphicNode *tmpGN2 = node2->getGraphicNode();
   tmpGN2->moveTo(p1);
   
   nodes[eins] = node2;
   node2->setValue(eins);
   
//    cerr << "Moving " << zwei << " to " << p1() << endl;
   
  }
 else
  {
   //          cerr << "Breche aus Gitter aus !!!" << endl;
   nodes[eins] = 0;
  }
       
 tmpGN1->moveTo(dest1);
 nodes[zwei] = node1;
 nodes[zwei]->setValue(zwei);

//  cerr << "Moving " << eins << " to " << dest1() << endl;

 // Neue Distanzen anpassen
 FFCoord oldDist = node1->getCoord();
 
 _updateDists(node1);
 
 if(node2 != NULL)
  {
   _updateDists(node2);
   return YES;
  }
 
 if(oldDist > node1->getCoord())
  return YES;
 
 return NO;
}

static int ffOrthoCmp(const FFNode *n1, const FFNode *n2)
{
 return ( (((*n1)->getCoord()) >= ((*n2)->getCoord())) ? 
          ((((*n1)->getCoord()) == ((*n2)->getCoord()))? 0 : 
           -1) : 
          1 );
}

BOOL FFOrtho::swapBadies()
{
 BOOL didSwap=NO;
 int anzNodes = graph->getNumNodes();
 FFNode *knoten = new FFNode[anzNodes];
 
 const FFSet<FFNode> knotenS = graph->allNodes();
 int k=0;
 
 for(Pix i=knotenS.first(); i!= 0; knotenS.next(i))
  {
   knoten[k++] = knotenS(i);
  }
 
 qsort(knoten, anzNodes, sizeof(FFNodeClass *), 
       (int (*)(const void *, const void *)) ffOrthoCmp);

//  int minSwap= std::min(100, anzNodes);
//  int topTen= std::max(anzNodes/2, minSwap);
 int topTen = anzNodes;
 
 for(k=0;k < topTen; k++)
  {
   FFCoord oldValue1 = knoten[k]->getCoord();
   FFGraphicNode *gn1 = knoten[k]->getGraphicNode();
   
   if(gn1 != NULL)
    for(int j= k+1; j<topTen; j++)
     {
      FFGraphicNode *gn2 = knoten[j]->getGraphicNode();
      if(gn2 != NULL)
       {
        FFCoord oldValue2 = knoten[j]->getCoord();
        
        FFCoord newValue1 = distSum(knoten[k], gn2->getCoords(), 
                                    knoten[j], gn1->getCoords());
        
        FFCoord newValue2 = distSum(knoten[j], gn1->getCoords(), 
                                    knoten[k], gn2->getCoords());
        if( ((newValue1 + newValue2) -(oldValue1 + oldValue2)) < 0)
         {
          // cerr << "Knoten " << knoten[k]->getName() << " von " 
          // << knoten[k]->getValue() << " nach " << knoten[j]->getValue() 
          // << endl;
          
          didSwap |= _doSwap(knoten[k]->getValue(), knoten[j]->getValue(), 
                             gn2->getCoords());
          oldValue1 = knoten[k]->getCoord();
          gn1 = knoten[k]->getGraphicNode();
         }
       }
     }
  }
 
 delete [] knoten;
 knoten = 0;
 
 return didSwap;
}

BOOL FFOrtho::swapNodes()
{
 BOOL didSwap = NO;
 
 const FFSet<FFNode> nodeS = graph->allNodes();
 for(Pix i=nodeS.first(); i!= 0; nodeS.next(i))
  {
   FFPoint med(0,0);
   FFNode tmpN= nodeS(i);
   FFGraphicNode *tmpGN = tmpN->getGraphicNode();
   
   if(tmpGN != NULL && ((tmpN->indeg() + tmpN->outdeg()) != 0 ))
    {
     const FFSet<FFEdge>& edgeS1 = tmpN->transToSucc();

     for(Pix j=edgeS1.first(); j!= 0; edgeS1.next(j))
      med += edgeS1(j)->target()->getGraphicNode()->getCoords();
     
     const FFSet<FFEdge>& edgeS2 = tmpN->transFromPred();
     for(Pix j=edgeS2.first(); j!= 0; edgeS2.next(j))
      med += edgeS2(j)->source()->getGraphicNode()->getCoords();
     
     med = med / (tmpN->indeg() + tmpN->outdeg());

     int myPos = tmpN->getValue();
     
     FFPoint istP = tmpGN->getCoords();
     
     int xFak = (int) ((med.v[0]+(xOffSet / 2)) / xOffSet);
     int yFak = (int) ((med.v[1]+(yOffSet / 2)) / yOffSet);

     wunschPos[0] = yFak * anzX + xFak; // an Wunschpoisition

     FFCoord schwellWert = FF_RES_VAL;
     int rerun = testWunschPos(tmpN, schwellWert);
     int mini = ffMinArray(distArray, FF_WSIZE);
     
     while((distArray[mini] > 0.0) && (rerun > 0))
      {
//        cerr << "Ich starte einen neuen Versuch" << endl;
       int ignoreValue= wunschPos[0];
       wunschPos[0] = wunschPos[rerun];
       rerun = testWunschPos(tmpN, schwellWert, ignoreValue);
       mini = ffMinArray(distArray, FF_WSIZE);
       schwellWert += 100.0;
      }
//      cerr << endl;
     
       
     if(distArray[mini] < 0.0)
      {
//        cerr << "Knoten " << tmpN->getName() << " von " << myPos << " nach " 
//             << wunschPos[mini] << endl;
       didSwap |= _doSwap(myPos, wunschPos[mini], wunschP[mini]);
//        FF_WIN_UPD();
      }
     else
      {
      }
    }
  }
 return didSwap;
 
}

void FFOrtho::_printEdgeSum(ostream &strom)
{
 FFCoord edgeSum = 0.0;
 const FFSet<FFNode>& tmpNS=graph->allNodes();
 
 for(Pix i= tmpNS.first(); i != 0; tmpNS.next(i))
  edgeSum += tmpNS(i)->getCoord();
 strom << edgeSum/2;
}

FFCoord FFOrtho::_getEdgeSum()
{
 FFCoord edgeSum = 0.0;
 const FFSet<FFNode>& tmpNS=graph->allNodes();
 
 for(Pix i= tmpNS.first(); i != 0; tmpNS.next(i))
  edgeSum += tmpNS(i)->getCoord();

 return edgeSum/2;
}

#define FF_PES(a);  cerr << a;\
 _printEdgeSum(cerr);\
 cerr << endl;


BOOL FFOrtho::_adjustPlacement()
{
 int randOff = FF_RAND;
 int numNodes = graph->getNumNodes();
 const FFSet<FFNode>& tmpNS = graph->allNodes();
 BOOL returnVal = YES;
 
 // Knoten normieren d.h. an minimale Koordinaten verschieben
 FFCoord minX= HUGE_VAL;
 FFCoord minY= HUGE_VAL;

 FFCoord maxX= -HUGE_VAL;
 FFCoord maxY= -HUGE_VAL;
 
 for(Pix i=tmpNS.first(); i!= 0; tmpNS.next(i))
  {
   FFNode tmpN=tmpNS(i);
   FFGraphicNode *tmpGN= tmpN->getGraphicNode();
   if(tmpGN != NULL)
    {
     const FFPoint& tmpP = tmpGN->getCoords();
     minX = std::min(minX, tmpP.v[0]);
     minY = std::min(minY, tmpP.v[1]);
     maxX = std::max(maxX, tmpP.v[0]);
     maxY = std::max(maxY, tmpP.v[1]);
    }
  }
 
 if( (maxX < minX) || (maxY < minY))
  {
   cerr << "There is something wrong" << endl;
   return NO;
  }
 
 
 FFCoord xShift = xOffSet * randOff - minX;
 FFCoord yShift = yOffSet * randOff - minY;

 maxX += xShift;
 maxY += yShift;
 
 anzX = (int)((maxX+xOffSet/2) / xOffSet) + randOff + 1;
 anzY = (int)((maxY+yOffSet/2) / yOffSet) + randOff + 1;
 count = anzX * anzY;
 
 cerr << "Breite: " << anzX << " Lnge: " << count/anzX << endl;

 nodes = new FFNode[count];
 for(int k=0; k< count; k++)
  nodes[k]= 0;

 for(Pix i=tmpNS.first(); i!= 0; tmpNS.next(i)) {
  FFNode tmpN=tmpNS(i);
  FFGraphicNode *tmpGN= tmpN->getGraphicNode();
  if(tmpGN != NULL) {
   FFPoint tmpP = tmpGN->getCoords();
   tmpP += FFPoint(xShift, yShift);
   int tmpX = (int)((tmpP.v[0]+xOffSet/2) / xOffSet);
   int tmpY = (int)((tmpP.v[1]+yOffSet/2) / yOffSet);
   
   wunschPos[0] = tmpY*anzX + tmpX;
     
#if(FF_WSIZE >= 5)
   wunschPos[1] = wunschPos[0] - anzX;     // ber Wunschposition
   wunschPos[2] = wunschPos[0] - 1;        // links von Wunschposition
   wunschPos[3] = wunschPos[0] + 1;        // rechts von Wunschposition
   wunschPos[4] = wunschPos[0] + anzX;     // unter Wunschposition
#endif

#if(FF_WSIZE >= 9)
   wunschPos[5] = wunschPos[1] - 1;   // Jetzt mit der 3x3 Matrix um
   wunschPos[6] = wunschPos[1] + 1;   // den Punkt
   wunschPos[7] = wunschPos[4] - 1;
   wunschPos[8] = wunschPos[4] + 1;
#endif
   int j=0;
   
   for(; j< FF_WSIZE ; j++) {
    if((wunschPos[j] <= count) && (wunschPos[j] >= 0) &&
       (nodes[wunschPos[j]] == NULL)) {
     nodes[wunschPos[j]] = tmpN;
     tmpN->setValue(wunschPos[j]);

     tmpP = FFPoint( (wunschPos[j] % anzX) * xOffSet, 
                     (wunschPos[j] / anzX) * yOffSet);
     break;
    }
   }
     
   if(j == FF_WSIZE) {
    returnVal = NO;
   }

   tmpGN->moveTo(tmpP);
  }
 }
 return returnVal;
}

void FFOrtho::_randomPlacement()
{
 int placeFak = 2;  // Wieviel Raum soll zwischen 2 Knoten sein
 int randOff = FF_RAND;
 int numNodes = graph->getNumNodes();
 const FFSet<FFNode>& tmpNS = graph->allNodes();
 
 anzX = (int)ceil(sqrt(numNodes));
 anzY = (int)ceil( ((float)numNodes)/anzX);

 anzX = (anzX -1) * placeFak +1;
 anzY = (anzY -1) * placeFak +1;
             
 anzX += 2 * randOff;
 anzY += 2 * randOff;
 
 count = anzX * anzY;
 
 cerr << "Breite: " << anzX << " Lnge: " << count/anzX << endl;
   
 nodes = new FFNode [count];

 for(int k=0; k< count; k++)
  nodes[k]= 0;
 
 // Startpositionen der Knoten setzen
 
 int tmpPos = randOff*anzX + randOff;
 FFCoord tmpMaxX = (anzX - (randOff+1)) * xOffSet;
 cerr << "Max X: " << tmpMaxX << endl;
   
 for(Pix i= tmpNS.first(); i != 0; tmpNS.next(i))
  {
   FFNode tmpN = tmpNS(i);
     
   if(tmpN->getGraphicNode())
    {
     nodes[tmpPos] = tmpN;
     tmpN->setValue(tmpPos);
     tmpN->getGraphicNode()->moveTo(FFPoint(aktuX, aktuY));

     aktuX += placeFak * xOffSet;
     tmpPos += placeFak;
     
     if(aktuX > tmpMaxX)
      {
       aktuX = randOff*xOffSet;
       aktuY += placeFak * yOffSet;
       tmpPos += (2 * randOff) - 1 ;
       tmpPos += (placeFak -1) * anzX;
      }
    }
  }
}


void FFOrtho::layout ()
{
 int randOff = FF_RAND;
 
 aktuX = randOff*xOffSet;
 aktuY = randOff*yOffSet;
 count = 0;
 
 BOOL tmpUpdate=YES;
 if (graph->getWindow ())
  {
   graph->getWindow()->setBusy ();
   tmpUpdate = graph->getWindow()->setUpdate(NO);
  }
 
 const FFSet<FFNode>& tmpNS = graph->allNodes();
 graph->straightenEdges();

 if(initPlacement == "random")
  _randomPlacement();
 else 
  while(_adjustPlacement() == NO)
   {
    cerr << "Reducing grid space and retry" << endl;
    setxoff(getxoff() /2);
    setyoff(getyoff() /2);
    if(inspector != NULL)
     inspector->inspect();
   } 
 
 // Initialisiere Entfernungen
//  FF_WIN_UPD();
 
 for(Pix i= tmpNS.first(); i != 0; tmpNS.next(i))
  {
   FFNode tmpN = tmpNS(i);
   if(tmpN->getGraphicNode() != NULL)
    tmpN->setCoord(distSum(tmpN));
   else
    tmpN->setCoord(HUGE_VAL);
  }
 
 FFCoord oldSum = _getEdgeSum()/2;

 FF_PES("Nach Knoten positionieren: ");

// Versuche Knoten an optimale Positionen zu setzen

 if(doSwapping)
  {
   while(swapNodes())
    {
     if(anim)
      {
      FF_PES("Nach Swappen: ");
      
      if (graph->getWindow ())
       graph->getWindow()->update();
      
      //      FF_WIN_UPD(3);
      }
   }
   
   FFCoord newSum = _getEdgeSum();
   cerr << "Swapping reduced edge length to " << (newSum/oldSum)*100 << "%."
        << endl;
  }
 
 if(doSwapBadies)
  {
   while(swapBadies())
    {
     if(anim)
      {
       FF_PES("Nach Badies: ");
       
       if (graph->getWindow ())
        graph->getWindow()->update();
      }
    }
 
   FFCoord newSum = _getEdgeSum();
   cerr << "Swapping badies reduced edge length to " << (newSum/oldSum)*100 
        << "%." << endl;
  }
 
 // Kantenpositionen
 if(doEdgeAdjustment)
  {
   if(get3D())
    _3DEdgePos();
   else
    _2DEdgePos();
  }
 
 if (graph->getWindow ())
  {
   graph->getWindow()->unsetBusy ();
   graph->getWindow()->setUpdate(tmpUpdate);
   graph->getWindow()->adjustWindowSize();
  }
 cerr << "Ready" << endl;
 
 delete [] nodes;
 nodes = 0;
}

void FFOrtho::_3DEdgePos()
{
 int randOff = FF_RAND;
 const FFSet<FFNode>& tmpNS= graph->allNodes();
 
 cerr << "Doing 3D edge adjustment" << endl;
 
 for(Pix i= tmpNS.first(); i != 0; tmpNS.next(i))
  {
   const FFSet<FFEdge>& tmpES = tmpNS(i)->transToSucc();
   for(Pix j= tmpES.first(); j!= 0; tmpES.next(j))
    {
     FFEdge tmpEdge = tmpES(j);
     FFNode tmpSource = tmpNS(i);
     FFNode tmpTarget = tmpEdge->target();
     FFGraphicNode *tmpGS = tmpSource->getGraphicNode();
     FFGraphicNode *tmpGT = tmpTarget->getGraphicNode();
     FFGraphicEdge *tmpGE = tmpEdge->getGraphicEdge();
     
     if(tmpGS && tmpGT && tmpGE)
      {
       FFPoint srcP = tmpGS->getCoords();
       FFPoint tgtP = tmpGT->getCoords();
       
       FFCoord z =  sqrt(( srcP.v[0] * srcP.v[1] + tgtP.v[0] * tgtP.v[1] ) /2);
       
       FFPoint tmpP1 = srcP + FFPoint(0,0,z);
       FFPoint tmpP3 = tgtP + FFPoint(0,0,z);
       FFPoint tmpP2(tmpP3.v[0], tmpP1.v[1], z);
         
       tmpGE->addPointAt(1,tmpP1);
       tmpGE->addPointAt(2,tmpP2);
       tmpGE->addPointAt(3,tmpP3);
      }
    }
  }
}

BOOL FFOrtho::_testNeighbor(const FFPoint& p1, const FFPoint& p2)
{
//  int row1 = pos1 / anzX;
//  int row2 = pos2 / anzX;
 
//  int col1 = pos1 % anzX;
//  int col2 = pos2 % anzX;

 int row1 = (int)(p1.v[1] / yOffSet);
 int row2 = (int)(p2.v[1] / yOffSet);
 
 int col1 = (int)(p1.v[0] / xOffSet);
 int col2 = (int)(p2.v[0] / xOffSet);
 
//  cerr << "Testing " << p1()<< " (" << row1 << "," << col1 << ")" << endl;
//  cerr << " to     " << p2()<< " (" << row2 << "," << col2 << ")" << endl;
 
 if(col1 == col2)
  {
   int start = std::min(row1, row2)*anzX+col1;
   int ziel =  std::max(row1, row2)*anzX+col1;
   BOOL frei=YES;
   
   for(int k=start+anzX; k < ziel; k+=anzX)
    {
//      cerr << " ... pos: " << k << endl;
     
     if((k<count) && (nodes[k] != NULL))
      frei = NO;
    }
   
   return frei;
  }
 else if(row1 == row2)
  {
   int start = std::min(col1, col2)+ row1*anzX;
   int ziel = std::max(col1, col2) + row2*anzX;
   BOOL frei=YES;
   
   for(int k=start+1; k < ziel; k++)
    {
//      cerr << " ... pos: " << k << endl;
     if((k<count) && (nodes[k] != NULL))
      frei = NO;
    }
   return frei;
  }

 return NO;
}

void FFOrtho::_2DEdgePos()
{
 int randOff = FF_RAND;
 int k=0;

 struct inOutStruct inOut[graph->getNumNodes()];
 memset(inOut, 0, sizeof(inOutStruct) * graph->getNumNodes());
 
 FFSet<FFEdge> nachbarn;

 const FFSet<FFNode>& tmpNS= graph->allNodes();
 
 for(Pix i= tmpNS.first(); i != 0; tmpNS.next(i))
  {
   tmpNS(i)->setValue(k++);
  }
 
 for(Pix i= tmpNS.first(); i != 0; tmpNS.next(i))
  {
   FFNode tmpSource = tmpNS(i);
   FFGraphicNode *tmpGS = tmpSource->getGraphicNode();
   
   const FFSet<FFEdge>& tmpES = tmpNS(i)->transToSucc();
   for(Pix j= tmpES.first(); j!= 0; tmpES.next(j))
    {
     FFEdge tmpEdge = tmpES(j);
     FFNode tmpTarget = tmpEdge->target();
     FFGraphicNode *tmpGT = tmpTarget->getGraphicNode();
     FFGraphicEdge *tmpGE = tmpEdge->getGraphicEdge();
     
     if(tmpGS && tmpGT && tmpGE)
      {
       FFPoint srcP = tmpGS->getCoords();
       FFPoint tgtP = tmpGT->getCoords();
       
       if(_testNeighbor(srcP, tgtP))
        {
         nachbarn.add(tmpEdge);
//          cerr << tmpSource->getName() << " und " << tmpTarget->getName()
//               << " sind Nachbarn." << endl;
        }
       else
        {
         _adjustEdge(tmpGS, tmpGT, inOut, tmpGE);
//          cerr << tmpSource->getName() << " und " << tmpTarget->getName()
//               << " sind keine Nachbarn." << endl;
        }
      }
    }
  }
 
 BOOL repeat=NO;
 FFSet<FFEdge> delSet;
 
 do
  {
   for(Pix i=delSet.first(); i!= 0; delSet.next(i))
    {
     nachbarn.del(delSet(i));
    }
   delSet.clear();
   repeat=NO;
   
   for(Pix i=nachbarn.first(); i!= 0; nachbarn.next(i))
    {
     FFEdge tmpEdge = nachbarn(i);
     
     FFNode tmpTN = tmpEdge->target();
     FFPoint tmpTP = tmpTN->getGraphicNode()->getCoords();
     
     FFNode tmpSN = tmpEdge->source();
     FFPoint tmpSP = tmpSN->getGraphicNode()->getCoords();
     
     if(((tmpTP.v[0] < tmpSP.v[0]) && (inOut[tmpSN->getValue()].l == NO)) ||
        ((tmpTP.v[0] > tmpSP.v[0]) && (inOut[tmpSN->getValue()].r == NO)) ||
        ((tmpTP.v[1] < tmpSP.v[1]) && (inOut[tmpTN->getValue()].o == NO)) ||
        ((tmpTP.v[1] > tmpSP.v[1]) && (inOut[tmpTN->getValue()].u == NO)))
      {
      }
     else
      {
       _adjustEdge(tmpSN->getGraphicNode(), tmpTN->getGraphicNode(),  
                   inOut, tmpEdge->getGraphicEdge());
       delSet.add(tmpEdge);
       repeat= YES;
      }
    }
  }
 while(repeat == YES);
 
}

void FFOrtho::_adjustEdge(FFGraphicNode *tmpGS, FFGraphicNode *tmpGT,
                          struct inOutStruct *inOut, 
                          FFGraphicEdge *tmpGE)
{
 int randOff = FF_RAND;
 const FFPoint& srcP = tmpGS->getCoords();
 const FFPoint& tgtP = tmpGT->getCoords();
 FFPoint tmpP1 = srcP;
 FFPoint tmpP3 = tgtP;
 FFPoint tmpP2(srcP.v[0], tgtP.v[1]);
 FFCoord tmpFakX = (xOffSet/2.0) / anzX;
 FFCoord tmpFakY = (yOffSet/2.0) / anzY;
 
 int tmpPos = (int)((tmpP2.v[1]/yOffSet)*anzX + tmpP2.v[0]/xOffSet);

 if((nodes[tmpPos] == NULL) && _testNeighbor(srcP, tmpP2) &&
    _testNeighbor(tgtP, tmpP2))
  {
   tmpGE->addPointAt(1,tmpP2);
  }
 else
  {
   if(srcP.v[1] > tgtP.v[1])
    {
     tmpP1.v[1] -= tmpGS->getRange().v[0] +
      (srcP.v[0] / xOffSet -randOff) * tmpFakX;
    }
   else
    {
     tmpP1.v[1] += tmpGS->getRange().v[0] +
      (srcP.v[0] / xOffSet -randOff) * tmpFakX;
    }
   if(tgtP.v[0] > srcP.v[0])
    {
     tmpP3.v[0] -= tmpGT->getRange().v[0] +
      (tgtP.v[1] / yOffSet -randOff) * tmpFakY;
    }
   else
    {
     tmpP3.v[0] +=  tmpGT->getRange().v[0] +
      (tgtP.v[1] / yOffSet -randOff) * tmpFakY;
    }
 
   tmpP2.v[0] = tmpP3.v[0]; 
   tmpP2.v[1] = tmpP1.v[1];
 
   tmpGE->addPointAt(1,tmpP1);
   tmpGE->addPointAt(2,tmpP2);
   tmpGE->addPointAt(3,tmpP3);
  }
 
 if(srcP.v[1] > tgtP.v[1])
  inOut[tmpGS->getNode()->getValue()].u = YES;
 else
  inOut[tmpGS->getNode()->getValue()].o = YES;
 
 if(tgtP.v[0] > srcP.v[0])
  inOut[tmpGT->getNode()->getValue()].l = YES;
 else
  inOut[tmpGT->getNode()->getValue()].r = YES;
}

