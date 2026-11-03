// Frozen pre-optimization barycenter bubble permutations; Falk Schreiber, 1994-97.
int reference_b_r (int *matrix1, int *matrix2, int max_r, int max_c, 
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

int reference_b_c (int *matrix1, int *matrix2, int max_r, int max_c, 
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

