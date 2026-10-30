//-*-Mode: C++;-*-
#ifndef _newclass_h_
#define _newclass_h_

#if __GNUG__ >= 2
#  pragma interface
#endif

#include <globdefs.h>
#include <ffgraph/FFPoint.h>

//---- newclass -----------------------------------------------------------

inline FFCoord rad(FFCoord x)
{
 return(x*M_PI/180);
}

inline FFCoord deg(FFCoord x)
{
 return(x*180/M_PI);
}

class FFMatrix 
{
	
public:
 FFPoint row[FFPOINT_SIZE];
 FFMatrix(const FFMatrix&);
 FFMatrix();
 FFMatrix(const FFPoint&, const FFPoint&, 
          const FFPoint&, const FFPoint&);
 
 BOOL operator == (const FFMatrix&) const;
 
 FFMatrix operator-() const;
 friend FFMatrix operator - (const FFMatrix&, const FFMatrix&);
 
 FFMatrix operator + (const FFMatrix&) const;
 FFMatrix operator* (const FFMatrix&) const;
 FFMatrix operator* (FFCoord) const;
 FFPoint operator* (const FFPoint&) const;
 friend FFPoint operator* (const FFPoint&, const FFMatrix&);
 FFMatrix operator / (FFCoord) const;
 
 void operator -= (const FFMatrix&);
 void operator += (const FFMatrix&);
 void operator *= (FFCoord);
 void operator *= (const FFMatrix&);
 void operator /= (FFCoord);
 
 FFMatrix& operator = (const FFMatrix&);
 char * operator()() const;
};

FFPoint rotateZ(const FFPoint&, FFCoord);
// FFPoint scale(const FFPoint&, FFCoord);
// FFPoint translate(const FFPoint&, FFCoord, FFCoord);

// void rotate(FFMatrix&, FFCoord);
// void skew(FFMatrix&, FFCoord);
// void italic(FFMatrix&, FFCoord);
// void scale(FFMatrix&, FFCoord);
// void scale(FFMatrix&, FFCoord, FFCoord);
// void translate(FFMatrix&, FFCoord, FFCoord);
// void flipX(FFMatrix&);
// void flipY(FFMatrix&);
// FFPoint flipAB(const FFPoint&, const FFPoint&, const FFPoint&);

// FFCoord getAngel(const FFPoint& );

#endif
