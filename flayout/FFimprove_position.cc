/*
// Copyright 1994-97 Falk Schreiber, Universitaet Passau
//
*/

#include <string.h>

#include <ffgraph/FFimprove_position.h>

void move_level (SNode *nodes, int anzND, FFCoord actx, int level, 
                 FFCoord distance, int anzN)
///////////////////////////////////////////////////////////////////
//                                                               //
// Die Knoten, die zu einem Level kleiner als "level" gehoeren   //
// und eine groessere (die gleiche) x-Koordinate als "actx"      //
// haben, werden um "distance" verschoben.                       //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int n,
    l;

  // Normale Knoten und einfache Dummies
  for (n = 1; n <= anzND; n++)
    if ((nodes[n].point.v[0] >= actx) &&
        (nodes[n].level < level))
      nodes[n].point.v[0] += distance;

  // Hier die posList der dummy-Knoten behandeln
  for (n = anzN; n <= anzND; n++)
    if (nodes[n].anzDum == 2) 
      if (nodes[n].level < level)
	{
	  l = nodes[n].level+1;
	  while ((l < level) && (l <= nodes[n].elevel))
	    {
	      if (nodes[n].posList[l].v[0] >= actx)
		nodes[n].posList[l].v[0] += distance;
	      l++;
	    }
	}
}

void make_node_list_up (int *nodeArray, SNode *nodes, 
			int lev, int *nodeNr, int *priority, 
                        int *done, int newmaxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Bestimmt im betrachteten Level fuer jeden Knoten die Priori-  //
// taet.                                                         //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i;

  // Knoten der Reihenfolge von links her betrachten
  for (i = 1; i <= nodeArray[lev*(newmaxS+1)]; i++)
    {
      nodeNr[i] = nodeArray[(newmaxS+1)*lev+i];
      done[i] = 0;
      if (nodes[nodeNr[i]].dummy == 0)
        {
          if (nodes[nodeNr[i]].outdeg > 0)
            priority[i] = nodes[nodeNr[i]].indeg + 1; 
          else
            priority[i] = 1;
        }
      else
        priority[i] = nodeArray[(newmaxS+1)*(lev+1)] + 2;
    }
}
    
void make_node_list_down (int *nodeArray, SNode *nodes, 
			  int lev, int *nodeNr, int *priority, 
			  int *done, int maxL, int newmaxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Bestimmt im betrachteten Level fuer jeden Knoten die          //
// Prioritaet.                                                   //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i;

  // Knoten der Reihenfolge von links her betrachten.
  for (i = 1; i <= nodeArray[(newmaxS+1)*lev]; i++)
    {
      nodeNr[i] = nodeArray[(newmaxS+1)*lev+i];
      done[i] = 0;
          
      if (nodes[nodeNr[i]].dummy == 0)
        priority[i] = nodes[nodeNr[i]].outdeg + 1;        
      else
        {
          if (lev < maxL)
            priority[i] = nodeArray[(newmaxS+1)*(lev+1)] + 2;
          else
            priority[i] = nodeArray[(newmaxS+1)*lev] + 2;	    
        }
    }
}	

int find_next_down (int nodesL, int *priority, int *done)
///////////////////////////////////////////////////////////////////
//                                                               //
// Liefert den Index des noch nicht bearbeiteten Elements mit    //
// der hoechsten Prioritaet von rechts.                          //
//                                                               //
///////////////////////////////////////////////////////////////////

{
  int index = 1, 
    highestPriority = 0,
    i;
		
  for (i = 1; i <= nodesL; i++)
    if ((done[i] == 0) && (priority[i] > highestPriority))
      {
        index = i;  
        highestPriority = priority[i];
      }

  return (index);
}

int find_next_up (int nodesL, int *priority, int *done)
///////////////////////////////////////////////////////////////////
//                                                               //
// Liefert den Index des noch nicht bearbeiteten Elements mit    //
// der hoechsten Prioritaet von links.                          //
//                                                               //
///////////////////////////////////////////////////////////////////

{
  int index = 1, 
    highestPriority = 0,
    i;  
		
  for (i = 1; i <= nodesL; i++)
    if ((done[i] == 0) && (priority[i] >= highestPriority))
      {
        index = i;  
        highestPriority = priority[i];
      }

  return (index);
}
	
