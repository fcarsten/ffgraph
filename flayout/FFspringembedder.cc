#include <ffgraph/FFSpring.h>
#include <ffgraph/FFInspector.h>

int ffBreak()
{
 cerr << "Breaking ... " << endl;
 return -1;
}


void FFSpring::init_FSSpringNodes (const FFSet <FFNode> &locNodes)
///////////////////////////////////////////////////////////////////
//                                                               //
// Initialisierung von FSSpringNodes. Zuordnung der Array-       //
// Elemente zu Knoten erfolgt ueber die temporaere Knotennummer  //
// (Wert in "value" des Knotens).                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int            value = 1;
  FFNode         tmpN;
  Pix            p, q;
  FFGraphicNode  *gn;
  const FFSet<FFNode>& allNodes = graph->allNodes();
  
  for (p = allNodes.first (); p != 0; allNodes.next (p))
   {       
    tmpN                = allNodes (p);
    gn                  = (*tmpN) ();
    value               = tmpN->getValue ();
    nodes[value].node   = tmpN;
    nodes[value].attrib = 0;

    // Testen, ob Knoten festgehalten werden soll.
    if (locNodes.contains (tmpN))
     nodes[value].move = NO;
    else
     nodes[value].move = YES;
    
    if (gn)
     {
      // Falls Knoten bereits Koordinaten (also Graphikrepr.) hat.
      nodes[value].coord.v[0] = gn->getx ();
      nodes[value].coord.v[1] = gn->gety ();
      
      if (dimension == 3)
       nodes[value].coord.v[2] = gn->getz ();
      else
       nodes[value].coord.v[2] = 0.0;
     }
    else
     {
      // falls  keine Grafikrepraesentation des Knotens
      nodes[value].coord.v[0] = 0.0;
      nodes[value].coord.v[1] = 0.0;
      nodes[value].coord.v[2] = 0.0;
     }

    const FFSet <FFEdge>& toSet = tmpN->transToSucc ();;

    for (q = toSet.first (); q != 0; toSet.next (q))
     {
      //      nodes[value].succ.add (toSet (q)->target ()->getValue ());
      nodes[value].conNodes.add (toSet (q)->target ()->getValue ());
     }
    
    const FFSet <FFEdge>& fromSet = tmpN->transFromPred ();
    for (q = fromSet.first (); q != 0; fromSet.next (q))
     {
      //      nodes[value].pred.add (fromSet (q)->source ()->getValue ());
      nodes[value].conNodes.add (fromSet (q)->source ()->getValue ());
     }
    
    nodes[value].toSucc   |= tmpN->transToSucc ();
    nodes[value].fromPred |= tmpN->transFromPred ();
   }

  // Ausgangswerte moeglichst nicht auf einem Punkt (einer Linie),
  // sondern weinigstens ein Knoten im Raum verteilt.
  if (nodes[value].move)
   {
    nodes[value].coord.v[1] = nodes[value].coord.v[1] + 1;
    if (dimension == 3)
     nodes[value].coord.v[2] = nodes[value].coord.v[2] + 1;
   }
}


void FFSpring::init_forces (FFPoint *forces, int anzN)
///////////////////////////////////////////////////////////////////
//                                                               //
// Initialisiert das Array "forces" der Laenge "anzN" vom        //
// Typ FFPoint mit 0-Werten.                                     //
//                                                               //
///////////////////////////////////////////////////////////////////
{   
  FFPoint point (0.0, 0.0, 0.0, 1.0);
  
  for (int i = 1; i <= anzN; i++)
    forces[i] = point;
}


void FFSpring::compute_forces (FFCoord optimalDistance, 
                               FFPoint *forces, int anzN)
