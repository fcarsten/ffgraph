//-*-Mode: C++;-*-
#ifndef _FFLAYOUT_LABEL_
#define _FFLAYOUT_LABEL_

#include <ffgraph/FFGraphLabel.h>

//---- newclass -----------------------------------------------------------

#define ffLayoutLabelType String("FFLayoutLabel")
class FFNodeClass;
class FFLayoutLabel;

class FFLayoutLabel : public FFGraphLabel
{
public:
    FFLayoutLabel() {_setType(ffLayoutLabelType);}
    virtual ~FFLayoutLabel() {}
    
    virtual void layout(FFSet <FFNodeClass *>&) {}
    virtual void layout() {}
};

#endif
