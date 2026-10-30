// -*- C++ -*-
//
// File:        misc / utils.cc
//
// $Id: utils.cc,v 1.4 1996/03/11 18:51:00 classen Exp $
//
//

#include <string.h>
#include <String.h>
#include <DLList.h>
#include <Pix.h>
#include <assert.h>

#include "globdefs.h"
#include <misc/utils.hh>

String listToTclList(DLList<String>& sList)
{
  String resultS = String("{");
  // The "inside" flag controls that spaces are only inserted between
  // entries and not in front of the first entry...
  BOOL inside = NO;
  for (Pix i = sList.first(); i != 0; sList.next(i), inside = YES) {
    if (inside) resultS += String(" ");
    resultS += String("{") + sList(i) + String("}");
  }
  resultS += String("}");

  return resultS;
}

char* listToTclListChar(DLList<String>& sList)
{
  String resultS = listToTclList(sList);
  assert (resultS != String(""));

  char* resultPtr = strdup((const char *)(resultS));
  assert (resultPtr != NULL);
  return resultPtr;
}

void tclListToList(String tclList, DLList<String>& sList)
{
  sList.clear();

  // Try to find out whether the whole Tcl list 
  // is an unbraced list, meaning that it is NOT enclosed in curly braces.
  // Also checks for unbalanced parenthesis.
  BOOL unbracedList = NO;
  int braceLevel = 0; // needed for lists {abc def} ghi
  int numOfBottomLevelBraces = 0; // needed for lists {abc def} {ghi jkl}
  BOOL backSlashWasLast = NO;
  for (int i = 0; (unsigned)i < tclList.length(); i++) {
    String currCharS = tclList.at(i,1);
    const char currChar = *(const char *)(currCharS);
    switch (currChar) {
    case '{': {
      if (! backSlashWasLast) {
        // This is a "real" Tcl brace, no escaped one...
        braceLevel++;
        continue;
      }
      // So we have an escaped opening brace.
      // Treat it as an ordinary character.  
      backSlashWasLast = NO;
      // The following is the all-important test:
      if (braceLevel == 0)
        unbracedList = YES;
      continue;
    }
    case '}': {
      if (! backSlashWasLast) {
        // This is a "real" Tcl brace, no escaped one...
        braceLevel--;
        if (braceLevel < 0) {
#ifdef DEBUG
          cerr << "Too many closing braces in " << tclList << endl;
#endif
          return;
        }
        if (braceLevel == 0)
          numOfBottomLevelBraces++;

        continue;
      }
      // So we have an escaped closing brace.
      // Treat it as an ordinary character.  
      backSlashWasLast = NO;
      // The following is the all-important test:
      if (braceLevel == 0)
        unbracedList = YES;
      continue;
    }
    case '\\': {
      if (backSlashWasLast) {
        // This is an escaped backslash.
        // Treat it as an ordinary character.  
        // The following is the all-important test:
        if (braceLevel == 0)
          unbracedList = YES;
      }
      backSlashWasLast = !backSlashWasLast;
      continue;
    }
    case ' ': {
      backSlashWasLast = NO;
      continue;
    }
    default: {
      // We don't care here whether backslash was last or not...
      backSlashWasLast = NO;
      // The following is the all-important test:
      if (braceLevel == 0)
        unbracedList = YES;
    }
    }
  } // end-for
      
  if (braceLevel > 0) {
#ifdef DEBUG
    cerr << "Missing closing brace in " << tclList << endl;
#endif
  }

  if (unbracedList || numOfBottomLevelBraces > 1)
    tclList = String("{") + tclList + String("}");

  BOOL afterFirstBrace = NO;
  BOOL insideEntry = NO;
  BOOL unbracedEntry = NO;
  backSlashWasLast = NO;
  int internalParenthesis = 0;
  String entry;
  for (int i = 0; (unsigned)i < tclList.length(); i++) {
    String currCharS = tclList.at(i,1);
    const char currChar = *(const char *)(currCharS);
    switch (currChar) {
    case '{': {
      if (backSlashWasLast) {
        backSlashWasLast = NO;
        entry += String("\\") + currCharS;
        continue;
      }
      if (! afterFirstBrace) {
        afterFirstBrace = YES;
        backSlashWasLast = NO;
        continue;
      }
      if (! insideEntry) {
        insideEntry = YES;
        unbracedEntry = NO;
        backSlashWasLast = NO;
        entry = String("");
        continue;
      }
      // So we have insideEntry == YES and backSlashWasLast == NO.
      // We only want to extract the top-level entries (which might be
      // Tcl-lists themselves).
      internalParenthesis++;
      entry += currCharS;
      continue;
      break;
    }
    case '}': {
      if (backSlashWasLast) {
        backSlashWasLast = NO;
        entry += String("\\") + currCharS;
        continue;
      }
      if (! afterFirstBrace) {
        cerr << "Error in tclListToList: illegal list beginning in "
             << tclList << endl;
        return;
      }
      if (! insideEntry) {
        // We are finished...
        return;
      }
      if (internalParenthesis == 0) {
        insideEntry = NO;
        backSlashWasLast = NO;
        sList.append(entry);
        entry = String("");
        continue;
      } 
      else {
        internalParenthesis--;
        entry += currCharS;
        continue;
      }
      break;
    }
    case '\\': {
      if (backSlashWasLast) {
        backSlashWasLast = NO;
        entry += String("\\\\");
        continue;
      }
      if (! insideEntry) {
        // This must be an entry delimited by spaces...
        unbracedEntry = YES;
        insideEntry = YES;
      }
      backSlashWasLast = YES;
      continue;
      break;
    }
    case ' ': {
      backSlashWasLast = NO;
      // Ignore leading spaces
      if (! afterFirstBrace) continue;
      if (insideEntry) {
        if (unbracedEntry) {
          // Entry delimited by spaces. Treat this space like a
          // closing brace...
          insideEntry = NO;
          unbracedEntry = NO;
          sList.append(entry);
          entry = String("");
          continue;
        }
        else {
          entry += currCharS;
          continue;
        }
      }
      else { // !insideEntry -> ignore
        continue;
      }
      break;
    }
    default: {
      if (backSlashWasLast) {
        // We have an escape sequence...
        backSlashWasLast = NO;
#ifdef DEBUG
        cerr << "Warning in tclListToList: escape sequence?? " 
             << tclList << endl;
#endif
        entry += currCharS;
        continue;
      }
      if (! afterFirstBrace) {
        cerr << "Error in tclListToList: illegal list beginning in "
             << tclList << endl;
        return;
      }
      if (! insideEntry) {
        // This must be an entry delimited by spaces...
        unbracedEntry = YES;
        insideEntry = YES;
        backSlashWasLast = NO;
        entry = currCharS;
        continue;
      }
      entry += currCharS;
      continue;
    }
    }
  } // end-for
}