///////////////////////////////////////////////////////////////////
//                                                               //
// Berechnet die Kraefte, die an den Knoten des Graphen wirken.  //
// Dabei werden die Koordinaten der Knoten in der Struktur       //
// "nodes" uebergeben. Das Ergebnis wird im Array "forces"       //
// zurueckgegeben.                                               //
// Die Vorgaenger und Nachfolger werden ueber die entsprechenden //
// Knotenmengen bestimmt. Kantengewichte finden hierbei keine    //
// Beruecksichtigung.                                            //
//                                                               //
///////////////////////////////////////////////////////////////////
{   
  int     k, nodeIndex1, nodeIndex2;
  Pix     p;
  FFPoint attractiveForce, repulsiveForce, force, delta, point;
  FFCoord attractiveFactor, repulsiveFactor, 
    sqrOptimalDistance = optimalDistance * optimalDistance;

  // Falls zweidimensionales Layout
  if (dimension == 2)
    {
      point.v[0] = 1.0;
      point.v[1] = 1.0;
      point.v[2] = 0.0;

      for (nodeIndex1 = 1; nodeIndex1 < anzN; nodeIndex1++)
        {        
          // Fuer alle Vorgaenger und Nachfolger
	  for (p = nodes[nodeIndex1].conNodes.first (); p != 0; 
	       nodes[nodeIndex1].conNodes.next (p))
            {
              
	      if (nodeIndex1 < nodes[nodeIndex1].conNodes (p))
                {  
                  nodeIndex2 = nodes[nodeIndex1].conNodes (p);
                  delta = nodes[nodeIndex2].coord - nodes[nodeIndex1].coord;
	     
                  if ((delta.v[0] == 0.0) && (delta.v[1] == 0.0))
		    delta = point;
		  
                  attractiveFactor = (sqrt (delta.v[0] * delta.v[0] +
                                            delta.v[1] * delta.v[1])) / 
		    optimalDistance;
                  attractiveForce  = delta * attractiveFactor;
                  repulsiveFactor  = sqrOptimalDistance / 
                    (delta.v[0] * delta.v[0] +
                     delta.v[1] * delta.v[1]);
                  repulsiveForce   = delta *  repulsiveFactor;
                  force            = attractiveForce - repulsiveForce;
                  forces[nodeIndex1] += force;
                  forces[nodeIndex2] -= force;
                  nodes[nodeIndex2].attrib = nodeIndex1;
                }
            }  

	  for (k = nodeIndex1 + 1; k <= anzN; k++)
            {
              if (nodes[k].attrib != nodeIndex1)
                {
                  nodeIndex2 = k;
                  delta = nodes[nodeIndex2].coord - nodes[nodeIndex1].coord;

                  if ((delta.v[0] == 0.0) && (delta.v[1] == 0.0)) 
		    delta = point;

                  repulsiveFactor = sqrOptimalDistance / 
                    (delta.v[0] * delta.v[0] + 
                     delta.v[1] * delta.v[1]);
                  repulsiveForce  = delta * repulsiveFactor;
                  forces[nodeIndex1] -= repulsiveForce;
                  forces[nodeIndex2] += repulsiveForce;
                }  
            }      
        }
    }
  
  // Falls dreidimensionales Layout
  if (dimension == 3)
    {
      point.v[0] = 1.0;
      point.v[1] = 1.0;
      point.v[2] = 1.0;

      for (nodeIndex1 = 1; nodeIndex1 < anzN; nodeIndex1++)
        {        
          // Fuer alle Vorgaenger- und Nachfolgerknoten
	  for (p = nodes[nodeIndex1].conNodes.first (); p != 0; 
	       nodes[nodeIndex1].conNodes.next (p))
            {

              if (nodeIndex1 < nodes[nodeIndex1].conNodes (p))
                {  
                  nodeIndex2 = nodes[nodeIndex1].conNodes (p);
                  delta = nodes[nodeIndex2].coord - nodes[nodeIndex1].coord;

                  if ((delta.v[0] == 0.0) && 
                      (delta.v[1] == 0.0) && 
                      (delta.v[2] == 0.0))
		    delta = point;

                  attractiveFactor = delta.abs () / optimalDistance;
                  attractiveForce  = delta * attractiveFactor;
                  repulsiveFactor  = sqrOptimalDistance / 
                    (delta.v[0] * delta.v[0] +
                     delta.v[1] * delta.v[1] + 
                     delta.v[2] * delta.v[2]);
                  repulsiveForce   = delta * repulsiveFactor;
                  force            = attractiveForce - repulsiveForce;
                  forces[nodeIndex1] += force;
                  forces[nodeIndex2] -= force;
                  nodes[nodeIndex2].attrib = nodeIndex1;
                }
            }  

	  for (k = nodeIndex1 + 1; k <= anzN; k++)
            {
              if (nodes[k].attrib != nodeIndex1)
                {
                  nodeIndex2 = k;
                  delta = nodes[nodeIndex2].coord - nodes[nodeIndex1].coord;

                  if ((delta.v[0] == 0.0) && 
                      (delta.v[1] == 0.0) && 
                      (delta.v[2] == 0.0)) 
		    delta = point;
                     
                  repulsiveFactor = sqrOptimalDistance / 
                    (delta.v[0] * delta.v[0] + 
                     delta.v[1] * delta.v[1] + 
                     delta.v[2] * delta.v[2]);
                  repulsiveForce  = delta * repulsiveFactor;
                  forces[nodeIndex1] -= repulsiveForce;
                  forces[nodeIndex2] += repulsiveForce;
                }  
            }      
        }
    }
}  


