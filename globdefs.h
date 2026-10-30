/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/10/25 16:13:02 $
// $RCSfile: ffGlobdefs.h.in,v $
// $Revision: 0.6 $
// $State: Exp $
//
*/

#ifndef GLOBDEF
#define GLOBDEF

// Load standard-library headers before legacy min/max macros are defined.
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <unistd.h>
using std::istream;
using std::ostream;
using std::iostream;
using std::cin;
using std::cout;
using std::cerr;
using std::clog;
using std::endl;

typedef int BOOL;

#define NO 0
#define YES 1

/* defines a floating point data type */

typedef float   FFCoord;

/* Defines initial capacity for some containers. Only change this if you know what
   you are doing !
   */

#ifndef DEFAULT_INITIAL_CAPACITY
#define DEFAULT_INITIAL_CAPACITY 5
#endif

/* Longitude of you Location */
#define AKTUL 13.4666666     /* Passau, Ludwigsplatz */

/* Lattitude of you Location */
#define AKTUB 48.5666666     /* Passau, Ludwigsplatz */

/* Horizontal distance to sea-level in kilometers */
#define UENN 0.302           /* Passau, Ludwigsplatz */

#define USE_VHSET
/* #define USE_VOHSET */  /* !!! Seems buggy !!! */  
/* #define USE_AVLSET */

/* You should not need to change anything below this */

#ifdef USE_VHSET
 #define FFSet VHSet
#else
#ifdef USE_AVLSET
 #define FFSet AVLSet 
#else 
#ifdef USE_VOHSET
 #define FFSet VOHSet
#endif /* VOH */
#endif /* AVL */
#endif /* VH */

/* Directory where the executable will be installed */

#define BINDIR "."

/* Directory where the tcl files will be installed */

#define FF_LIBDIR "./lib/ffgraph"

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

// #define CFCHAR(s) ((const char *)s)

typedef enum {
  KNOTEN,
  KANTE
} elementTyp;
	
/* typedef enum {
  NO_ACTION,
  CREATE_MODE,
  EDIT_MODE
} workingMode;
*/

typedef enum {
  UNDEF=0,
  ZOMBIE,
  VALID
} stateOfGraphElement;
  
#endif
