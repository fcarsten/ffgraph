/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:11 $
// $RCSfile: FFPoint.h,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifndef FFPOINT
#define FFPOINT

#include <math.h>
#include "globdefs.h"

#define FF_EPSILON 0.000000001

// Wird FFPOINT_SIZE geaendert, muss auch der Konstruktor und
// der Konstruktor der Matrix Klasse geaendert werden

#define FFPOINT_SIZE 4

class FFPoint
{       /* point */
public:
 FFCoord  v[FFPOINT_SIZE];
 
 FFPoint();
 FFPoint(const FFPoint& p);
 FFPoint(FFCoord nx, FFCoord ny, FFCoord nz=0.0, FFCoord nw=1.0);
 
 inline FFCoord brieftraeger(const FFPoint& p) const
 {
  FFCoord result=0;
  for(int i=0; i< FFPOINT_SIZE; i++)
   result += fabs(v[i]-p.v[i]);
  return result;
 }

 BOOL valid();
 
 BOOL operator == (const FFPoint& p) const;
 BOOL operator != (const FFPoint& p) const;
 
 FFPoint operator-() const;
 friend FFPoint operator - (const FFPoint& p1, const FFPoint& p2);
 
 FFPoint operator + (const FFPoint& p) const;
 FFCoord operator * (const FFPoint& p) const;
 FFPoint operator * (FFCoord i) const;
 FFPoint operator / (FFCoord i) const;
 
 void operator -= (const FFPoint& p);
 void operator += (const FFPoint& p);
 void operator *= (FFCoord i);
 void operator /= (FFCoord i);
 
 FFPoint& operator = (const FFPoint& v);
 char * operator()() const;

 FFCoord abs() const;
 FFCoord abs2D() const;  
 FFCoord absabs2D() const;  
 FFCoord absabs() const;  
};

#endif