void FFSpring::compute_forces_edges (FFCoord optimalDistance, 
                                     FFPoint *forces, int anzN)
///////////////////////////////////////////////////////////////////
//                                                               //
// Berechnet die Kraefte, die an den Knoten des Graphen wirken.  //
// Dabei werden die Koordinaten der Knoten in der Struktur       //
// "nodes" uebergeben. Das Ergebnis wird im Array "forces"       //
// zurueckgegeben.                                               //
// Die Bestimmung der Vorgaenger- und Nachfolgerknoten erfolgt   //
// ueber die ein- und ausgehenden Kanten. Hier werden Kantenge-  //
// wichte beruecksichtigt.                                       //
//                                                               //
///////////////////////////////////////////////////////////////////
{    
  FFPoint attractiveForce, repulsiveForce, force, delta, point;
  FFCoord attractiveFactor, repulsiveFactor, weight;
  int     k, nodeIndex1, nodeIndex2;
  Pix     p;

  // Falls zweidimensionales Layout
  if (dimension == 2)
    {
      point.v[0] = 1.0;
      point.v[1] = 1.0;
      point.v[2] = 0.0;

      for (nodeIndex1 = 1; nodeIndex1 < anzN; nodeIndex1++)
        {        
          // alle ausgehenden Kanten betrachten
	  for (p = nodes[nodeIndex1].toSucc.first (); p != 0; 
	       nodes[nodeIndex1].toSucc.next (p))
            {

              if ((nodeIndex1 < 
		   nodes[nodeIndex1].toSucc (p)->target ()->getValue ()) &&
		  (nodes[nodeIndex1].toSucc (p)->getWeight () != 0.0))
                {  
                  nodeIndex2 = 
		    nodes[nodeIndex1].toSucc (p)->target ()->getValue ();
                  weight = nodes[nodeIndex1].toSucc (p)->getWeight ();
		  delta  = nodes[nodeIndex2].coord - nodes[nodeIndex1].coord;
		  
                  if ((delta.v[0] == 0.0) && (delta.v[1] == 0.0))
		    delta = point;

                  attractiveFactor = (sqrt (delta.v[0] * delta.v[0] +
                                            delta.v[1] * delta.v[1])) / 
		    optimalDistance;
                  attractiveForce = delta * attractiveFactor * weight;
                  repulsiveFactor = (optimalDistance * optimalDistance ) /
                    (delta.v[0] * delta.v[0] + 
                     delta.v[1] * delta.v[1]);
                  repulsiveForce = (delta * repulsiveFactor) / weight;
                  force          = attractiveForce - repulsiveForce;

                  if(! force.valid())
                   ffBreak();
                  
                  forces[nodeIndex1] += force;
                  forces[nodeIndex2] -= force;
                  nodes[nodeIndex2].attrib = nodeIndex1;
                }
            }  
	  
          // alle hereinfuehrenden Kanten betrachten
	  for (p = nodes[nodeIndex1].fromPred.first (); p != 0; 
	       nodes[nodeIndex1].fromPred.next (p))
            {

              if ((nodeIndex1 < 
		   nodes[nodeIndex1].fromPred (p)->source ()->getValue ()) &&
		  (nodes[nodeIndex1].fromPred (p)->getWeight () != 0.0))
                {  
                  nodeIndex2 = 
		    nodes[nodeIndex1].fromPred (p)->source ()->getValue ();
		  weight = nodes[nodeIndex1].fromPred (p)->getWeight ();
                  delta  = nodes[nodeIndex2].coord - nodes[nodeIndex1].coord;
	     
                  if ((delta.v[0] == 0.0) && (delta.v[1] == 0.0))
		    delta = point;

                  attractiveFactor = (sqrt (delta.v[0] * delta.v[0] +
                                            delta.v[1] * delta.v[1])) / 
		    optimalDistance;
                  attractiveForce = delta * attractiveFactor * weight;
                  repulsiveFactor = (optimalDistance * optimalDistance) / 
                    (delta.v[0] * delta.v[0] + 
                     delta.v[1] * delta.v[1]);
                  repulsiveForce = (delta * repulsiveFactor) / weight;
                  force          = attractiveForce - repulsiveForce;

                  if(! force.valid())
                   ffBreak();

                  forces[nodeIndex1] += force;
                  forces[nodeIndex2] -= force;
                  nodes[nodeIndex2].attrib = nodeIndex1;
                }
            }  

	  for (k = nodeIndex1 + 1; k <= anzN; k++)
            {
              if (nodes[k].attrib != nodeIndex1)
                {
                  nodeIndex2 = k;
                  delta      = nodes[nodeIndex2].coord - nodes[nodeIndex1].coord;

                  if ((delta.v[0] == 0.0) && 
                      (delta.v[1] == 0.0)) 
		    delta = point;
                  
                  repulsiveFactor = (optimalDistance * optimalDistance)  / 
                    (delta.v[0] * delta.v[0] + 
                     delta.v[1] * delta.v[1]);
                  repulsiveForce = delta * repulsiveFactor;
                  forces[nodeIndex1] -= repulsiveForce;
                  forces[nodeIndex2] += repulsiveForce;
                }  
            }      
        }
    }
  
  // Falls dreidimensionales Layout
  if (dimension == 3)
    {
      point.v[0] = 1.0;
      point.v[1] = 1.0;
      point.v[2] = 1.0;

      for (nodeIndex1 = 1; nodeIndex1 < anzN; nodeIndex1++)
        {        
	  // alle ausgehenden Kanten betrachten
	  for (p = nodes[nodeIndex1].toSucc.first (); p != 0; 
	       nodes[nodeIndex1].toSucc.next (p))
            {

              if ((nodeIndex1 < 
		   nodes[nodeIndex1].toSucc (p)->target ()->getValue ()) &&
		  (nodes[nodeIndex1].toSucc (p)->getWeight () != 0.0))
                {  
                  nodeIndex2 = 
		    nodes[nodeIndex1].toSucc (p)->target ()->getValue ();
		  weight = nodes[nodeIndex1].toSucc (p)->getWeight ();
                  delta  = nodes[nodeIndex2].coord - nodes[nodeIndex1].coord;

                  if ((delta.v[0] == 0.0) && 
                      (delta.v[1] == 0.0) && 
                      (delta.v[2] == 0.0))
		    delta = point;

                  attractiveFactor = delta.abs() / (optimalDistance / weight);
                  attractiveForce  = delta * attractiveFactor;
                  repulsiveFactor  = ((optimalDistance / weight) * 
				      (optimalDistance / weight)) / 
                    (delta.v[0] * delta.v[0] +
                     delta.v[1] * delta.v[1] + 
                     delta.v[2] * delta.v[2]);
                  repulsiveForce = delta * repulsiveFactor;
                  force          = (attractiveForce - repulsiveForce) * weight;
                  forces[nodeIndex1] += force;
                  forces[nodeIndex2] -= force;
                  nodes[nodeIndex2].attrib = nodeIndex1;
                }
            }

          // alle hereinfuehrenden Kanten betrachten
	  for (p = nodes[nodeIndex1].fromPred.first (); p != 0; 
	       nodes[nodeIndex1].fromPred.next (p))
            {

              if ((nodeIndex1 <  
		   nodes[nodeIndex1].fromPred (p)->source ()->getValue ()) &&
		  (nodes[nodeIndex1].fromPred (p)->getWeight () != 0.0))
                {  
                  nodeIndex2 = 
		    nodes[nodeIndex1].fromPred (p)->source ()->getValue ();
		  weight = nodes[nodeIndex1].fromPred (p)->getWeight ();
                  delta  = nodes[nodeIndex2].coord - nodes[nodeIndex1].coord;

                  if ((delta.v[0] == 0.0) && 
                      (delta.v[1] == 0.0) && 
                      (delta.v[2] == 0.0))
		    delta = point;
                  
                  attractiveFactor = delta.abs() / (optimalDistance / weight);
                  attractiveForce  = delta * attractiveFactor;
                  repulsiveFactor  = ((optimalDistance / weight) *
				      (optimalDistance / weight)) / 
                    (delta.v[0] * delta.v[0] +
                     delta.v[1] * delta.v[1] + 
                     delta.v[2] * delta.v[2]);
                  repulsiveForce = delta * repulsiveFactor;
                  force          = (attractiveForce - repulsiveForce) * weight;
                  forces[nodeIndex1] += force;
                  forces[nodeIndex2] -= force;
                  nodes[nodeIndex2].attrib = nodeIndex1;
                }
            }  

	  for (k = nodeIndex1 + 1; k <= anzN; k++)
            {
              if (nodes[k].attrib != nodeIndex1)
                {
                  nodeIndex2 = k;
                  delta = nodes[nodeIndex2].coord - nodes[nodeIndex1].coord;

                  if ((delta.v[0] == 0.0) && 
                      (delta.v[1] == 0.0) && 
                      (delta.v[2] == 0.0)) 
		    delta = point;

                  repulsiveFactor = (optimalDistance * optimalDistance) / 
                    (delta.v[0] * delta.v[0] + 
                     delta.v[1] * delta.v[1] + 
                     delta.v[2] * delta.v[2]);
                  repulsiveForce = delta * repulsiveFactor;
                  forces[nodeIndex1] -= repulsiveForce;
                  forces[nodeIndex2] += repulsiveForce;
                }  
            }      
        }
    }
}  


