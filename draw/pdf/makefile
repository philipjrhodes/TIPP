
# libharu is supposed to build automatically, but will fail if "cmake -DCMAKE_INSTALL_PREFIX=/$HOME/git/TIPP ."
# hasn't been run.
#
# also autoconf mk-configure, and automake might be needed.  Also, look at "autoreconf -i"

# and libpng is a dependency


TIPP_DIR := $(shell git rev-parse --git-dir | xargs dirname)

${TIPP_DIR}/lib/libhpdfs.a:
	cd libharu; \
	cmake -DCMAKE_INSTALL_PREFIX=${TIPP_DIR} . && \
	#./buildconf.sh && \
	#./configure && \
	make && \
	#make install prefix=${TIPP_DIR} && \
	make install  && \
	#ln -s ${TIPP_DIR}/lib/libhpdf.a  ${TIPP_DIR}/lib/libhpdfs.a
	
clean: 
	${MAKE} -C libharu clean 
	rm -f pdf.o ${TIPP_DIR}/lib/libhpdf*
