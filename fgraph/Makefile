# -*- makefile -*-
# 
# generated at 30.01.1995 by Andreas Holzmann
#

### Start of Directory configuration section. ###

pvtdir = ..

include $(pvtdir)/Makefile.head.common

DEFS = 

YFLAGS = -d
LFLAGS = 

### End of Direcory configuration section. ###

srcs =	RomeParser.cc RomeScanner.cc GTParser.cc GTScanner.cc ffError.cc FFParser.cc  FFScanner.cc FFEdgeClass.cc FFGraphClass.cc \
	FFNodeClass.cc FFGraph.cc FFLabelClass.cc FFSubGraphLabel.cc \
	FFGraphLabel.cc FFTINodeLabel.cc FFTIEdgeLabel.cc FFTIGraphLabel.cc

objs = $(srcs:.cc=.o)

all lib:	libFFGraph.a
.PHONY: all

libFFGraph.a: $(objs)
	$(AR) rcv $@ $?
	$(RANLIB) $@

dep depend: $(srcs) 
	$(MAKE) dependimpl
.PHONY: dep depend

generate generateall:
.PHONY:	generate generateall

clean cleanall:
	rm -fr libFFGraph.a FFGraphlib *.o core \#* core tmp* *_pure_*.a
.PHONY: clean cleanall

distclean distcleanall:	clean
	rm -fr *~ *% Makefile.dependencies
	touch Makefile.dependencies
.PHONY: distclean distcleanall

realclean realcleanall: distclean
.PHONY: realclean realcleanall

%.o     : %.cc
	$(CCC) -c $(CCFLAGS) $(INCLUDES) $(DEFS) $<

%.cc    : %.y
	$(BISONXX) $(YFLAGS) -o $*.cc -h$*.h  $*.y

%.cc    : %.l
	$(FLEXXX) $(LFLAGS) -8 -h$*.h -o$*.cc $*.l
	sed s/'Header:\ \/usr\/fsys\/odin\/a\/vern'/' HALLO '/g $@ >$@.tmp.tmp
	mv -f $@.tmp.tmp $@


include $(pvtdir)/Makefile.tail.common
