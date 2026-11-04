/*
// Copyright 1994-97 Falk Schreiber, Universitaet Passau
//
*/

#include <ffgraph/FFreduce_crossing.h>
#include "FFcrossing_count.h"
#include <algorithm>
#include <limits>
#include <vector>
#include <memory>
#include "FFsugi_sizes.h"
#ifdef FFGRAPH_VERIFY_SUGIYAMA_COUNTS
#include <stdexcept>
#endif

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
  int previous = 0;

  for (int i = 1; i <= max; i++)
    if (vector[i] != 0.0)
      {
        if (previous != 0 && vector[previous] > vector[i])
          return (NO);
        previous = i;
      }
    
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
  std::int64_t r1 = 0, r2 = 0;
  int j;
	
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
  std::int64_t r1 = 0, r2 = 0;
  int i;
	
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

namespace ffgraph_sugiyama {
CrossingCount countCrossings(const int* matrix, int r, int c, int maxS)
{
  if (r < 2 || c < 2) return 0;
  std::vector<CrossingCount> columns(static_cast<std::size_t>(c) + 1, 0);
  CrossingCount previousEdges = 0, result = 0;
  for (int row = 1; row <= r; ++row) {
    const int* cells = matrix + static_cast<std::size_t>(row) * maxS;
    auto right = previousEdges;
    for (int col = 1; col <= c; ++col) {
      right -= columns[col];
      // Count earlier-row endpoints strictly to the right. Equal columns
      // share a target and do not cross; only cells equal to 1 are edges.
      if (cells[col] == 1) result += right;
    }
    // Update after the entire row so shared-source edges cannot cross.
    for (int col = 1; col <= c; ++col)
      if (cells[col] == 1) { ++columns[col]; ++previousEdges; }
  }
#ifdef FFGRAPH_VERIFY_SUGIYAMA_COUNTS
  // Diagnostic builds compare every actual matrix state with the former
  // pair enumeration. This code is absent from normal application builds.
  CrossingCount reference = 0;
  for (int upper = 1; upper < r; ++upper)
    for (int lower = upper + 1; lower <= r; ++lower)
      for (int left = 1; left < c; ++left)
        if (matrix[lower * maxS + left] == 1)
          for (int right = left + 1; right <= c; ++right)
            if (matrix[upper * maxS + right] == 1) ++reference;
  if (reference != result) throw std::logic_error("Sugiyama crossing counter disagrees with reference");
#endif
  return result;
}
}

namespace {
int narrowCrossings(ffgraph_sugiyama::CrossingCount count) {
  // Preserve the legacy int helper signatures without signed overflow.
  // Layout decisions use the full-width counts, not these saturated wrappers.
  return static_cast<int>(std::min(count,
    static_cast<ffgraph_sugiyama::CrossingCount>(std::numeric_limits<int>::max())));
}
}

int number_of_crossings(int* matrix, int r, int c, int maxS) {
  return narrowCrossings(ffgraph_sugiyama::countCrossings(matrix, r, c, maxS));
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
	
  std::unique_ptr<FFCoord[]> buffer(new FFCoord[ffgraph_sugiyama::arrayCount<FFCoord>(maxS)]);
  b = buffer.get();

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


  return (0);
}

int r_c (int *matrix1, int *matrix2, int max_r, int max_c, int maxS)
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
	
  std::unique_ptr<FFCoord[]> buffer(new FFCoord[ffgraph_sugiyama::arrayCount<FFCoord>(maxS)]);
  b = buffer.get();

  //  Spaltenbarycenter berechnen 
  for (i = 1; i <= max_c; i++)
    b[i] = column_barycenter (matrix1, i, max_r, maxS);

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


  return (0);
}

