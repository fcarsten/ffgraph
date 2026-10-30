#include <string.h>
#include <assert.h>

#include <ffgraph/FFSpqr.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFLayoutLabel.h>
#include <ffgraph/FFTIEdgeLabel.h>
#include <ffgraph/FFTINodeLabel.h>
#include <ffgraph/FFTIGraphLabel.h>
#include <container/VOHSet.h>
#include <container/AVLSet.h>
#include <ffgraph/FFSpqrNodeLabel.h>
#include <ffgraph/FFSugi.h>

#define SPQR_DEBUG

FFSpqr::FFSpqr () : spqr(YES)
{
 _setType(ffSpqrType);
 spqrNodeIndex = spqr->addNodeLabel(new FFSpqrNodeLabel);
}


FFSpqr::~FFSpqr ()
{
 tiNodeIndex=-1;
 tiEdgeIndex=-1;
}

void FFSpqr::init(const FFGraph& g)
{ 
 FFLayoutLabel::init(g);
 tiEdgeIndex= graph->getEdgeLabelIndex(ffTIEdgeLabel);
 tiNodeIndex= graph->getNodeLabelIndex(ffTINodeLabel);
 inspector = 0;
#ifdef SPQR_DEBUG
 spqr->setName("SPQR Debug Window");
 spqr->createGraphic(new FFTclWindow);
 spqr->addGraphLabel(new FFSugi);
#endif
}

void FFSpqr::layout ()
{
 cerr << "Starting Layout" << endl;
 
 if((tiEdgeIndex= graph->getEdgeLabelIndex(ffTIEdgeLabel)) == -1)
  {
   graph->addGraphLabel(new FFTIGraphLabel);
   if((tiEdgeIndex= graph->getEdgeLabelIndex(ffTIEdgeLabel)) == -1)
    return;
  }

 if((tiNodeIndex= graph->getNodeLabelIndex(ffTINodeLabel)) == -1)
  return;
 
 spqr->clear();
 
 FFNode tNode = 0;
 FFNode sNode = 0;
 FFEdge stEdge = 0;
 
 FFSet<FFEdge> alleKanten;
 alleKanten |= graph->allEdges();
 int k=0;
 for(Pix i =alleKanten.first(); i!= 0; alleKanten.next(i))
  {
   alleKanten(i)->setValue(k++);
  }
 
 // Looking for s and t node

 const FFSet<FFNode>& alleKnoten = graph->allNodes();
 for(Pix i=alleKnoten.first(); i!= 0; alleKnoten.next(i))
  {
   sNode= alleKnoten(i);
   
   if(tiLabel(sNode)->getDfsNum() == 1)
    {
     const FFSet<FFEdge>& zu= sNode->transToSucc();

     for(Pix j=zu.first(); j!= 0; zu.next(j))
      {
       tNode = zu(j)->target();
       if(tiLabel(tNode)->getDfsNum() == 0)
        {
         alleKanten.del(zu(j));
         stEdge = zu(j);
         break;
        }
      }
     break;
    }
  }
 
 if((sNode == NULL) || (tNode == NULL))
  return;
 
 int maxST= tiLabel(tNode)->getSTNum();
 FFNode *done = new FFNode [maxST+1];
 propAllocNode = new FFNode [maxST+1];
 for(int ki=0; ki< maxST+1; ki++)
  propAllocNode[ki]= 0;
 
 // correspNode = new FFNode[graph->allEdges().length()];
 
 for(Pix i =alleKnoten.first(); i!= 0; alleKnoten.next(i))
  {
   FFNode tmpN= alleKnoten(i);
   tmpN->setValue(tiLabel(tmpN)->getSTNum());
   done[tmpN->getValue()]= tmpN;
  }

 FFNode firstN = addQNode(sNode, tNode);
 
 firstN->setName("s/t");
 // spqr->setStartNode(firstN);
 
 propAllocNode[tiLabel(tNode)->getSTNum()] = firstN;
 propAllocNode[tiLabel(sNode)->getSTNum()] = firstN;
 // correspNode[stEdge->getValue()] = firstN;
 
 done[tiLabel(tNode)->getSTNum()]= 0;
 done[tiLabel(sNode)->getSTNum()] = 0;
 int lowNotDone=1;
 
 while(1)
  {
   while((done[lowNotDone] == NULL) && (lowNotDone < maxST))
    lowNotDone ++;
   
   if(lowNotDone >= maxST)
    break;
   
   FFNode aktuN = done[lowNotDone];
   const FFSet<FFEdge>& von= aktuN->transFromPred();
   
   FFEdge fromFather= von(von.first());
   FFNode father = fromFather->source();
    
   alleKanten.del(fromFather);

   SLList<FFNode> pfad;
   FFNode sink= _markPath(done, alleKanten, aktuN, pfad);

   if(testEdge(father, sink))
    {
     FFNode tmpN =insertEdge(father, sink);
     if(tmpN != NULL)
      while(pfad.length() != 0)
       {
        tmpN = insertNode(pfad.front(), tmpN);
        pfad.remove_front();
       }
     else
      {
       cerr << "Couldn't insert Edge" << endl;
#ifdef SPQR_DEBUG
       fromFather->getGraphicEdge()->setColor("green");
#endif
       showOrder();
       return;
      }
    }
   else
    {
     cerr << "Graph is not planar" << endl;
     break;
    }
  }
 
 for(Pix i= alleKanten.first(); i != 0; alleKanten.next(i))
  {
   FFEdge tmpE=alleKanten(i);
   if((propAllocNode[tiLabel(tmpE->source())->getSTNum()] != NULL) &&
      (propAllocNode[tiLabel(tmpE->target())->getSTNum()] != NULL))
    {
     if(insertEdge(tmpE->source(), tmpE->target()) == NULL)
      {
#ifdef SPQR_DEBUG
       alleKanten(i)->getGraphicEdge()->setColor("green");
#endif
       showOrder();
       return;
      }
    }
  }

 showOrder();
 
 delete [] propAllocNode;
 // delete [] correspNode;
 delete [] done;
}


