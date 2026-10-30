# -*- makefile -*-
# 
# generated at 30.01.1995 by Andreas Holzmann
#

### Start of Directory configuration section. ###

pvtdir = ..

include $(pvtdir)/Makefile.head.common

DEFS = 

### End of Direcory configuration section. ###

srcs =	FFShow.cc FFRandom.cc FFOrtho.cc FFOrthoInsp.cc FFNaiv.cc FFspringembedder.cc  \
	FFsugiyama.cc FFlayout_algorithmen.cc FFreduce_crossing.cc  \
	FFSpring.cc FFSugi.cc FFSpringInsp.cc FFSugiInsp.cc FFNaivInsp.cc \
	FFimprove_position.cc FFKaese.cc FFSpqr.cc FFSpqrNodeLabel.cc
objs = $(srcs:.cc=.o)


all lib:	libFFlayout.a
.PHONY: all

libFFlayout.a: $(objs)
	$(AR) rcv $@ $?
	$(RANLIB) $@

dep depend: $(srcs) 
	$(MAKE) dependimpl
.PHONY: dep depend

generate generateall:
.PHONY:	generate generateall

clean cleanall:
	rm -fr libFFlayout.a Flayoutlib *.o core  \#* core tmp* 
.PHONY: clean cleanall

distclean distcleanall:	clean
	rm -fr Makefile.dependencies *% *~
	touch Makefile.dependencies
.PHONY: distclean distcleanall

realclean realcleanall: distclean
.PHONY: realclean realcleanall



%.o     : %.cc
	$(CCC) -c $(CCFLAGS) $(INCLUDES) $(DEFS) $<


include $(pvtdir)/Makefile.tail.common