int upper_barycenter (int node, SNode *nodes, int level)
///////////////////////////////////////////////////////////////////
//                                                               //
// Mittelwert der Koordinate x ueber aller Vorgaengerknoten.     //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int result = 0;
  Pix p;
  int actCoord = (int) nodes[node].point.v[0];

  for (p = nodes[node].pred.first (); p != 0; nodes[node].pred.next (p))
    result += (int) nodes[nodes[node].pred (p)].point.v[0];
  
  // Einfache Dummies bleiben unter Umstaenden dort, wo sie stehen. 
  if ((nodes[node].anzDum == 1) && (result < actCoord))
    return (actCoord);
  
  if (nodes[node].anzDum == 2)
  {
    if (nodes[node].level != level)
      {
	if (nodes[node].level+1 == level)
	  result = actCoord;
	else
	  result = (int) nodes[node].posList[level-1].v[0];

	// nodes[node].posList[level].v[0] tritt bei langen (Dummy-) Kanten
	// auf und entspricht actCoord fuer normale Knoten
	if (result < nodes[node].posList[level].v[0])
	  return ((int) nodes[node].posList[level].v[0]);
	else 
	  return (result);
      }
    else
      {
	// also (nodes[node].level == level)
	// falls wir am letzten Knoten einer langen Dummy-Kante sind 
	if ((nodes[node].edgeNum == nodes[node-1].edgeNum) &&
	    (nodes[node-1].level != level-1))
	  return ((int) nodes[node-1].posList[level-1].v[0]);
	if (result < actCoord)
	  return (actCoord);
      }
  }
  // Division durch 0 vermeiden!
  if ((result == 0) && (nodes[node].indeg == 0))
    return (0);
  if ((result == 0) && (nodes[node].indeg != 0))
    return (1);
  else 	
    return (result / nodes[node].indeg);
}

int lower_barycenter (int node, SNode *nodes, int level)
///////////////////////////////////////////////////////////////////
//                                                               //
// Mittelwert der Koordinate x ueber aller Nachfolgeknoten.      //
// Dabei Beruecksichtigung, dass Kanten ueber mehrere Level      //
// moeglichst geradlinig zu zeichnen sind und bestimmte Knoten   //
// in ihrer Position nicht mehr veraendert werden sollen.        //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int result = 0;
  FFSet <int> predSuccN;
  Pix ps,
    pp;
  int tmpN;
  
  // Falls Dummies ueberlanger Kanten
  if (nodes[node].anzDum == 2)
  {
    if (nodes[node].level < level)
      {
	if (nodes[node].elevel == level)
	  return ((int) nodes[node+1].point.v[0]);
	else
	  return ((int) nodes[node].posList[level+1].v[0]);
      }
    else
      {
	// Falls "node" der erste von zwei aufeinanderfolgenden 
	// dummies ist
	if ((nodes[node].level == level) &&
	    (nodes[node].edgeNum == nodes[node+1].edgeNum) &&
	    (nodes[node+1].level == level+1))
	  return ((int) nodes[node+1].point.v[0]);
	if ((nodes[node].level == level) &&
	    (nodes[node].edgeNum == nodes[node+1].edgeNum) &&
	    (nodes[node+1].level != level+1))
	  return ((int) nodes[node].posList[level+1].v[0]);
      }
  }

  // Gib 0 zurueck, wenn ein Knoten nur einen Nachfolger hat, 
  // dieser Nachfolger jedoch mehrere Vorgaenger besitzt und 
  // ausserdem seine Position durch make_up nicht veraendert wurde.
  if (nodes[node].outdeg == 1) 
    {
      ps = nodes[node].succ.first ();
      tmpN = nodes[node].succ (ps);

      if (nodes[tmpN].indeg > 1)
	{
	  // Check *all* predecessors of the successor: if it lies left of
	  // any of them, return 0 (= no preference, node stays put).
	  // Otherwise fall through to the normal barycenter computation
	  // (which yields the successor's x for outdeg == 1).
	  // NOTE: originally an 'else return <succ x>' inside the loop made
	  // it stop after the first predecessor.
	  for (pp = nodes[tmpN].pred.first (); pp != 0; 
	       nodes[tmpN].pred.next (pp))
	    if (nodes[tmpN].point.v[0] < nodes[nodes[tmpN].pred (pp)].point.v[0])
	      return (0);
	}
    }
    
  for (ps = nodes[node].succ.first (); ps != 0; nodes[node].succ.next (ps))
    result += (int) nodes[nodes[node].succ (ps)].point.v[0];

  if (result == 0)
    return (0);
  else 	
    {
      ps = nodes[node].succ.first ();
      if (ps != 0)
	if ((nodes[node].anzDum == 2) &&
	    (nodes[nodes[node].succ (ps)].dummy == 0))
	  return ((int) (nodes[node].point.v[0]));
    }

  return (result / nodes[node].outdeg);
}

void make_down (int *nodeArray, int lev, SNode *nodes, int *nodeNr, 
		int *priority, int *done, int minDiff,
		int anzND, int newmaxS, int maxL, int anzN)
