# -*- makefile -*-3D
# 
# generated at 30.01.1995 by Andreas Holzmann
#

### Start of Directory configuration section. ###

pvtdir = ..

include $(pvtdir)/Makefile.head.common

DEFS = 

### End of Direcory configuration section. ###

srcs = FFClusterWindow.cc FFTclBasicWindow.cc FFTclSubWindow.cc FFPoint.cc FFMatrix.cc \
	FFWindow.cc FFGraphicNode.cc FFGraphicEdge.cc \
	FFTclInspector.cc FFTclGraphInsp.cc \
	FFTclWindow.cc FFTcl3DWindow.cc

csrc= tkAppInit.c tkImgPmap.c

objs = $(csrc:.c=.o) $(srcs:.cc=.o) 

all lib: libFFGraphic.a
.PHONY: all

libFFGraphic.a: $(objs)
	$(AR) rcv $@ $?
	$(RANLIB) $@

dep depend: $(srcs) 
	$(MAKE) dependimpl
.PHONY: dep depend

generate generateall:
.PHONY:	generate generateall

clean cleanall:
	rm -fr libFFGraphik.a libFFGraphic.a *.o core \#* core tmp* *_pure_*.a
.PHONY: clean cleanall

distclean distcleanall:	clean
	rm -fr Makefile.dependencies *~ *%
	touch Makefile.dependencies
.PHONY: distclean distcleanall

realclean realcleanall: distclean
.PHONY: realclean realcleanall

%.o :	../misc/%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFS) $<

%.o     : %.cc
	$(CCC) -c $(CCFLAGS) $(INCLUDES) $(DEFS) $<

include $(pvtdir)/Makefile.tail.common
