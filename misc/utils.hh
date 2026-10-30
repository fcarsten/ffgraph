// -*- C++ -*-
//
// File:        misc / utils.hh
//
// $Id: utils.hh,v 1.1.1.1 1995/10/27 12:39:05 classen Exp $
//
//

#ifndef _utils_hh
#define _utils_hh

#include <String.h>
#include <DLList.h>

String listToTclList(DLList<String>& sList);
char* listToTclListChar(DLList<String>& sList);

void tclListToList(String tclList, DLList<String>& sList);
DLList<String> tclListToList(String tclList);

// Remove leading and trailing spaces
String stripSurroundingSpaces(String s);
void splitAtUnquotedSpaces(String inputString, DLList<String>& sList);
DLList<String> splitAtUnquotedSpaces(String inputString);

#endif
//EOF