FFCoord FFSpring::compute_maxForce (FFPoint *forces, int anzN, 
                                    FFCoord *absForces)
///////////////////////////////////////////////////////////////////
//                                                               //
// Bestimmt die groesste Kraft aus dem Array "forces".           //
//                                                               //
///////////////////////////////////////////////////////////////////
{    
 FFCoord maxAbsForce = 0.0;
      
 for (int i = 1 ; i <= anzN; i++)
  {  
   absForces[i] = forces[i].abs ();
   
   if (absForces[i] > maxAbsForce)
    maxAbsForce = absForces[i];
  }
   
 return (maxAbsForce);
}
 

void FFSpring::compute_nodePosition (FFPoint *forces, 
                                     int anzN, FFCoord temperature, 
                                     FFCoord *absForces)
 ///////////////////////////////////////////////////////////////////
//                                                               //
// Berechnet entsprechend der an den Knoten wirkenden Kraefte    //
// ("forces") und der maximal zulaessigen Verschiebung der       //
// Knoten ("temperature") die neuen Knotenpositionen.            //
//                                                               //
///////////////////////////////////////////////////////////////////
{ 
  FFPoint shift;
  int     i;

  // Falls fuer 2-D
  if (dimension == 2)
    {
      for (i = 1;  i <= anzN; i++)
        { 
	  // Falls Knoten nicht festgehalten werden soll
	  if (nodes[i].move)
	    {
	      if (absForces[i] < temperature)
		{ 
		  shift.v[0] = forces[i].v[0];
		  shift.v[1] = forces[i].v[1];
		  shift.v[2] = 0.0;
		}
	      else
		{  
		  shift.v[0] = forces[i].v[0] / absForces[i] * temperature;
		  shift.v[1] = forces[i].v[1] / absForces[i] * temperature;
		  shift.v[2] = 0.0;
		}
	      nodes[i].coord += shift;
	    }
        }
    }

  // Fuer 3-D
  if (dimension == 3)
    {
      for (i = 1;  i <= anzN; i++)
        {
	  // Falls Knoten nicht festgehalten werden soll
	  if (nodes[i].move)
	    {
	      if (absForces[i] < temperature)
		{ 
		  shift.v[0] = forces[i].v[0];
		  shift.v[1] = forces[i].v[1];
		  shift.v[2] = forces[i].v[2];
		}
	      else
		{  
		  shift.v[0] = forces[i].v[0] / absForces[i] * temperature;
		  shift.v[1] = forces[i].v[1] / absForces[i] * temperature;
		  shift.v[2] = forces[i].v[2] / absForces[i] * temperature;
		}
	      nodes[i].coord += shift;
	    }
	}
    }   
}


