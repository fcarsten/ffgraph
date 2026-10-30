//-*-Mode: C++;-*-
#ifndef FFEDGELABEL
#define FFEDGELABEL

#include "FFLabelClass.h"
class FFEdgeClass;

//---- newclass -----------------------------------------------------------
#define ffEdgeLabel String("FFEdgeLabel")

class FFEdgeLabel : public FFLabelClass
{
friend class FFEdgeClass;

protected:
  FFEdgeClass *edge;

public:
  inline FFEdgeLabel(){edge = 0;
    _setType(ffEdgeLabel);}
  inline virtual void init( FFEdgeClass *e) { edge = e; }
  virtual FFEdgeLabel *proto() const = 0;
  inline virtual ~FFEdgeLabel(){}
};

#endif
