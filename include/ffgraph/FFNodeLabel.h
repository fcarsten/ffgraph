//-*-Mode: C++;-*-
#ifndef FFNODELABEL
#define FFNODELABEL

#include "FFLabelClass.h"
class FFNodeClass;

//---- newclass -----------------------------------------------------------

#define ffNodeLabel String("FFNodeLabel")

class FFNodeLabel : public FFLabelClass
{
  friend class FFNodeClass;

protected:
  FFNodeClass *node;

public:
	inline FFNodeLabel(){node = 0; _setType(ffNodeLabel);}
	inline virtual void init( FFNodeClass *n) { node = n; }
	virtual FFNodeLabel *proto() const = 0;
	inline virtual ~FFNodeLabel(){}
};

#endif
