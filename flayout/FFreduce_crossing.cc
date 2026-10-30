/*
// Copyright 1994-97 Falk Schreiber, Universitaet Passau
//
*/

#include <ffgraph/FFreduce_crossing.h>

void make_matrix (SNode *nodes, int level, int *matrix, int maxS, 
		  int newmaxS, int *nodeArray)
///////////////////////////////////////////////////////////////////
//                                                               //
// Erzeugt die Adjazenzmatrix zum Level "level" des Graphen.     //
// Dabei hat die Matrix folgendes aussehen:                      //
//           			   			         //
//                | 1.Knoten | 2.Knoten ... des Levels "level"+1 //  
// 1.Knoten       |				                 //
// 2.Knoten       |					         //
// 3.Knoten       |					         //
// ... 						                 //
// des Levels "level"				                 //
//  					                         //
// Inhalt: 1, falls Kante bzw. 0 sonst		                 //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  Pix p;
  int i, 
    n;

  // Da nicht alle Eintraege verwendet werden ...
  for (n = 0; n < maxS; n++)
    for (i = 0; i < maxS; i++)
      matrix[n*maxS + i] = 0;
  
  for (n = 1; n <= nodeArray[(newmaxS+1)*level]; n++)
    matrix[n*maxS] = nodeArray[(newmaxS+1)*level+n];
  
  for (n = 1; n <= nodeArray[(newmaxS+1)*(level+1)]; n++)
    matrix[n] = nodeArray[(newmaxS+1)*(level+1)+n];
  
  for (n = 1; n <= nodeArray[(newmaxS+1)*level]; n++)
    for (p = nodes[nodeArray[(newmaxS+1)*level+n]].succ.first (); p != 0; 
	 nodes[nodeArray[(newmaxS+1)*level+n]].succ.next (p))
      {   
	if ((nodes[nodeArray[(newmaxS+1)*level+n]].anzDum == 2) &&
	    (level < nodes[nodeArray[(newmaxS+1)*level+n]+1].level-1)) 
	  {
	    i = 1;
	    while ((i <= nodeArray[(newmaxS+1)*(level+1)]) &&
		   nodeArray[(newmaxS+1)*(level+1)+i] != 
		   nodeArray[(newmaxS+1)*level+n]) 
	      i++; 
	    if (nodeArray[(newmaxS+1)*(level+1)+i] == 
		nodeArray[(newmaxS+1)*level+n]) 
	      matrix[n*maxS + i] = 1;
	  }
	else
	  {
	    i = 1;
	    while(nodeArray[(newmaxS+1)*(level+1)+i] != 
		  nodes[nodeArray[(newmaxS+1)*level+n]].succ (p)) 
	      i++;
	    matrix[n*maxS + i] = 1;
	  }
      }
}

void store_new_positions (SNode *nodes, int *matrix, int level,
			  int *nodeArray, int maxS, int newmaxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Speichert die neu berechneten Positionen der Knoten der Level //
// "level" und "level+1" (Positionen in der Adjazenzmatrix) als  //
// x-Koordinaten im Graphen.                                     //
//                                                               //
///////////////////////////////////////////////////////////////////
{	
  int i;
  
  for (i = 1; i <= nodeArray[(newmaxS+1)*level]; i++)
    nodeArray[(newmaxS+1)*level+i] = matrix[i*maxS];

  for (i = 1; i <= nodeArray[(newmaxS+1)*(level+1)]; i++)
    nodeArray[(newmaxS+1)*(level+1)+i] = matrix[i];
}

BOOL sorted (FFCoord *vector, int max)
///////////////////////////////////////////////////////////////////
//                                                               //
// Prueft, ob die Elemente vector[1], ..., vector[max] in auf-   //
// steigender Reihenfolge sortiert sind. Elemente mit dem Wert   //
// 0.0 bleiben unberuecksichtigt.                                //
// "max": maximale Position im Vektor.                           //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i;

  for (i = 1; i < max; i++)
    if ((vector[i] > vector [i+1]) && 
        (vector[i+1] != 0.0))	  
      return (NO);
    
  return (YES);
}

