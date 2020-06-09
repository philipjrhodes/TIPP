GCC = gcc
CFLAGS = -c -std=c99 -lm -O3
RBOX_EXE = rbox

DESTDIR = /usr/local
BINDIR	= $(DESTDIR)/bin
INCDIR	= $(DESTDIR)/include
LIBDIR	= $(DESTDIR)/lib
DOCDIR	= $(DESTDIR)/share/doc/qhull
MANDIR	= $(DESTDIR)/share/man/man1

# if you do not have enscript, try a2ps or just use lpr.  The files are text.
PRINTMAN = enscript -2rl
PRINTC = enscript -2r
# PRINTMAN = lpr
# PRINTC = lpr

#for Gnu's gcc compiler, -O3 for optimization, -g for debugging, -pg for profiling
# Qhull uses less memory with 32-bit builds (-m32)
# -fpic  needed for gcc x86_64-linux-gnu.  Not needed for mingw
# caller may defined CC_WARNINGS
M32       = -m32
CC        = gcc
#CC_OPTS1  = -O3 -ansi -I../../src -fpic $(CC_WARNINGS) ${M32}
CC_OPTS1  = -O3 -ansi -fpic $(CC_WARNINGS)

# for Sun's cc compiler, -fast or O2 for optimization, -g for debugging, -Xc for ANSI
#CC       = cc
#CC_OPTS1 = -Xc -v -fast -I../../src 

# for Silicon Graphics cc compiler, -O2 for optimization, -g for debugging
#CC       = cc
#CC_OPTS1 = -ansi -O2 -I../../src 

# for Next cc compiler with fat executable
#CC       = cc
#CC_OPTS1 = -ansi -O2 -I../../src -arch m68k -arch i386 -arch hppa

# For loader, ld, 
CC_OPTS2 = $(CC_OPTS1)

# Default targets for make

all: $(RBOX_EXE)

clean:
	rm -f *.o 
	
cleanall: clean
	rm -f qconvex qdelaunay qhalf qvoronoi qhull *.exe
	rm -f core user_eg user_eg2 testqset libqhullstatic.a

install:
#	mkdir -p $(BINDIR)
#	mkdir -p $(DOCDIR)
#	mkdir -p $(INCDIR)/libqhull
#	mkdir -p $(LIBDIR)
#	mkdir -p $(MANDIR)
#	cp -p qconvex qdelaunay qhalf qhull qvoronoi rbox $(BINDIR)
#	cp -p libqhullstatic.a $(LIBDIR)
#	cp -p ../../html/qhull.man $(MANDIR)/qhull.1
#	cp -p ../../html/rbox.man $(MANDIR)/rbox.1
#	cp -p ../../html/* $(DOCDIR)
#	cp *.h $(INCDIR)/libqhull

new:	cleanall all


# LIBQHULL_OBJS_1 ordered by frequency of execution with small files at end.  Better locality.
# Same definitions as ../../Makefile

LIBQHULLS_OBJS_1= global.o stat.o geom2.o poly2.o merge.o \
	libqhull.o geom.o poly.o qset.o mem.o random.o 

LIBQHULLS_OBJS_2= $(LIBQHULLS_OBJS_1) usermem.o userprintf.o io.o user.o

LIBQHULLS_OBJS= $(LIBQHULLS_OBJS_2)  rboxlib.o userprintf_rbox.o


$(RBOX_EXE): rbox.o $(LIBQHULLS_OBJS)
	$(GCC) $(CFLAGS) $^ -o $@ 

rbox.o: rbox.c
	$(GCC) $(CFLAGS) -o $@ $<

rboxlib.o: rboxlib.c
	$(GCC) $(CFLAGS) -o $@ $<

usermem.o: usermem.c
	$(GCC) $(CFLAGS) -o $@ $<

userprintf.o: userprintf.c
	$(GCC) $(CFLAGS) -o $@ $<

userprintf_rbox.o: userprintf_rbox.c
	$(GCC) $(CFLAGS) -o $@ $<

qset.o: qset.c
	$(GCC) $(CFLAGS) -o $@ $<

random.o: random.c
	$(GCC) $(CFLAGS) -o $@ $<

libqhull.o: libqhull.c
	$(GCC) $(CFLAGS) -o $@ $<

mem.o: mem.c
	$(GCC) $(CFLAGS) -o $@ $<

merge.o: merge.c
	$(GCC) $(CFLAGS) -o $@ $<

user.o: user.c
	$(GCC) $(CFLAGS) -o $@ $<

geom.o: geom.c
	$(GCC) $(CFLAGS) -o $@ $<

geom2.o: geom2.c
	$(GCC) $(CFLAGS) -o $@ $<

global.o: global.c
	$(GCC) $(CFLAGS) -o $@ $<

io.o: io.c
	$(GCC) $(CFLAGS) -o $@ $<

poly.o: poly.c
	$(GCC) $(CFLAGS) -o $@ $<

poly2.o: poly2.c
	$(GCC) $(CFLAGS) -o $@ $<

stat.o: stat.c
	$(GCC) $(CFLAGS) -o $@ $<

# compile qhull without using bin/libqhullstatic.a
#qhull_all: qconvex.o qdelaun.o qhalf.o qvoronoi.o unix.o user_eg.o user_eg2.o rbox.o testqset.o $(LIBQHULLS_OBJS)
#qhull_all: rbox.o $(LIBQHULLS_OBJS)
#	$(CC) -o qconvex $(CC_OPTS2) -lm $(LIBQHULLS_OBJS_2) qconvex.o
#	$(CC) -o qdelaunay $(CC_OPTS2) -lm $(LIBQHULLS_OBJS_2) qdelaun.o
#	$(CC) -o qhalf $(CC_OPTS2) -lm $(LIBQHULLS_OBJS_2) qhalf.o
#	$(CC) -o qvoronoi $(CC_OPTS2) -lm $(LIBQHULLS_OBJS_2) qvoronoi.o
#	$(CC) -o qhull $(CC_OPTS2) -lm $(LIBQHULLS_OBJS_2) unix.o 
#	$(CC) -o rbox $(CC_OPTS2) -lm $(LIBQHULLS_OBJS) rbox.o
#	$(CC) -o user_eg $(CC_OPTS2) -lm $(LIBQHULLS_OBJS_2) user_eg.o 
#	$(CC) -o user_eg2 $(CC_OPTS2) -lm $(LIBQHULLS_OBJS_1) user_eg2.o  usermem.o userprintf.o io.o
#	$(CC) -o testqset $(CC_OPTS2) -lm mem.o qset.o usermem.o testqset.o
#	-ar -rs libqhullstatic.a $(LIBQHULLS_OBJS)
	#libqhullstatic.a is not needed for qhull
	#If 'ar -rs' fails try using 'ar -s' with 'ranlib'
	#ranlib libqhullstatic.a


# end of Makefile
