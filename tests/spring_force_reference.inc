// Frozen pre-optimization kernels, 3 November 2026; differential compatibility oracle.
// Original copyright Carsten Friedrich, Universitaet Passau (1994).
void ReferenceForces::compute_forces (FFCoord optimalDistance, 
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


void ReferenceForces::compute_forces_edges (FFCoord optimalDistance, 
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
                   referenceBreak();
                  
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
                   referenceBreak();

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


