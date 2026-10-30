#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

#include <stream.h>
#include <String.h>
#include <container/AVLMap.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicNode.h>

// void ffBreak() 
// {
// }

main(int argc, char **argv)
{
 String outFile("out.g"); 
 FILE *fd= stdin;
 char buf[1024];
 int nodeMode = 1;
 FFGraph graph;
 AVLMap<String, FFNode> nodeHash(NULL);
 
 // graph->_setDirected(NO);

 if(argc > 1)
  {
   outFile = String(argv[1]) + ".g";
   fd = fopen (argv[1], "r");
  }
   
 while(fgets(buf, 1024, fd))
  {
   String zeile(buf);
 
   if(zeile.contains("edges"))
    nodeMode = 0;
   else
    {
     if(nodeMode == 1)
      {
       FFNode node = graph->addNode();
       node->setName(zeile.before('\n'));
       nodeHash[zeile.before('\n')] = node;
      }
     else
      {
       String felder[3];
       split(zeile, felder, 3, String("@"));
       String a(felder[0]);
       String b(felder[1]);
       
       FFNode s= nodeHash[a];
       FFNode t= nodeHash[b.before('\n')];
       
       if( (s == NULL) || (t == NULL))
        cerr << "s:" << s << " t:" << t << endl;
       else
       {
        FFEdge edge = graph->addEdge(s,t);
       }
      }
    }
  }

//  graph->eliminateParallelEdges();

 const FFSet<FFNode>& knoten = graph->allNodes();
 
//  graph->createGraphic(NULL);

//  for(Pix i=knoten.first(); i!= 0; knoten.next(i))
//   {
//    FFNode tmpN=knoten(i);
//    tmpN->setValue(tmpN->indeg() + tmpN->outdeg());
//   }
 
//  for(Pix i=knoten.first(); i!= 0; knoten.next(i))
//   {
//    FFNode tmpN=knoten(i);
//    if(tmpN->getValue() == 1)
//     {
//      if(tmpN->indeg() == 1)
//       {
//        FFEdge tmpE= tmpN->transFromPred()(tmpN->transFromPred().first());
//        tmpE->source()->getGraphicNode()->setWidth(tmpE->source()->
//                                                    getGraphicNode()->
//                                                    getWidth() + 5);
//        graph->removeNode(tmpN);
//       }
//      else if(tmpN->outdeg() == 1)
//       {
//        FFEdge tmpE= tmpN->transToSucc()(tmpN->transToSucc().first());
//        tmpE->target()->getGraphicNode()->setWidth(tmpE->target()->
//                                                    getGraphicNode()->
//                                                    getWidth() + 5);
//        graph->removeNode(tmpN);
//       }
//      else
//       cerr << "Node would go to Nirvana" << endl;
     
//     }
//   }
 
 graph->saveToFile(outFile);
}

/*
;;; Local Variables: ***
;;; compile-command:"c++ gd97C2g.cc -o gd97C2g -I../include -I. -L../fgraph -L../container -L../fgraphic -lFFGraph -lFFGraphic -lFFcontainer -lm -lg++" ***
;;; End: ***
*/
