/* 
 * tkImgPmap.c --
 *
 *	This procedure implements images of type "pixmap" for Tk. The only
 *      portable file format for this kind of colorfull pixel data is
 *	the XPM file format (my greetings to Arnaud Le Hors!)
 *
 * Copyright (c) 1994 The Regents of the University of California.
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 *
 * Derived from tkImgBmap.c as a template by Harald Albrecht
 * (albrecht@igpm.rwth-aachen.de)
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* Sorry -- no sccsid here, I'm doing it the traditional way round... */
/* Last update: 95/03/30                                              */

#include <ffgraph/ffConfig.h>
#include <tk.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_X11_XPM_H
#if(TK_MAJOR_VERSION >= 4)

#include <X11/xpm.h>

typedef void *Tk_PixmapHandle;

/*
 * The following data structure represents the master for a pixmap
 * image:
 */

typedef struct PixmapMaster {
    Tk_ImageMaster tkMaster;	/* Tk's token for image master. */
    Tcl_Interp *interp;		/* Interpreter for application that is
				 * using image. */
    Tcl_Command imageCmd;	/* Token for image command (used to delete
				 * it when the image goes away).  NULL means
				 * the image command has already been
				 * deleted. */
    int width, height;		/* Dimensions of image. */
    char *data;			/* Data comprising xpm pixmap.  May
				 * be NULL if no data.  Malloc'ed. */
    char *maskData;             /* Mask Data added by Carsten Friedrich */
    Tk_Uid bgUid;		/* Value of -background option (malloc'ed). */
    char *fileString;		/* Value of -file option (malloc'ed). */
    char *dataString;		/* Value of -data option (malloc'ed). */
    char *maskFileString;	/* Value of -maskfile option (malloc'ed). */
    char *maskDataString;	/* Value of -maskdata option (malloc'ed). */
    struct PixmapInstance *instancePtr;
				/* First in list of all instances associated
				 * with this master. */
} PixmapMaster;

/*
 * The following data structure represents all of the instances of an
 * image that lie within a particular window:
 */

typedef struct PixmapInstance {
    int refCount;		/* Number of instances that share this
				 * data structure. */
    PixmapMaster *masterPtr;	/* Pointer to master for image. */
    Tk_Window tkwin;		/* Window in which the instances will be
				 * displayed. */
    XColor *bg;			/* Background color for displaying image. */
    Pixmap bitmap;		/* The bitmap to display. */
    Pixmap mask;		/* Mask: only display bitmap pixels where
				 * there are 1's here. */
    GC gc;			/* Graphics context for displaying bitmap.
				 * None means there was an error while
				 * setting up the instance, so it cannot
				 * be displayed. */
    struct PixmapInstance *nextPtr;
				/* Next in list of all instance structures
				 * associated with masterPtr (NULL means
				 * end of list). */
} PixmapInstance;


/*
 * Hash table used to provide access to photo images from C code.
 */

static Tcl_HashTable imgPixmapHash;
static int imgPixmapHashInitialized;	/* set when Tcl_InitHashTable done */



/*
 * The type record for pixmap images:
 */

static int		ImgPmapCreate _ANSI_ARGS_((Tcl_Interp *interp,
			    const char *name, int objc, Tcl_Obj *const objv[],
			    const Tk_ImageType *typePtr, Tk_ImageMaster master,
			    ClientData *clientDataPtr));
static ClientData	ImgPmapGet _ANSI_ARGS_((Tk_Window tkwin,
			    ClientData clientData));
static void		ImgPmapDisplay _ANSI_ARGS_((ClientData clientData,
			    Display *display, Drawable drawable, 
			    int imageX, int imageY, int width,
			    int height, int drawableX,
			    int drawableY));
static void		ImgPmapFree _ANSI_ARGS_((ClientData clientData,
			    Display *display));
static void		ImgPmapDelete _ANSI_ARGS_((ClientData clientData));

Tk_ImageType tkPixmapImageType = {
    "pixmap",			/* name */
    ImgPmapCreate,		/* createProc */
    ImgPmapGet,			/* getProc */
    ImgPmapDisplay,		/* displayProc */
    ImgPmapFree,		/* freeProc */
    ImgPmapDelete,		/* deleteProc */
    NULL,			/* postscriptProc */
    NULL,			/* nextPtr */
    NULL			/* reserved */
};

/*
 * Information used for parsing configuration specs:
 */

