#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <ffgraph/FFMatrix.h>
#include <math.h>
#include <string.h>
#include <stream.h>
#include <sstream>
#include <string>

//---- newclass ---------------------------------------------------------

FFMatrix::FFMatrix(const FFMatrix& m)
{
 for(int i= 0; i<FFPOINT_SIZE; i++)
  row[i]=m.row[i];
}

FFMatrix::FFMatrix()
{
}

FFMatrix::FFMatrix(const FFPoint& a, const FFPoint& b, 
                   const FFPoint& c, const FFPoint& d)
{
 row[0]=a;
 row[1]=b;
 row[2]=c;
 row[3]=d;
}

BOOL FFMatrix::operator==(const FFMatrix& m) const
{
 BOOL tmpB = YES;
 for(int i= 0; i<FFPOINT_SIZE; i++)
  tmpB = tmpB && (row[i] == m.row[i]);
 
 return tmpB;
}

FFMatrix FFMatrix::operator-() const
{
 FFMatrix tmpM;
 for(int i= 0; i<FFPOINT_SIZE; i++)
  tmpM.row[i] = -row[i];	

 return tmpM;
}

FFMatrix operator - (const FFMatrix& m1, const FFMatrix& m2)
{
 FFMatrix tmpM;
 for(int i= 0; i<FFPOINT_SIZE; i++)
  tmpM.row[i] = m1.row[i] - m2.row[i];	
 return tmpM;
}

FFMatrix FFMatrix::operator+(const FFMatrix& m) const
{
 FFMatrix tmpM;
 for(int i= 0; i<FFPOINT_SIZE; i++)
  tmpM.row[i] = row[i] + m.row[i];	

 return tmpM;
}

FFMatrix FFMatrix::operator*(FFCoord x) const
{
 FFMatrix tmpM;
 for(int i= 0; i<FFPOINT_SIZE; i++)
  tmpM.row[i] = row[i] *x;	

 return tmpM;
}

FFMatrix FFMatrix::operator*(const FFMatrix& m) const
{ 
 FFMatrix tmpM;
	
 for(int i= 0; i<FFPOINT_SIZE; i++)
  {
   for(int k=0; k<FFPOINT_SIZE; k++)
    {
     tmpM.row[i].v[k] = 0.0;
     for(int j=0; j<FFPOINT_SIZE; j++)
      {
       tmpM.row[i].v[k] += row[i].v[j]*m.row[j].v[k];
      }
    }
  }
 return tmpM;
}

FFPoint FFMatrix::operator*(const FFPoint& p) const
{
 FFPoint tmpP;
	
 for(int i= 0; i<FFPOINT_SIZE; i++)
  {	
   tmpP.v[i] = 0.0;
   for(int k=0; k<FFPOINT_SIZE; k++)
    tmpP.v[i] += row[i].v[k]* p.v[k];
  }	
 return tmpP;
}
	
FFPoint operator* (const FFPoint& p, const FFMatrix& m) 
{
 FFPoint tmpP;
 for(int i=0; i<FFPOINT_SIZE; i++)
  {
   tmpP.v[i] = 0.0;
   for(int k=0; k<FFPOINT_SIZE; k++)
    tmpP.v[i] += m.row[k].v[i]*p.v[k];
  }

 return tmpP;
}	

FFMatrix FFMatrix::operator / (FFCoord x) const
{
 FFMatrix tmpM;
 for(int i= 0; i<FFPOINT_SIZE; i++)
  tmpM.row[i] = row[i]/x;
		
 return tmpM;
}

void FFMatrix::operator -=(const FFMatrix& m)
{
 for(int i= 0; i<FFPOINT_SIZE; i++)
  row[i] -= m.row[i];
}

void FFMatrix::operator +=(const FFMatrix& m)
{
 for(int i= 0; i<FFPOINT_SIZE; i++)
  row[i] += m.row[i];
}

void FFMatrix::operator *=(const FFMatrix& m)
{
 *this = *this*m;
}	

void FFMatrix::operator *=(FFCoord x)
{
 for(int i= 0; i<FFPOINT_SIZE; i++)
  row[i] *= x;
}

void FFMatrix::operator /=(FFCoord x)
{
 for(int i= 0; i<FFPOINT_SIZE; i++)
  row[i] /= x;
}