///////////////////////////////////////////////////////////////////
//                                                               //
// Positioniert die Knoten des Levels 'lev'. Beruecksichtigt,    //
// ob die Knoten und die Kantenstuetzpunkte auf Gitterpunkte     //
// gesetzt werden sollen oder nicht.                             //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i, 
    index = 1, // aktueller Index in nodeNr
    j, 
    d, 
    k, 
    origMinDiff = minDiff, 
    tmpN, // aktueller Knoten
    predN, // Knoten im Vorgaengerlevel
    nodesL = nodeArray[(newmaxS+1)*lev], // Knoten im aktuellen Level
    level;
  FFCoord optimalPosition, 
    distance, 
    possibleDistance,
    actPosition, 
    actx,
    predPos = 0.0, // Position eines Knotens im Level lev-1
    node_j,
    node_jm,
    node_jp;
  BOOL move;
  Pix p;

  for (i = 1; i <= nodesL; i++)
    {	
      index = find_next_down (nodesL, priority, done);
      tmpN = nodeNr[index];
      optimalPosition = upper_barycenter (tmpN, nodes, lev);
      
      // aktuelle Position
      if ((nodes[tmpN].anzDum == 2) &&
	  (nodes[tmpN].level != lev))
	actPosition = nodes[tmpN].posList[lev].v[0];
      else
	actPosition = nodes[tmpN].point.v[0];

      if (optimalPosition == 0)
        optimalPosition = actPosition;
   
      // Knoten rechts neben optimaler Position
      if (optimalPosition < actPosition)
        {
          distance = actPosition - optimalPosition;
          possibleDistance = 0;
          j = index;
          move = YES;

          do
            {
              if (move)
              {
		if (j > 1)
		  {
		    if (nodes[nodeNr[j-1]].dummy != 0) 
		      move = NO;
		    
		    // hier minimalen Abstand berechnen 		
		    minDiff = origMinDiff + 
		      (int) (nodes[nodeNr[j-1]].width / 2.0) +
		      (int) (nodes[nodeNr[j]].width / 2.0);
		    
		    // Hier fuer Knoten aktuelle Position bestimmen,
		    // abhaengig von echter Position oder Position in
		    // Positionsliste fuer dummies.
		    if ((nodes[nodeNr[j]].anzDum == 2) &&
			(nodes[nodeNr[j]].level != lev))
		      node_j = nodes[nodeNr[j]].posList[lev].v[0];
		    else
		      node_j = nodes[nodeNr[j]].point.v[0];
		    
		    if ((nodes[nodeNr[j-1]].anzDum == 2) &&
			(nodes[nodeNr[j-1]].level != lev))
		      node_jm = nodes[nodeNr[j-1]].posList[lev].v[0];
		    else
		      node_jm = nodes[nodeNr[j-1]].point.v[0];
		    
		    possibleDistance += (node_j -  node_jm - minDiff);
		  }
		else       // j == 1 , also kein Knoten links 
		  {
		    minDiff = origMinDiff +
		      (int) (nodes[nodeNr[j]].width / 2.0);
		    
		    if ((nodes[nodeNr[j]].anzDum == 2) &&
			(nodes[nodeNr[j]].level != lev))
		      node_j = nodes[nodeNr[j]].posList[lev].v[0];
		    else
		      node_j = nodes[nodeNr[j]].point.v[0];
		    
		    possibleDistance += (node_j - minDiff);
		  }
              }
              j--;
            }
          while ((j >= 1) && !(done[j]));
   
          if (possibleDistance < distance)
            distance = possibleDistance;

          j = index;
          while (distance > 0)
            {
              if (j == 1)
                d = (int) distance;
              else
                {
		  minDiff = origMinDiff + 
		    (int) (nodes[nodeNr[j]].width / 2.0) +
		    (int) (nodes[nodeNr[j-1]].width / 2.0);
                  
		  if ((nodes[nodeNr[j]].anzDum == 2) &&
		      (nodes[nodeNr[j]].level != lev))
		    node_j = nodes[nodeNr[j]].posList[lev].v[0];
		  else
		    node_j = nodes[nodeNr[j]].point.v[0];
		  
		  if ((nodes[nodeNr[j-1]].anzDum == 2) &&
		      (nodes[nodeNr[j-1]].level != lev))
		    node_jm = nodes[nodeNr[j-1]].posList[lev].v[0];
		  else
		    node_jm = nodes[nodeNr[j-1]].point.v[0];

                  d = (int) (node_j - node_jm - minDiff);
        
		  if (d > distance)
                    d = (int) distance;
                }
	      
              for (k = j; k <= index; k++)    
		if ((nodes[nodeNr[k]].anzDum == 2) &&
		    (nodes[nodeNr[k]].level != lev))
		  nodes[nodeNr[k]].posList[lev].v[0] = 
		    nodes[nodeNr[k]].posList[lev].v[0] - d;
		else
		  nodes[nodeNr[k]].point.v[0] =
		    nodes[nodeNr[k]].point.v[0] - d;
	      
              j--;
              distance -= d;
            } 

          // Falls Dummy-Knoten, und dieser steht nicht unterhalb seines 
          // Vorgaengers oder falls Knoten mit nur einem Vorgaenger und dieser
          // Vorgaenger ist Dummy-Knoten und der Knoten steht nicht
          // unter seinem Vorgaenger.
	  predN = 0;
	  if ((nodes[tmpN].anzDum == 2) &&
	      (nodes[tmpN].level == lev) &&
	      (nodes[tmpN].edgeNum == nodes[tmpN-1].edgeNum) &&
	      (nodes[tmpN-1].level != lev-1))
	    {
	      // also falls der Knoten der letzte Knoten einer ueberlangen 
	      // Dummy-Kante ist ...
	      predN = tmpN-1;
	      predPos = nodes[predN].posList[lev-1].v[0];
	    }
	  else
	    {
	      p = nodes[tmpN].pred.first ();          
	      if (p != 0)
		{
		  predN = nodes[tmpN].pred (p);
		  predPos = nodes[predN].point.v[0];
		}
	    }
	  
	  if (predN != 0)
	    if ((nodes[tmpN].dummy != 0) ||
		((nodes[predN].dummy != 0) &&
		 (nodes[tmpN].indeg == 1)))
	      {
		if ((nodes[tmpN].anzDum == 2) &&
		    (nodes[tmpN].level != lev))
		  node_j = nodes[tmpN].posList[lev].v[0];
		else
		  node_j = nodes[tmpN].point.v[0];
		
		if ((node_j > predPos) &&
		    (nodes[predN].outdeg == 1))
                    {
                      distance = node_j - predPos;
                      actx = node_j;
                      level = nodes[tmpN].level; 
                      
                      move_level (nodes, anzND, actx, level, distance, anzN);
                    }
	      }
	} 
      else
	{
	  // optimalPosition >= actPosition
	  // Knoten links seiner optimalen Position
        	
          distance = optimalPosition - actPosition;
          possibleDistance = 0;
          j = index;
          
	  do
	    {
	      if (j < nodesL)
		{
		  // hier min. Abstand berechnen 
		  minDiff = origMinDiff +
		    (int) (nodes[nodeNr[j]].width / 2.0) +
		    (int) (nodes[nodeNr[j+1]].width / 2.0);
		  
		  // Hier fuer Knoten aktuelle Position bestimmen,
		  // abhaengig von echter Position oder Position in
		  // Positionsliste fuer dummies.
		  if ((nodes[nodeNr[j]].anzDum == 2) &&
		      (nodes[nodeNr[j]].level != lev))
		    node_j = nodes[nodeNr[j]].posList[lev].v[0];
		  else
		    node_j = nodes[nodeNr[j]].point.v[0];
		  
		  if ((nodes[nodeNr[j+1]].anzDum == 2) &&
		      (nodes[nodeNr[j+1]].level != lev))
		    node_jp = nodes[nodeNr[j+1]].posList[lev].v[0];
		  else
		    node_jp = nodes[nodeNr[j+1]].point.v[0];
		  
		  possibleDistance += (node_jp - node_j - minDiff);
		}	
	      else  
		{
		  //  j == nodesL, also kein Knoten rechts
		  possibleDistance += distance;
		}
	      j++;
	    }
	  while ((j <= nodesL) && !(done[j]));
	  
	  if (possibleDistance < distance)
	    distance = possibleDistance;
	  
	  j = index;
	  while (distance > 0)
	    {
	      if (j == nodesL)
		d = (int) distance;
	      else
		{
		  minDiff = origMinDiff + 
		    (int) (nodes[nodeNr[j]].width / 2.0) +
		    (int) (nodes[nodeNr[j+1]].width / 2.0);
		  
		  // Hier fuer Knoten aktuelle Position bestimmen,
		  // abhaengig von echter Position oder Position in
		  // Positionsliste fuer dummies.
		  if ((nodes[nodeNr[j]].anzDum == 2) &&
		      (nodes[nodeNr[j]].level != lev))
		    node_j = nodes[nodeNr[j]].posList[lev].v[0];
		  else
		    node_j = nodes[nodeNr[j]].point.v[0];
		  
		  if ((nodes[nodeNr[j+1]].anzDum == 2) &&
		      (nodes[nodeNr[j+1]].level != lev))
		    node_jp = nodes[nodeNr[j+1]].posList[lev].v[0];
		  else
		    node_jp = nodes[nodeNr[j+1]].point.v[0];

		  d = (int) (node_jp - node_j - minDiff);
		  if (d > distance)
		    d = (int) distance;
		}
	      
	      // Alle folgenden Knoten ebenfalls verschieben
	      for (k = index; k <= j; k++)
		if ((nodes[nodeNr[k]].anzDum == 2) &&
		    (nodes[nodeNr[k]].level != lev))
		  nodes[nodeNr[k]].posList[lev].v[0] = 
		    nodes[nodeNr[k]].posList[lev].v[0] + d;
		else
		  nodes[nodeNr[k]].point.v[0] =
		    nodes[nodeNr[k]].point.v[0] + d;
	      
	      j++;
	      distance -= d;
	    }
	}
      done[index] = 1;
    }
}

