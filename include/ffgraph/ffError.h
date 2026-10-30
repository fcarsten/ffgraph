/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: holzmann $
// $Date: 1995/11/20 11:46:04 $
// $RCSfile: ffError.h,v $
// $Revision: 1.3 $
// $State: Exp $
//
// $Log: ffError.h,v $
 * Revision 1.3  1995/11/20  11:46:04  holzmann
 * gcc 2.7.1
 *
 * Revision 1.2  1995/11/15  12:51:21  friedric
 * Anpassung an gcc 2.7.1
 *
 * Revision 1.1  1995/11/01  12:02:20  friedric
 * Neue Graphen
 *
 * Revision 0.5.0.1  1995/10/20  12:13:29  friedric
 * test
 *
 * Revision 1.1  1994/09/05  15:39:25  friedric
 * Neue Verzeichnis Struktur. Auschecken mit -d
 *
 * Revision 1.9  1994/08/20  17:43:39  friedric
 * RCS Variablen eingefuegt
 *
*/

#ifndef FFERROR
#define FFERROR

typedef enum {
  NOTICE,
  USER_WARNING,
  USER_ERROR,
ERRORS_WITH_FILE_AND_LINENUMBER,
  RUNTIME_WARNING,
  RUNTIME_ERROR,
  INTERNAL_WARNING,
  INTERNAL_ERROR,
  CF_TCL,
ERRORS_WITH_PROGRAM_ABORT,
  FATAL
} errGrade;

typedef enum {
  NO_MEMORY,
  EMPTY_GRAPH,
  GRAPH_NOT_CONNECTED,
  NO_TREE,
  NO_GRAPHIK_CONTEXT,
  GRAPH_TOO_LARGE,
  GRAPH_TOO_WIDE,
  NODEREP_UNBOUND,
  EDGEREP_UNBOUND,
  NO_NODE_FOUND,
  UNKNOWN
} errKind;

void _ffError(errGrade grade, errKind kind, char const *file, int line);
void _ffError(errGrade grade, const char *msg, char const *file, int line);

typedef void (kindErrorHandler)(errGrade,errKind, const char *,int);
typedef void (charErrorHandler)(errGrade, const char *, const char *, int);

void setErrorHandlers(kindErrorHandler, charErrorHandler);

#define ffError(grade, kind) _ffError(grade, kind, __FILE__, __LINE__)

#endif
