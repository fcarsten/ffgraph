#include <unistd.h>
#include <stream.h>
#include <ffgraph/FFTIGraphLabel.h>
#include <ffgraph/FFTINodeLabel.h>
#include <ffgraph/FFTIEdgeLabel.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphWindow.h>
#include <container/SLStack.h>

void FFTIGraphLabel::event(labelEvent event, labelEventStruct *)
{
 switch(event)
  {
  case ADD_NODE:
  case ADD_EDGE:
  case AFTER_DEL_NODE:
  case AFTER_DEL_EDGE:
  case SET_STARTNODE:
  case DEL_STARTNODE:
  case SET_ENDNODE:
  case REV_EDGE:
  case SOURCE_CHANGE:
  case TARGET_CHANGE:
  case DIRECTEDNESS_CHANGE:
  case DEL_ENDNODE:
   {
    if(!isLoading)
     dfs();
    break;
   }
  case START_LOAD:
   {
    isLoading = YES;
    break;
   }
  case END_LOAD:
   {
    isLoading = NO;
    dfs();
    break;
   }
  default:
   {
   }
  }
}

void FFTIGraphLabel::init(const FFGraph& g) 
{
 FFGraphLabel::init(g);
 tiNodeLabel = graph->addNodeLabel(new FFTINodeLabel);
 tiEdgeLabel = graph->addEdgeLabel(new FFTIEdgeLabel);
 dfs();
}

FFTIGraphLabel::FFTIGraphLabel()
{
 _setType(ffTIGraphLabelType);
 isLoading=NO;
 tiNodeLabel = -1;
 tiEdgeLabel = -1;
 sNode = 0;
 tNode = 0;
}

void FFTIGraphLabel::_stNumbering()
{
 SLStack<FFNode> zuTun;

 FFTINodeLabel *nl= 0;
 FFTIEdgeLabel *el= 0;

 sNode = 0;
 tNode = 0;

 Pix i = 0;
 int aktuST = 0;
 
 // Suche S und T Knoten und stelle sicher, da der Graph zweifach Zusammen-
 // hangskomponente ist.
 
 const FFSet<FFNode>& alle= graph->allNodes();
 for(i= alle.first(); i != 0; alle.next(i))
  {
   nl = (FFTINodeLabel *)alle(i)->getLabel(tiNodeLabel);
   
   if(nl->getArtPkt() == YES)
    {
     // cerr << "Graph ist keine zweifach Zusammenhangskomponente!" << endl;
     return;
    }
   if(nl->getDfsNum() == 0)
    {
     tNode = alle(i);
     alle(i)->setName("t");
     nl->setSTFlag(YES);
    }
   else if(nl->getDfsNum() == 1)
    {
     sNode = alle(i);
     alle(i)->setName("s");
     nl->setSTFlag(YES);
    }
   else
    nl->setSTFlag(NO);
  }

 const FFSet<FFEdge>& alleKanten= graph->allEdges();
 for(i= alleKanten.first(); i!= 0; alleKanten.next(i))
  {
   FFEdge tmpE=alleKanten(i);
   
   el = (FFTIEdgeLabel *)tmpE->getLabel(tiEdgeLabel);
   if(el != NULL)
   {
    if((tmpE->source() == tNode) && (tmpE->target() == sNode))
     {
      el->setSTFlag(YES);
     }
    else
     el->setSTFlag(NO);
   }
   
  }
 
 if((sNode == NULL) || (tNode == NULL))
  {
   // cerr << "Couldn't find s or t node" << endl;
   return;
  }
 
 // Ab hier wird die s-t Nummerierung wirklich berechnet

 zuTun.push(sNode);
 // (*sNode)->setColor("red");
 // cerr << "Push :" << sNode->getName() << endl;
 // graph->getWindow()->update();
 
 while(!zuTun.empty())
  {
   FFNode tmpN = zuTun.pop();
   // cerr << "Pop :" << tmpN->getName() << endl;
   // (*tmpN)->setColor("black");
   // graph->getWindow()->update();
     
   BOOL allDone = YES;

   const FFSet<FFEdge>& tmpS = tmpN->transToSucc();
   for(i = tmpS.first(); i != 0; tmpS.next(i))
    {
     FFEdge tmpE = tmpS(i);
     el = (FFTIEdgeLabel *)tmpE->getLabel(tiEdgeLabel);

     if(el->getSTFlag() == NO)
      {
       FFNode tmpT= tmpE->target();
       allDone = NO;
       _pushPath(zuTun, tmpT, tmpE);
       zuTun.push(tmpN);

       // cerr << "Push :" << tmpN->getName() << endl;
       // (*tmpN)->setColor("red");
       // graph->getWindow()->update();
       break;
      }
    }
   
   if(allDone)
    {
     const FFSet<FFEdge>& tmpTS = tmpN->transFromPred();
     for(i = tmpTS.first(); i != 0; tmpTS.next(i))
      {
       FFEdge tmpE = tmpTS(i);
       el = (FFTIEdgeLabel *)tmpE->getLabel(tiEdgeLabel);

       if(el->getSTFlag() == NO)
        {
         FFNode tmpT = tmpE->source();
         allDone = NO;
         _pushPath(zuTun, tmpT, tmpE);
         zuTun.push(tmpN);
         // cerr << "Push :" << tmpN->getName() << endl;
         // (*tmpN)->setColor("red");
         // graph->getWindow()->update();
         break;
        }
      }
    }
   
   if(allDone)
    {
     nl = (FFTINodeLabel *)tmpN->getLabel(tiNodeLabel);
     nl->stNum = aktuST ++;
     tmpN->setName(tmpN->getName() + String(":") + dec(aktuST-1));
    }
  }
 nl = (FFTINodeLabel *)tNode->getLabel(tiNodeLabel);
 nl->stNum = aktuST ++;
 tNode->setName(tNode->getName() + String(":") + dec(aktuST-1));
}