void copy_matrix (int *matrix1, int *matrix2, int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Kopiert die Matrix "matrix1" nach "matrix2".                  //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i,
    j;

  for (i = 0; i < maxS; i++)
    for (j = 0; j < maxS; j++)
      matrix2[i*maxS + j] = matrix1[i*maxS + j];
}

BOOL equal_matrix (int *matrix1, int *matrix2, int r, int c, 
		   int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Prueft, ob Matrizen "matrix1" und "matrix2" bis zur Zeile     //
// "r" und zur Spalte "c" uebereinstimmen.                       //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i,
    j;

  for (i = 0; i <= r; i++)
    for (j = 0; j <= c; j++)
      if (matrix1[i*maxS + j] != matrix2[i*maxS + j])
        return (NO);

  return (YES);
}

FFCoord row_barycenter (int *matrix, int i, int max, int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Barycenter der "i"-ten Zeile der Matrix "matrix" bis zum      // 
// Index "max".                                                  //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int r1 = 0, 
    r2 = 0,
    j;
	
  for (j = 1; j <= max; j++)
    if (matrix[i*maxS + j] == 1)
      {
	r1 += j;
	r2++;
      }

  if (r2 == 0)
    return (0.0);
  else
    return ((FFCoord) r1 / (FFCoord) r2);
}

FFCoord column_barycenter (int *matrix, int j, int max, int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Barycenter der "j"-ten Spalte der Matrix "matrix" bis zum     //
// Index "max".                                                  //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int r1 = 0, 
    r2 = 0,
    i;
	
  for (i = 1; i <= max; i++)
    if (matrix[i*maxS + j] == 1)
      {
	r1 += i;
	r2++;
      }

  if (r2 == 0)
    return (0.0);
  else
    return ((FFCoord) r1 / (FFCoord) r2);
}

int number_of_crossings (int *matrix, int r, int c, int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Berechnet Anzahl der Ueberkreuzungen ("r" Anzahl der Zeilen,  //
// "c" Anzahl der Spalten).                                      //
// (Hier koennte man mit einem aufwendigeren Verfahren noch      //
// Laufzeit rausholen ...)                                       //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int j, 
    k, 
    alpha, 
    beta, 
    result = 0;

  for (j = 1; j <= r-1; j++)
    for (k = j+1; k <= r; k++)
      for (alpha = 1; alpha <= c-1; alpha++)
        if (matrix[k*maxS + alpha] == 1) 
          for (beta = alpha+1; beta <= c; beta++)
            if (matrix[j*maxS + beta] == 1) 
              result++;

  return (result);
}

void exch_double (FFCoord *x1, FFCoord *x2)
///////////////////////////////////////////////////////////////////
//                                                               //
// Vertauscht den Inhalt von "x1" und "x2".                      //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFCoord h;
  
  h = *x1;
  *x1 = *x2;
  *x2 = h;
}

void exch_rows (int *matrix, int r1, int r2, int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Vertauscht die Zeilen "r1" und "r2" der Matrix "matrix".      //
// (Einfache Implementierung)                                    //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int h,
    j;	
	
  for (j = 0; j < maxS; j++)
    {
      h = matrix[r1*maxS + j];
      matrix[r1*maxS + j] = matrix[r2*maxS + j]; 
      matrix[r2*maxS + j] = h;
    }
}

void exch_columns (int *matrix, int c1, int c2, int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Vertauscht die Spalten "c1" und "c2" der Matrix "matrix".     //
// (Einfache Implementierung)                                    //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int h,
    i;
 
  for (i = 0; i < maxS; i++)
    {
      h = matrix[i*maxS + c1];
      matrix[i*maxS + c1] = matrix[i*maxS + c2];
      matrix[i*maxS + c2] = h;
    }
}