void make_up (int *nodeArray, int lev, SNode *nodes, int *nodeNr, 
	      int *priority, int *done, int minDiff, 
	      int newmaxS, int anzN)
///////////////////////////////////////////////////////////////////
//                                                               //
// Positioniert die Knoten des Levels 'lev'. Beruecksichtigt,    //
// ob die Knoten und die Kantenstuetzpunkte auf Gitterpunkte     //
// gesetzt werden sollen oder nicht.                             //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i, 
    index, 
    j, 
    d, 
    k, 
    origMinDiff = minDiff, 
    nodesL = nodeArray[(newmaxS+1)*lev], // Zahl der Knoten im 
                                         // aktuellen Level
    tmpN; // aktueller Knoten
  FFCoord optimalPosition, 
    distance, 
    possibleDistance,
    actPosition,
    node_j,
    node_jp,
    node_jm;
  BOOL move;

  for (i = 1; i <= nodesL; i++)
    {	
      index = find_next_up (nodesL, priority, done);
      tmpN = nodeNr[index];
      optimalPosition = lower_barycenter (tmpN, nodes, lev);
      
      // aktuelle Position
      if ((nodes[tmpN].anzDum == 2) &&
	  (nodes[tmpN].level != lev))
	actPosition = nodes[tmpN].posList[lev].v[0];
      else
	actPosition = nodes[tmpN].point.v[0];

      if (optimalPosition == 0)
        optimalPosition = actPosition;

      // Knoten rechts neben optimaler Position
      if (optimalPosition < actPosition)
        {
          distance = actPosition - optimalPosition;
          possibleDistance = 0;
          j = index;
          move = YES;
	  
          do
            {
              if (move)
              {
		if (j > 1)
		  {
		    if (nodes[nodeNr[j-1]].dummy != 0) 
		      move = NO;

		    // hier minimalen Abstand berechnen 
		    minDiff = origMinDiff + 
		      (int) (nodes[nodeNr[j-1]].width / 2.0) +
		      (int) (nodes[nodeNr[j]].width / 2.0);
		    
		    // Hier fuer Knoten aktuelle Position bestimmen,
		    // abhaengig von echter Position oder Position in
		    // Positionsliste fuer dummies.
		    if ((nodes[nodeNr[j]].anzDum == 2) &&
			(nodes[nodeNr[j]].level != lev))
		      node_j = nodes[nodeNr[j]].posList[lev].v[0];
		    else
		      node_j = nodes[nodeNr[j]].point.v[0];
		    
		    if ((nodes[nodeNr[j-1]].anzDum == 2) &&
			(nodes[nodeNr[j-1]].level != lev))
		      node_jm = nodes[nodeNr[j-1]].posList[lev].v[0];
		    else
		      node_jm = nodes[nodeNr[j-1]].point.v[0];
		    
		    possibleDistance += (node_j - node_jm - minDiff);
		  }
		else       // j == 1 , also kein Knoten links 
		  {
		    minDiff = origMinDiff +
		      (int) (nodes[nodeNr[j]].width / 2.0);
		    
		    if ((nodes[nodeNr[j]].anzDum == 2) &&
			(nodes[nodeNr[j]].level != lev))
		      node_j = nodes[nodeNr[j]].posList[lev].v[0];
		    else
		      node_j = nodes[nodeNr[j]].point.v[0];
		    
		    possibleDistance += (node_j - minDiff);
		  }
              }
              j--;
            }
          while ((j >= 1) && !(done[j]));
	  
          if (possibleDistance < distance)
            distance = possibleDistance;		
          j = index;
          while (distance > 0)
            {
              if (j == 1)
                d = (int) distance;
              else
                {
		  minDiff = origMinDiff +
		    (int) (nodes[nodeNr[j-1]].width / 2.0) +
		    (int) (nodes[nodeNr[j]].width / 2.0);

		  // Hier fuer Knoten aktuelle Position bestimmen,
		  // abhaengig von echter Position oder Position in
		  // Positionsliste fuer dummies.
		  if ((nodes[nodeNr[j]].anzDum == 2) &&
		      (nodes[nodeNr[j]].level != lev))
		    node_j = nodes[nodeNr[j]].posList[lev].v[0];
		  else
		    node_j = nodes[nodeNr[j]].point.v[0];
		  
		  if ((nodes[nodeNr[j-1]].anzDum == 2) &&
		      (nodes[nodeNr[j-1]].level != lev))
		    node_jm = nodes[nodeNr[j-1]].posList[lev].v[0];
		  else
		    node_jm = nodes[nodeNr[j-1]].point.v[0];	      
		  
                  d = (int) (node_j - node_jm - minDiff);
                  if (d > distance)
                    d = (int) distance;
                }

	      // Alle vorhergehenden Knoten mit verschieben
              for (k = j; k <= index; k++)   
		if ((nodes[nodeNr[k]].anzDum == 2) &&
		    (nodes[nodeNr[k]].level != lev))
		  nodes[nodeNr[k]].posList[lev].v[0] = 
		    nodes[nodeNr[k]].posList[lev].v[0] - d;
		else
		  nodes[nodeNr[k]].point.v[0] =
		    nodes[nodeNr[k]].point.v[0] - d;
	      
              j--;
              distance -= d;
            } 
        }
      else 
	{
	  // optimalPosition >= actPosition 
	  // Knoten links neben optimaler Position
          distance = optimalPosition - actPosition;
          possibleDistance = 0;
          j = index;
          move = YES;
	  
          do
            {
              if (move)
              {
		if (j < nodesL)
		  {
		    if ((nodes[nodeNr[j]].dummy != 0) && 
			(index != j))
		      move = NO;
		    else
		      {
			// hier minimalen Abstand berechnen 
			minDiff = origMinDiff +
			  (int) (nodes[nodeNr[j]].width / 2.0) +
			  (int) (nodes[nodeNr[j+1]].width / 2.0);
			
			// Hier fuer Knoten aktuelle Position bestimmen,
			// abhaengig von echter Position oder Position in
			// Positionsliste fuer dummies.
			if ((nodes[nodeNr[j]].anzDum == 2) &&
			    (nodes[nodeNr[j]].level != lev))
			  node_j = nodes[nodeNr[j]].posList[lev].v[0];
			else
			  node_j = nodes[nodeNr[j]].point.v[0];
			
			if ((nodes[nodeNr[j+1]].anzDum == 2) &&
			    (nodes[nodeNr[j+1]].level != lev))
			  node_jp = nodes[nodeNr[j+1]].posList[lev].v[0];
			else
			  node_jp = nodes[nodeNr[j+1]].point.v[0];
			
			possibleDistance += (node_jp - node_j - minDiff);
		      }
		  }      
		else
		  {
		    //  j == nodesL, 
		    // also kein Knoten rechts
		    if ((index == nodesL) || (nodes[nodeNr[nodesL]].dummy == 0))
		      possibleDistance += distance;
		  }
              }
              j++;
            }
          while ((j <= nodesL) && !(done[j]));
	  
          if (possibleDistance < distance)
            distance = possibleDistance;
          j = index;
          while (distance > 0)
            {
              if (j == nodesL)
                d = (int) distance;
              else
                {
		  minDiff = origMinDiff + 
		    (int) (nodes[nodeNr[j]].width / 2.0) +
		    (int) (nodes[nodeNr[j+1]].width / 2.0);

		  if ((nodes[nodeNr[j]].anzDum == 2) &&
		      (nodes[nodeNr[j]].level != lev))
		    node_j = nodes[nodeNr[j]].posList[lev].v[0];
		  else
		    node_j = nodes[nodeNr[j]].point.v[0];
		  
		  if ((nodes[nodeNr[j+1]].anzDum == 2) &&
		      (nodes[nodeNr[j+1]].level != lev))
		    node_jp = nodes[nodeNr[j+1]].posList[lev].v[0];
		  else
		    node_jp = nodes[nodeNr[j+1]].point.v[0];
		  
                  d = (int) (node_jp - node_j - minDiff);
                  if (d > distance)
                    d = (int) distance;
                }

	      // Alle folgenden Knoten mit verschieben
	      for (k = index; k <= j; k++)
		if ((nodes[nodeNr[k]].anzDum == 2) &&
		    (nodes[nodeNr[k]].level != lev))
		  nodes[nodeNr[k]].posList[lev].v[0] = 
		    nodes[nodeNr[k]].posList[lev].v[0] + d;
		else
		  nodes[nodeNr[k]].point.v[0] =
		    nodes[nodeNr[k]].point.v[0] + d;
                
              j++;
              distance -= d;
            }
        }
      done[index] = 1;
    }
}

