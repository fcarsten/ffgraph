/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:08 $
// $RCSfile: FFLabelClass.h,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifndef FF_LABEL_CLASS
#define FF_LABEL_CLASS

#include <String.h>

#include "globdefs.h"

typedef enum {
 ADD_NODE,
 ADD_EDGE,
 DEL_NODE,
 DEL_EDGE,
 AFTER_DEL_NODE,
 AFTER_DEL_EDGE,
 DEL_GRAPH,
 REV_EDGE,
 SET_STARTNODE,
 DEL_STARTNODE,
 SET_ENDNODE,
 DEL_ENDNODE,
 NAME_CHANGE,
 ADD_NODELABEL,
 ADD_EDGELABEL,
 ADD_GRAPHLABEL,
 DEL_NODELABEL,
 DEL_EDGELABEL,
 DEL_GRAPHLABEL,
 SOURCE_CHANGE,
 TARGET_CHANGE,
 DIRECTEDNESS_CHANGE,
 START_LOAD,
 END_LOAD,
 MULTIPLE_CHANGES
} labelEvent;
	
class FFNodeClass;
class FFEdgeClass;
class FFGraphClass;

typedef union {
	FFNodeClass *node;
	FFEdgeClass *edge;
	FFGraphClass *graph;
	int index;
} labelEventStruct;

class FFLabelClass
{
private:
 String _type;
 
protected:
 int index;
 void _setType(const String& s);   
public:
 FFLabelClass(){index=-1;}
 virtual ~FFLabelClass(){}
 
 virtual BOOL parse(const String&) {return YES;}
 virtual String unparse() {return String("");}
 
 virtual const String& type() const {return _type;}
 virtual String baseType() const; 

 virtual BOOL isSaveable() const {return NO;}  // Shall we save the label 
 virtual BOOL shouldCopy() const {return YES;} // Shall we copy the label when copying the graph?
 virtual BOOL isInspectable() const {return NO;}
  
 virtual BOOL compare(const FFLabelClass*) const {return NO;}
 virtual void event(labelEvent, labelEventStruct *) {}
 virtual void assign(const FFLabelClass*) {}
 friend FFGraphClass;
 friend FFNodeClass;
 friend FFEdgeClass;
};
#endif
