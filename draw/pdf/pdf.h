#ifndef PDF_H
#define PDF_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "hpdf.h"

extern HPDF_Doc  pdf;
extern HPDF_Font font;
extern HPDF_Page page;

#ifdef __cplusplus  
	extern "C" {
#endif
	
void draw_triangle (
           double        x0,
           double        y0,
           double        x1,
           double        y1,
           double        x2,
           double        y2
          );

// Initialize a pdf
int createPDF();

// Write PDF to a file and free resources. 
// The filename will end with '.pdf'
int savePDF(const char* filename);

#ifdef __cplusplus  
	}
#endif

#endif // PDF_H


