/*
// This file is based on the scanner from the GraphLet project
//
// $Author: friedric $
// $Date: 1996/05/06 14:17:43 $
// $RCSfile: GTParser.y,v $
// $Revision: 1.5 $
// $State: Exp $
//
*/

/* (C) Universitaet Passau 1986-1991 */

%name GTScanner

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

#include "GTParser.h"

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
%define LEX_PARAM gtTokenValue &gtlval
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

class GTControl : private GTScanner, public GTParser
{
 public:
  GTControl ();
  virtual ~GTControl ();
  void gtPrintError (char* s);
  void Parse (void);
  void Parse (istream& input, FFGraphClass *g);
  void Parse (const String& iString, FFGraphClass *g);

 protected:
  int gtScan();
};
%}

{number} {
 gtlval.integer = atoi ((const char *)yytext);
 return GTParser::INTEGER;
}

{floatnumber}	{
 gtlval.real = atof ((const char *)yytext);
 return GTParser::REAL;
}

{quotation_mark}{character_in_string}*{quotation_mark}	{
 gtlval.str = strdup((const char *)(yytext+1));
 gtlval.str[strlen(gtlval.str)-1] = (char)0;
 
 return GTParser::STRING;
}

{left_brace}		return GTParser::BEGIN_LIST;
{right_brace}		return GTParser::END_LIST;
{left_bracket}		return GTParser::BEGIN_LIST;
{right_bracket}		return GTParser::END_LIST;


[a-zA-Z_][a-zA-Z0-9_:]*	{
 gtlval.key = strdup((const char *)yytext);
 return GTParser::KEY;
}


{newline}		{
 lineCount ++;
}

{white_space}			;
^{hash_mark}{anything}	; /* Comment */
.						; /* Error ... */
%%

int GTScanner::iInputCode(char *buffer, int &result, int max_size)
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

String GTScanner::escapesym (char c)
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

void GTControl::gtPrintError(char *s )
{
  strstream tmpS;
  tmpS << s << " in line " << lineCount+1 << ends;
  ffError(USER_ERROR, tmpS.str());
}

GTControl::GTControl() {};

GTControl::~GTControl() {};

int GTControl::gtScan()
{
  int t=GTControl::ScanValue(gtTheTokenValue);
  return t;
}

extern FFGraphClass *parseFFGraph;
extern FFNodeClass *currentNode;
extern FFEdgeClass *currentEdge;

// AVLSet<String> unmatchedLabels;

void GTControl::Parse(const String& iString, FFGraphClass *g)
{
  istrstream *input;
  current_node = NULL;
  current_edge = NULL;
  gmlHash = new AVLMap<int,FFNode>(NULL);
  
  parseFFGraph=g;
  lineCount=0;

  cerr << "Parsing GT Graph" << endl;
  
  //  unmatchedLabels.clear();
  
  iIn = input = new istrstream ((const char*)iString, iString.length ());
  gtParse ();
  delete (input);
  delete gmlHash;
}

void GTControl::Parse(istream& input, FFGraphClass *g)
{
  cerr << "Parsing GT Graph" << endl;

  current_node = NULL;
  current_edge = NULL;
  parseFFGraph= g;
  gmlHash = new AVLMap<int,FFNode>(NULL);
  
  lineCount=0;
  
  iIn = &input;
  gtParse();
  delete gmlHash;
}
