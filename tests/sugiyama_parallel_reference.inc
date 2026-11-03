// Frozen endpoint-scan preprocessing; Falk Schreiber, 1994-97.
void reference_rem_par_edges (SNode *nodes, SEdge *edges, 
		    int anzE, int &anzPar, int type,
		    DLList <int> &parEdges)
///////////////////////////////////////////////////////////////////
//                                                               //
// Markiert (und loescht aus temporaerer Struktur) parallele     // 
// Kanten.                                                       //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int e, 
    j, 
    source, 
    target;
  Pix q;

  anzPar = 0;
  for (j = 1; j <= anzE; j++)
    if (edges[j].type == 0)
      for (e = 1; e <= anzE; e++)
	{

	  // Bereits geloeschte Kanten nicht noch einmal beruecksichtigen
	  if ((e != j) &&
	      (edges[e].type == 0) &&
	      (edges[e].source == edges[j].source) &&
	      (edges[e].target == edges[j].target))
	    {
	      anzPar++;
	      edges[e].type = type;
	      // type ist 2 oder 3, je nachdem, ob es sich um eine 
	      // echte parallele Kante handelt oder eine parallele Kante
	      // nach Umdrehen der Rueckwaertskanten
	      parEdges.append (e);
	      source = edges[e].source;
	      target = edges[e].target;

	      // Succ-Knoten loeschen (nodes aktualisieren)
	      q = nodes[source].succ.first ();
	      while (q != 0)
		{
		  if (nodes[source].succ (q) == target)
		    {
		      nodes[source].succ.del (q);
		      q = 0;
		      (nodes[source].outdeg)--;
		    }
		  else
		    nodes[source].succ.next (q);
		}
	      
	      // Pred-Knoten loeschen
	      q = nodes[target].pred.first ();
	      while (q != 0)
		{
		  if (nodes[target].pred (q) == source)
		    {
		      nodes[target].pred.del (q);
		      q = 0;
		      (nodes[target].indeg)--;
		    }
		  else
		    nodes[target].pred.next (q);
		}
	    }
	}
}  

