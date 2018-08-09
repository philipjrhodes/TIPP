
TIPP_DIR := $(shell git rev-parse --git-dir | xargs dirname)

${TIPP_DIR}/lib/libhpdfs.a:
	cd libharu; \
	./buildconf.sh && \
	./configure && \
	make && \
	make install prefix=${TIPP_DIR}

pdf.o: pdf.c pdf.h
	gcc -c pdf.c
	
clean: 
	rm -f pdf.o