void correct_lines (SNode *nodes, int *nodeArray, int minDiff, int anzN, 
		    int anzND, FFCoord offSet, int maxL, int newmaxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Leerraeume zwischen Kanten werden beseitigt.                  //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i, 
    j,
    l;
  FFCoord x = offSet, 
    nextPos,
 //   nodePos,
    maxPos = 0.0;

  // Bestimme maximale x-Position
  for (i = 1; i <= anzND; i++)
    {
      if (nodes[i].point.v[0] > maxPos)
	maxPos = nodes[i].point.v[0];
      if (nodes[i].anzDum == 2)
	for (l = nodes[i].level+1; l <= nodes[i].elevel; l++)
	  if (nodes[i].posList[l].v[0] > maxPos)
	    maxPos = nodes[i].posList[l].v[0];
    }
  
  while (x < maxPos)
    {
      j = 1;
      
      while (j <= anzND)
        {
	  if (nodes[j].point.v[0] == x)
	    {
	      x = x + minDiff;
	      j = 0;
	    }
	  if ((j != 0) && (nodes[j].anzDum == 2))
	    for (l = nodes[j].level+1; l <= nodes[j].elevel; l++)
	      {
		if (nodes[j].posList[l].v[0] == x)
		  {
		    x = x + minDiff;
		    j = 0;
		  } 
	      }
	  j++;
	}
      
      if (x < maxPos)
        {
          nextPos = maxPos;
          for (i = 1; i <= anzND; i++)
	    {
	      if ((nodes[i].point.v[0] > x) && 
		  (nodes[i].point.v[0] < nextPos))
		nextPos = nodes[i].point.v[0];

	      if (nodes[i].anzDum == 2)
		for (l = nodes[i].level+1; l <= nodes[i].elevel; l++)
		  if ((nodes[i].posList[l].v[0] > x) && 
		      (nodes[i].posList[l].v[0] < nextPos))
		    nextPos = nodes[i].posList[l].v[0];
	    }
	  
          nextPos = nextPos - x;
          for (i = 1; i <= anzND; i++)
	    {
	      if (nodes[i].point.v[0] > x)
		nodes[i].point.v[0] = nodes[i].point.v[0] - nextPos;
	      
	      if (nodes[i].anzDum == 2)
		for (l = nodes[i].level+1; l <= nodes[i].elevel; l++)
		  if (nodes[i].posList[l].v[0] > x)  
		    nodes[i].posList[l].v[0] = nodes[i].posList[l].v[0] 
		      - nextPos;
	    }
	    
          x = x + minDiff;
        }
    }  
}