//
// void FFTIGraphLabel::_pushPath(...)
//
// Remark:
// Efficiency of this algorithm could be improved by computing the father
// of a node and its successor with the same low number at computing the dfs !!!
//
// Complexity: _O_(n^2) (could be reduced to O(n) by the means mentioned above)
//

void FFTIGraphLabel::_pushPath(SLStack<FFNode>& zuTun, FFNode node, 
                               FFEdge edge)
{
 SLStack<FFNode> revStack;

 FFTINodeLabel *nl = (FFTINodeLabel *)node->getLabel(tiNodeLabel);
 FFTIEdgeLabel *el=  (FFTIEdgeLabel *)edge->getLabel(tiEdgeLabel);
 int goal = nl->getLowNum();
 
 // cerr << "Pushpath: " << node->getName() << endl;
 
 if((el->getTiType() == TI_BACKWARD) && (edge->target() == node))
  {
   // cerr << "Case 1" << endl;
   el->setSTFlag(YES); 
   return;
  }
 else if((el->getTiType() == TI_TREE) && (edge->target() == node))
  {
   FFNode pathNode = node;
   // cerr << "Case 2" << endl;
   
   while(nl != NULL)
    {
     nl->setSTFlag(YES);
     el->setSTFlag(YES);
     revStack.push(pathNode);

     // Looking for a successor with the same low number
     // could be computed more efficiently at DFS !!!
     
     const FFSet<FFEdge>& alleSucc = pathNode->transToSucc();
     for(Pix i=alleSucc.first(); i != 0; alleSucc.next(i))
      {
       el = (FFTIEdgeLabel *)alleSucc(i)->getLabel(tiEdgeLabel);
       nl = 0;
       if(el->getTiType() == TI_TREE)
        {
         pathNode = alleSucc(i)->target();
         nl = (FFTINodeLabel *)pathNode->getLabel(tiNodeLabel);

         if(nl->getLowNum() == goal)
          break;
         else
          {
           nl = 0;
           // el->setSTFlag(YES);
          }
        }
      }
    }

   pathNode = revStack.top();
   
   const FFSet<FFEdge>& alleSucc = pathNode->transToSucc();
   for(Pix i=alleSucc.first(); i != 0; alleSucc.next(i))
    {
     FFEdge pathEdge = alleSucc(i);
     el = (FFTIEdgeLabel *)pathEdge->getLabel(tiEdgeLabel);

     if(el->getTiType() == TI_BACKWARD)
      {
       pathNode = pathEdge->target();
       
       nl = (FFTINodeLabel *)pathNode->getLabel(tiNodeLabel);
       
       if(nl->getLowNum() == goal)
        break;
       else
        nl = 0;
      }
    }
   if(nl != NULL)
    {
     el->setSTFlag(YES);
     nl->setSTFlag(YES); // Nicht sicher. Geht aus Beschreibung des
     // Algorithmus nicht genau hervor. Ist evtl. sowieso immer YES !!!!
    }
  }
 else if((el->getTiType() == TI_BACKWARD) && (edge->source() == node))
  {
   FFNode pathNode = node;
   el->setSTFlag(YES);
   // cerr << "Case 3" << endl;

   while((nl != NULL) && (nl->getSTFlag() == NO))
    {
     nl->setSTFlag(YES);
     el->setSTFlag(YES);
     revStack.push(pathNode);
     nl = 0;
     
     // computing the father of a node. Could be done more effciently
     // while computing DFS !!!
     
     const FFSet<FFEdge>& allePred = pathNode->transFromPred();
     for(Pix i=allePred.first(); i != 0; allePred.next(i))
      {
       el = (FFTIEdgeLabel *)allePred(i)->getLabel(tiEdgeLabel);
       if(el->getTiType() == TI_TREE)
        {
         pathNode = allePred(i)->source();
         nl = (FFTINodeLabel *)pathNode->getLabel(tiNodeLabel);
         el->setSTFlag(YES);
         break;
        }
      }
    }
  }
 else
  {
   cerr << "Case shouldn't happen!" << endl;
   el->setSTFlag(YES); // Helps terminating, result still wrong !!!
  }
 
 while(! revStack.empty())
  {
   FFNode tmpN = revStack.pop();
   // cerr << "Push :" << tmpN->getName() << endl;
   // (*tmpN)->setColor("red");
   
   zuTun.push(tmpN);
  }
}