void reverse_r (int *matrix, int r1, int r2, int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Kehrt die Reihenfolge der Zeilen "r1" bis "r2" in der Matrix  //
// "matrix" um.                                                  //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i, 
    j;

  for (i = r1, j = r2; i < j; i++, j--)
    exch_rows (matrix, i, j, maxS);
}

void reverse_c (int *matrix, int c1, int c2, int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Kehrt die Reihenfolge der Spalten "c1" bis "c2" in der Matrix //
// "matrix" um.                                                  //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i, 
    j;

  for (i = c1, j = c2; i < j; i++, j--)
    exch_columns (matrix, i, j, maxS);
}

int r_r (int *matrix1, int *matrix2, int max_r, int max_c, 
	 int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Die Reihenfolge der Zeilen mit gleichem Barycenter wird um-   //
// gekehrt.                                                      // 
// Rueckgabewert 1, falls Fehler.                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFCoord *b;
  int i,
    j;
	
  b = new FFCoord [maxS];
  if (b == 0)
    return (1);

  //  Zeilenbarycenter berechnen 
  for (i = 1; i <= max_r; i++)
    b[i] = row_barycenter(matrix1, i, max_c, maxS);
	
  //  Reihenfolge der Zeilen mit gleichem Barycenter umkehren 
  for (i = 1; i < max_r; i++)
    {
      j = i;
      while ((j < max_r) && (b[j+1] == b[j]))
        j++;

      if (j > i)
        {
          reverse_r (matrix1, i, j, maxS);
          if (matrix2 != 0)
            reverse_c (matrix2, i, j, maxS);
        
          i = j;
        } 
    }

  delete[] b;
  return (0);
}

int r_c (int *matrix1, int *matrix2, int max_c, int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Die Reihenfolge der Spalten mit gleichem Barycenter wird      //
// umgekehrt.                                                    //
// Rueckgabewert 1, falls Fehler.                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFCoord *b;
  int i,
    j;
	
  b = new FFCoord [maxS];
  if (b == 0)
    return (1);

  //  Spaltenbarycenter berechnen 
  for (i = 1; i <= max_c; i++)
    b[i] = column_barycenter (matrix1, i, max_c, maxS);

  //  Reihenfolge der Spalten mit gleichem Barycenter umkehren 
  for (i = 1; i < max_c; i++)
    {
      j = i;
      while ((j < max_c) && (b[j+1] == b[j]))
        j++;

      if (j > i)
        {
          reverse_c (matrix1, i, j, maxS);
          if (matrix2 != 0)
            reverse_r (matrix2, i, j, maxS);
        
          i = j;
        } 
    }

  delete[] b;
  return (0);
}

int b_r (int *matrix1, int *matrix2, int max_r, int max_c, 
	 int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Ordnet Zeilen anhand des Barycenter der Zeilen, Nullzeilen    //
// bleiben stehen.                                               //
// Rueckgabewert 1, falls Fehler.                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFCoord *b;
  int k, 
    i, 
    j;
 
  b = new FFCoord [maxS];
  if (b == 0)
    return (1);

  for (i = 1; i <= max_r; i++)
    b[i] = row_barycenter (matrix1, i, max_c, maxS);   

  for (j = max_r; j > 1; j--)
    if (b[j] != 0.0)						
      for (i = 1; i < j; i++)
        if (b[i] != 0.0)
          {
            k = i+1;
            while (b[k] == 0.0)
              k++;

            if (b[i] > b[k])
              {
                exch_double (&b[i], &b[k]);
                exch_rows (matrix1, i, k, maxS);

                if (matrix2 != 0)
                  exch_columns (matrix2, i, k, maxS);
              }
          }
  
  delete[] b;
  return (0);
}					