FFCoord FFSpring::cool (FFCoord maxForce, FFCoord vibration, 
                        FFCoord &coolMaxForce1,
                        FFCoord &coolMaxForce2, FFCoord &coolMaxForce3, 
                        FFCoord &coolMaxForce4, int &coolPhase)
 ///////////////////////////////////////////////////////////////////
 //                                                               //
// Berechnet abhaengig von "maxForce" die fuer diesen Schleichen-//
// durchlauf maximal zulaessige Verschiebung ("temperature") der //
// Knoten. Nachdem der Graph zu schwingen angefangen hat, wird   //
// die maximale zulaessige Verschiebung erheblich niedriger ge-  //
// setzt, so dass eine Schwingung nicht mehr auftritt (else-     //
// Zweig).                                                       //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFCoord temperature;

  coolMaxForce4 = coolMaxForce3;
  coolMaxForce3 = coolMaxForce2;
  coolMaxForce2 = coolMaxForce1;
  coolMaxForce1 = maxForce;

  // Test, ob Graph schwingt 
  if ((coolMaxForce4 != 0.0) &&
      (fabs ((coolMaxForce1-coolMaxForce3) / coolMaxForce1) < vibration) &&
      (fabs ((coolMaxForce2-coolMaxForce4) / coolMaxForce2) < vibration))
    coolPhase = 2;

  if (coolPhase == 1)
    temperature = sqrt (maxForce);
  else          
    temperature = sqrt (maxForce) / 15;
    
  return (temperature);
}