//
// !!! May only be called from FFSpqr::layout as tiNodeLabel and tiEdgeLabel
//     must be set correctly
//

FFNode FFSpqr::_markPath(FFNode done[], FFSet<FFEdge>& allE, 
                       FFNode startN, SLList<FFNode>& pfad)
{
 FFNode aktuN = startN;
 
 cerr << "Marking Path" << endl;
 
 while(done[tiLabel(aktuN)->getSTNum()] != NULL)
  {
   done[tiLabel(aktuN)->getSTNum()] = 0;
   pfad.append(aktuN);
   FFEdge tmpE= (aktuN->transToSucc())(aktuN->transToSucc().first());
   allE.del(tmpE);
   aktuN= tmpE->target();
  }
 return aktuN;
}

FFNode FFSpqr::addQNode(FFNode a, FFNode b)
{
 cerr << "Adding Q-Node" << endl;
 assert(a!=NULL);
 assert(b!=NULL); 
 FFNode firstN = spqr->addNode();
 spqrLabel(firstN)->setSpqrType(Q_NODE);
 spqrLabel(firstN)->setFirst(a);
 spqrLabel(firstN)->setSecond(b);
 return firstN;
}

FFNode FFSpqr::addSNode()
{
 cerr << "Adding S-Node" << endl;
 FFNode newS= spqr->addNode();
 newS->setName("S");
 spqrLabel(newS)->setSpqrType(S_NODE);
 
 return newS;
}

FFNode FFSpqr::addPNode()
{
 cerr << "Adding P-Node" << endl;

 FFNode newP= spqr->addNode();
 newP->setName("P");
 spqrLabel(newP)->setSpqrType(P_NODE);
 return newP;
}

FFNode FFSpqr::addRNode()
{
 cerr << "Adding R-Node" << endl;
 FFNode newR= spqr->addNode();
 newR->setName("R");
 spqrLabel(newR)->setSpqrType(R_NODE);
 return newR;
}

FFEdge FFSpqr::getFatherEdge(FFNode knoten)
{
 if(knoten->transFromPred().length() == 0)
  return NULL;
 return((knoten->transFromPred())(knoten->transFromPred().first()));
}


FFNode FFSpqr::_preliminaryTransformationS(FFNode my, FFNode lambda1)
{
 cerr << "Can't do preliminary S transformation" << endl;
 assert(0);
}