static Tk_ConfigSpec configSpecs[] = {
    {TK_CONFIG_UID, "-background", (char *) NULL, (char *) NULL,
	(char *) NULL, Tk_Offset(PixmapMaster, bgUid), TK_CONFIG_NULL_OK, NULL},
    {TK_CONFIG_STRING, "-data", (char *) NULL, (char *) NULL,
	(char *) NULL, Tk_Offset(PixmapMaster, dataString), TK_CONFIG_NULL_OK, NULL},
    {TK_CONFIG_STRING, "-file", (char *) NULL, (char *) NULL,
	(char *) NULL, Tk_Offset(PixmapMaster, fileString), TK_CONFIG_NULL_OK, NULL},
    {TK_CONFIG_STRING, "-maskdata", (char *) NULL, (char *) NULL,
	(char *) NULL, Tk_Offset(PixmapMaster, maskDataString),
	TK_CONFIG_NULL_OK, NULL},
    {TK_CONFIG_STRING, "-maskfile", (char *) NULL, (char *) NULL,
	(char *) NULL, Tk_Offset(PixmapMaster, maskFileString),
	TK_CONFIG_NULL_OK, NULL},
    {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
	(char *) NULL, 0, 0, NULL}
};

/*
 * The following data structure is used to describe the state of
 * parsing a bitmap file or string.  It is used for communication
 * between GetBitmapData and NextBitmapWord.
 */

#define MAX_WORD_LENGTH 100
typedef struct cfParseInfo {
    char *string;		/* Next character of string data for bitmap,
				 * or NULL if bitmap is being read from
				 * file. */
    FILE *f;			/* File containing bitmap data, or NULL
				 * if no file. */
    char word[MAX_WORD_LENGTH+1];
				/* Current word of bitmap data, NULL
				 * terminated. */
    int wordLength;		/* Number of non-NULL bytes in word. */
} cfParseInfo;

/*
 * Prototypes for procedures used only locally in this file:
 */

static int		cfNextBitmapWord _ANSI_ARGS_((cfParseInfo \
						      *parseInfoPtr));
static char *		cfGetBitmapData _ANSI_ARGS_((Tcl_Interp *interp,
			    char *string, char *fileName,
			    int *widthPtr, int *heightPtr));
static char *		GetPixmapData _ANSI_ARGS_((Tcl_Interp *interp,
			    char *string, char *fileName,
			    int *widthPtr, int *heightPtr));
static int		ImgPmapCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, const char **argv));
static void		ImgPmapCmdDeletedProc _ANSI_ARGS_((
			    ClientData clientData));
static void		ImgPmapConfigureInstance _ANSI_ARGS_((
			    PixmapInstance *instancePtr));
static int		ImgPmapConfigureMaster _ANSI_ARGS_((
			    PixmapMaster *masterPtr, int argc, const char **argv,
			    int flags));


