/*  Utility routines related to celestial positions */
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
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "position.h"
#include "precess.h"
#include "dsssubs.h"
#include "zsubs.h"
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

/* internal prototypes */
int momnt (float ara[9][9], float x, float y, int nx, int ny, float momar[6],
   float fblank);
void matvmu (float m[3][3], float vi[3], float vo[3], int n);

int CDpos(float xpix, float ypix, double xref, double yref,
      float xrefpix, float yrefpix, float xinc, float yinc, float rot,
      float cd1[2], float cd2[2], char *type, double *xpos, double *ypos)
/*-----------------------------------------------------------------------*/
/* routine to determine accurate position for pixel coordinates from IRAF*/
/* style CD matrix.  Note: xinc, yinc, and rot can be derived from cd1   */
/* and cd2 and should be compatible with them.                           */
/* returns 0 if successful otherwise:                                    */
/* 1 = angle too large for projection;                                   */
/* does: -SIN, -TAN, -ARC, -NCP, -GLS, -MER, -AIT projections            */
/* anything else is linear                                               */
/* Input:                                                                */
/*   f   xpix    x pixel number  (RA or long without rotation)           */
/*   f   ypiy    y pixel number  (dec or lat without rotation)           */
/*   d   xref    x reference coordinate value (deg)                      */
/*   d   yref    y reference coordinate value (deg)                      */
/*   f   xrefpix x reference pixel                                       */
/*   f   yrefpix y reference pixel                                       */
/*   f   xinc    x coordinate increment (deg)                            */
/*   f   yinc    y coordinate increment (deg)                            */
/*   f   rot     rotation (deg)  (from N through E)                      */
/*   c  *type    projection type code e.g. "-SIN";                       */
/*   f   cd1[2]  first column of CD matrix                               */
/*   f   cd2[2]  second column of CD matrix                              */
/* Output:                                                               */
/*   d   *xpos   x (RA) coordinate (deg)                                 */
/*   d   *ypos   y (dec) coordinate (deg)                                */
/*-----------------------------------------------------------------------*/
 {double dx, dy, l, m;

  /*   Offset from ref pixel  */
  dx = (xpix-xrefpix);
  dy = (ypix-yrefpix);

  /* convert to l and m  */
  l = cd1[0]*dx + cd1[1]*dy;
  m = cd2[0]*dx + cd2[1]*dy;

  /* determine position */
  return worldposlm (l, m, xref, yref, xinc, yinc, rot, type, xpos, ypos);
}  /* End of CDpos */

int worldpos(float xpix, float ypix, double xref, double yref,
      float xrefpix, float yrefpix, float xinc, float yinc, float rot,
      char *type, double *xpos, double *ypos)
/*-----------------------------------------------------------------------*/
/* routine to determine accurate position for pixel coordinates          */
/* returns 0 if successful otherwise:                                    */
/* 1 = angle too large for projection;                                   */
/* does: -SIN, -TAN, -ARC, -NCP, -GLS, -MER, -AIT projections            */
/* anything else is linear                                               */
/* Input:                                                                */
/*   f   xpix    x pixel number  (RA or long without rotation)           */
/*   f   ypiy    y pixel number  (dec or lat without rotation)           */
/*   d   xref    x reference coordinate value (deg)                      */
/*   d   yref    y reference coordinate value (deg)                      */
/*   f   xrefpix x reference pixel                                       */
/*   f   yrefpix y reference pixel                                       */
/*   f   xinc    x coordinate increment (deg)                            */
/*   f   yinc    y coordinate increment (deg)                            */
/*   f   rot     rotation (deg)  (from N through E)                      */
/*   c  *type    projection type code e.g. "-SIN";                       */
/* Output:                                                               */
/*   d   *xpos   x (RA) coordinate (deg)                                 */
/*   d   *ypos   y (dec) coordinate (deg)                                */
/*-----------------------------------------------------------------------*/
 {double cosr, sinr, dx, dy, temp;
  double cond2r=1.745329252e-2;

/*   Offset from ref pixel  */
  dx = (xpix-xrefpix) * xinc;
  dy = (ypix-yrefpix) * yinc;
/*   Take out rotation  */
  cosr = cos(rot*cond2r);
  sinr = sin(rot*cond2r);
  if (rot!=0.0)
    {temp = dx * cosr - dy * sinr;
     dy = dy * cosr + dx * sinr;
     dx = temp;}
  /* determine position */
  return worldposlm (dx, dy, xref, yref, xinc, yinc, rot, type, xpos, ypos);
}  /* End of CDpos */

