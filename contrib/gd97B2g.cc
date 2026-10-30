#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

#include <stream.h>
#include <String.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>

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
 double maxWeight=0.0;
 
 graph->_setDirected(NO);

 if(argc > 1)
  {
   outFile = String(argv[1]) + ".g";
   fd = fopen (argv[1], "r");
  }
   
 while(fgets(buf, 1024, fd))
  {
   String zeile(buf);
 
   if(zeile.contains("<LINKS>"))
    nodeMode = 0;
   else
    {
     if(nodeMode == 1)
      {
       FFNode node = graph->addNode();
       node->setValue(atoi(String(zeile.before('@'))));
       node->setName(String(zeile.after('@')).before('\n'));
       cerr << node->getValue() << ":" << node->getName() << endl;
       
      }
     else
      {
       int felderAnz = zeile.freq(" ");
       String felder[felderAnz+2];
       split(zeile, felder, felderAnz+2, RXwhite);
       int a= atoi(felder[0]);
       int b= atoi(felder[1]);
       double weight = std::min(1/(atof(felder[2])/100.0), 10);
       maxWeight = MAX(maxWeight, weight);
       cerr << a << ":" << b << ":" << weight << endl;
       
       FFNode s= graph->nodeWithValue(a);
       FFNode t= graph->nodeWithValue(b);

       if( (s == NULL) || (t == NULL))
        cerr << "s:" << s << " t:" << t << endl;
       else
       {
        FFEdge edge = graph->addEdge(s,t);
        edge->setWeight(weight);
       }
      }
    }
  }
 cout << "Max weight: " << maxWeight << endl;
 
 graph->saveToFile(outFile);
}

/*
;;; Local Variables: ***
;;; compile-command:"c++ gd97B2g.cc -o gd97B2g -I../include -I. -L../fgraph -L../container -L../fgraphic -lFFGraph -lFFGraphic -lFFcontainer -lm -lg++" ***
;;; End: ***
*/
