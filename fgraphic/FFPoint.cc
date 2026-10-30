/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:53 $
// $RCSfile: FFPoint.cc,v $
// $Revision: 1.2 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: friedric $ $RCSfile: FFPoint.cc,v $ $Date: 1995/11/01 12:02:53 $ $Revision: 1.2 $ $State: Exp $"

#include <cmath>

#include <globdefs.h>
#include <ffgraph/FFPoint.h>
#include <stream.h>
#include <sstream>
#include <string>
#include <string.h>

FFPoint::FFPoint()
{
	for(int i=0; i<FFPOINT_SIZE-1; i++)
		v[i]=0.0;
	v[FFPOINT_SIZE-1]= 1.0;
}

FFPoint::FFPoint(const FFPoint& p)
{
	for(int i=0; i<FFPOINT_SIZE; i++)
		v[i]= p.v[i];
}

 FFPoint::FFPoint(FFCoord nx, FFCoord ny, FFCoord nz, FFCoord nw)
{
  v[0] =nx;
  v[1] =ny;
  v[2] =nz;
  v[3] =nw;
}


BOOL FFPoint::operator == (const FFPoint& p) const
{
 BOOL tmpB=YES;
 for(int i=0; i<FFPOINT_SIZE; i++)
  tmpB = tmpB && (fabs(p.v[i] - v[i]) < FF_EPSILON);
 
 return tmpB;
 // Fremde bewegliche Sache i,242
}  

BOOL FFPoint::operator != (const FFPoint& p) const
{
 BOOL tmpB=NO;
 for(int i=0; i<FFPOINT_SIZE; i++)
  tmpB = tmpB || (fabs(p.v[i] - v[i]) > FF_EPSILON);
 
 return tmpB;
}  

FFPoint FFPoint::operator - () const
{	
	FFPoint tmpP;
	for(int i=0; i<FFPOINT_SIZE-1; i++)
		tmpP.v[i] = -v[i];	
  return tmpP;
}

FFPoint operator - (const FFPoint& p1, const FFPoint& p2) 
{
	FFPoint tmpP;
	for(int i=0; i<FFPOINT_SIZE-1; i++)
		tmpP.v[i] = p1.v[i] - p2.v[i];	

  return tmpP;
}

FFPoint FFPoint::operator + (const FFPoint& p) const
{
	FFPoint tmpP;
	for(int i=0; i<FFPOINT_SIZE-1; i++)
		tmpP.v[i] = v[i]+p.v[i];
		
  return tmpP;
}

FFCoord FFPoint::operator * (const FFPoint& p) const
{
	FFCoord x=0.0;
	for(int i=0; i<FFPOINT_SIZE-1; i++)
		x += 	v[i]*p.v[i];
		
  return x;
}

FFPoint FFPoint::operator * (FFCoord p) const
{
	FFPoint tmpP;
	for(int i=0; i<FFPOINT_SIZE-1; i++)
		tmpP.v[i] = v[i]*p;
		
  return tmpP;
}

FFPoint FFPoint::operator / (FFCoord p) const
{
  FFPoint tmpP;
  if(p == 0)
    p = 0.000000001;

  for(int i=0; i<FFPOINT_SIZE-1; i++)
    tmpP.v[i] = v[i]/p;
		
  return tmpP;
}

void FFPoint::operator /= (FFCoord p)
{
  if(p == 0)
    p = 0.000000001;

  for(int i=0; i<FFPOINT_SIZE-1; i++)
    v[i] /= p;
}

void FFPoint::operator *= (FFCoord p)
{
	for(int i=0; i<FFPOINT_SIZE-1; i++)
		v[i] += p;
}

void FFPoint::operator += (const FFPoint& p)
{
	for(int i=0; i<FFPOINT_SIZE-1; i++)
		v[i] += p.v[i];
}

void FFPoint::operator -= (const FFPoint& p)
{
	for(int i=0; i<FFPOINT_SIZE-1; i++)
		v[i] -= p.v[i];
}

FFPoint& FFPoint::operator = (const FFPoint& p)
{
	for(int i=0; i<FFPOINT_SIZE; i++)
		v[i] = p.v[i];
  return *this;
}

BOOL FFPoint::valid()
{
 for(int i=0; i<FFPOINT_SIZE; i++)
  if (!std::isfinite(v[i]))
   return NO;
 
 return YES;
}

char * FFPoint::operator()() const
{
 static std::string buffer;
 std::ostringstream out;
 out << "(" << v[0] << " , " << v[1] << " , " << v[2] << " , "
     << v[3] << ")";
 buffer = out.str();
 return buffer.data();
}

FFCoord FFPoint::abs2D() const
{
  return (hypot (v[0], v[1]));  
}

FFCoord FFPoint::absabs2D() const
{
 return (v[0]*v[0] + v[1]*v[1]);  
}

FFCoord FFPoint::abs() const
{
 FFCoord x =0;
 for(int i=0; i<FFPOINT_SIZE-1; i++)
  x += v[i]*v[i];
 return sqrt(x);
}

FFCoord FFPoint::absabs() const
{
 FFCoord x =0;
 for(int i=0; i<FFPOINT_SIZE-1; i++)
  x += v[i]*v[i];
 return x;
}