int worldposlm(double dx, double dy, double xref, double yref, 
	       float xinc, float yinc, float rot, char *type, 
	       double *xpos, double *ypos)
/*-----------------------------------------------------------------------*/
/* routine to determine accurate position for pixel coordinates from     */
/* offsets from the reference position.                                  */
/* returns 0 if successful otherwise:                                    */
/* 1 = angle too large for projection;                                   */
/* does: -SIN, -TAN, -ARC, -NCP, -GLS, -MER, -AIT projections            */
/* anything else is linear                                               */
/* Input:                                                                */
/*   f   dx      x coordinate offset  (RA or long)                       */
/*   f   dy      y coordinate offset  (dec or lat)                       */
/*   d   xref    x reference coordinate value (deg)                      */
/*   d   yref    y reference coordinate value (deg)                      */
/*   f   xinc    x coordinate increment (deg)                            */
/*   f   yinc    y coordinate increment (deg)                            */
/*   f   rot     rotation (deg)  (from N through E)                      */
/*   c  *type    projection type code e.g. "-SIN";                       */
/* Output:                                                               */
/*   d   *xpos   x (RA) coordinate (deg)                                 */
/*   d   *ypos   y (dec) coordinate (deg)                                */
/*-----------------------------------------------------------------------*/
 {double cosr, sinr, dz;
  double sins, coss, dect, rat, dt, l, m, mg, da, dd, cos0, sin0;
  double dec0, ra0, decout, raout;
  double geo1, geo2, geo3;
  double cond2r=1.745329252e-2;
  double twopi = 6.28318530717959, deps = 1.0e-5;
  int   i, itype;
  char ctypes[8][5] ={"-SIN","-TAN","-ARC","-NCP", "-GLS", "-MER", "-AIT",
     "-STG"};

/*   rotation  */
  cosr = cos(rot*cond2r);
  sinr = sin(rot*cond2r);
/*  find type  */
  itype = 0;  /* default type is linear */
  for (i=0;i<8;i++) if (!strncmp(type, ctypes[i], 4)) itype = i+1;
/* default, linear result for error return  */
  *xpos = xref + dx;
  *ypos = yref + dy;
/* convert to radians  */
  ra0 = xref * cond2r;
  dec0 = yref * cond2r;
  l = dx * cond2r;
  m = dy * cond2r;
  sins = l*l + m*m;
  decout = 0.0;
  raout = 0.0;
  cos0 = cos(dec0);
  sin0 = sin(dec0);
/* process by case  */
  switch (itype) {
    case 0:   /* linear */
      rat =  ra0 + l;
      dect = dec0 + m;
      break;
    case 1:   /* -SIN sin*/ 
      if (sins>1.0) return 1;
      coss = sqrt (1.0 - sins);
      dt = sin0 * coss + cos0 * m;
      if ((dt>1.0) || (dt<-1.0)) return 1;
      dect = asin (dt);
      rat = cos0 * coss - sin0 * m;
      if ((rat==0.0) && (l==0.0)) return 1;
      rat = atan2 (l, rat) + ra0;
      break;
    case 2:   /* -TAN tan */
      if (sins>1.0) return 1;
      dect = cos0 - m * sin0;
      if (dect==0.0) return 1;
      rat = ra0 + atan2 (l, dect);
      dect = atan (cos(rat-ra0) * (m * cos0 + sin0) / dect);
      break;
    case 3:   /* -ARC Arc*/
      if (sins>=twopi*twopi/4.0) return 1;
      sins = sqrt(sins);
      coss = cos (sins);
      if (sins!=0.0) sins = sin (sins) / sins;
      else
	sins = 1.0;
      dt = m * cos0 * sins + sin0 * coss;
      if ((dt>1.0) || (dt<-1.0)) return 1;
      dect = asin (dt);
      da = coss - dt * sin0;
      dt = l * sins * cos0;
      if ((da==0.0) && (dt==0.0)) return 1;
      rat = ra0 + atan2 (dt, da);
      break;
    case 4:   /* -NCP North celestial pole*/
      dect = cos0 - m * sin0;
      if (dect==0.0) return 1;
      rat = ra0 + atan2 (l, dect);
      dt = cos (rat-ra0);
      if (dt==0.0) return 1;
      dect = dect / dt;
      if ((dect>1.0) || (dect<-1.0)) return 1;
      dect = acos (dect);
      if (dec0<0.0) dect = -dect;
      break;
    case 5:   /* -GLS global sinusoid */
      dect = dec0 + m;
      if (fabs(dect)>twopi/4.0) return 1;
      coss = cos (dect);
      if (fabs(l)>twopi*coss/2.0) return 1;
      rat = ra0;
      if (coss>deps) rat = rat + l / coss;
      break;
    case 6:   /* -MER mercator*/
      dt = yinc * cosr + xinc * sinr;
      if (dt==0.0) dt = 1.0;
      dy = (yref/2.0 + 45.0) * cond2r;
      dx = dy + dt / 2.0 * cond2r;
      dy = log (tan (dy));
      dx = log (tan (dx));
      geo2 = dt * cond2r / (dx - dy);
      geo3 = geo2 * dy;
      geo1 = cos (yref*cond2r);
      if (geo1<=0.0) geo1 = 1.0;
      rat = l / geo1 + ra0;
      if (fabs(rat - ra0) > twopi) return 1; /* added 10/13/94 DCW/EWG */
      dt = 0.0;
      if (geo2!=0.0) dt = (m + geo3) / geo2;
      dt = exp (dt);
      dect = 2.0 * atan (dt) - twopi / 4.0;
      break;
    case 7:   /* -AIT Aitoff*/
      dt = yinc*cosr + xinc*sinr;
      if (dt==0.0) dt = 1.0;
      dt = dt * cond2r;
      dy = yref * cond2r;
      dx = sin(dy+dt)/sqrt((1.0+cos(dy+dt))/2.0) -
	  sin(dy)/sqrt((1.0+cos(dy))/2.0);
      if (dx==0.0) dx = 1.0;
      geo2 = dt / dx;
      dt = xinc*cosr - yinc* sinr;
      if (dt==0.0) dt = 1.0;
      dt = dt * cond2r;
      dx = 2.0 * cos(dy) * sin(dt/2.0);
      if (dx==0.0) dx = 1.0;
      geo1 = dt * sqrt((1.0+cos(dy)*cos(dt/2.0))/2.0) / dx;
      geo3 = geo2 * sin(dy) / sqrt((1.0+cos(dy))/2.0);
      rat = ra0;
      dect = dec0;
      if ((l==0.0) && (m==0.0)) break;
      dz = 4.0 - l*l/(4.0*geo1*geo1) - ((m+geo3)/geo2)*((m+geo3)/geo2) ;
      if ((dz>4.0) || (dz<2.0)) return 1;;
      dz = 0.5 * sqrt (dz);
      dd = (m+geo3) * dz / geo2;
      if (fabs(dd)>1.0) return 1;;
      dd = asin (dd);
      if (fabs(cos(dd))<deps) return 1;;
      da = l * dz / (2.0 * geo1 * cos(dd));
      if (fabs(da)>1.0) return 1;;
      da = asin (da);
      rat = ra0 + 2.0 * da;
      dect = dd;
      break;
    case 8:   /* -STG Sterographic*/
      dz = (4.0 - sins) / (4.0 + sins);
      if (fabs(dz)>1.0) return 1;
      dect = dz * sin0 + m * cos0 * (1.0+dz) / 2.0;
      if (fabs(dect)>1.0) return 1;
      dect = asin (dect);
      rat = cos(dect);
      if (fabs(rat)<deps) return 1;
      rat = l * (1.0+dz) / (2.0 * rat);
      if (fabs(rat)>1.0) return 1;
      rat = asin (rat);
      mg = 1.0 + sin(dect) * sin0 + cos(dect) * cos0 * cos(rat);
      if (fabs(mg)<deps) return 1;
      mg = 2.0 * (sin(dect) * cos0 - cos(dect) * sin0 * cos(rat)) / mg;
      if (fabs(mg-m)>deps) rat = twopi/2.0 - rat;
      rat = ra0 + rat;
      break;
  }
/*  return ra in range  */
  raout = rat;
  decout = dect;
  if (raout-ra0>twopi/2.0) raout = raout - twopi;
  if (raout-ra0<-twopi/2.0) raout = raout + twopi;
  if (raout < 0.0) raout += twopi; /* added by DCW 10/12/94 */

/*  correct units back to degrees  */
  *xpos  = raout  / cond2r;
  *ypos  = decout  / cond2r;
  return 0;
}  /* End of worldposlm */