DLList<String> tclListToList(String tclList)
{
  DLList<String> resultList;
  tclListToList(tclList, resultList);
  return resultList;
}


// Remove leading and trailing spaces
String stripSurroundingSpaces(String s)
{
  BOOL found;
  do {
    found = NO;
    if (s.at(0,1) == String(" ")) {
      found = YES;
      s.erase(0, 1);
    }
    int lastpos = s.length()-1;
    if (s.at(lastpos, 1) == String(" ")) {
      found = YES;
      s.erase(static_cast<String::size_type>(lastpos), 1);
    }
  } while (found);
  return s;
}

void splitAtUnquotedSpaces(String inputString, DLList<String>& sList)
{
  sList.clear();
  BOOL insideEntry = NO;
  BOOL unbracedEntry = NO;
  BOOL backSlashWasLast = NO;
  String entry;
  for (int i = 0; (unsigned)i < inputString.length(); i++) {
    String currCharS = inputString.at(i,1);
    const char currChar = *(const char *)(currCharS);
    switch (currChar) {
    case '\'': {
      if (! insideEntry) {
        insideEntry = YES;
        unbracedEntry = NO;
        backSlashWasLast = NO;
        entry = String("");
        continue;
      }
      if (backSlashWasLast) {
        backSlashWasLast = NO;
        entry += String("\\") + currCharS;
        continue;
      }
      backSlashWasLast = NO;
      // The following allows unescaped quotes in unquoted entries:
      if (unbracedEntry) {
        entry += currCharS;
        continue;
      }
      // So we have insideEntry == YES and no backslash which
      // means that this must be the terminating quote...
      insideEntry = NO;
      sList.append(entry);
      entry = String("");
      continue;
      break;
    }
    case '\\': {
      if (backSlashWasLast) {
        backSlashWasLast = NO;
        entry += String("\\\\");
        continue;
      }
      if (! insideEntry) {
        // This must be an entry delimited by spaces...
        unbracedEntry = YES;
        insideEntry = YES;
      }
      backSlashWasLast = YES;
      continue;
      break;
    }
    case ' ': {
      backSlashWasLast = NO;
      // Ignore leading spaces
      if (insideEntry) {
        if (unbracedEntry) {
          // Entry delimited by spaces. Treat this space like a
          // closing brace...
          insideEntry = NO;
          unbracedEntry = NO;
          sList.append(entry);
          entry = String("");
          continue;
        }
        else {
          entry += currCharS;
          continue;
        }
      }
      else { // !insideEntry -> ignore this blank, it separates entries
        continue;
      }
      break;
    }
    default: {
      if (! insideEntry) {
        // This must be an entry delimited by spaces...
        unbracedEntry = YES;
        insideEntry = YES;
        backSlashWasLast = NO;
        entry = currCharS;
        continue;
      }
      if (backSlashWasLast) {
        // We have an escape sequence...
        backSlashWasLast = NO;
#ifdef DEBUG
        cerr << "Warning in inputStringToList: escape sequence?? " 
             << inputString << endl;
#endif
        entry += currCharS;
        continue;
      }
      entry += currCharS;
      continue;
    }
    }
  } // end-for
}

DLList<String> splitAtUnquotedSpaces(String inputString)
{
  DLList<String> resultList;
  splitAtUnquotedSpaces(inputString, resultList);
  return resultList;
}


// EOF