void FFSpqr::_collectSuccessors(FFNode n, FFSet<FFNode>& menge)
{
 cerr << "Collecting successors" << endl;
 
 if(spqrLabel(n)->getSpqrType() == Q_NODE)
  {
   menge.add(spqrLabel(n)->getFirst());
   menge.add(spqrLabel(n)->getSecond());   
  }
 
 const FFSet<FFEdge>& nach= n->transToSucc();
 for(Pix i=nach.first(); i!= 0; nach.next(i))
  {
   _collectSuccessors(nach(i)->target(), menge);
  }
}


FFNode FFSpqr::insertEdge(FFNode source, FFNode sink)
{
 cerr << "Inserting Edge" << endl;
 
 FFNode propSource= propAllocNode[tiLabel(source)->getSTNum()];
 FFNode propSink= propAllocNode[tiLabel(sink)->getSTNum()];
 FFNode lambda1, lambda2;

 assert(propSource != NULL);
 assert(propSink != NULL);

 FFNode my= leastCommonAncestor(propSource, propSink, lambda1, lambda2);

 SpqrType myType= spqrLabel(my)->getSpqrType();
 
 //
 // Wir fangen gerade an und haben nur einen Q-Knoten von s nach t
 //
 
 if(spqr->allNodes().length() == 1)
  {
   FFNode newP= addPNode();
   
   FFNode newQ= addQNode(spqrLabel(my)->getFirst(),
                         spqrLabel(my)->getSecond());
   
   addEdge(newP, my);
   addEdge(newP, newQ);
   
   propAllocNode[tiLabel(source)->getSTNum()]= newP;
   propAllocNode[tiLabel(sink)->getSTNum()]= newP;
   
   return newQ;
  }
 
 FFNode newQ = 0;
 
 if(tiLabel(source)->getSTNum() < tiLabel(sink)->getSTNum())
  newQ = addQNode(source, sink);
 else
  newQ = addQNode(sink, source);

 if(propSource == propSink)
  { // Kante zwischen 2 Knoten mit gleichem propAllocNode
   finalTransformation(propSource, newQ);
   return newQ;
  }
 else if(my == propSink)
  {
   FFNode chi = findLowestChi(propSink, propSource, sink);
   if(chi == propSource)
    finalTransformation(chi, newQ);
   else
    {
     return NULL;
     pathCondensation(propSource, lambda1);
     finalTransformation(chi, lambda2, newQ);
    }
   return newQ;
  }
 else if (my == propSource)
  {
   FFNode chi = findLowestChi(propSource, propSink, source);
   if(chi == propSink)
    finalTransformation(chi, newQ);
   else
    {
     return NULL;
     pathCondensation(propSink, lambda2);
     finalTransformation(chi, lambda1, newQ);
    }
   return newQ;
  }
 else
  {
   return NULL;
   pathCondensation(propSource, lambda1);
   pathCondensation(propSink, lambda2);
   finalTransformation(my, lambda1, lambda2, newQ);
  }
 return newQ;
}

void FFSpqr::pathCondensation(FFNode, FFNode)
{
 cerr << "Can't do path condensation" << endl;
 assert(0);
}

void FFSpqr::finalTransformation(FFNode, FFNode, FFNode, FFNode)
{
 cerr << "Can't do final transformation with 4 args" << endl; 
 assert(0);
}

void FFSpqr::finalTransformation(FFNode, FFNode, FFNode)
{
 cerr << "Can't do final transformation with 3 args" << endl;
 assert(0);
}

void FFSpqr::finalTransformation(FFNode chi, FFNode newQ)
{
 cerr << "Doing final transformation" << endl;
 
 SpqrType chiType= spqrLabel(chi)->getSpqrType();
 
 if(chiType == P_NODE)
  {
   addEdge(chi, newQ);
  }
 else if(chiType == S_NODE)
  {
   insertSP(chi, newQ);
  }
 else if(chiType == R_NODE)
  {
   cerr << "Can't handle case: same alloc node is R-Node" << endl;
   assert(0);
  }
}

FFNode FFSpqr::findLowestChi(FFNode top, FFNode bottom, 
                             FFNode node)
{
 cerr << "Finding lowest Chi" << endl;
 
 FFNode wanderer=bottom;
 
 while((wanderer != top) && (spqrLabel(wanderer)->getFirst() != node) &&
      (spqrLabel(wanderer)->getSecond() != node))
  {
   wanderer= getFatherEdge(wanderer)->source();
  }
 return wanderer;
}