namespace {
int barycenterPermutation(int* matrix, int* partner, int rows, int columns, int stride, bool byRows) {
  std::vector<int> slots;
  std::vector<std::pair<FFCoord,int>> order;
  const int count = byRows ? rows : columns;
  for (int i = 1; i <= count; ++i) {
    const auto value = byRows ? row_barycenter(matrix,i,columns,stride) : column_barycenter(matrix,i,rows,stride);
    if (value != 0) { slots.push_back(i); order.emplace_back(value,i); }
  }
  std::stable_sort(order.begin(),order.end(),[](const auto& a, const auto& b) { return a.first < b.first; });
  bool changed = false;
  for (std::size_t i = 0; i < slots.size(); ++i) changed |= slots[i] != order[i].second;
  if (!changed) return 0;
  // Decompose destination <- source into disjoint cycles. Fixed nonzero slots,
  // like zero-barycenter slots, need no copying. Keep the same permutation for
  // the neighboring matrix's opposite axis, including metadata and padding.
  std::vector<int> sourceForSlot(count+1);
  for (std::size_t i = 0; i < slots.size(); ++i) sourceForSlot[slots[i]] = order[i].second;
  std::vector<unsigned char> visited(count+1,0);
  std::vector<int> cycleStarts;
  for (int start : slots) {
    if (visited[start] || sourceForSlot[start] == start) continue;
    cycleStarts.push_back(start);
    int current = start;
    do {
      visited[current] = 1;
      current = sourceForSlot[current];
    } while (current != start);
  }
  // Rows are contiguous; columns are processed one physical row at a time.
  // Only a single row of scratch is needed, rather than slots.size()*stride.
  std::vector<int> savedRow(byRows || partner ? stride : 0);
  auto apply = [&](int* cells, bool rowWise) {
    if (rowWise) {
      for (int start : cycleStarts) {
        std::copy_n(cells+std::size_t(start)*stride,stride,savedRow.data());
        int current = start;
        while (sourceForSlot[current] != start) {
          const int source = sourceForSlot[current];
          std::copy_n(cells+std::size_t(source)*stride,stride,cells+std::size_t(current)*stride);
          current = source;
        }
        std::copy_n(savedRow.data(),stride,cells+std::size_t(current)*stride);
      }
    } else {
      for (int row = 0; row < stride; ++row) {
        int* values = cells+std::size_t(row)*stride;
        for (int start : cycleStarts) {
          const int saved = values[start];
          int current = start;
          while (sourceForSlot[current] != start) {
            values[current] = values[sourceForSlot[current]];
            current = sourceForSlot[current];
          }
          values[current] = saved;
        }
      }
    }
  };
  apply(matrix,byRows);
  if (partner) apply(partner,!byRows);
  return 0;
}
}
int b_r(int* matrix, int* partner, int rows, int columns, int stride) {
#ifdef FFGRAPH_VERIFY_SUGIYAMA_PERMUTATIONS
  return ffgraph_test::checkedPermutation(matrix,partner,rows,columns,stride,true,barycenterPermutation);
#else
  return barycenterPermutation(matrix,partner,rows,columns,stride,true);
#endif
}
int b_c(int* matrix, int* partner, int rows, int columns, int stride) {
#ifdef FFGRAPH_VERIFY_SUGIYAMA_PERMUTATIONS
  return ffgraph_test::checkedPermutation(matrix,partner,rows,columns,stride,false,barycenterPermutation);
#else
  return barycenterPermutation(matrix,partner,rows,columns,stride,false);
#endif
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

  std::unique_ptr<FFCoord[]> buffer(new FFCoord[ffgraph_sugiyama::arrayCount<FFCoord>(maxS)]);
  b = buffer.get();
  
  for (level = 0; level < maxL-1; level++)
    {
      for (i = 1; i <= nodeArray[(newmaxS+1)*(level+1)]; i++)
        b[i] = column_barycenter (matrixarray + level*matrixSize, i, 
				  nodeArray[(newmaxS+1)*level], maxS);
      
      if (sorted (b, nodeArray[(newmaxS+1)*(level+1)]))
	{
	  test = r_c (matrixarray + level*matrixSize, 
		      matrixarray + (level+1)*matrixSize,
		      nodeArray[(newmaxS+1)*level],
		      nodeArray[(newmaxS+1)*(level+1)], maxS);
	  if (test != 0)
	    return (1);
	}
      else 
        {

          return (0);
        }
    }

  for (i = 1; i <= nodeArray[(newmaxS+1)*maxL]; i++)
    b[i] = column_barycenter (matrixarray + (maxL-1)*matrixSize, i,
			      nodeArray[(newmaxS+1)*(maxL-1)], maxS);
  
  if (sorted (b, nodeArray[(newmaxS+1)*maxL]))
    {
      test = r_c (matrixarray + (maxL-1)*matrixSize, 0, 
		  nodeArray[(newmaxS+1)*(maxL-1)],
		  nodeArray[(newmaxS+1)*maxL], maxS);
      if (test != 0)
	return (1);
    }


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

  std::unique_ptr<FFCoord[]> buffer(new FFCoord[ffgraph_sugiyama::arrayCount<FFCoord>(maxS)]);
  b = buffer.get();

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

ffgraph_sugiyama::CrossingCount ffgraph_sugiyama::countCrossingsArray(
    const int* a, int maxS, int newmaxS, int matrixSize, int maxL, const int* nodeArray)
///////////////////////////////////////////////////////////////////
//                                                               //
// Berechnet Anzahl der Ueberkreuzungen (Summe ueber alle        //
// Ebenen).                                                      //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  CrossingCount result = 0;
	
  for (int i = 0; i < maxL; i++)
    result += countCrossings (a + static_cast<std::size_t>(i)*matrixSize,
				   nodeArray[(newmaxS+1)*i],
                                   nodeArray[(newmaxS+1)*(i+1)], maxS);  
  return (result);
}

