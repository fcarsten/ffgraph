

/* (C) Universitaet Passau 1986-1991 */

%name FFScanner

%{/**********************************************************************/
/*									*/
/*				stdsanner.l				*/
/*									*/
/************************************************************************/


#include <String.h>
#include <stream.h>
#include <strstream.h>
#include <list>

#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>

#include "FFParser.h"


/* ** Marion : yywrap, yyerror muss fuer flex 2.4.6 zur Verfuegung gestellt werden! 
yywrap() { return 1; };

void yyerror(char *s) 
{ 
  cerr << "Scanner Error :" << s << endl;
}

*/

#define LEXDEBUG
%}

%header{
#include <list>
%}

%define LEX ScanValue
%define INPUT iInputCode
%define INPUT_NOCODE
%define LEX_PARAM tokenValue &fflval
%define MEMBERS \
        protected: \
        String stringValue; \
        std::list<String> tokenText; \
        String escapesym(char c); \
        istream* iIn; \
			int lineCount;

digit		[0-9]
number		-?{digit}+

left_brace	\{
right_brace	\}
no_right_brace	[^\}]
dollar		\$
no_dollar	[^\$]

begin_graphed_internals	{left_brace}{dollar}
end_graphed_internals	{dollar}{right_brace}

quotation_mark		\"
no_quotation_mark	[^\"]

white_space	[ \t]
newline		"\n"

%s string

%%

%header{
#include <strstream.h>
#include <fstream.h>

#include <ffgraph/ffError.h>
#include <ffgraph/FFGraphClass.h>

class FFControl : private FFScanner, public FFParser
{
 public:
  FFControl ();
  virtual ~FFControl ();
  void PrintError (char* s);
  void Parse (void);
  void Parse (istream& input, FFGraphClass *g);
  void Parse (const String& iString, FFGraphClass *g);

 protected:
  int Scan();
};
%}

<INITIAL>{number}	{
			/* Ganze Zahl (auch negativ !) */
  			fflval.nr = atoi ((const char *)yytext);
			return FFParser::NUMBER;
		}

<INITIAL>\" {BEGIN (string); stringValue = "";}

<string>\" {BEGIN (INITIAL); 
						tokenText.push_back(stringValue);
						fflval.text = tokenText.back().data();
						return FFParser::IDENTIFIER;}
<string>[^\"\\]+ {stringValue += (const char *)yytext; }
<string>"\\\\" {stringValue += "\\"; }
<string>"\\\n" {stringValue += "\n"; lineCount++;}
<string>\\.	{stringValue += escapesym (yytext[1]); }

{left_brace}{no_dollar}{no_right_brace}*	{

/* Kommentar : { irgendwas }		*/

input (); /* right_brace noch mitlesen	*/
}

{begin_graphed_internals}	{
return FFParser::BEGIN_GRAPHED_INTERNALS;
}

{end_graphed_internals}		{
return FFParser::END_GRAPHED_INTERNALS;
}


GRAPH		return FFParser::GRAPH;
END		return FFParser::END;
"="		return ('=');
";"		return (';');
GRAPHS		return FFParser::GRAPHS;
DIRECTED	return FFParser::DIRECTED;
UNDIRECTED	return FFParser::UNDIRECTED;
NP		return FFParser::NODEPLACE;
FF_C  return FFParser::FF_COLOR;
FF_3D  return FFParser::FF_COORD;
FF_R  return FFParser::FF_RADIUS;
FF_L  return FFParser::FF_LABEL;
FF_I  return FFParser::FF_ICON;
FF_NI  return FFParser::FF_NEW_ICON;
FF_WE  return FFParser::FF_WEIGHT;
FF_NM  return FFParser::FF_NEW_MASK;
FF_SC  return FFParser::FF_SCALE;
FF_RX  return FFParser::FF_ROTATE_X;
FF_RY  return FFParser::FF_ROTATE_Y;
FF_RZ  return FFParser::FF_ROTATE_Z;
EL		return FFParser::EDGEPLACE;
SN		return FFParser::STARTNODE;
EN		return FFParser::ENDNODE;
FF_LO return FFParser::FF_LABELOFFSET;

","		return (',');
[a-zA-Z]+	return FFParser::UNKNOWN_INTERNAL;


{newline}	{lineCount ++;}
{white_space}	;

.		{
			return yytext[0];
			/*  = alles hier nicht aufgefuehrte	*/
		}

%%

int FFScanner::iInputCode(char *buffer, int &result, int max_size)
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

String FFScanner::escapesym (char c)
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

void FFControl::PrintError(char *s )
{
  strstream tmpS;
  tmpS << s << " in line " << lineCount+1 << ends;
  ffError(USER_ERROR, tmpS.str());
}

FFControl::FFControl() {};

FFControl::~FFControl() {};

int FFControl::Scan()
{
  int t=FFControl::ScanValue(theTokenValue);
  return t;
}

extern FFGraphClass *parseFFGraph;
AVLSet<String> unmatchedLabels;

void FFControl::Parse(const String& iString, FFGraphClass *g)
{
  istrstream *input;
  parseFFGraph=g;
  lineCount=0;

  tokenText.clear();

  unmatchedLabels.clear();
  
  iIn = input = new istrstream ((const char*)iString, iString.length ());
  iParse ();
  delete (input);
}

void FFControl::Parse(istream& input, FFGraphClass *g)
{
  parseFFGraph= g;
  lineCount=0;

  tokenText.clear();
  
  iIn = &input;
  iParse();
}