void FFSpqr::setTarget(FFEdge e, FFNode n2)
{
 e->setTarget(n2);
 spqrLabel(n2)->updatePoles();
 spqrLabel(e->source())->updatePoles();
}

void FFSpqr::setSource(FFEdge e, FFNode n1)
{
 FFNode n2=e->target();

 FFNode oldS= e->source();
 spqrLabel(oldS)->del(e);
 spqrLabel(oldS)->updatePoles();
 
 FFSpqrNodeLabel *nl1= spqrLabel(n1);
 FFSpqrNodeLabel *nl2= spqrLabel(n2);
 switch(nl1->getSpqrType())
  {
  case S_NODE:
   {
    _insertEdgeToSNode(e, n1);
   }
  case R_NODE:
   {
    cerr << "Can't handle R-Nodes" << endl;
    break;
   }
  case Q_NODE:
  case P_NODE:
  default:
   {
    break;
   }
  }
 
 e->setSource(n1);
 spqrLabel(n1)->updatePoles();
}


FFEdge FFSpqr::addEdge(FFNode n1, FFNode n2)
{
 if(n1 == n2)
  {
   cerr << "Trying to insert reflexiv edge to spqr tree" << endl;
   return NULL;
  }
 FFEdge newE= 0;
 
 FFSpqrNodeLabel *nl1= spqrLabel(n1);
 FFSpqrNodeLabel *nl2= spqrLabel(n2);
 
 switch(nl1->getSpqrType())
  {
  case P_NODE:
   {
    if((nl1->getFirst() != nl2->getFirst()) ||
       (nl1->getSecond() != nl2->getSecond()))
     {
      cerr << "Trying to insert incompatible node under P-Node" << endl;
     }
    newE= spqr->addEdge(n1,n2);
    break;
   }
  case R_NODE:
   {
    cerr << "Can't handle R-Nodes" << endl;
    break;
   }
  case Q_NODE:
   {
    cerr << "Q-Nodes can't have successors!!!" << endl;
    break;
   }
  case S_NODE:
   {
    newE= spqr->addEdge(n1,n2);
    _insertEdgeToSNode(newE, n1);
    break;
   }
  default:
   {
    cerr << "Trying to add Edge to unknown node type" << endl;
    break;
   }
  }
 spqrLabel(n2)->updatePoles();
 spqrLabel(n1)->updatePoles();
 
 return newE;
}


void FFSpqr::_insertEdgeToSNode(FFEdge e, FFNode n)
{
 FFSpqrNodeLabel *nl1= spqrLabel(n);
 FFSpqrNodeLabel *nl2= spqrLabel(e->target());

 if(nl1->getSpqrType() != S_NODE)
  {
   cerr << "FFSpqr::_insertEdgeToSNode without S node!" << endl;
   return;
  }
 
 FFNode zweiter=nl2->getSecond();
    
 const DLList<FFEdge>& nach= nl1->edgeToSucc();

 for(Pix i=nach.first(); i!= 0; nach.next(i))
  {
   FFEdge tmpE=nach(i);
   FFNode tmpN=spqrLabel(tmpE->target())->getFirst();
   
   if(tiLabel(tmpN)->getSTNum() == tiLabel(zweiter)->getSTNum())
    {
     nl1->insertBefore(i, e);
     return;
    }
   else if(tiLabel(tmpN)->getSTNum() > tiLabel(zweiter)->getSTNum())
    {
     cerr << "Adding inconsistent edge to S-Node" << endl;
     nl1->insertBefore(i, e);
     return;
    }
  }
 nl1->append(e);
}

//
// Adds newQ parallel to the part between l1 and l2 of S-Node my
//

void FFSpqr::insertSP(FFNode my, FFNode newQ)
{
 cerr << "Inserting SP" << endl;

 if(spqrLabel(my)->getSpqrType() != S_NODE)
  {
   cerr << "MY NOT A S NODE!!!" << endl;
   assert(0);
  }

 FFNode newS= addSNode();
 FFNode newP= addPNode();

 FFEdge newE=transform(my, newS, spqrLabel(newQ)->getFirst(), 
                       spqrLabel(newQ)->getSecond());
  
 addEdge(newP, newQ);
 addEdge(newP, newS);
 setTarget(newE, newP); // No update of propAllocNodes as P cannot be one
}

