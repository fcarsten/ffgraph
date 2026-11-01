/*
// This file is based on the scanner from the GraphLet project
//
// $Author: friedric $
// $Date: 1996/05/06 14:17:43 $
// $RCSfile: RomeParser.y,v $
// $Revision: 1.5 $
// $State: Exp $
//
*/

/* (C) Universitaet Passau 1986-1991 */

%name RomeScanner

%{/**********************************************************************/
/*									*/
/*				stdsanner.l				*/
/*									*/
/************************************************************************/


#include <String.h>
#include <stream.h>
#include <strstream.h>

#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <container/AVLMap.h>

#include "RomeParser.h"

const int SCANNER_KEY_MAXLEN=1000;

/* ** Marion : yywrap, yyerror muss fuer flex 2.4.6 zur Verfuegung
gestellt werden!

yywrap() { return 1; };

void yyerror(char *s) 
{ 
  cerr << "Scanner Error :" << s << endl;
}

*/

#define LEXDEBUG
%}

%define LEX ScanValue
%define INPUT iInputCode
%define INPUT_NOCODE
%define LEX_PARAM romeTokenValue &romelval
%define MEMBERS \
        protected: \
        String stringValue; \
        String escapesym(char c); \
        istream* iIn; \
			int lineCount;

digit		[0-9]
number		-?{digit}+
floatnumber			-?{digit}+\.{digit}+

left_brace	\{
right_brace	\}
left_bracket		\[
right_bracket		\]


quotation_mark					\"
no_quotation_mark				[^\"]
escaped_character				\\.
escaped_newline					\\\n
no_quotation_mark_or_backslash	[^\"\\]
character_in_string  ({escaped_character}|{escaped_newline}|{no_quotation_mark_or_backslash})

white_space						[ \t]
newline							"\n"

hash_mark						\#
anything						.*

%s string
%%

%header{
#include <strstream.h>
#include <fstream.h>

#include <ffgraph/ffError.h>
#include <ffgraph/FFGraphClass.h>

class RomeControl : private RomeScanner, public RomeParser
{
 public:
  RomeControl ();
  virtual ~RomeControl ();
  void romePrintError (char* s);
  void Parse (void);
  void Parse (istream& input, FFGraphClass *g);
  void Parse (const String& iString, FFGraphClass *g);

 protected:
  int romeScan();
};
%}

{number} {
 romelval.integer = atoi ((const char *)yytext);
 return RomeParser::INTEGER;
}

{floatnumber}	{
 romelval.real = atof ((const char *)yytext);
 return RomeParser::REAL;
}

{quotation_mark}{character_in_string}*{quotation_mark}	{
 // STRING and KEY are always rejected by the Rome grammar. Keep their
 // text in scanner-owned storage while the parser reports that error.
 stringValue = String((const char *)(yytext+1), strlen((const char *)yytext)-2);
 romelval.str = stringValue.data();
 
 return RomeParser::STRING;
}

{left_brace}		return RomeParser::BEGIN_LIST;
{right_brace}		return RomeParser::END_LIST;
{left_bracket}		return RomeParser::BEGIN_LIST;
{right_bracket}		return RomeParser::END_LIST;
{hash_mark}             return RomeParser::HASHMARK;
{newline}              return RomeParser::NEWLINE;
[a-zA-Z_][a-zA-Z0-9_:]*	{
 stringValue = (const char *)yytext;
 romelval.key = stringValue.data();
 return RomeParser::KEY;
}


{newline}		{
 lineCount ++;
}

{white_space}			;
^{hash_mark}{anything}	; /* Comment */
.						; /* Error ... */
%%

int RomeScanner::iInputCode(char *buffer, int &result, int max_size)
{
  if( iIn->eof() || iIn->fail() )
    {
      result = YY_NULL;
    } 
  else 
    {
      iIn->getline(buffer, max_size - 2, '\n');
      result = strlen (buffer);
      buffer[result] = '\n';
      buffer[result+1] = 0;
      result += 1;
      if( iIn->bad() ) 
	{
	  YY_FATAL_ERROR( "read() in flex scanner failed" );
	}
    }
  return(result);
}

String RomeScanner::escapesym (char c)
{
  String ret;
  
  switch (c)
    {
    case '\\': ret = "\\"; break;
    case '\"': ret = "\""; break;
    default: ret = c;
    }
  return ret;
}

void RomeControl::romePrintError(char *s )
{
  strstream tmpS;
  tmpS << s << " in line " << lineCount+1 << ends;
  ffError(USER_ERROR, tmpS.str());
}

RomeControl::RomeControl() {};

RomeControl::~RomeControl() {
  if (yy_current_buffer) yy_delete_buffer(yy_current_buffer);
}

int RomeControl::romeScan()
{
  int t=RomeControl::ScanValue(romeTheTokenValue);
  return t;
}

extern FFGraphClass *parseFFGraph;
extern FFNodeClass *currentNode;
extern FFEdgeClass *currentEdge;

// AVLSet<String> unmatchedLabels;

void RomeControl::Parse(const String& iString, FFGraphClass *g)
{
  istrstream *input;
  current_node = NULL;
  current_edge = NULL;
  gmlHash = new AVLMap<int,FFNode>(NULL);
  
  parseFFGraph=g;
  lineCount=0;
  parseFFGraph->_setDirected(NO);
  
  cerr << "Parsing Rome Graph" << endl;
  
  //  unmatchedLabels.clear();
  
  iIn = input = new istrstream ((const char*)iString, iString.length ());
  romeParse ();
  delete (input);
  delete gmlHash;
}

void RomeControl::Parse(istream& input, FFGraphClass *g)
{
  cerr << "Parsing Rome Graph" << endl;

  current_node = NULL;
  current_edge = NULL;
  parseFFGraph= g;
  parseFFGraph->_setDirected(NO);
  gmlHash = new AVLMap<int,FFNode>(NULL);
  
  lineCount=0;
  
  iIn = &input;
  romeParse();
  delete gmlHash;
}
