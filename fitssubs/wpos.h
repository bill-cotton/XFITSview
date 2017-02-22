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

#ifndef WPOS_H
#define WPOS_H

void AxisLabel(double pos, char* axis, char* label);
void ra2hms(double ra, char* rach, char* rast);
void dec2dms(double dec, char* decch, char* decst);
void rahms(double ra, int *h, int *m, float *s);
void decdms(double dec, int *d, int *m, float *s);
int hmsra(int h, int m, float s, double *ra);
int dmsdec(int d, int m, float s, double *dec);
int GetWPos(int naxes, float pix[3], double ref[3], float refpix[3], 
   float inc[3], float rot[3], char type[3][9], double pos[3]);
void dss_pos(ImageDescriptor *desc, float xpix, float ypix, 
	     double *ra, double *dec);
/* generic position translation */
int get_wpos(ImageDescriptor *desc, float pix[3], double pos[3]);
int get_xypix(ImageDescriptor *desc, double xpos, double ypos, 
	      float *xpix, float *ypix);
#endif
