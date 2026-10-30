#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFInspector.h>

BOOL FFGraphLabel::hasOwnInspector() const
{
//   if(inspector != NULL)
//     return YES;
  return NO;
}

void FFGraphLabel::updateInspector()
{
 if(!inspector)
  _buildInspector();
 
 if(inspector)
  inspector->inspect();
}

void FFGraphLabel::showInspector()
{
 if(!inspector)
  _buildInspector();

 if(inspector)
  {
   inspector->show();
   inspector->inspect();
  }
}

FFGraphLabel::~FFGraphLabel()
{
 if(inspector)
  {
   inspector->hide();
   delete inspector;
   inspector = 0;
  }  
}