void FFTIGraphLabel::_adjEdgeDir()
{
 const FFSet<FFEdge>& alleKanten = graph->allEdges();
 Pix i= 0;
 FFTINodeLabel *nlS= 0;
 FFTINodeLabel *nlT= 0;
 FFTIEdgeLabel *el= 0;

 for(i=alleKanten.first(); i!= 0; alleKanten.next(i))
  {
   nlS = (FFTINodeLabel *)alleKanten(i)->source()->getLabel(tiNodeLabel);
   nlT = (FFTINodeLabel *)alleKanten(i)->target()->getLabel(tiNodeLabel);
   el = (FFTIEdgeLabel *)alleKanten(i)->getLabel(tiEdgeLabel);
   if(el->getTiType() == TI_TREE)
    {
     if(nlS->getDfsNum() > nlT->getDfsNum())
      alleKanten(i)->revert();
    }
   else if(nlS->getDfsNum() < nlT->getDfsNum())
    alleKanten(i)->revert();
  } 
}

void FFTIGraphLabel::_adjEdgeDirST()
{
 const FFSet<FFEdge>& alleKanten = graph->allEdges();
 Pix i= 0;
 FFTINodeLabel *nlS= 0;
 FFTINodeLabel *nlT= 0;
// FFTIEdgeLabel *el= 0;

 for(i=alleKanten.first(); i!= 0; alleKanten.next(i))
  {
   nlS = (FFTINodeLabel *)alleKanten(i)->source()->getLabel(tiNodeLabel);
   nlT = (FFTINodeLabel *)alleKanten(i)->target()->getLabel(tiNodeLabel);
//   el = (FFTIEdgeLabel *)alleKanten(i)->getLabel(tiEdgeLabel);
   if(nlS->getSTNum() > nlT->getSTNum())
    alleKanten(i)->revert();
  } 
}