/*
 *----------------------------------------------------------------------
 *
 * ImgPmapCreate --
 *
 *	This procedure is called by the Tk image code to create "test"
 *	images.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	The data structure for a new image is allocated.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
static int
ImgPmapCreate(interp, name, objc, objv, typePtr, master, clientDataPtr)
    Tcl_Interp *interp;		/* Interpreter for application containing image. */
    const char *name;		/* Name to use for image. */
    int objc;			/* Number of option arguments. */
    Tcl_Obj *const objv[];	/* Option/value pairs as Tcl objects. */
    const Tk_ImageType *typePtr; /* Pointer to our type record (not used). */
    Tk_ImageMaster master;	/* Token for image, used in later callbacks. */
    ClientData *clientDataPtr;	/* Store manager's token for image here. */
{
    PixmapMaster *masterPtr;
    int isNew, i;
    const char **argv;
    Tcl_HashEntry *entry;

    /* Tk 8.6 passes image options as Tcl objects. The legacy Tk option
     * parser remains public API, but consumes an argv-style string vector. */
    argv = (const char **) ckalloc((unsigned)((objc > 0 ? objc : 1) * sizeof(char *)));
    for (i = 0; i < objc; i++) {
	argv[i] = Tcl_GetString(objv[i]);
    }

    masterPtr = (PixmapMaster *) ckalloc(sizeof(PixmapMaster));
    masterPtr->tkMaster = master;
    masterPtr->interp = interp;
    masterPtr->imageCmd = Tcl_CreateCommand(interp, name, ImgPmapCmd,
	    (ClientData) masterPtr, ImgPmapCmdDeletedProc);
    masterPtr->width = masterPtr->height = 0;
    masterPtr->data = NULL;
    masterPtr->maskData = NULL;
    masterPtr->bgUid = NULL;
    masterPtr->fileString = NULL;
    masterPtr->dataString = NULL;
    masterPtr->maskFileString = NULL;
    masterPtr->maskDataString = NULL;
    masterPtr->instancePtr = NULL;
    if (ImgPmapConfigureMaster(masterPtr, objc, argv, 0) != TCL_OK) {
	ckfree((char *) argv);
	ImgPmapDelete((ClientData) masterPtr);
	return TCL_ERROR;
    }
    ckfree((char *) argv);

    /* Enter this pixmap image in the hash table. */
    if (!imgPixmapHashInitialized) {
	Tcl_InitHashTable(&imgPixmapHash, TCL_STRING_KEYS);
	imgPixmapHashInitialized = 1;
    }
    entry = Tcl_CreateHashEntry(&imgPixmapHash, name, &isNew);
    Tcl_SetHashValue(entry, masterPtr);

    *clientDataPtr = (ClientData) masterPtr;
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ImgPmapConfigureMaster --
 *
 *	This procedure is called when a pixmap image is created or
 *	reconfigured.  It process configuration options and resets
 *	any instances of the image.
 *
 * Results:
 *	A standard Tcl return value.  If TCL_ERROR is returned then
 *	an error message is left in masterPtr->interp->result.
 *
 * Side effects:
 *	Existing instances of the image will be redisplayed to match
 *	the new configuration options.
 *
 *----------------------------------------------------------------------
 */

static int
ImgPmapConfigureMaster(masterPtr, argc, argv, flags)
    PixmapMaster *masterPtr;	/* Pointer to data structure describing
				 * overall bitmap image to (reconfigure). */
    int argc;			/* Number of entries in argv. */
    const char **argv;	/* Pairs of configuration options for image. */
    int flags;			/* Flags to pass to Tk_ConfigureWidget,
				 * such as TK_CONFIG_ARGV_ONLY. */
{
    PixmapInstance *instancePtr;
    int maskWidth, maskHeight;

    if (Tk_ConfigureWidget(masterPtr->interp, Tk_MainWindow(masterPtr->interp),
	    configSpecs, argc, argv, (char *) masterPtr, flags) != TCL_OK) {
	return TCL_ERROR;
    }

    /*
     * Parse the bitmap and/or mask to create binary data.  Make sure that
     * the bitmap and mask have the same dimensions.
     */

    if (masterPtr->data != NULL) {
	ckfree(masterPtr->data);
	masterPtr->data = NULL;
    }
    if ((masterPtr->fileString != NULL) || (masterPtr->dataString != NULL)) {
	masterPtr->data = GetPixmapData(masterPtr->interp,
		masterPtr->dataString, masterPtr->fileString,
		&masterPtr->width, &masterPtr->height);
	if (masterPtr->data == NULL) {
	    return TCL_ERROR;
	}
    }
    if (masterPtr->maskData != NULL) {
	ckfree(masterPtr->maskData);
	masterPtr->maskData = NULL;
    }
    if ((masterPtr->maskFileString != NULL)
	    || (masterPtr->maskDataString != NULL)) {
	if (masterPtr->data == NULL) {
	    Tcl_SetObjResult(masterPtr->interp,
			    Tcl_NewStringObj("can't have mask without pixmap", -1));
	    return TCL_ERROR;
	}
	masterPtr->maskData = cfGetBitmapData(masterPtr->interp,
		masterPtr->maskDataString, masterPtr->maskFileString,
		&maskWidth, &maskHeight);
	if (masterPtr->maskData == NULL) {
	    return TCL_ERROR;
	}
	if ((maskWidth != masterPtr->width)
		|| (maskHeight != masterPtr->height)) {
	    ckfree(masterPtr->maskData);
	    masterPtr->maskData = NULL;
	    printf("Mask (%d, %d) <-> Image (%d, %d)\n", (int) maskWidth,
		   (int) maskHeight, (int)masterPtr->width, (int)
		   masterPtr->height);
	    Tcl_SetObjResult(masterPtr->interp,
			    Tcl_NewStringObj("pixmap and mask have different sizes", -1));
	    return TCL_ERROR;
	}
    }

    /*
     * Cycle through all of the instances of this image, regenerating
     * the information for each instance.  Then force the image to be
     * redisplayed everywhere that it is used.
     */

    for (instancePtr = masterPtr->instancePtr; instancePtr != NULL;
	    instancePtr = instancePtr->nextPtr) {
	ImgPmapConfigureInstance(instancePtr);
    }
    Tk_ImageChanged(masterPtr->tkMaster, 0, 0, masterPtr->width,
	    masterPtr->height, masterPtr->width, masterPtr->height);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ImgPmapConfigureInstance --
 *
 *	This procedure is called to create displaying information for
 *	a bitmap image instance based on the configuration information
 *	in the master.  It is invoked both when new instances are
 *	created and when the master is reconfigured.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Generates errors via Tk_BackgroundError if there are problems
 *	in setting up the instance.
 *
 *----------------------------------------------------------------------
 */

static void
ImgPmapConfigureInstance(instancePtr)
    PixmapInstance *instancePtr;	/* Instance to reconfigure. */
{
    PixmapMaster *masterPtr = instancePtr->masterPtr;
    XColor *colorPtr;
    XGCValues gcValues;
    GC gc;
    unsigned int mask;
    int xpmError;
    XpmAttributes xpmAttr;
    XpmColorSymbol xpmTransparentColor[1] =
    	{{ NULL, "none", 0 }};

    /*
     * For each of the options in masterPtr, translate the string
     * form into an internal form appropriate for instancePtr.
     */

    if ( masterPtr->bgUid != 0 ) {
        colorPtr = Tk_GetColor(masterPtr->interp, instancePtr->tkwin,
		               masterPtr->bgUid);
    	if (colorPtr == NULL) {
	    goto error;
    	}
    	if (instancePtr->bg != NULL) {
	    Tk_FreeColor(instancePtr->bg);
    	}
    	instancePtr->bg = colorPtr;
    } else {
        instancePtr->bg = NULL;
    }

    if (instancePtr->bitmap != None) {
	XFreePixmap(Tk_Display(instancePtr->tkwin), instancePtr->bitmap);
	instancePtr->bitmap = None;
    }
    if (instancePtr->mask != None) {
    	XFreePixmap(Tk_Display(instancePtr->tkwin), instancePtr->mask);
    	instancePtr->mask = None;
    }
    
    if (masterPtr->maskData != NULL) {
	instancePtr->mask = XCreateBitmapFromData(
		Tk_Display(instancePtr->tkwin),
		RootWindowOfScreen(Tk_Screen(instancePtr->tkwin)),
		masterPtr->maskData, (unsigned) masterPtr->width,
		(unsigned) masterPtr->height);
    }

    if (masterPtr->data != NULL) {
	xpmAttr.valuemask    = XpmCloseness | XpmDepth;
	if ( instancePtr->bg != NULL ) {
	    xpmAttr.valuemask            |= XpmColorSymbols;
            xpmTransparentColor[0].pixel  = instancePtr->bg->pixel;
	    xpmAttr.colorsymbols          = xpmTransparentColor;
	    xpmAttr.numsymbols            = 1;
	}
	xpmAttr.closeness    = 65535;
	xpmAttr.depth        = DisplayPlanes(Tk_Display(instancePtr->tkwin),
	                           Tk_ScreenNumber(instancePtr->tkwin));
	xpmError = XpmCreatePixmapFromBuffer(
		Tk_Display(instancePtr->tkwin),
		RootWindowOfScreen(Tk_Screen(instancePtr->tkwin)),
		masterPtr->data,
		&(instancePtr->bitmap),
	        NULL,
		&xpmAttr);
	if ( xpmError != XpmSuccess ) {
	    XpmFreeAttributes(&xpmAttr);
	    instancePtr->bitmap = None;
	    goto error;
	}
/*		instancePtr->bg == NULL ? &(instancePtr->mask) : NULL, */
	XpmFreeAttributes(&xpmAttr);
    }

    if (masterPtr->data != NULL) {
	mask = GCGraphicsExposures;
	gcValues.graphics_exposures = False;
        if ( instancePtr->bg != NULL ) {
	    gcValues.background = instancePtr->bg->pixel;
	    mask |= GCBackground;
	}
	if (instancePtr->mask != None) {
	    gcValues.clip_mask = instancePtr->mask;
	    mask |= GCClipMask;
	}
	gc = Tk_GetGC(instancePtr->tkwin, mask, &gcValues);
    } else {
	gc = None;
    }
    if (instancePtr->gc != None) {
	Tk_FreeGC(Tk_Display(instancePtr->tkwin), instancePtr->gc);
    }
    instancePtr->gc = gc;
    return;

  error:
    /*
     * An error occurred: clear the graphics context in the instance to
     * make it clear that this instance cannot be displayed.  Then report
     * the error.
     */

    if (instancePtr->gc != None) {
	Tk_FreeGC(Tk_Display(instancePtr->tkwin), instancePtr->gc);
    }
    instancePtr->gc = None;
    Tcl_AddErrorInfo(masterPtr->interp, "\n    (while configuring image \"");
    Tcl_AddErrorInfo(masterPtr->interp, Tk_NameOfImage(masterPtr->tkMaster));
    Tcl_AddErrorInfo(masterPtr->interp, "\")");
    Tk_BackgroundError(masterPtr->interp);
}

/*
 *----------------------------------------------------------------------
 *
 * cfGetBitmapData --
 *
 *	Given a file name or ASCII string, this procedure parses the
 *	file or string contents to produce binary data for a bitmap.
 *
 * Results:
 *	If the bitmap description was parsed successfully then the
 *	return value is a malloc-ed array containing the bitmap data.
 *	The dimensions of the data are stored in *widthPtr and *heightPtr.
 *	If an error occurred, NULL is returned and an error message is
 *	left in interp->result.
 *
 * Side effects:
 *	A bitmap is created.
 *
 *----------------------------------------------------------------------
 */

static char *
cfGetBitmapData(interp, string, fileName, widthPtr, heightPtr)
    Tcl_Interp *interp;			/* For reporting errors. */
    char *string;			/* String describing bitmap.  May
					 * be NULL. */
    char *fileName;			/* Name of file containing bitmap
					 * description.  Used only if string
					 * is NULL.  Must not be NULL if
					 * string is NULL. */
    int *widthPtr, *heightPtr;		/* Dimensions of bitmap get returned
					 * here. */
{
  int width, height, numBytes;
  char *p, *end;
  cfParseInfo pi;
  char *data = NULL;
  
  pi.string = string;
  if (string == NULL) 
    {
      pi.f = fopen(fileName, "r");
      if (pi.f == NULL) 
	{
	  Tcl_AppendResult(interp, "couldn't read bitmap file \"",
			   fileName, "\": ", Tcl_PosixError(interp), 
			   (char *) NULL);
	  return NULL;
	}
    } 
  else 
    {
      pi.f = NULL;
    }

  /*
   * Parse the lines that define the dimensions of the bitmap,
   * plus the first line that defines the bitmap data (it declares
   * the name of a data variable but doesn't include any actual
   * data).  These lines look something like the following:
   *
   *		#define foo_width 16
   *		#define foo_height 16
   *		#define foo_x_hot 3
   *		#define foo_y_hot 3
   *		static char foo_bits[] = {
   *
   * The x_hot and y_hot lines may or may not be present.  It's
   * important to check for "char" in the last line, in order to
   * reject old X10-style bitmaps that used shorts.
   */
  
  width = 0;
  height = 0;
  while (1) 
    {
      if (cfNextBitmapWord(&pi) != TCL_OK) 
	{
	  goto error;
	}
      if ((pi.wordLength >= 6) && (pi.word[pi.wordLength-6] == '_')
	  && (strcmp(pi.word+pi.wordLength-6, "_width") == 0)) 
	{
	  if (cfNextBitmapWord(&pi) != TCL_OK) 
	    {
	      goto error;
	    }
	  width = strtol(pi.word, &end, 0);
	  if ((end == pi.word) || (*end != 0)) 
	    {
	      goto error;
	    }
	}
      else if ((pi.wordLength >= 7) && (pi.word[pi.wordLength-7] == '_')
	       && (strcmp(pi.word+pi.wordLength-7, "_height") == 0)) 
	{
	  if (cfNextBitmapWord(&pi) != TCL_OK) 
	    {
	      goto error;
	    }
	  height = strtol(pi.word, &end, 0);
	  if ((end == pi.word) || (*end != 0)) 
	    {
	      goto error;
	    }
	} 
      else if ((pi.word[0] == 'c') && (strcmp(pi.word, "char") == 0)) 
	{
	  while (1) 
	    {
	      if (cfNextBitmapWord(&pi) != TCL_OK) 
		{
		  goto error;
		}
	      if ((pi.word[0] == '{') && (pi.word[1] == 0)) 
		{
		  goto getData;
		}
	    }
	} 
      else if ((pi.word[0] == '{') && (pi.word[1] == 0)) 
	{
	  Tcl_AppendResult(interp, "format error in bitmap data; ",
			   "looks like it's an obsolete X10 bitmap file",
			   (char *) NULL);
	  goto errorCleanup;
	}
    }
  
  /*
   * Now we've read everything but the data.  Allocate an array
   * and read in the data.
   */
  
 getData:
  if ((width <= 0) || (height <= 0)) 
    {
      goto error;
    }
  numBytes = ((width+7)/8) * height;
  data = (char *) ckalloc((unsigned) numBytes);
  
  for (p = data; numBytes > 0; p++, numBytes--) 
    {
      if (cfNextBitmapWord(&pi) != TCL_OK) 
	{
	  goto error;
	}
      *p = strtol(pi.word, &end, 0);
      if (end == pi.word) 
	{
	  goto error;
	}
    }
  
  /*
   * All done.  Clean up and return.
   */
  
  if (pi.f != NULL) 
    {
      fclose(pi.f);
    }
  *widthPtr = width;
  *heightPtr = height;
  return data;
  
 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("format error in bitmap data", -1));
 errorCleanup:
  if (data != NULL) 
    {
      ckfree(data);
    }
  if (pi.f != NULL) 
    {
      fclose(pi.f);
    }
  return NULL;
}

/*
 *----------------------------------------------------------------------
 *
 * cfNextBitmapWord --
 *
 *	This procedure retrieves the next word of information (stuff
 *	between commas or white space) from a bitmap description.
 *
 * Results:
 *	Returns TCL_OK if all went well.  In this case the next word,
 *	and its length, will be availble in *parseInfoPtr.  If the end
 *	of the bitmap description was reached then TCL_ERROR is returned.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
cfNextBitmapWord(parseInfoPtr)
    cfParseInfo *parseInfoPtr;		/* Describes what we're reading
					 * and where we are in it. */
{
    char *src, *dst;
    int c;

    parseInfoPtr->wordLength = 0;
    dst = parseInfoPtr->word;
    if (parseInfoPtr->string != NULL) {
	for (src = parseInfoPtr->string; isspace((unsigned char)*src) || (*src == ',');
		src++) {
	    if (*src == 0) {
		return TCL_ERROR;
	    }
	}
	for ( ; !isspace((unsigned char)*src) && (*src != ',') && (*src != 0); src++) {
	    *dst = *src;
	    dst++;
	    parseInfoPtr->wordLength++;
	    if (parseInfoPtr->wordLength > MAX_WORD_LENGTH) {
		return TCL_ERROR;
	    }
	}
	parseInfoPtr->string = src;
    } else {
	for (c = getc(parseInfoPtr->f); isspace((unsigned char)c) || (c == ',');
		c = getc(parseInfoPtr->f)) {
	    if (c == EOF) {
		return TCL_ERROR;
	    }
	}
	for ( ; !isspace((unsigned char)c) && (c != ',') && (c != EOF);
		c = getc(parseInfoPtr->f)) {
	    *dst = c;
	    dst++;
	    parseInfoPtr->wordLength++;
	    if (parseInfoPtr->wordLength > MAX_WORD_LENGTH) {
		return TCL_ERROR;
	    }
	}
    }
    if (parseInfoPtr->wordLength == 0) {
	return TCL_ERROR;
    }
    parseInfoPtr->word[parseInfoPtr->wordLength] = 0;
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * GetPixmapData --
 *
 *	Given a file name or ASCII string, this procedure parses the
 *	file or string contents to produce binary data for a bitmap.
 *
 * Results:
 *	If the bitmap description was parsed successfully then the
 *	return value is a malloc-ed array containing the bitmap data.
 *	The dimensions of the data are stored in *widthPtr and *heightPtr.
 *	If an error occurred, NULL is returned and an error message is
 *	left in interp->result.
 *
 * Side effects:
 *	A bitmap is created.
 *
 *----------------------------------------------------------------------
 */

static char *
GetPixmapData(interp, string, fileName, widthPtr, heightPtr)
    Tcl_Interp *interp;			/* For reporting errors. */
    char *string;			/* String describing bitmap.  May
					 * be NULL. */
    char *fileName;			/* Name of file containing bitmap
					 * description.  Used only if string
					 * is NULL.  Must not be NULL if
					 * string is NULL. */
    int *widthPtr, *heightPtr;		/* Dimensions of pixmap get returned
					 * here. */
{
    FILE        *f;
    int         numBytes;
    XpmImage    image;
    char        *data = NULL;
    Tcl_DString fName;
    char        *ExpandedFileName;

    if ( string == NULL ) {
        ExpandedFileName = Tcl_TildeSubst(interp, fileName, &fName);
	f = fopen(ExpandedFileName, "r");
	Tcl_DStringFree(&fName);
	if ( f == NULL ) {
	    Tcl_AppendResult(interp, "couldn't read pixmap file \"",
		    fileName, "\": ", Tcl_PosixError(interp), (char *) NULL);
	    return NULL;
	}
	fseek(f, 0, SEEK_END);
	numBytes = ftell(f) + 1;
	fseek(f, 0, SEEK_SET);
	data = (char *) ckalloc((unsigned) numBytes);
	if ( data != NULL ) {
	    fread(data, 1, numBytes, f);
	}
	fclose(f);
    } else {
	data = ckalloc((unsigned) strlen(string) + 1);
	if ( data != NULL ) {
	    strcpy(data, string);
	}
    }

    if ( data == NULL ) {
        Tcl_AppendResult(interp, "out of memory while reading XPM pixmap",
                                 (char *) NULL);
        goto errorCleanup;
    }

    if ( XpmCreateXpmImageFromBuffer(data, &image, NULL) != XpmSuccess ) {
        Tcl_AppendResult(interp, "error reading XPM pixmap",
                                 (char *) NULL);
        goto errorCleanup;
    }
    *widthPtr = image.width; *heightPtr = image.height;
    XpmFreeXpmImage(&image);
    return data;

    errorCleanup:
    if (data != NULL) {
	ckfree(data);
    }
    return NULL;
}

/*
 *--------------------------------------------------------------
 *
 * ImgPmapCmd --
 *
 *	This procedure is invoked to process the Tcl command
 *	that corresponds to an image managed by this module.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *--------------------------------------------------------------
 */

static int
ImgPmapCmd(clientData, interp, argc, argv)
    ClientData clientData;	/* Information about button widget. */
    Tcl_Interp *interp;		/* Current interpreter. */
    int argc;			/* Number of arguments. */
    const char **argv;	/* Argument strings. */
{
    PixmapMaster *masterPtr = (PixmapMaster *) clientData;
/*     Tk_Window fenster = NULL; */
    
    int c, code;
    size_t length;

/*     if(masterPtr && masterPtr->instancePtr) */
/*      fenster = masterPtr->instancePtr->tkwin; */
    
    if (argc < 2) {
	char message[128];
	snprintf(message, sizeof(message),
		"wrong # args: should be \"%.50s option ?arg arg ...?\"",
		argv[0]);
	Tcl_SetObjResult(interp, Tcl_NewStringObj(message, -1));
	return TCL_ERROR;
    }
    c = argv[1][0];
    length = strlen(argv[1]);
    if ((c == 'c') && (strncmp(argv[1], "cget", length) == 0)
	    && (length >= 2)) {
	if (argc != 3) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
		    argv[0], " cget option\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	return Tk_ConfigureValue(interp, Tk_MainWindow(interp), configSpecs,
		(char *) masterPtr, argv[2], 0);
    } else if ((c == 'c') && (strncmp(argv[1], "configure", length) == 0)
	    && (length >= 2)) {
	if (argc == 2) {
	    code = Tk_ConfigureInfo(interp, Tk_MainWindow(interp),
		    configSpecs, (char *) masterPtr, (char *) NULL, 0);
	} else if (argc == 3) {
	    code = Tk_ConfigureInfo(interp, Tk_MainWindow(interp),
		    configSpecs, (char *) masterPtr, argv[2], 0);
	} else {
	    code = ImgPmapConfigureMaster(masterPtr, argc-2, argv+2,
		    TK_CONFIG_ARGV_ONLY);
	}
	return code;
    } else if ((c == 'c') && (strncmp(argv[1], "copy", length) == 0))
     {
      if(argc == 3)
       {
/*         PixmapMaster *srcHandle=NULL; */
        
/*         printf("I should copy from %s now!\n", argv[2]); */

/* 	if ((srcHandle = Tk_FindPhoto(argv[2])) == NULL) { */
/* 	    Tcl_AppendResult(interp, "image \"", argv[2], "\" doesn't", */
/* 		    " exist or is not a pixmap image", (char *) NULL); */
/* 	    return TCL_ERROR; */
/*         } */
        
/*         masterPtr->width = srcHandle->width; */
/*         masterPtr->height = srcHandle->height; */
/*         masterPtr->bgUid = srcHandle->bgUid; */
/*         masterPtr->fileString = strdup(srcHandle->fileString); */
/*         masterPtr->dataString = strdup(srcHandle->dataString); */
/*         masterPtr->maskFileString = strdup(srcHandle->maskFileString); */
/*         masterPtr->maskDataString = strdup(srcHandle->maskDataString); */
/*         masterPtr->data = malloc(masterPtr->width*masterPtr->height); */
        Tcl_AppendResult(interp, "copy not implemented yet.", 
                         (char *) NULL);

       }
      else
       Tcl_AppendResult(interp, "bad number of arguments for copy", 
                        (char *) NULL);
     } else {
      Tcl_AppendResult(interp, "bad option \"", argv[1],
                       "\": must be cget or configure", (char *) NULL);
      return TCL_ERROR;
     }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * ImgPmapGet --
 *
 *	This procedure is called for each use of a bitmap image in a
 *	widget.
 *
 * Results:
 *	The return value is a token for the instance, which is passed
 *	back to us in calls to ImgPmapDisplay and ImgPmapFree.
 *
 * Side effects:
 *	A data structure is set up for the instance (or, an existing
 *	instance is re-used for the new one).
 *
 *----------------------------------------------------------------------
 */

static ClientData
ImgPmapGet(tkwin, masterData)
    Tk_Window tkwin;		/* Window in which the instance will be
				 * used. */
    ClientData masterData;	/* Pointer to our master structure for the
				 * image. */
{
    PixmapMaster *masterPtr = (PixmapMaster *) masterData;
    PixmapInstance *instancePtr;

    /*
     * See if there is already an instance for this window.  If so
     * then just re-use it.
     */

    for (instancePtr = masterPtr->instancePtr; instancePtr != NULL;
	    instancePtr = instancePtr->nextPtr) {
	if (instancePtr->tkwin == tkwin) {
	    instancePtr->refCount++;
	    return (ClientData) instancePtr;
	}
    }

    /*
     * The image isn't already in use in this window.  Make a new
     * instance of the image.
     */

    instancePtr = (PixmapInstance *) ckalloc(sizeof(PixmapInstance));
    instancePtr->refCount = 1;
    instancePtr->masterPtr = masterPtr;
    instancePtr->tkwin = tkwin;
    instancePtr->bg = NULL;
    instancePtr->bitmap = None;
    instancePtr->mask = None;
    instancePtr->gc = None;
    instancePtr->nextPtr = masterPtr->instancePtr;
    masterPtr->instancePtr = instancePtr;
    ImgPmapConfigureInstance(instancePtr);

    /*
     * If this is the first instance, must set the size of the image.
     */

    if (instancePtr->nextPtr == NULL) {
	Tk_ImageChanged(masterPtr->tkMaster, 0, 0, 0, 0, masterPtr->width,
		masterPtr->height);
    }

    return (ClientData) instancePtr;
}

/*
 *----------------------------------------------------------------------
 *
 * ImgPmapDisplay --
 *
 *	This procedure is invoked to draw a bitmap image.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	A portion of the image gets rendered in a pixmap or window.
 *
 *----------------------------------------------------------------------
 */

static void
ImgPmapDisplay(clientData, display, drawable, imageX, imageY, width,
	height, drawableX, drawableY)
    ClientData clientData;	/* Pointer to PixmapInstance structure for
				 * for instance to be displayed. */
    Display *display;		/* Display on which to draw image. */
    Drawable drawable;		/* Pixmap or window in which to draw image. */
    int imageX, imageY;		/* Upper-left corner of region within image
				 * to draw. */
    int width, height;		/* Dimensions of region within image to draw. */
    int drawableX, drawableY;	/* Coordinates within drawable that
				 * correspond to imageX and imageY. */
{
    PixmapInstance *instancePtr = (PixmapInstance *) clientData;
    int masking;

    /*
     * If there's no graphics context, it means that an error occurred
     * while creating the image instance so it can't be displayed.
     */

    if (instancePtr->gc == None) {
	return;
    }

    /*
     * If masking is in effect, must modify the mask origin within
     * the graphics context to line up with the image's origin.
     * Then draw the image and reset the clip origin, if there's
     * a mask.
     */

    masking = (instancePtr->mask != None) || (instancePtr->bg == NULL);
    if (masking) {
	XSetClipOrigin(display, instancePtr->gc, drawableX - imageX,
		drawableY - imageY);
    }
    XCopyArea(display, instancePtr->bitmap, drawable, instancePtr->gc,
	      imageX, imageY, width, height, drawableX, drawableY);
    if (instancePtr->mask != None) {
	XSetClipOrigin(display, instancePtr->gc, 0, 0);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * ImgPmapFree --
 *
 *	This procedure is called when a widget ceases to use a
 *	particular instance of an image.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Internal data structures get cleaned up.
 *
 *----------------------------------------------------------------------
 */

static void
ImgPmapFree(clientData, display)
    ClientData clientData;	/* Pointer to PixmapInstance structure for
				 * for instance to be displayed. */
    Display *display;		/* Display containing window that used image. */
{
    PixmapInstance *instancePtr = (PixmapInstance *) clientData;
    PixmapInstance *prevPtr;

    instancePtr->refCount--;
    if (instancePtr->refCount > 0) {
	return;
    }

    /*
     * There are no more uses of the image within this widget.  Free
     * the instance structure.
     */

    if (instancePtr->bg != NULL) {
	Tk_FreeColor(instancePtr->bg);
    }
    if (instancePtr->bitmap != None) {
	XFreePixmap(display, instancePtr->bitmap);
    }
    if (instancePtr->mask != None) {
	XFreePixmap(display, instancePtr->mask);
    }
    if (instancePtr->gc != None) {
	Tk_FreeGC(display, instancePtr->gc);
    }
    if (instancePtr->masterPtr->instancePtr == instancePtr) {
	instancePtr->masterPtr->instancePtr = instancePtr->nextPtr;
    } else {
	for (prevPtr = instancePtr->masterPtr->instancePtr;
		prevPtr->nextPtr != instancePtr; prevPtr = prevPtr->nextPtr) {
	    /* Empty loop body */
	}
	prevPtr->nextPtr = instancePtr->nextPtr;
    }
    ckfree((char *) instancePtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ImgPmapDelete --
 *
 *	This procedure is called by the image code to delete the
 *	master structure for an image.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Resources associated with the image get freed.
 *
 *----------------------------------------------------------------------
 */

static void
ImgPmapDelete(masterData)
    ClientData masterData;	/* Pointer to PixmapMaster structure for
				 * image.  Must not have any more instances. */
{
    PixmapMaster *masterPtr = (PixmapMaster *) masterData;

    if (masterPtr->instancePtr != NULL) {
	panic("tried to delete bitmap image when instances still exist");
    }
    masterPtr->tkMaster = NULL;
    if (masterPtr->imageCmd != NULL) {
 	Tcl_DeleteCommand(masterPtr->interp,
 		Tcl_GetCommandName(masterPtr->interp, masterPtr->imageCmd));
    }
    if (masterPtr->data != NULL) {
	ckfree(masterPtr->data);
    }
    if (masterPtr->maskData != NULL) {
	ckfree(masterPtr->maskData);
    }
    Tk_FreeOptions(configSpecs, (char *) masterPtr, (Display *) NULL, 0);
    ckfree((char *) masterPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * ImgPmapCmdDeletedProc --
 *
 *	This procedure is invoked when the image command for an image
 *	is deleted.  It deletes the image.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The image is deleted.
 *
 *----------------------------------------------------------------------
 */

static void
ImgPmapCmdDeletedProc(clientData)
    ClientData clientData;	/* Pointer to BitmapMaster structure for
				 * image. */
{
    PixmapMaster *masterPtr = (PixmapMaster *) clientData;

    masterPtr->imageCmd = NULL;
    if (masterPtr->tkMaster != NULL) {
	Tk_DeleteImage(masterPtr->interp, Tk_NameOfImage(masterPtr->tkMaster));
    }
}


Tk_PixmapHandle Tk_FindPixmap(char *imageName)
{
    Tcl_HashEntry *entry;

    if (!imgPixmapHashInitialized) {
	return NULL;
    }
    entry = Tcl_FindHashEntry(&imgPixmapHash, imageName);
    if (entry == NULL) {
	return NULL;
    }
    return (Tk_PixmapHandle) Tcl_GetHashValue(entry);
}

#endif
#endif