int xypix(double xpos, double ypos, double xref, double yref, 
      float xrefpix, float yrefpix, float xinc, float yinc, float rot,
      char *type, float *xpix, float *ypix)
/*-----------------------------------------------------------------------*/
/* routine to determine accurate pixel coordinates for an RA and Dec     */
/* returns 0 if successful otherwise:                                    */
/* 1 = angle too large for projection;                                   */
/* 2 = bad values                                                        */
/* does: -SIN, -TAN, -ARC, -NCP, -GLS, -MER, -AIT projections            */
/* anything else is linear                                               */
/* Input:                                                                */
/*   d   xpos    x (RA) coordinate (deg)                                 */
/*   d   ypos    y (dec) coordinate (deg)                                */
/*   d   xref    x reference coordinate value (deg)                      */
/*   d   yref    y reference coordinate value (deg)                      */
/*   f   xrefpix x reference pixel                                       */
/*   f   yrefpix y reference pixel                                       */
/*   f   xinc    x coordinate increment (deg)                            */
/*   f   yinc    y coordinate increment (deg)                            */
/*   f   rot     rotation (deg)  (from N through E)                      */
/*   c  *type    projection type code e.g. "-SIN";                       */
/* Output:                                                               */
/*   f  *xpix    x pixel number  (RA or long without rotation)           */
/*   f  *ypiy    y pixel number  (dec or lat without rotation)           */
/*-----------------------------------------------------------------------*/
 {double dx, dy, dz, sinr, cosr;
  int    iret;
  double cond2r=1.745329252e-2;

/* get coordinate offset */
  iret = xypixlm (xpos, ypos, xref, yref, xinc, yinc, rot, type, &dx, &dy);

/*  Correct for rotation */
  cosr = cos(rot*cond2r);
  sinr = sin(rot*cond2r);
  dz = dx*cosr + dy*sinr;
  dy = dy*cosr - dx*sinr;
  dx = dz;
/*     convert to pixels  */
  *xpix = dx / xinc + xrefpix;
  *ypix = dy / yinc + yrefpix;
  return iret;
}  /* end xypix */

