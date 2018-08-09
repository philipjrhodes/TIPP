
TIPP_DIR := $(shell git rev-parse --git-dir | xargs dirname)

${TIPP_DIR}/lib/libhpdfs.a:
	cd libharu; \
	./buildconf.sh && \
	./configure && \
	make && \
	make install prefix=${TIPP_DIR} && \
	ln -s ${TIPP_DIR}/lib/libhpdf.a  ${TIPP_DIR}/lib/libhpdfs.a
	
clean: 
	${MAKE} -C libharu clean 
	rm -f pdf.o ${TIPP_DIR}/lib/libhpdf*
