//-*-Mode: C++;-*-
#ifndef FFGRAPHLABEL
#define FFGRAPHLABEL

#include "FFLabelClass.h"
#include "FFGraph.h"

//---- newclass -----------------------------------------------------------

#define ffGraphLabel String ("FFGraphLabel")
class FFInspector;

class FFGraphLabel : public FFLabelClass
{
protected:
 FFGraph graph;
 FFInspector *inspector;
 virtual void _buildInspector() {};
 
public:
  FFGraphLabel()
    {inspector = 0; _setType(ffGraphLabel);}
  virtual void init(const FFGraph& g) { graph = g; }
  virtual FFGraphLabel *proto() const = 0;
  virtual BOOL hasOwnInspector() const; 
  virtual void showInspector();
  virtual void updateInspector();
    
  virtual FFGraph getGraph() {return graph;}
  virtual ~FFGraphLabel();
};

#endif