int CDpix(double xpos, double ypos, double xref, double yref, 
      float xrefpix, float yrefpix, float xinc, float yinc, float rot,
      float icd1[2], float icd2[2], char *type, float *xpix, float *ypix)
/*-----------------------------------------------------------------------*/
/* routine to determine accurate pixel coordinates for an RA and Dec     */
/* uses IRAF  style CD matrix.  Note: xinc, yinc, and rot can be derived */
/* from icd1 and icd2 and should be compatible with them.                */
/* returns 0 if successful otherwise:                                    */
/* 1 = angle too large for projection;                                   */
/* 2 = bad values                                                        */
/* does: -SIN, -TAN, -ARC, -NCP, -GLS, -MER, -AIT projections            */
/* anything else is linear                                               */
/* Input:                                                                */
/*   d   xpos    x (RA) coordinate (deg)                                 */
/*   d   ypos    y (dec) coordinate (deg)                                */
/*   d   xref    x reference coordinate value (deg)                      */
/*   d   yref    y reference coordinate value (deg)                      */
/*   f   xrefpix x reference pixel                                       */
/*   f   yrefpix y reference pixel                                       */
/*   f   xinc    x coordinate increment (deg)                            */
/*   f   yinc    y coordinate increment (deg)                            */
/*   f   rot     rotation (deg)  (from N through E)                      */
/*   f   icd1[2] first column of inverse CD matrix                       */
/*   f   icd2[2] second column of inverse CD matrix                      */
/*   c  *type    projection type code e.g. "-SIN";                       */
/* Output:                                                               */
/*   f  *xpix    x pixel number  (RA or long without rotation)           */
/*   f  *ypiy    y pixel number  (dec or lat without rotation)           */
/*-----------------------------------------------------------------------*/
 {double l, m, dx, dy;
  int    iret;

  /* get coordinate offset */
  iret = xypixlm (xpos, ypos, xref, yref, xinc, yinc, rot, type, &l, &m);

  /*  Correct by inverse CD matrix */
  dx = icd1[0]*l + icd1[1]*m;
  dy = icd2[0]*l + icd2[1]*m;

  /*     convert to pixels  */
  *xpix = dx + xrefpix;
  *ypix = dy + yrefpix;
  return iret;
}  /* end CDpix */