FFMatrix& FFMatrix::operator = (const FFMatrix& m)
{
 for(int i= 0; i<FFPOINT_SIZE; i++)
  row[i] = m.row[i];
 return *this;
}

char * FFMatrix::operator()() const
{
 static std::string buffer;
 std::ostringstream out;
 out << "| " << row[0]() << " |\n| " << row[1]() << " |\n| " <<
  row[2]() << " |\n| " << row[3]() << " |";
 buffer = out.str();
 return buffer.data();
}



FFPoint rotateZ(const FFPoint& p, FFCoord w)
{
 FFPoint tmpP(cos(rad(w)),sin(rad(w)),0);
 FFMatrix tmpM(tmpP,
               FFPoint(-sin(rad(w)),cos(rad(w)),0),
               FFPoint(0,0,1,0), FFPoint(0,0,0,1));
 
 return p*tmpM;
}

// void scale(FFMatrix& m, FFCoord s)
// {
//  FFMatrix tmpM(FFPoint(s,0,0), FFPoint(0,s,0),
//                FFPoint(0,0,1));
//  m = tmpM * m;
//  // m = m * tmpM;
// }

// void scale(FFMatrix& m, FFCoord s, FFCoord s2)
// {
//  FFMatrix tmpM(FFPoint(s,0,0), FFPoint(0,s2,0),
//                FFPoint(0,0,1));
//  m = tmpM * m;
//  // m = m * tmpM;
// }

// void translate(FFMatrix& m, FFCoord x, FFCoord y)
// {
//  FFMatrix tmpM(FFPoint(1,0,0), FFPoint(0,1,0),
//                FFPoint(x,y,1));
//  m = tmpM * m;
//  // m = m * tmpM;
// }

// void skew(FFMatrix& m, FFCoord w)
// {
//  FFMatrix tmpM(FFPoint(1,0,0),
//                FFPoint(-sin(rad(w)),cos(rad(w)),0),
//                FFPoint(0,0,1));
 
//  m = tmpM * m;
// }

// void italic(FFMatrix& m, FFCoord w)
// {
//  FFMatrix tmpM(FFPoint(1,0,0),
//                FFPoint(-tan(rad(w)),1,0),
//                FFPoint(0,0,1));
 
//  m = tmpM * m;
// }

// void flipX(FFMatrix& m)
// {
//  FFMatrix tmpM(FFPoint(1,0,0),
//                FFPoint(0,-1,0),
//                FFPoint(0,0,1));
 
//  m = tmpM * m;
// }

// void flipY(FFMatrix& m)
// {
//  FFMatrix tmpM(FFPoint(-11,0,0),
//                FFPoint(0,1,0),
//                FFPoint(0,0,1));
 
//  m = tmpM * m;
// }

// FFPoint rotate(const FFPoint& p, FFCoord w)
// {
//  FFPoint tmpP(cos(rad(w)),sin(rad(w)),0);
 
//  FFMatrix tmpM(tmpP,
//                FFPoint(-sin(rad(w)),cos(rad(w)),0),
//                FFPoint(0,0,1));

//  return p * tmpM;
// }

// FFPoint scale(const FFPoint& p, FFCoord s)
// {
//  FFMatrix tmpM(FFPoint(s,0,0), FFPoint(0,s,0),
//                FFPoint(0,0,1));
//  return p * tmpM;
// }

// FFPoint translate(const FFPoint& p, FFCoord x, FFCoord y)
// {
//  FFMatrix tmpM(FFPoint(1,0,0), FFPoint(0,1,0),
//                FFPoint(x,y,1));
//  return p*tmpM;
// }

// FFPoint flipAB(const FFPoint& p, const FFPoint& a, const FFPoint& b)
// {
//  FFPoint p2 = p-a;
//  FFPoint b2 = b-a;
//  FFCoord alpha = getAngel(b2);
//  p2 = rotate(p2, alpha);
//  FFPoint p3 = FFPoint(-p2.getX(), p2.getY());
//  p3 = rotate(p3, -alpha);
//  return p3+a;
// }

// FFCoord getAngel(const FFPoint& tmpP)
// {
//  if(tmpP.getX() > 0)
//   return -deg(acos(-tmpP.getY() / tmpP.abs2D()));
  
//  return deg(acos(-tmpP.getY() / tmpP.abs2D()));
// }
