# -*- makefile -*-
# 
# generated at 30.01.1995 by Andreas Holzmann
#

### Start of Directory configuration section. ###

pvtdir = ..

include $(pvtdir)/Makefile.head.common

DEFS = 

### End of Direcory configuration section. ###

c++srcs = FFStacks.cc FFVHSets.cc FFVOHSets.cc StdContainer.cc FFLists.cc FFAVLSets.cc \
	FFMPlex.cc FFAVLMaps.cc FFMFSets.cc 
csrcs =  
srcs = $(csrcs) $(c++srcs)
objs = $(c++srcs:.cc=.o) $(csrcs:.c=.o) 

lib : libFFcontainer.a

libFFcontainer.a: $(objs)
	$(AR) rcv $@ $?
	$(RANLIB) $@


dep depend: $(srcs) 
	$(MAKE) dependimpl
.PHONY: dep depend

dependall: depend
.PHONY: dependall

clean cleanall:
	rm -fr libFFcontainer.a Containerlib *.o core \#* core tmp* 
.PHONY: clean cleanall

distclean distcleanall:	clean
	rm -f *% *~
	rm -f Makefile.dependencies
	touch Makefile.dependencies
.PHONY: distclean distcleanall

realclean realcleanall: distclean
.PHONY: realclean realcleanall


%.o     : %.cc
	$(CCC) -c $(CCFLAGS) $(INCLUDES) $(DEFS) $<

%.o     : %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFS) $<

# libcontainer_$(LIBN).a: $(OFILE_DIR) $(LIBFILES)
#	$(AR) rcv $@ $(LIBFILES)
#	$(RANLIB) $@

include $(pvtdir)/Makefile.tail.common