FFEdge FFSpqr::transform(FFNode top, FFNode newN,
                         FFNode source, FFNode sink)
{
 cerr << "Transform" << endl;
 
 SpqrType st1= spqrLabel(top)->getSpqrType();
 SpqrType st2= spqrLabel(newN)->getSpqrType();
 FFEdge newE= 0;
 
 if((st1 == S_NODE) && (st2 == S_NODE))
   newE= transformSS( top,  newN,  source,  sink);
 else if((st1 == P_NODE) || (st2 == P_NODE))
   newE= transformAnyP( top,  newN,  source,  sink);
 else
  cerr << "Can't transform" << endl;
  
 return newE;
}

FFEdge FFSpqr::transformAnyP(FFNode top, FFNode newN,
                             FFNode source, FFNode sink)
{
 cerr << "Transform any P" << endl;

 if(spqrLabel(top)->getSpqrType() == S_NODE)
  {
   const DLList<FFEdge>& nach= spqrLabel(top)->edgeToSucc();

   Pix i=nach.first();
   while(spqrLabel(nach(i)->target())->getFirst() != source)
    {
     nach.next(i);
     assert(i != NULL);
    }
   while(spqrLabel(nach(i)->target())->getSecond() != sink)
    {
     nach(i)->setSource(newN);
     if(spqrLabel(newN)->getSpqrType() == S_NODE)
      {
       cerr << "Error calling transform with two S Nodes" << endl;
       // spqrLabel(newN)->append(nach(i));
      }
     spqrLabel(top)->del(i);
     assert(i != NULL);
    }

   nach(i)->setSource(newN);
   if(spqrLabel(newN)->getSpqrType() == S_NODE)
    {
     cerr << "Error calling transform with two S Nodes" << endl;
     // spqrLabel(newN)->append(nach(i));
    }
   
   spqrLabel(top)->del(i);
  }
 else // top is no S node
  {
   const FFSet<FFEdge>& nach= top->transToSucc();
   FFEdge sourceE= 0;
   FFEdge sinkE= 0;
   
   for(Pix i= nach.first(); i!= 0; nach.next(i))
    {
     FFEdge tmpE= nach(i);
     if(tmpE->target() == sink)
       sinkE= tmpE;
     else if(tmpE->target() == source)
       sourceE= tmpE;
     
     sourceE->setSource(newN);
     sinkE->setSource(newN);
     if(spqrLabel(newN)->getSpqrType() == S_NODE)
      {
       spqrLabel(newN)->append(sourceE);
       spqrLabel(newN)->append(sinkE);
      }
    }
  }

 return addEdge(top, newN);
}

void FFSpqr::showOrder()
{
 const FFSet<FFNode>& knoten=spqr->allNodes();
 for(Pix i=knoten.first(); i!= 0; knoten.next(i))
  {
   FFNode tmpN=knoten(i);
   if(spqrLabel(tmpN)->getSpqrType() == S_NODE)
    {
     int k=1;
     const DLList<FFEdge>& nach= spqrLabel(tmpN)->edgeToSucc();
     for(Pix j=nach.first(); j!= 0; nach.next(j))
      {
       FFNode tmpN2= nach(j)->target();
       tmpN2->setName(tmpN2->getName() + "("+dec(k++)+")");
      }
     tmpN->setName(tmpN->getName() + "("+dec(nach.length())+")");
    }
  }
}