int b_c (int *matrix1, int *matrix2, int max_r, int max_c, 
	 int maxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Ordnet Spalten anhand des Barycenter der Spalten, Nullzeilen  //
// bleiben stehen.                                               //
// Rueckgabewert 1, falls Fehler.                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFCoord *b;
  int k, 
    i, 
    j;

  b = new FFCoord [maxS];
  if (b == 0)
    return (1);
  
  for (i = 1; i <= max_c; i++)
    b[i] = column_barycenter (matrix1, i, max_r, maxS);
	
  for (j = max_c; j > 1; j--)
    if (b[j] != 0.0)						
      for (i = 1; i < j; i++)
        if (b[i] != 0.0)
          {
            k = i+1;
            while (b[k] == 0.0)
              k++;

            if (b[i] > b[k])
              {
                exch_double (&b[i], &b[k]);
                exch_columns (matrix1, i, k, maxS);
		
                if (matrix2 != 0)
                  exch_rows (matrix2, i, k, maxS);
              }
          }
  
  delete[] b;
  return (0);
}	

int phase1_down (int *matrixarray, int matrixSize, int *nodeArray, 
		 int maxL, int maxS, int newmaxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Ordnet Spalten aller Matrizen (je eine Matrix fuer Uebergang  //
// von Level i zu Level i+1) anhand des Barycenter der Spalten.  //
// Rueckgabewert 1, falls Fehler.                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int test,
    i;

  for (i = 0; i < maxL-1; i++)
    {
      test = b_c (matrixarray + i*matrixSize, 
		  matrixarray + (i+1)*matrixSize,
		  nodeArray[(newmaxS+1)*i], 
		  nodeArray[(newmaxS+1)*(i+1)], maxS);
      
      if (test != 0)
	return (1);
    }
      
  test = b_c (matrixarray + (maxL-1)*matrixSize, 0, 
	      nodeArray[(newmaxS+1)*(maxL-1)], 
	      nodeArray[(newmaxS+1)*(maxL)], maxS);
  
  if (test != 0)
    return (1);

  return (0);
}

int phase1_up (int *matrixarray, int matrixSize, int *nodeArray, 
	       int maxL, int maxS, int newmaxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Ordnet Zeilen aller Matrizen (je eine Matrix fuer Uebergang   //
// von Level i zu Level i+1) anhand des Barycenter der Zeilen.   //
// Rueckgabewert 1, falls Fehler.                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int test,
    i;

  for (i = maxL-1; i > 0; i--)
    {
      test = b_r (matrixarray + i*matrixSize, 
		  matrixarray + (i-1)*matrixSize,
		  nodeArray[(newmaxS+1)*i], 
		  nodeArray[(newmaxS+1)*(i+1)], maxS);

      if (test != 0)
	return (1);
    }

  test = b_r (matrixarray, 0, nodeArray[0], nodeArray[(newmaxS+1)], maxS);
  if (test != 0)
    return (1);
  
  return (0);
}

int phase2_down (int *matrixarray, int matrixSize, int *nodeArray,
		 int maxL, int maxS, int newmaxS)
///////////////////////////////////////////////////////////////////
//                                                               //
// Von oben nach unten werden in allen Levels die Spalten mit    //
// gleichem Barycenter in der Reihenfolge umgekehrt, solange die //
// Spaltenbarycenter geordnet sind. Sind diese in einem Level    //
// nicht geordnet, so bricht der Algorithmus ab.                 //
// Rueckgabewert 1, falls Fehler.                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{ 
  FFCoord *b;
  int level,
    i,
    test = 0;

  b = new FFCoord [maxS];
  if (b == 0)
    return (1);
  
  for (level = 0; level < maxL-1; level++)
    {
      for (i = 1; i <= nodeArray[(newmaxS+1)*(level+1)]; i++)
        b[i] = column_barycenter (matrixarray + level*matrixSize, i, 
				  nodeArray[(newmaxS+1)*level], maxS);
      
      if (sorted (b, nodeArray[(newmaxS+1)*(level+1)]))
	{
	  test = r_c (matrixarray + level*matrixSize, 
		      matrixarray + (level+1)*matrixSize,
		      nodeArray[(newmaxS+1)*(level+1)], maxS);
	  if (test != 0)
	    return (1);
	}
      else 
        {
          delete[] b;
          return (0);
        }
    }

  for (i = 1; i <= nodeArray[(newmaxS+1)*maxL]; i++)
    b[i] = column_barycenter (matrixarray + (maxL-1)*matrixSize, i,
			      nodeArray[(newmaxS+1)*(maxL-1)], maxS);
  
  if (sorted (b, nodeArray[(newmaxS+1)*maxL]))
    {
      test = r_c (matrixarray + (maxL-1)*matrixSize, 0, 
		  nodeArray[(newmaxS+1)*maxL], maxS);
      if (test != 0)
	return (1);
    }

  delete[] b;
  return (0);
}	

int phase2_up (int *matrixarray, int matrixSize, int *nodeArray, 
	       int maxL, int maxS, int newmaxS)
/////////////////////////////////////////////////////////////////// 
//                                                               //
// Von unten nach oben werden in allen Levels die Zeilen mit     //
// gleichem Barycenter in der Reihenfolge umgekehrt, solange die //
// Zeilenbarycenter geordnet sind. Sind diese in einem Level     //
// nicht geordnet, so bricht der Algorithmus ab.                 //
// Rueckgabewert 1, falls Fehler.                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{ 
  FFCoord *b;
  int level,
    i,
    test;

  b = new FFCoord [maxS];
  if (b == 0)
    return (1);

  for (level = maxL-1; level > 0; level--)
    {
      for (i = 1; i <= nodeArray[(newmaxS+1)*level]; i++)
        b[i] = row_barycenter (matrixarray + level*matrixSize, i,
                               nodeArray[(newmaxS+1)*(level+1)], maxS);
      
      if (sorted(b, nodeArray[(newmaxS+1)*level]))
	{
	  test = r_r (matrixarray + level*matrixSize, 
		      matrixarray + (level-1)*matrixSize,
		      nodeArray[(newmaxS+1)*level], 
		      nodeArray[(newmaxS+1)*(level+1)], maxS);
	  if (test != 0)
	    return (1);
	}
      else
        {
          delete[] b;
          return (0);
        }
    }
  
  for (i = 1; i <= nodeArray[0]; i++)
    b[i] = row_barycenter (matrixarray, i, nodeArray[(newmaxS+1)], maxS);
  
  if (sorted (b, nodeArray[0]))
    {
      test = r_r (matrixarray, 0, nodeArray[0], nodeArray[(newmaxS+1)], maxS);
      if (test != 0)
	return (1);
    }
  
  delete[] b;
  return (0);
}