int xypixlm(double xpos, double ypos, double xref, double yref, 
	    float xinc, float yinc, float rot, char *type, 
	    double *dx, double *dy)
/*-----------------------------------------------------------------------*/
/* routine to determine accurate coordinate offsets for an RA and Dec     */
/* returns 0 if successful otherwise:                                    */
/* 1 = angle too large for projection;                                   */
/* 2 = bad values                                                        */
/* does: -SIN, -TAN, -ARC, -NCP, -GLS, -MER, -AIT projections            */
/* anything else is linear                                               */
/* Input:                                                                */
/*   d   xpos    x (RA) coordinate (deg)                                 */
/*   d   ypos    y (dec) coordinate (deg)                                */
/*   d   xref    x reference coordinate value (deg)                      */
/*   d   yref    y reference coordinate value (deg)                      */
/*   f   xrefpix x reference pixel                                       */
/*   f   yrefpix y reference pixel                                       */
/*   f   xinc    x coordinate increment (deg)                            */
/*   f   yinc    y coordinate increment (deg)                            */
/*   f   rot     rotation (deg)  (from N through E)                      */
/*   c  *type    projection type code e.g. "-SIN";                       */
/* Output:                                                               */
/*   f  *xpix    x pixel number  (RA or long without rotation)           */
/*   f  *ypiy    y pixel number  (dec or lat without rotation)           */
/*-----------------------------------------------------------------------*/
 {double dz, r, ra0, dec0, ra, dec, coss, sins, dt, da, dd, sint, ddx, ddy;
  double l, m, geo1, geo2, geo3, sinr, cosr;
  double cond2r=1.745329252e-2, deps=1.0e-5, twopi=6.28318530717959;
  int   i, itype;
  char ctypes[8][5] ={"-SIN","-TAN","-ARC","-NCP", "-GLS", "-MER", "-AIT",
     "-STG"};

  /* 0h wrap-around tests added by D.Wells 10/12/94: */
  dt = (xpos - xref);
  if (dt > +180) xpos -= 360;
  if (dt < -180) xpos += 360;
  /* NOTE: changing input argument xpos is OK (call-by-value in C!) */

/* default values - linear */
  *dx = xpos - xref;
  *dy = ypos - yref;
  dz = 0.0;
/*     check axis increments - bail out if either 0 */
  if ((xinc==0.0) || (yinc==0.0)) {*dx=0.0; *dy=0.0; return 2;}

/*  find type  */
  itype = 0;  /* default type is linear */
  for (i=0;i<8;i++) if (!strncmp(type, ctypes[i], 4)) itype = i+1;
  if (itype==0) return 0;  /* done if linear */

/*  rotation */
  r = rot * cond2r;
  cosr = cos (r);
  sinr = sin (r);

/* Non linear position */
  ra0 = xref * cond2r;
  dec0 = yref * cond2r;
  ra = xpos * cond2r;
  dec = ypos * cond2r;

/* compute direction cosine */
  coss = cos (dec);
  sins = sin (dec);
  l = sin(ra-ra0) * coss;
  sint = sins * sin(dec0) + coss * cos(dec0) * cos(ra-ra0);
/* process by case  */
  switch (itype) {
    case 1:   /* -SIN sin*/ 
	 if (sint<0.0) return 1;
	 m = sins * cos(dec0) - coss * sin(dec0) * cos(ra-ra0);
      break;
    case 2:   /* -TAN tan */
	 if (sint<=0.0) return 1;
	 m = sins * sin(dec0) + coss * cos(dec0) * cos(ra-ra0);
	 l = l / m;
	 m = (sins * cos(dec0) - coss * sin(dec0) * cos(ra-ra0)) / m;
      break;
    case 3:   /* -ARC Arc*/
	 m = sins * sin(dec0) + coss * cos(dec0) * cos(ra-ra0);
	 if (m<-1.0) m = -1.0;
	 if (m>1.0) m = 1.0;
	 m = acos (m);
	 if (m!=0) 
	    m = m / sin(m);
	 else
	    m = 1.0;
	 l = l * m;
	 m = (sins * cos(dec0) - coss * sin(dec0) * cos(ra-ra0)) * m;
      break;
    case 4:   /* -NCP North celestial pole*/
	 if (dec0==0.0) 
	     return 1;  /* can't stand the equator */
	 else
	   m = (cos(dec0) - coss * cos(ra-ra0)) / sin(dec0);
      break;
    case 5:   /* -GLS global sinusoid */
	 dt = ra - ra0;
	 if (fabs(dec)>twopi/4.0) return 1;
	 if (fabs(dec0)>twopi/4.0) return 1;
	 m = dec - dec0;
	 l = dt * coss;
      break;
    case 6:   /* -MER mercator*/
	 dt = yinc * cosr + xinc * sinr;
	 if (dt==0.0) dt = 1.0;
	 ddy = (yref/2.0 + 45.0) * cond2r;
	 ddx = ddy + dt / 2.0 * cond2r;
	 ddy = log (tan (ddy));
	 ddx = log (tan (ddx));
	 geo2 = dt * cond2r / (ddx - ddy);
	 geo3 = geo2 * ddy;
	 geo1 = cos (yref*cond2r);
	 if (geo1<=0.0) geo1 = 1.0;
	 dt = ra - ra0;
	 l = geo1 * dt;
	 dt = dec / 2.0 + twopi / 8.0;
	 dt = tan (dt);
	 if (dt<deps) return 2;
	 m = geo2 * log (dt) - geo3;
	 break;
    case 7:   /* -AIT Aitoff*/
	 l = 0.0;
	 m = 0.0;
	 da = (ra - ra0) / 2.0;
	 if (fabs(da)>twopi/4.0) return 1;
	 dt = yinc*cosr + xinc*sinr;
	 if (dt==0.0) dt = 1.0;
	 dt = dt * cond2r;
	 ddy = yref * cond2r;
	 ddx = sin(ddy+dt)/sqrt((1.0+cos(ddy+dt))/2.0) -
	     sin(ddy)/sqrt((1.0+cos(ddy))/2.0);
	 if (ddx==0.0) ddx = 1.0;
	 geo2 = dt / ddx;
	 dt = xinc*cosr - yinc* sinr;
	 if (dt==0.0) dt = 1.0;
	 dt = dt * cond2r;
	 ddx = 2.0 * cos(ddy) * sin(dt/2.0);
	 if (ddx==0.0) ddx = 1.0;
	 geo1 = dt * sqrt((1.0+cos(ddy)*cos(dt/2.0))/2.0) / ddx;
	 geo3 = geo2 * sin(ddy) / sqrt((1.0+cos(ddy))/2.0);
	 dt = sqrt ((1.0 + cos(dec) * cos(da))/2.0);
	 if (fabs(dt)<deps) return 3;
	 l = 2.0 * geo1 * cos(dec) * sin(da) / dt;
	 m = geo2 * sin(dec) / dt - geo3;
      break;
    case 8:   /* -STG Sterographic*/
	 da = ra - ra0;
	 if (fabs(dec)>twopi/4.0) return 1;
	 dd = 1.0 + sins * sin(dec0) + coss * cos(dec0) * cos(da);
	 if (fabs(dd)<deps) return 1;
	 dd = 2.0 / dd;
	 l = l * dd;
	 m = dd * (sins * cos(dec0) - coss * sin(dec0) * cos(da));
      break;
  }  /* end of itype switch */

/*   back to degrees  */
  *dx = l / cond2r;
  *dy = m / cond2r;
  return 0;
}  /* end xypixlm */