void FFTIGraphLabel::_markArtPkt()
{
 Pix j= 0;
 Pix i= 0;
 FFTIEdgeLabel *el= 0;
 FFTINodeLabel *nl= 0;
 FFTINodeLabel *nl2= 0;
 
 const FFSet<FFNode>& alleKnoten = graph->allNodes();
 for(i=alleKnoten.first(); i!= 0; alleKnoten.next(i))
  {
   int count = 0;
   nl = (FFTINodeLabel *)alleKnoten(i)->getLabel(tiNodeLabel);
   const FFSet<FFEdge>& zu= alleKnoten(i)->transToSucc();
   
   // Zurcksetzen auf Kein Artikulationspunkt

   nl->setArtPkt(NO);
   // (*alleKnoten(i))->setColor("black");
   

   // Test auf Artikulationspunkte 

   for(j=zu.first(); j!= 0; zu.next(j))
    {
     el = (FFTIEdgeLabel *)zu(j)->getLabel(tiEdgeLabel);
     if(el && (el->getTiType() == TI_TREE))
      {
       count ++;
       
       nl2 = (FFTINodeLabel *)zu(j)->target()->getLabel(tiNodeLabel);
       if(nl->getDfsNum() != 0)
        {
         if(nl->getDfsNum() <= nl2->getLowNum())
          {
           nl->setArtPkt(YES);
           // (*alleKnoten(i))->setColor("blue");
          }
        }
       else if(count == 2)
        {
         nl->setArtPkt(YES);
         // (*alleKnoten(i))->setColor("blue");
        }
      }
    }

   const FFSet<FFEdge>& nach= alleKnoten(i)->transFromPred();
   
   for(j=nach.first(); j!= 0; nach.next(j))
    {
     el = (FFTIEdgeLabel *)nach(j)->getLabel(tiEdgeLabel);
     if(el && (el->getTiType() == TI_TREE))
      {
       nl2 = (FFTINodeLabel *)nach(j)->source()->getLabel(tiNodeLabel);
       if(nl->getDfsNum() != 0)
        {
         if(nl->getDfsNum() <= nl2->getLowNum())
          {
           nl->setArtPkt(YES);
           // (*alleKnoten(i))->setColor("blue");
          }
        }
       else if(count == 2)
        {
         nl->setArtPkt(YES);
         // (*alleKnoten(i))->setColor("blue");
        }
      }
    }
  }
 
}

void FFTIGraphLabel::dfs()
{ 
 aktuDfs =0;
 aktuComp =0;
 aktuLow =0;
 Pix i;

 FFSet<FFEdge> edgeSet;
 
 edgeSet |= graph->allEdges();
 for(i = edgeSet.first(); i != 0; edgeSet.next(i))
  {
   FFTIEdgeLabel *el = (FFTIEdgeLabel *)edgeSet(i)->getLabel(tiEdgeLabel);
   el->setTiType(TI_NO);
  }
 
 todo.clear();
 todo |= graph->allNodes();
 for(i = todo.first(); i != 0; todo.next(i))
  {
   FFTINodeLabel *nl = (FFTINodeLabel *)todo(i)->getLabel(tiNodeLabel);
   nl->dfsNum = -1;
   nl->compNum = -1;
   nl->lowNum = 0;
  }
 
 if(graph->getStartNode() != NULL) 
 {
  if(graph->getDirected())
   {
    //    _dfsDir(graph->getStartNode());
   }
  else
   {
    _dfsUndir(graph->getStartNode());
   }
 }

 while(todo.length() != 0)
  if(graph->getDirected())
   {
    _dfsDir(todo(todo.first()));
    //todo.clear(); // !!!! MUSS WIEDER RAUS
   }
  else
   {
    _dfsUndir(todo(todo.first()));
   }
 if(graph->getDirected() == NO)
  {
    _markArtPkt();
    _adjEdgeDir();
    _stNumbering();
    _adjEdgeDirST();
  }
}