int improve_positions (int *nodeArray, SNode *nodes, int maxL, 
		       int anzN, int dummies, FFCoord xDistance, 
		       FFCoord yDistance, FFPoint offset, int orient, 
		       BOOL grid, int sameSize, int newmaxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Bestimmt fuer die Knoten levelweise die beste Position und    //
// positioniert die Knoten.                                      //
// Rueckgabewert 1 bei Fehler.                                   //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFCoord min,
    max = 0.0, // Fuer max. Groesse eines Knotens
    h;
//    test;
  int i,
    j,
    l,
    tmpN,
    anzND = anzN+dummies, // anzND - Anzahl echte Knoten und Dummy-Knoten
    minDiff = (int) xDistance, // minimaler Abstand zwischen Knoten
    *nodeNr, 
    *priority,
    *done;
  FFPoint actOff = offset;
  Pix p;

  priority = new int [anzND+1];
  if (priority == 0)
    return (1);

  done = new int [anzND+1];
  if (done == 0)
    {
      delete[] priority;
      return (1);
    }

  nodeNr = new int [anzND+1];
  if (nodeNr == 0)
    {
      delete[] done;
      delete[] priority;
      return (1);
    }

  // maximale Position der dummy-Knoten bestimmen
  for (l = 0; l <= maxL; l++)
    for (i = 1; i <= nodeArray[l*(newmaxS+1)]; i++)
      if (nodes[nodeArray[l*(newmaxS+1)+i]].maxPos < i)
	nodes[nodeArray[l*(newmaxS+1)+i]].maxPos = i;
  
  // Falls der einem Dummy folgende Knoten groessere maxPos hat,
  // dann diese verwenden
  for (i = anzN; i <= anzN + dummies; i++)
    {
      p = nodes[i].succ.first ();
      if (p != 0)
	{
	  tmpN = nodes[i].succ (p);
	  
	  if ((nodes[tmpN].dummy == 0) && 
	      (nodes[i].maxPos < nodes[tmpN].maxPos))
	    nodes[i].maxPos = nodes[tmpN].maxPos;
	}
    }

  // Dummy-Knoten aktualisieren (falls 2 dummies pro Kante)
  for (i = anzN; i <= anzN + dummies; i++)
    if (nodes[i].anzDum == 2)
      {
	if (nodes[i].maxPos < nodes[i+1].maxPos)
	  nodes[i].maxPos = nodes[i+1].maxPos;
	else
	  nodes[i+1].maxPos = nodes[i].maxPos;
	
	i++;
      }
  
  // Initiale Positionen setzen, dabei
  // levelweise durchgehen
  for (l = 0; l <= maxL; l++)
    {
      // Den ersten Knoten auf erste Position setzen 
      j = 1;

      // Alle Positionen durchgehen
      for (i = 1; i <= nodeArray[l*(newmaxS+1)]; i++)
	{
	  tmpN = nodeArray[l*(newmaxS+1)+i];

	  // Falls kein Dummy
	  if (nodes[tmpN].dummy == 0)
	    {
	      nodes[tmpN].point.v[0] = j;
	      nodes[tmpN].maxPos = j;
	      j++;
	    }
	  else
	    {
	      // Falls einfacher Dummy
	      if (nodes[tmpN].anzDum == 1)
		{
		  if (nodes[tmpN].maxPos >= j)
		    {
		      nodes[tmpN].point.v[0] = nodes[tmpN].maxPos;
		      j = nodes[tmpN].maxPos + 1;
		    }
		  else
		    {
		      nodes[tmpN].point.v[0] = j;
		      nodes[tmpN].maxPos = j;
		      j++;
		    }
		}
	      else
		{
		  // Nun die dummies fuer lange Kanten, also 
		  // anzDum == 2
		  if (nodes[tmpN].maxPos >= j)
		    {
		      if (nodes[tmpN].level == l)
			nodes[tmpN].point.v[0] = nodes[tmpN].maxPos;
		      else
			nodes[tmpN].posList[l].v[0] = nodes[tmpN].maxPos;
		      
		      j = nodes[tmpN].maxPos + 1;
		    }
		  else
		    {
		      // nodes[tmpN].anzDum == 2 und 
		      // die aktuelle Position ist groesser als die in 
		      // maxPos stehende
		      
		      if (nodes[tmpN].level == l)
			nodes[tmpN].point.v[0] = j;
		      else
			nodes[tmpN].posList[l].v[0] = j;

		      nodes[tmpN].maxPos = j;
			  
		      // Falls noch ein zweiter dummy dazugehoert und noch 
		      // nicht korrigiert wurde
		      if (nodes[tmpN].edgeNum ==
			  nodes[tmpN+1].edgeNum)
			nodes[tmpN+1].maxPos = j;
		    
		      j++;
		    }
		}
	    }
	}
    }

  if (orient == 2)
    {
      h = actOff.v[0];
      actOff.v[0] = actOff.v[1];
      actOff.v[1] = h;
      
      minDiff = (int) yDistance;
    }    
  
  for (i = 1; i <= anzN; i++)
    if (nodes[i].width > max)
      max = nodes[i].width;
  
  // Falls Knoten und Kantenstuetzpunkte nicht auf Gitterpunkte gesetzt 
  // werden sollen, bekommen diese komplett berechnete (absolute) Werte,
  // welche im folgenden verschoben werden.
  // Sollen Gitterpunkte benutzt werden, so erhalten Knoten und 
  // Kantenstuetzpunkte relative Werte, diese werden entsprechend der 
  // Flaechenausnutzung verschoben und erst danach in absolute Werte 
  // (Gitterpositionen) umgewandelt.
  if (grid == NO)
    {
      // Eine erste Plazierung vornehmen.
      for (i = 1; i <= anzND; i++)
	{
	  if (nodes[i].anzDum == 2)
	    {
	      nodes[i].point.v[0] = (nodes[i].point.v[0] - 1) * 
		(minDiff + (int) max) + actOff.v[0]; 
	      for (l = nodes[i].level+1; l <= nodes[i].elevel; l++)
		nodes[i].posList[l].v[0] = (nodes[i].posList[l].v[0] - 1) * 
		  (minDiff + (int) max) + actOff.v[0];
	    }
	  else
	    nodes[i].point.v[0] = (nodes[i].point.v[0] - 1) * 
	      (minDiff + (int) max) + actOff.v[0]; 
	}
    }
  else
    {
      minDiff = 1;
      
      for (i = 1; i <= anzN; i++)
	nodes[i].width = 0.0;
    }

  for (i = 1; i <= maxL; i++)
    {
      make_node_list_down (nodeArray, nodes, i, nodeNr, priority, done, 
			   maxL, newmaxS);
      make_down (nodeArray, i, nodes, nodeNr, priority, done, minDiff, 
		 anzND, newmaxS, maxL, anzN);
    }

  for (i = maxL-1; i >= 0; i--)
    {
      make_node_list_up (nodeArray, nodes, i, nodeNr, priority, done, 
			 newmaxS);
      make_up (nodeArray, i, nodes, nodeNr, priority, done, minDiff, newmaxS,
	       anzN); 
    }

  // Fuer Gitter
  if (grid != NO)
    {
      // Icongroesse beruecksichtigt
      if (sameSize == 0)
	max = 0.0;
      
      if (orient == 1)
	for (i = 1; i <= anzND; i++)
	  {
	    nodes[i].point.v[0] = (nodes[i].point.v[0] - 1) * 
	      (xDistance + max) + actOff.v[0]; 
	    if (nodes[i].anzDum == 2)
	      for (l = nodes[i].level+1; l <= nodes[i].elevel; l++)
		nodes[i].posList[l].v[0] = (nodes[i].posList[l].v[0] - 1) * 
		  (xDistance + max) + actOff.v[0]; 
	  }
      else
	{
	  for (i = 1; i <= anzND; i++)
	    {
	      nodes[i].point.v[0] = (nodes[i].point.v[0] - 1) * 
		(yDistance + max) + actOff.v[0]; 	   
	      if (nodes[i].anzDum == 2)
		for (l = nodes[i].level+1; l <= nodes[i].elevel; l++)
		  nodes[i].posList[l].v[0] = (nodes[i].posList[l].v[0] - 1) * 
		    (yDistance + max) + actOff.v[0]; 
	    }
	}
    }
  
  // Positionierung des Graphen relativ zum Offset
  // Bestimme minimale x-Position
  min = nodes[1].point.v[0];
  for (i = 1; i <= anzND; i++)
    {
      if (nodes[i].point.v[0] < min)
	min = nodes[i].point.v[0];
      if (nodes[i].anzDum == 2)
	for (l = nodes[i].level+1; l <= nodes[i].elevel; l++)
	  if (nodes[i].posList[l].v[0] < min)
	    min = nodes[i].posList[l].v[0];
    }

  min = min - offset.v[0];
  // Falls min ungleich 0 (geringer Spielraum zugelassen)
  if ((min < -1.0) || (min > 1.0))
    for (i = 1; i <= anzND; i++)
      {
	nodes[i].point.v[0] = nodes[i].point.v[0] - min; 
	if (nodes[i].anzDum == 2)
	  for (l = nodes[i].level+1; l <= nodes[i].elevel; l++)
	    nodes[i].posList[l].v[0] = nodes[i].posList[l].v[0] - min; 
      }
  
  // Abstaende zwischen Kanten berichtigen
  // Bisher nur fuer Icongroesse nicht beruecksichtigt und 
  // Icongroesse maximal sowie Verwendung eines Gitters.
  if ((sameSize == 0) || 
      ((sameSize == 1) && (grid != NO)))
    {
      if (orient == 1)
	minDiff = (int) xDistance + (int) max;
      else
	minDiff = (int) yDistance + (int) max;
      
      correct_lines (nodes, nodeArray, minDiff, anzN, anzND, 
		     offset.v[0], maxL, newmaxS);
    } 

  delete[] nodeNr;
  delete[] done;
  delete[] priority;
  
  return (0);
}