int momnt (float ara[9][9], float x, float y, int nx, int ny, float momar[6],
   float fblank)
/*-----------------------------------------------------------------------*/
/*  Calculate all 0th, 1st, and 2nd moments of a nx*ny subarray of ara,  */
/*  centered at x,y.  nx and ny should be odd.                           */
/*  Inputs:                                                              */
/*     ara     Input data array                                          */
/*     x       x-center for moment calculation (1-rel)                   */
/*     y       y-center                                                  */
/*     nx      # of points to include in x-direction. nx                 */
/*             should be odd.  The points will be centered               */
/*             about x (rounded)                                         */
/*     ny      # of points in y-direction                                */
/*  Outputs:                                                             */
/*     momar   00,10,20,01,11,02 yx-moments of ara                       */
/*     return  0 => o.k.                                                 */
/*             1 => subarray doesn't fit in main array                   */
/*-----------------------------------------------------------------------*/
  {
    long  ind, i, j, k, nj, indx, indy, iax1, iax2, iay1, iay2;
    float s, t, arg, prod;
/*      compute loop limits (1-rel)   */
    i = x + 0.5;
    iax1 = i - nx/2;
    iax2 = iax1 + nx - 1;
    i = y + 0.5;
    iay1 = i - ny/2;
    iay2 = iay1+ ny - 1;
/*      check loop limits             */
    if ((iax1<1) || (iax2>9) || (iay1<1) || (iay2>9)) return 1;
/*      compute moments               */
    ind = 0;
    for (i = 1; i<=3; i++) {
      nj = 4 - i;
      for (j=1; j<=nj; j++) {
	ind = ind + 1;
	s = 0.0;
	for (indx=iax1; indx<=iax2; indx++) {
	  for (indy=iay1; indy<=iay2; indy++) {
	    t = ara[indy-1][indx-1];
	    if (t!=fblank) {
	      if (i>1) {
		prod = 1.0; 
		arg = indx - x;
		for (k=1; k<i; k++) prod *= arg;
		t = t * prod;}     /* ((indx - x)**(i-1)); */
	      if (j>1) {
		prod = 1.0; 
		arg = indy - y;
		for (k=1; k<j; k++) prod *= arg;
		t = t * prod;}  /* ((indy - y)**(j-1));*/
	      s = s + t;}
	  }
	}
	momar[ind-1] = s;
      }
    }
    return 0;
  }  /* end of momnt */

