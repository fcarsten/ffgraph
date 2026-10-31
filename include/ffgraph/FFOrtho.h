#ifndef FF_ORTHO
#define FF_ORTHO

#include <globdefs.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>

#include <ffgraph/FFLayoutLabel.h>
#include <String.h>

#define ffOrthoType String("FFOrtho")
#define ORTHO_X_OFFSET 100
#define ORTHO_Y_OFFSET 100
#define ORTHO_Z_OFFSET 100


// Groesse der Nachbarschaft in der ich nach potentiellen Tausch-
// kandidaten suche.  Im 2D-Fall die 3x3 Zellen in der Ebene (FF_WSIZE_2D),
// im 3D-Fall zusaetzlich die 3x3 Zellen der Ebenen davor und dahinter
// (FF_WSIZE_3D).  FF_WSIZE ist die Groesse der Arrays.
#define FF_WSIZE_2D 9
#define FF_WSIZE_3D 27
#define FF_WSIZE FF_WSIZE_3D

// Schwellwert ab dem ich eine Position zur neuen Wunschposition mache
#define FF_RES_VAL 500

struct inOutStruct
{
 BOOL o,u,l,r;
};

class FFOrtho : public FFLayoutLabel
{
 FFCoord aktuX;
 FFCoord aktuY;
 FFCoord xOffSet;
 FFCoord yOffSet;
 FFCoord zOffSet;   // Gitterabstand in z-Richtung (nur 3D)

 BOOL threeD;
 BOOL anim;
 BOOL doSwapping;
 BOOL doSwapBadies;
 BOOL doEdgeAdjustment;
 String initPlacement;
 
 // void _layout(FFSet<FFNode>&);
 // FFCoord _layout(FFNode);
 int tiIndex;

 // Fuer Swap nodes
 int count;       // Anzahl der Knoten mit graphischer Rep.
 FFNode *nodes;   // Array der Knoten mit graphischer Rep.
 int anzX;        // Anzahl der Knoten pro Reihe in nodes
 int anzY;        // Anzahl der Knoten pro Spalte in nodes
 int anzZ;        // Anzahl der Ebenen (Layer) in nodes; 1 im 2D-Fall
 int wsize;       // Anzahl der benutzten Eintraege in wunschPos (9 oder 27)
 
 // Index einer Gitterzelle: (z * anzY + y) * anzX + x
 inline int _cellIndex(int x, int y, int z) const
  {return (z * anzY + y) * anzX + x;}
 // Gitterkoordinaten (Weltkoordinaten) einer Zelle
 FFPoint _cellPoint(int pos) const;
 // Fuellt wunschPos[0..wsize-1] mit der Nachbarschaft der Zelle center
 void _fillWunschPos(int center);
 // Zelle (Index) zu einer Weltkoordinate; ohne Randpruefung
 int _pointToCell(const FFPoint&) const;
 
 FFCoord distArray[FF_WSIZE];
 int wunschPos[FF_WSIZE];
 FFPoint wunschP[FF_WSIZE];
 void _printEdgeSum(ostream &);
 FFCoord _getEdgeSum();
 BOOL _doSwap(int, int, FFPoint);

 void _2DEdgePos();
 void _3DEdgePos();     // z-y-x Kantenfuehrung im 3D-Gitter

 void _adjustEdge(FFGraphicNode *, FFGraphicNode *, struct inOutStruct *, 
                  FFGraphicEdge *);
 BOOL _testNeighbor(const FFPoint&, const FFPoint&);

public:
 virtual BOOL swapNodes();
 virtual BOOL swapBadies();
 virtual int testWunschPos(FFNode, FFCoord, int =-1);

 inline BOOL get3D(){return threeD;}
 inline void set3D(BOOL b){threeD = b;}

 inline BOOL getSwapping(){return doSwapping;}
 inline void setSwapping(BOOL b){doSwapping = b;}

 inline BOOL getSwapBadies(){return doSwapBadies;}
 inline void setSwapBadies(BOOL b){doSwapBadies = b;}

 inline BOOL getEdgeAdjust(){return doEdgeAdjustment;}
 inline void setEdgeAdjust(BOOL b){doEdgeAdjustment = b;}

 inline const String& getInitPlacement(){return initPlacement;}
 inline void setInitPlacement(const String& b){initPlacement = b;}

 inline BOOL getAnim(){return anim;}
 inline void setAnim(BOOL b){anim = b;}
 
 FFOrtho ();
 virtual ~FFOrtho (){};

 BOOL _adjustPlacement();
 void _randomPlacement();

 void setxoff(FFCoord x) {xOffSet = x;}
 void setyoff(FFCoord y) {yOffSet = y;}
 void setzoff(FFCoord z) {zOffSet = z;}
 
 FFCoord getxoff() {return xOffSet;}
 FFCoord getyoff() {return yOffSet;}
 FFCoord getzoff() {return zOffSet;}
 
 virtual BOOL hasOwnInspector() const {return YES;} 
 
 virtual void init(const FFGraph& g);
 virtual BOOL parse(const String&) {return YES;}
 virtual String unparse() {return String("");}
 virtual FFGraphLabel* proto() const {return new FFOrtho;}
 virtual BOOL isInspectable() const {return YES;}
 
 void layout (FFSet <FFNodeClass *>&) {layout();}
 void layout ();
};

#endif