FFEdge FFSpqr::transformSS(FFNode top, FFNode newN, 
                         FFNode source, FFNode sink)
{
 cerr << "Transform SS" << endl;

 FFSet<FFNode> toMove;
 FFSet<FFNode> toStay;
 
 assert(source!=NULL);
 assert(sink!=NULL);
 
 const DLList<FFEdge>& nach= spqrLabel(top)->edgeToSucc();
 Pix i=nach.first();
 
 while(spqrLabel(nach(i)->target())->getFirst() != source)
  {
   _collectSuccessors(nach(i)->target(), toStay);
   nach.next(i);
   assert(i!=NULL);
  }
 while(spqrLabel(nach(i)->target())->getSecond() != sink)
  {
   FFEdge tmpE=nach(i);
   _collectSuccessors(tmpE->target(), toMove);
   tmpE->setSource(newN);
   spqrLabel(newN)->append(tmpE);
   
   spqrLabel(top)->del(i);
   assert(i!=NULL);
  }
 
 _collectSuccessors(nach(i)->target(), toMove);
 nach(i)->setSource(newN);
 
 spqrLabel(newN)->append(nach(i));
 FFEdge resultEdge=spqr->addEdge(top, newN);
 
 spqrLabel(top)->insertBefore(i, resultEdge);
 spqrLabel(top)->del(i);
 
 while(i != NULL)
  {
   _collectSuccessors(nach(i)->target(), toStay);
   nach.next(i);
  }

 // 
 // Anpassen der proper allocation nodes
 //
 
 for(Pix j=toMove.first(); j!= 0; toMove.next(j))
  {
   FFNode tmpN=toMove(j);
   int tmpST= tiLabel(tmpN)->getSTNum();
   
   if((propAllocNode[tmpST] == top) && (!toStay.contains(tmpN)))
    {
     propAllocNode[tmpST]= newN;
    }
  }
 
 spqrLabel(newN)->updatePoles();
 spqrLabel(top)->updatePoles();
 
 return resultEdge;
}

FFNode FFSpqr::leastCommonAncestor(FFNode n1, FFNode n2, 
                                   FFNode& l1, FFNode& l2)
{
 cerr << "Least common ancestor" << endl;

 if(n1 == n2)
  {
   l1= 0;
   l2= 0;
   return n1;
  }
 
 DLList<FFNode> pfad1;
 DLList<FFNode> pfad2;
 pfad1.clear();
 pfad2.clear();
 
 FFEdge aktuE = getFatherEdge(n1);
 pfad1.append(n1);
 cerr << "   " << n1->getName()<< "(" << n1 << ") ";
 
 while(aktuE != NULL)
  {
   cerr << "   " << aktuE->source()->getName()<< "(" << aktuE->source() 
        << ") ";
   pfad1.prepend(aktuE->source());
   aktuE = getFatherEdge(aktuE->source());
  }
 
 aktuE = getFatherEdge(n2);
 cerr << endl << "   " << n2->getName()<< "(" << n2 << ") ";
 pfad2.append(n2);
 
 while(aktuE != NULL)
  {
   FFNode tmpN=aktuE->source();
   cerr << "   " << tmpN->getName() << "(" << tmpN << ") ";
   pfad2.prepend(tmpN);
   aktuE = getFatherEdge(tmpN);
  }
 cerr << endl;
 
 FFNode aktuNode= 0;

 do
  {
   FFNode tmpN2=pfad2.front();
   FFNode tmpN1=pfad1.front();
   cerr << "Comparing " << tmpN1 << " and " << tmpN2 << endl;

   if(tmpN1 != tmpN2)
    {
     cerr << "Compare failed" << endl;
     break;
    }

   aktuNode=tmpN1;

   pfad1.del_front();
   pfad2.del_front();
  }
 while((!pfad1.empty()) && (!pfad2.empty()));
 
 if(!pfad1.empty())
  l1= pfad1.front();
 else 
  l1= 0;
 
 if(!pfad2.empty())
  l2= pfad2.front();
 else
  l2= 0;
 
 cerr << "  Returning: " << aktuNode << endl;
 return aktuNode;
}

FFNode FFSpqr::insertNode(FFNode neu, FFNode target)
{
 cerr << "Insert Node" << endl;

 FFEdge vonKante = getFatherEdge(target);
 assert(vonKante != NULL);
 FFNode father= vonKante->source();
 
 FFNode tmpN=spqrLabel(target)->getSecond();
 FFNode result= addQNode(neu, tmpN);
 spqrLabel(target)->setSecond(neu);
 
 if(spqrLabel(father)->getSpqrType() == S_NODE)
  {
   addEdge(father, result);
  }
 else
  {
   FFNode newS= addSNode();
   addEdge(newS, target);
   addEdge(newS, result);
   setTarget(vonKante, newS);
  }

 
 FFEdge tmpE= getFatherEdge(result);
 assert(tmpE != NULL);
 
 propAllocNode[tiLabel(neu)->getSTNum()] = tmpE->source();

 return result;
}