void FFTIGraphLabel::_dfsDir(FFNode knoten)
{
 Pix i;
 
 FFTINodeLabel *nl = (FFTINodeLabel *) knoten->getLabel(tiNodeLabel);
 todo.del(knoten);
 
 if(nl != NULL)
  {
   nl->dfsNum = aktuDfs ++;
   nl->lowNum = nl->dfsNum;
   knoten->setName(dec(nl->dfsNum));
   
   const FFSet<FFEdge>& transTo = knoten->transToSucc();
   for(i = transTo.first(); i!= 0; transTo.next(i))
    {
     FFNode tmpN = transTo(i)->target();
     FFEdge tmpE=transTo(i);
     FFTINodeLabel *tmpL = (FFTINodeLabel *) tmpN->getLabel(tiNodeLabel);
     FFTIEdgeLabel *tmpEL = (FFTIEdgeLabel *) tmpE->getLabel(tiEdgeLabel);
     
     if(todo.contains(tmpN))
      {
       tmpEL->setTiType(TI_TREE);
       (*tmpE)->setColor("black");
       _dfsDir(tmpN);         
      }
     else if (tmpL->compNum >= 0)
      {
       if (tmpL->dfsNum > nl->dfsNum)
        {
         tmpEL->setTiType(TI_FORWARD);
         (*tmpE)->setColor("blue");
        }
       else
        {
         tmpEL->setTiType(TI_CROSS);
         (*tmpE)->setColor("green");
        }
      }
     else 
      {
       tmpEL->setTiType(TI_BACKWARD);
       (*tmpE)->setColor("gray66");
      }
       nl->lowNum = std::min(nl->lowNum, tmpL->lowNum);
    }
   nl->compNum = aktuComp++;   
  }
}

void FFTIGraphLabel::_dfsUndir(FFNode knoten)
{
 Pix i;
 
 FFTINodeLabel *nl = (FFTINodeLabel *) knoten->getLabel(tiNodeLabel);
 todo.del(knoten);
 
 if(nl != NULL)
  {
   nl->dfsNum = aktuDfs ++;
   nl->lowNum = nl->dfsNum;
   
   const FFSet<FFEdge>& transTo = knoten->transToSucc();
   for(i = transTo.first(); i!= 0; transTo.next(i))
    {
     FFTIEdgeLabel *el = (FFTIEdgeLabel *) transTo(i)->getLabel(tiEdgeLabel);
     if(el->getTiType() == TI_NO)
      {
       FFNode tmpN = transTo(i)->target();
       if(todo.contains(tmpN))
        {
         el->setTiType(TI_TREE);
         (*transTo(i))->setColor("black");
         _dfsUndir(tmpN);         
         nl->lowNum = std::min(nl->lowNum, ((FFTINodeLabel *)
                                       tmpN->getLabel(tiNodeLabel))->lowNum);
        }
       else // Backward oder Cross
        {
         el->setTiType(TI_BACKWARD);
         (*transTo(i))->setColor("gray66");
         nl->lowNum = std::min(nl->lowNum, ((FFTINodeLabel *)
                                       tmpN->getLabel(tiNodeLabel))->dfsNum);
        }
      }
    }
   
   const FFSet<FFEdge>& transFrom = knoten->transFromPred();
   for(i = transFrom.first(); i!= 0; transFrom.next(i))
    {
     FFTIEdgeLabel *el = (FFTIEdgeLabel *) transFrom(i)->getLabel(tiEdgeLabel);
     if(el->getTiType() == TI_NO)
      {
       FFNode tmpN = transFrom(i)->source();
       if(todo.contains(tmpN))
        {
         el->setTiType(TI_TREE);
         (*transTo(i))->setColor("black");
         _dfsUndir(tmpN);
         nl->lowNum = std::min(nl->lowNum, ((FFTINodeLabel *)
                                       tmpN->getLabel(tiNodeLabel))->lowNum);
        }
       else // Backward oder Cross
        {
         el->setTiType(TI_BACKWARD);
         (*transTo(i))->setColor("gray66");
         nl->lowNum = std::min(nl->lowNum, ((FFTINodeLabel *)
                                       tmpN->getLabel(tiNodeLabel))->dfsNum);
        }
      }
    }
   knoten->setName(dec(nl->dfsNum)+ String(":") + dec(nl->lowNum));
   nl->compNum = aktuComp++;   
  }
}