BOOL equal_matrixarray (int *a1, int *a2, int maxS, int newmaxS, int maxL, 
			int matrixSize, int *nodeArray)
///////////////////////////////////////////////////////////////////
//                                                               //
// Prueft, ob Matrizenarrays "a1" und "a2" bis zur Zeile         //
// "r" und zur Spalte "c" uebereinstimmen.                       //
//                                                               //
///////////////////////////////////////////////////////////////////
{      
  int level;

  for (level = 0; level < maxL; level++)
    if (! equal_matrix(a1 + level*matrixSize, a2 + level*matrixSize, 
                       nodeArray[(newmaxS+1)*level], 
		       nodeArray[(newmaxS+1)*(level+1)], maxS))
      return (NO);
  
  return (YES);
}

void copy_matrixarray (int *a1, int *a2, int maxS, int maxL, 
		       int matrixSize)
///////////////////////////////////////////////////////////////////
//                                                               //
// Kopiert das Matrizenarray "a1" nach "a2".                     //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i;

  for (i = 0; i < maxL; i++)
    copy_matrix (a1 + i*matrixSize, a2 + i*matrixSize, maxS);
}

int number_of_crossings_matrixarray (int *a, int maxS, int newmaxS, 
				     int matrixSize, int maxL, 
				     int *nodeArray)
