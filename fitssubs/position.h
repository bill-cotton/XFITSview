/*  Headers for utility routines related to celestial positions */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1997
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
#include "imagdesc.h"

#ifndef POSITION_H
#define POSITION_H

int worldpos(float xpix, float ypix, double xref, double yref, 
      float xrefpix, float yrefpix, float xinc, float yinc, float rot,
      char *type, double *xpos, double *ypos);
int CDpos(float xpix, float ypix, double xref, double yref,
      float xrefpix, float yrefpix, float xinc, float yinc, float rot,
      float cd1[2], float cd2[2], char *type, double *xpos, double *ypos);
int xypix(double xpos, double ypos, double xref, double yref, 
      float xrefpix, float yrefpix, float xinc, float yinc, float rot,
      char *type, float *xpix, float *ypix);
int CDpix(double xpos, double ypos, double xref, double yref, 
      float xrefpix, float yrefpix, float xinc, float yinc, float rot,
      float icd1[2], float icd2[2], char *type, float *xpix, float *ypix);
int pfit (float a[9][9], float *s, float dx[2], float fblank);
int worldposlm(double dx, double dy, double xref, double yref, 
	       float xinc, float yinc, float rot, char *type, 
	       double *xpos, double *ypos);
int xypixlm(double xpos, double ypos, double xref, double yref, 
      float xinc, float yinc, float rot, char *type, double *dx, double *dy);
#endif