int number_of_crossings_matrixarray(int* a, int maxS, int newmaxS,
                                   int matrixSize, int maxL, int* nodeArray) {
  return narrowCrossings(ffgraph_sugiyama::countCrossingsArray(
    a, maxS, newmaxS, matrixSize, maxL, nodeArray));
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
  ffgraph_sugiyama::CrossingCount ks, k;
  int n1,
    n2, 
    i,
    test; // Zur Ueberpruefung, ob Funktionen korrekt abgearbeitet wurden
  const int matrixSize = ffgraph_sugiyama::matrixEntries(maxS);
  const int entries = ffgraph_sugiyama::arrayCount<int>(ffgraph_sugiyama::matrixArrayEntries(maxL,maxS));
  std::unique_ptr<int[]> aOwner(new int[entries]), a1Owner(new int[entries]);
  std::unique_ptr<int[]> a2Owner(new int[entries]), asOwner(new int[entries]);
  a = aOwner.get(); a1 = a1Owner.get(); a2 = a2Owner.get(); as = asOwner.get();

  // Matrix erstellen
  for (i = 0; i < maxL; i++)
    make_matrix (nodes, i, a+i*matrixSize, maxS, newmaxS, nodeArray);
	
  copy_matrixarray (a, as, maxS, maxL, matrixSize);
  
  // Zum Vergleich initiale Zahl der Ueberkreuzungen bestimmen.
  ks = ffgraph_sugiyama::countCrossingsArray (as, maxS, newmaxS, matrixSize,
					maxL, nodeArray);
  
  n1 = 0;
  do
    {
      copy_matrixarray (a, a1, maxS, maxL, matrixSize);
      test = phase1_down (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
      if (test != 0)
	return (1);

      k = ffgraph_sugiyama::countCrossingsArray (a, maxS, newmaxS, matrixSize,
					   maxL, nodeArray);
      
      if (k < ks)
        {
          ks = k;
          copy_matrixarray (a, as, maxS, maxL, matrixSize);
        }
      test = phase1_up (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
      if (test != 0)
	return (1);

      k = ffgraph_sugiyama::countCrossingsArray (a, maxS, newmaxS, matrixSize,
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

          k = ffgraph_sugiyama::countCrossingsArray (a, maxS, newmaxS, matrixSize,
					       maxL, nodeArray);
	  
          if (k < ks)
            {
              ks = k;
              copy_matrixarray (a, as, maxS, maxL, matrixSize);
            }
			
          test = phase1_up (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
	  if (test != 0)
	    return (1);

          k = ffgraph_sugiyama::countCrossingsArray (a, maxS, newmaxS, matrixSize,
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

          k = ffgraph_sugiyama::countCrossingsArray (a, maxS, newmaxS, matrixSize,
					       maxL, nodeArray);
	  
          if (k < ks)
            {
              ks = k;
              copy_matrixarray (a, as, maxS, maxL, matrixSize);
            }
          
          test = phase1_down (a, matrixSize, nodeArray, maxL, maxS, newmaxS);
	  if (test != 0)
	    return (1);

          k = ffgraph_sugiyama::countCrossingsArray (a, maxS, newmaxS, matrixSize,
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
  ffgraph_sugiyama::CrossingCount k, ks;
  int i,
    test;

  const int entries = ffgraph_sugiyama::arrayCount<int>(ffgraph_sugiyama::matrixEntries(maxS));
  std::unique_ptr<int[]> mOwner(new int[entries]), m0Owner(new int[entries]), msOwner(new int[entries]);
  std::unique_ptr<FFCoord[]> bOwner(new FFCoord[ffgraph_sugiyama::arrayCount<FFCoord>(maxS)]);
  m = mOwner.get(); m0 = m0Owner.get(); ms = msOwner.get(); b = bOwner.get();

  make_matrix (nodes, 0, m, maxS, newmaxS, nodeArray);
  copy_matrix (m, ms, maxS);

  // "ks" enthaelt die bisher kleinste Anzahl Ueberkreuzungen.
  ks = ffgraph_sugiyama::countCrossings (m, nodeArray[0], nodeArray[(newmaxS+1)], maxS);
  
  while (YES)
    {
      do 
        {
	  // m0 = m
          copy_matrix (m, m0, maxS);
          test = b_r (m, 0, nodeArray[0], nodeArray[(newmaxS+1)], maxS);
	  if (test != 0)
	    return (1);

          k = ffgraph_sugiyama::countCrossings (m, nodeArray[0], nodeArray[(newmaxS+1)],
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
	  
	  k = ffgraph_sugiyama::countCrossings (m, nodeArray[0], nodeArray[(newmaxS+1)],
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

      test = r_c (m, 0, nodeArray[0], nodeArray[(newmaxS+1)], maxS);
      if (test != 0)
	return (1);
	
      for (i = 1; i <= nodeArray[0]; i++)
        b[i] = row_barycenter (m, i, nodeArray[(newmaxS+1)], maxS);
      
      if (sorted (b, nodeArray[0]))
        break;
    }

  // neue Positionen merken (in "nodeArray" aktualisieren)
  store_new_positions (nodes, ms, 0, nodeArray, maxS, newmaxS);






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

  if (cross_alg != 0 || maxL == 0) return 0;
  try {
    ffgraph_sugiyama::levelEntries(maxL,newmaxS);
    maxS = ffgraph_sugiyama::checkedAdd(maxS,1);
    ffgraph_sugiyama::arrayCount<int>(ffgraph_sugiyama::reductionEntries(maxL,maxS));
  } catch (const ffgraph_sugiyama::SizeError&) { return 1; }

  
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