void FFSpring::write_nodePosition (int anzN )
///////////////////////////////////////////////////////////////////
//                                                               //
// Uebertraegt die neuen Koordinaten der Knoten in den           //
// urspruenglich uebergebenen Graphen.                           //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFGraphicNode *gn;

  for (int i = 1; i <= anzN; i++)
    {
     // cerr << "Moving node to: " << nodes[i].coord() << endl;
     
     gn = (*nodes[i].node) ();
     if (gn)
      gn->moveTo (nodes[i].coord);  
    }
}


void FFSpring::change_optDist (FFCoord &optimalDistance)
///////////////////////////////////////////////////////////////////
//                                                               //
// Passt "optimalDistance" an den Wert der Kantengewichte an.    //
// Da sich "optimalDistance" auf ein Kantengewicht von 1.0       //
// bezieht, wuerden hohe Kantengewichte den Graphen sehr eng um  //
// einen Punkt ziehen. Dies wird durch eine Anpassung von        //
// "optimalDistance" verhindert.                                 //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFCoord        maxWeight;
  FFSet <FFEdge> allEdges;
  Pix            p;

  graph->allEdges (allEdges);
  p = allEdges.first ();
  if (p != 0)
    {
      maxWeight = allEdges (allEdges.first ())->getWeight ();
      
      for (p = allEdges.first (); p != 0; allEdges.next (p))
	if (allEdges (p)->getWeight () > maxWeight)
	  maxWeight = allEdges (p)->getWeight ();
      
      if (maxWeight == 0.0)
	maxWeight = 1.0;
      if ((optimalDistance / maxWeight) < 5.0)
	optimalDistance = (sqrt (maxWeight*maxWeight)) * 5.0;
    }
    
  if(optimalDistance > 5000)
   ffBreak();
  
}