void matvmu (float m[3][3], float vi[3], float vo[3], int n)
/*-----------------------------------------------------------------------*/
/*  Matrix-vector multiplication  vo = vi * m                            */
/*  Inputs:                                                              */
/*     m       Input matrix                                              */
/*     vi      Input vector                                              */
/*     n       Array dimension                                           */
/*  Outputs:                                                             */
/*     vo      Output vector                                             */
/*-----------------------------------------------------------------------*/
{
  int  i, j;
  float s;

  for (i=0; i<n; i++) {
    s = 0.0;
    for (j=0; j<n; j++) s = s + m[j][i] * vi[j];
    vo[i] = s;
  }
}  /* end of matvmu */

int pfit (float a[9][9], float *s, float dx[2], float fblank)
/*--------------------------------------------------------------------*/
/*  Make a parabolic least-squares fit to a 9x9 matrix about the      */
/*  peak value in an array and determine strength and position        */
/*  of maximum.                                                       */
/*   Inputs:                                                          */
/*     a       Data input array[dec][RA]                              */
/*     fblank  Value for blanked pixel                                */
/*  Outputs:                                                          */
/*     dx  Position of max relative to a[5][5]                        */
/*     s   Strength of max                                            */
/*     return  0=OK else failed                                       */
/*--------------------------------------------------------------------*/
{
   float  absmax, x, y, temp[6], momar[6], d;
   float mat[3][3] = {{0.55555, -0.33333, -0.33333}, {-0.33333, 0.5, 0.0},
		     {-0.33333, 0.0, 0.5}};
   int ix, iy, ixmax, iymax;
/* default return values */
  dx[0] = 0;
  dx[1] = 0;
  *s = a[3][3];
/*  find peak in array    */
  absmax = 0.0; ixmax = -1; iymax = -1;
    for (ix=0; ix<9; ix++) {
       for (iy=0; iy<9; iy++) {
	  if ((a[iy][ix]!=fblank) && (fabs(a[iy][ix])>absmax)) 
	    {absmax=fabs(a[iy][ix]); ixmax = ix; iymax = iy;}
       }
    }
/* check for valid data */
  if ((ixmax<0) || (iymax<0)) return 1;
/*  00, 01, 02, 10, 11, 20 */
  x = ixmax+1;
  y = iymax+1;
/*  default values       */
  dx[0] = x - 5.0;
  dx[1] = y - 5.0;
  *s = a[iymax][ixmax];
  if (momnt (a, x, y, 3, 3, momar, fblank)) return 1;
/*  multiply matrix * even moms  yields const & quadratic terms */
  temp[0] = momar[0];
  temp[1] = momar[2];
  temp[2] = momar[5];
  matvmu (mat, temp, &temp[3], 3);
/*  pick up linear & cross term  */
  temp[0] = momar[1] / 6.;
  temp[1] = momar[3] / 6.;
  temp[2] = momar[4] / 4.;
/*  offset of peak */
  d = 4.* temp[4] * temp[5] - (temp[2]*temp[2]);
  if (d==0.0) return 2;
  dx[0] = (temp[2]*temp[0] - 2.*temp[1]*temp[4]) / d;
  dx[1] = (temp[2]*temp[1] - 2.*temp[0]*temp[5]) / d;
/*  value of peak */
  *s = temp[3] + dx[0]*(temp[1] + dx[0]*temp[5]
       + dx[1]*temp[2]) + dx[1]*(temp[0]+dx[1]*temp[4]);
  dx[0] = dx[0] + x - 5.0;  /* correct wrt center of input array */
  dx[1] = dx[1] + y - 5.0;
  return 0;
} /* end of pfit */
