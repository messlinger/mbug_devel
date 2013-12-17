#==========================================================================
# Linux Makefile for MBUG Device Library
#==========================================================================

.SUFFIXES:

vpath %.c ../src
vpath %.cpp ../src

#==========================================================================

.PHONY: all
all: lib tool

#==========================================================================
	
LIB_OBJS =  mbug.o  mbug_2110.o  mbug_2151.o  mbug_2165.o  mbug_2810.o  \
            mbug_2811.o  mbug_2818.o  mbug_2820.o	
LIB_VER = 1.0.2
LIB_MVER = 1

.PHONY: lib
lib: libmbug.a libmbug.so

libmbug.a: $(LIB_OBJS)
	$(AR) -rcs  $@  $(LIB_OBJS)
	cp  $@  lib/$@
	
libmbug.so: $(LIB_OBJS)
	$(LD) -shared -soname  $@.$(LIB_MVER)  -o $@.$(LIB_VER)  $(LIB_OBJS)  -lusb -lc
	ln -sf  $@.$(LIB_VER)  $@
	cp  $@.$(LIB_VER) lib/$@.$(LIB_VER)
	ln -sf  lib/$@.$(LIB_VER)  lib/$@

#==========================================================================

CBINS =  lsmbug  mbug_2110  mbug_2151  mbug_2165  mbug_2810  mbug_2811  mbug_2818  mbug_2820 
CXXBINS =  mbug_demo
BINS =  $(CBINS)  $(CXXBINS)

.PHONY: tool
tool: $(BINS)

lsmbug:    lsmbug.c         libmbug.so
mbug_2110: mbug_2110_tool.c libmbug.so
mbug_2151: mbug_2151_tool.c libmbug.so
mbug_2165: mbug_2165_tool.c libmbug.so
mbug_2810: mbug_2810_tool.c libmbug.so
mbug_2811: mbug_2811_tool.c libmbug.so
mbug_2818: mbug_2818_tool.c libmbug.so
mbug_2820: mbug_2820_tool.c libmbug.so
mbug_demo: mbug_demo.cpp libmbug.so

$(CBINS):
	$(CC)  -o  $@  $<  -I../include  -L.  -lmbug
	cp  $@  bin/$@

$(CXXBINS): 
	$(CXX)  -o  $@  $<  -I../include  -L.  -lmbug
	cp  $@  bin/$@

#==========================================================================
	
%.o: %.c
	$(CC)  -c  -fPIC  -I../include  $<
	
#==========================================================================

.PHONY: clean
clean:
	$(RM)  *.o  *.so*  *.a
	$(RM)  $(BINS)

.PHONY: realclean
realclean: clean
	$(RM)  $(addprefix lib/,  *.o  *.a  *.so*)
	$(RM)  $(addprefix bin/,  $(BINS))

#==========================================================================

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
libdir = $(exec_prefix)/lib
includedir = $(prefix)/include
sysconfdir = /etc

#==========================================================================

.PHONY: install
install: all
	#___library___
	mkdir  -p  $(libdir)
	cp  libmbug.so.$(LIB_VER)  $(libdir)
	chmod  a+r  $(libdir)/libmbug.so.$(LIB_VER)
	ln -sf  $(libdir)/libmbug.so.$(LIB_VER)  $(libdir)/libmbug.so
	ldconfig -v 2>/dev/nul | grep -c libmbug || (echo "\n$(libdir)" >> $(sysconfdir)/ld.so.conf && ldconfig)
	#___headers___
	mkdir -p  $(includedir)
	cp  ../include/*.*  $(includedir)
	#___tools___
	mkdir  -p  $(bindir)
	cp  $(BINS)  $(bindir)

#==========================================================================
	
.PHONY: uninstall
uninstall:
	rm -f  $(libdir)/libmbug*
	rm -f  $(bindir)/mbug_*
	rm -f  $(includedir)/mbug_*
	