///////////////////////////////////////////////////////////////////
//                                                               //
// Berechnet Anzahl der Ueberkreuzungen (Summe ueber alle        //
// Ebenen).                                                      //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int result = 0,
    i;
	
  for (i = 0; i < maxL; i++)
    result += number_of_crossings (a + i*matrixSize, 
				   nodeArray[(newmaxS+1)*i],
                                   nodeArray[(newmaxS+1)*(i+1)], maxS);  
  return (result);
}

int bc_n (SNode *nodes, int maxS, int newmaxS, int maxL, 
	  int iteration1, int iteration2, int *nodeArray)
////////////////////////////////////////////////////////////////////
//                                                                //
// Algorithmus fuer einen Graphen mit mindestens 3 Level.         //
// Zuerst wird die Phase 1 ("down" - "up") solange durchlaufen,   //
// bis man am Ende der Phase dieselben Matrizen wie am Anfang hat //
// oder eine bestimmte Anzahl ("iterations") von Iterationen      //
// erreicht ist. Mit der dadurch bestimmten Loesung (Matrix "as") //
// wird die Phase 2 gestartet. In der Phase 2 wird zuerst die     //
// Prozedur "down" ausgefuehrt, darauf folgt Phase 1 ("down" -    //
// "up"), dann wird die Prozedur "up" ausgefuehrt, wiederum ge-   //
// folgt von Phase 1 ("up" - "down"). Auch die Phase 2 wird so-   //
// lange durchlaufen, bis man am Ende dieselben Matrizen wie am   //
// Anfang hat oder eine bestimmte Anzahl von Iterationen erreicht //
// ist.                                                           //
// Rueckgabewert 1, falls Fehler.                                //
//                                                                //
////////////////////////////////////////////////////////////////////
{
  int *a, 
    *a1, 
    *a2, 
    *as;
  int ks, 
    k, 
    n1, 
    n2, 
    i,
    test; // Zur Ueberpruefung, ob Funktionen korrekt abgearbeitet wurden
  int matrixSize = maxS * maxS; 
  
  a  = new int [(maxL+1) * matrixSize];
  if (a == 0)
    return (1);

  a1 = new int [(maxL+1) * matrixSize]; 
  if (a1 == 0)
    {
      delete[] a;
      return (1);
    }

  a2 = new int [(maxL+1) * matrixSize];  
  if (a1 == 0)
    {
      delete[] a1;
      delete[] a;
      return (1);
    }

  // "a1" und "a2" dienen zum Vergleich, 
  // um das Abbruchkriterium zu bestimmen. 
  as = new int [(maxL+1) * matrixSize];
  if (as == 0)
    {
      delete[] a2;
      delete[] a1;
      delete[] a;
      return (1);
    }
  
  // Matrix erstellen
  for (i = 0; i < maxL; i++)
    make_matrix (nodes, i, a+i*matrixSize, maxS, newmaxS, nodeArray);
	
  copy_matrixarray (a, as, maxS, maxL, matrixSize);
  
  // Zum Vergleich initiale Zahl der Ueberkreuzungen bestimmen.
  ks = number_of_crossings_matrixarray (as, maxS, newmaxS, matrixSize, 
					maxL, nodeArray);
  
  n1 = 0;
  do
    {
      copy_matrixarray (a, a1, maxS, maxL, matrixSize);
      test = phase1_down (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
      if (test != 0)
	return (1);

      k = number_of_crossings_matrixarray (a, maxS, newmaxS, matrixSize, 
					   maxL, nodeArray);
      
      if (k < ks)
        {
          ks = k;
          copy_matrixarray (a, as, maxS, maxL, matrixSize);
        }
      test = phase1_up (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
      if (test != 0)
	return (1);

      k = number_of_crossings_matrixarray (a, maxS, newmaxS, matrixSize, 
					   maxL, nodeArray);

      if (k < ks)
        {
          ks = k;
          copy_matrixarray (a, as, maxS, maxL, matrixSize);
        }		
    }
  
  while ((++n1 < iteration1) && 
         (! equal_matrixarray (a, a1, maxS, newmaxS, maxL, matrixSize, 
                               nodeArray)));

  if (! equal_matrixarray (a, as, maxS, newmaxS, maxL, matrixSize, 
			   nodeArray))
    copy_matrixarray (as, a, maxS, maxL, matrixSize);

  n2 = 0;
  do 
    {	
      copy_matrixarray (a, a2, maxS, maxL, matrixSize);
      test = phase2_down (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
      if (test != 0)
	return (1);

      n1 = 0;

      do
        {
          copy_matrixarray (a, a1, maxS, maxL, matrixSize);
          test = phase1_down (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
	  if (test != 0)
	    return (1);

          k = number_of_crossings_matrixarray (a, maxS, newmaxS, matrixSize, 
					       maxL, nodeArray);
	  
          if (k < ks)
            {
              ks = k;
              copy_matrixarray (a, as, maxS, maxL, matrixSize);
            }
			
          test = phase1_up (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
	  if (test != 0)
	    return (1);

          k = number_of_crossings_matrixarray (a, maxS, newmaxS, matrixSize, 
					       maxL, nodeArray);

          if (k < ks)
            {
              ks = k;
              copy_matrixarray (a, as, maxS, maxL, matrixSize);
            }		
        }
      while ((++n1 < iteration1) && 
             (! equal_matrixarray (a, a1, maxS, newmaxS, maxL, matrixSize, 
                                   nodeArray)));
      
      test = phase2_up (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
      if (test != 0)
	return (1);

      n1 = 0;		
      do
        {
          copy_matrixarray (a, a1, maxS, maxL, matrixSize);
			
          test = phase1_up (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
	  if (test != 0)
	    return (1);

          k = number_of_crossings_matrixarray (a, maxS, newmaxS, matrixSize, 
					       maxL, nodeArray);
	  
          if (k < ks)
            {
              ks = k;
              copy_matrixarray (a, as, maxS, maxL, matrixSize);
            }
          
          test = phase1_down (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
	  if (test != 0)
	    return (1);

          k = number_of_crossings_matrixarray (a, maxS, newmaxS, matrixSize, 
					       maxL, nodeArray);

          if (k < ks)
            {
              ks = k;
              copy_matrixarray (a, as, maxS, maxL, matrixSize);
            }		
        }
      while ((++n1 < iteration1) && 
             (! equal_matrixarray (a, a1, maxS, newmaxS, maxL, matrixSize, 
                                   nodeArray)));
    }	
  while ((++n2 < iteration2) && 
         (! equal_matrixarray (a, a2, maxS, newmaxS, maxL, matrixSize, 
                               nodeArray)));	

  // Nun die neuen Positionen zurueckschreiben
  for (i = 0; i < maxL; i += 2)
    store_new_positions (nodes, as + i*matrixSize, i, nodeArray, 
			 maxS, newmaxS);

  if (i == maxL)
    store_new_positions (nodes, as + (maxL-1)*matrixSize, maxL-1,
                         nodeArray, maxS, newmaxS);
  
  delete[] as;
  delete[] a2;
  delete[] a1;
  delete[] a;

  return (0);
}

int bc_2 (SNode *nodes, int maxS, int newmaxS, int *nodeArray)
///////////////////////////////////////////////////////////////////
//                                                               //
// Algorithmus zum Layout fuer einen Graph mit 2 Leveln.         //
// Rueckgabewert 1, falls Fehler.                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFCoord *b;
  int *m;
  int *m0, 
    *ms;
  int k, 
    ks, 
    i,
    test;

  m = new int [maxS * maxS];
  if (m == 0)
    return (1);

  m0 = new int [maxS * maxS];
  if (m0 == 0)
    {
      delete[] m;
      return (1);
    }

  ms = new int [maxS * maxS];
  if (ms == 0)
    {
      delete[] m0;
      delete[] m;
      return (1);
    }

  b = new FFCoord [maxS];
  if (b == 0)
    {
      delete[] ms;
      delete[] m0;
      delete[] m;
      return (1);
    }
      
  make_matrix (nodes, 0, m, maxS, newmaxS, nodeArray);
  copy_matrix (m, ms, maxS);

  // "ks" enthaelt die bisher kleinste Anzahl Ueberkreuzungen.
  ks = number_of_crossings (m, nodeArray[0], nodeArray[(newmaxS+1)], maxS);
  
  while (YES)
    {
      do 
        {
	  // m0 = m
          copy_matrix (m, m0, maxS);
          test = b_r (m, 0, nodeArray[0], nodeArray[(newmaxS+1)], maxS);
	  if (test != 0)
	    return (1);

          k = number_of_crossings (m, nodeArray[0], nodeArray[(newmaxS+1)],
				   maxS);

          if (k < ks)
	    {	
	      // ms = m
	      copy_matrix (m, ms, maxS);
	      ks = k;
	    }
	  
	  test = b_c (m, 0, nodeArray[0], nodeArray[(newmaxS+1)], maxS);
	  if (test != 0)
	    return (1);
	  
	  k = number_of_crossings (m, nodeArray[0], nodeArray[(newmaxS+1)], 
				   maxS);
	  
	  if (k < ks)
	    {	
	      // ms = m
	      copy_matrix (m, ms, maxS);
	      ks = k;
	    }
	}  
      while (! equal_matrix (m0, m, nodeArray[0], nodeArray[(newmaxS+1)], 
			     maxS));
      
      test = r_r (m, 0, nodeArray[0], nodeArray[(newmaxS+1)], maxS);
      if (test != 0)
	return (1);
      
      for (i = 1; i <= nodeArray[(newmaxS+1)]; i++)
        b[i] = column_barycenter (m, i, nodeArray[0], maxS);
		
      if (! sorted (b, nodeArray[(newmaxS+1)]))
        {
          copy_matrix (m, m0, maxS);
          continue;
        }

      test = r_c (m, 0, nodeArray[(newmaxS+1)], maxS);
      if (test != 0)
	return (1);
	
      for (i = 1; i <= nodeArray[0]; i++)
        b[i] = row_barycenter (m, i, nodeArray[(newmaxS+1)], maxS);
      
      if ((sorted (b, nodeArray[0])) ||
	  (sorted (b, nodeArray[(newmaxS+1)])))
        break;
    }

  // neue Positionen merken (in "nodeArray" aktualisieren)
  store_new_positions (nodes, ms, 0, nodeArray, maxS, newmaxS);

  delete[] b;
  delete[] m;
  delete[] m0;
  delete[] ms;

  return (0);
}

int reduce_crossings (SNode *nodes,  
		      int maxL, int maxS, int newmaxS, int anzND, 
		      int cross_alg, int *nodeArray,
		      int iteration1, int iteration2)
///////////////////////////////////////////////////////////////////
//                                                               //
// Plaziere die Knoten so, dass sich die Kanten moeglichst wenig //
// ueberschneiden.                                               //
// "nodes" enthalt die Knoten entsprechend "SNode"               //
// "anzND" ist die Summe echter und dummy-Knoten                 //
// "nodeArray" ist ein Gitter, welches die relativewn Positionen //
// der Knoten enthaelt. Fuer Dummy-Knoten ueber mehrere Level    //
// muss gelten: Sie muessen auf allen dazwischenliegenden Leveln // 
// vorkommen.                                                    //
// Rueckgabewert 1, falls Fehler.                                //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int test = 0;

  maxS++;
  
  if (cross_alg == 0)  //  Algorithmus mittels Adjazenzmatrizen
    {
      if (maxL == 0)
	return (0);
      if (maxL == 1) 
	test = bc_2 (nodes, maxS, newmaxS, nodeArray);
      else
	test = bc_n (nodes, maxS, newmaxS, maxL, iteration1, iteration2, 
		     nodeArray); 
    }

  // Der Bubbling-Algorithmus wurde abgeschafft 
  // (schlechte Ergebnisse und nur wenig bessere Laufzeit)
  return (test);
}