void FFSpring::springembedder_layout (const FFSet <FFNode> &locNodes)
///////////////////////////////////////////////////////////////////
//                                                               //
//               Hauptprozedur des Algorithmus                   //
//                                                               //
///////////////////////////////////////////////////////////////////
{
 FFPoint        *forces;        
 //   FSSpringNodes  *nodes;   // Knoten werden in einem Array gespeichert 
 // (schneller Zugriff)
 FFCoord        *absForces, 
  actMaxForce, temperature, 
  maxForce, 
  optimalDistance = optDistance, 
  coolMaxForce1 = 0.0,
  coolMaxForce2 = 0.0,
  coolMaxForce3 = 0.0,
  coolMaxForce4 = 0.0;
 int            anzN, maxIter,
   iterationCount = 0, 
  iteration      = 0,
  coolPhase      = 1;
 
 anzN = graph->getNumNodes ();

 forces    = new FFPoint [anzN + 1];
 absForces = new FFCoord [anzN + 1];
 nodes     = new FSSpringNodes [anzN + 1];
 
  // Bestimme entsprechend Qualitaet und Graphgroesse "maxForce" 
  // und "maxIter"
  if (quality > 0)
    {
      maxForce = 50.5 - (5.0 * quality);
      maxIter  = int ((5000 * quality) / anzN);  
      if (maxIter < 30)
	maxIter = 30;
    }
  else if(quality == 0)
    {
      maxForce = 0.5;
      maxIter = 0;
      // cerr << "Doing infinite ...." << endl;
    }
  else
    {
      maxForce = 3.0;
      maxIter = maxIteration;
    }
    
  // Initialisierungen
  // stopBit = NO;
  change_optDist (optimalDistance);
  init_forces (forces, anzN);
  init_FSSpringNodes (locNodes);

  // Berechnung ...
  if (useEdgeWeight)
    compute_forces_edges (optimalDistance, forces, anzN);
  else
    compute_forces (optimalDistance, forces, anzN);

  actMaxForce = compute_maxForce (forces, anzN, absForces);  

  // ... solange wiederholen, bis gute Verteilung oder Abbruchkriterium
  // erreicht
  while ((((actMaxForce > maxForce) && (iterationCount < maxIter)) ||
          (maxIter == 0)) &&
         (stopBit == NO))
    {
      init_forces (forces, anzN);
      if (useEdgeWeight)
        compute_forces_edges (optimalDistance, forces, anzN);
      else
        compute_forces (optimalDistance, forces, anzN);
      
      actMaxForce = compute_maxForce (forces, anzN, absForces);
      temperature = cool (actMaxForce, 0.3, coolMaxForce1, 
                          coolMaxForce2, coolMaxForce3, coolMaxForce4, 
                          coolPhase);
      
      compute_nodePosition (forces, anzN,  temperature, absForces);
      iterationCount++;
      iteration++;

      // Falls Animation waehrend der Berechnung,
      // nach bestimmten Berechnungsdurchlaeufen
      // die Koordinaten der Knoten im Graphikfenster aktualisieren.
      if ((animation && (iteration == animationIntervals)) ||
          ((maxIter == 0) && (iteration == 100)))
        {
         write_nodePosition (anzN);
         if(graph->getWindow() != NULL)
          graph->getWindow()->update();
         
         iteration = 0;
        } 
      if(inspector != NULL)
       inspector->update();
      
    }

  // Ergebnis als Koordinaten fuer Knoten im Graphen uebertragen.
  write_nodePosition (anzN);

  delete[] absForces;
  delete[] forces;
  delete[] nodes; 
  nodes = 0;
}


void FFSpring::springembedder (const FFSet<FFNode>& locNodes)
///////////////////////////////////////////////////////////////////
//                                                               //
// Der Graph wird fuer den Algorithmus vorbereitet (Entfernen    //
// reflexiver und paralleler Kanten). Das eigentliche Layout     //
// des Graphen (der Teilgraphen) erfolgt in der Prozedur         //
// "springembedder_layout".                                      //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  if (graph->getNumNodes () > 0)
    {
      if (graph->getNumNodes () > 1)
        { 
          // Vorbereitung und Layout fuer Graphen mit mehreren Knoten
          // (Kanten begradigen, Knoten durchgehend numerieren)
          graph->straightenEdges ();
          number_nodes (graph, 0);  
          springembedder_layout (locNodes); 
        }
      else
       {  
        // Nur ein Knoten zu layouten, also diesen Knoten an den Offset-
        // Punkt setzen.
        const FFSet <FFNode>& allNodes= graph->allNodes();
        FFGraphicNode  *gn= (*allNodes (allNodes.first ())) ();
        if (gn)
         gn->moveTo (offset);
       }
    }
  
  if (graph->getWindow () != NULL)
   graph->getWindow ()->adjustWindowSize ();
}

