/*  Utility routines related to celestial positions on DSS images*/
/*-----------------------------------------------------------------------
*  Copyright (C) 1995,1996
*  Associated Universities, Inc. Washington DC, USA.
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*-----------------------------------------------------------------------*/
#include "mydefs.h"
#ifndef DSSSUBS_H
#define DSSSUBS_H
/* get celestial position from pixel value */
void dsseq (double xpixsz, double ypixsz, double ra0, double dec0, 
	    double xoff0, double yoff0, double dssx[13], double dssy[13], 
	    float xpix, float ypix, double *ra, double *dec, Integer *ierr);

/* get pixel from celestial position */
void dsspix (double scale, double xpixsz, double ypixsz, double ra0, 
	     double dec0, double xoff0, double yoff0, double dssx[13], 
	     double dssy[13], double ra, double dec, 
	     float *xpix, float *ypix, Integer *ierr);
/* fix up header info to show equivalent WCS */
void fixcoo (double xpixsz, double ypixsz, double ra0, double dec0, 
	     double ppo[6], double amdx[13], double amdy[13],
	     float cnpix[2], Integer dim0, Integer dim1, double scale,
	     char* ctype0, char* ctype1, float* cdelt0, float* cdelt1,
	     float* crpix0, float* crpix1, float* crota0, float* crota1,
	     double* crval0, double* crval1, Integer *ierr);
#endif /* DSSSUBS_H */
