/*  Utility routines related to celestial positions on DSS images*/
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
#include "dsssubs.h"
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

/* note: these routines re from the AIPS program SKYVE by Mark Calebretta */

/* internal prototypes */
void dsscrd (double dssx[13], double dssy[13], double x, double y, 
	     double *xi, double *eta, double *dxidx, double *dxidy,
	     double *detadx, double *detady, Integer *ierr);
void eqstd (double ra0, double dec0, double ra, double dec, 
	    double *xi, double *eta, Integer *ierr);


void dsseq (double xpixsz, double ypixsz, double ra0, double dec0, 
	    double xoff0, double yoff0, double dssx[13], double dssy[13], 
	    float xpix, float ypix, double *ra, double *dec, Integer *ierr)
/*-----------------------------------------------------------------------
*     dsseq computes the J2000.0 equatorial coordinates of the specified
*     Digitized Sky Survey pixel coordinates.
*
*     Given:
*          xpixsz      D     Plate pixel size in X and Y, micron.
*      and ypixsz      D
*          ra0,dec0    D     Plate centre J2000.0 right ascension and
*                            declination, in degrees.
*          xoff0,yoff0 D     Plate centre offsets, micron.
*          dssx(13)    D     The coefficients for the plate solution for
*          dssy(13)          computing standard plate coordinates
*                            (XI,ETA) in arcsec from plate offsets
*                            (X,Y), in mm.
*          xpix,ypix   R     DSS pixel coordinates.
*
*     Returned:
*          ra,dec      D     Required J2000.0 right ascension and
*                            declination, in degrees.
*          ierr        I     Error status, 0 means success.
*
*     Called:
*          none
*
*     Algorithm:
*          The equations for converting DSS pixel coordinates to
*          offsets from the plate centre are given on page 10 of the
*          booklet supplied with the Digitized Sky Survey CD set.
*
*          The equations for computing standard DSS plate coordinates
*          from plate offsets are given on page 11 of the booklet
*          supplied with the Digitized Sky Survey CD:
*
*             xi  = a1*x + a2*y + a3 + a4*x*x + a5*x*y + a6*y*y +
*                   a7*(x*x+y*y) + a8*x*x*x + a9*x*x*y + a10*x*y*y +
*                   a11*y*y*y + a12*x*(x*x+y*y) + a13*x*(x*x+y*y)**2
*
*             eta = b1*y + b2*x + b3 + b4*y*y + b5*x*y + b6*x*x +
*                   b7*(x*x+y*y) + b8*y*y*y + b9*x*y*y + b10*x*x*y +
*                   b11*x*x*x + b12*y*(x*x+y*y) + b13*y*(x*x+y*y)**2
*
*          The equations for computing J2000.0 right ascension and
*          declination from the standard coordinates are given on page
*          11 of the booklet supplied with the Digitized Sky Survey CD.
*          However, note that there is a misprint in the equation for
*          declination, the COS(RA0) term should be COS(RA-RA0).
*
*     Notes:
*       1)
*
*     Author:
*          Mark Calabretta, Australia Telescope.
*          Origin; 1994/08/03  Code last modified; 1994/08/04
*          Translated to c by W. D. Cotton, NRAO
*-----------------------------------------------------------------------*/
{
  double    as2r, cdec, d2r, eta, f, pi, tdec, x, xi, xx, xxyy, xy, y, yy;
/*     Pi. */
  pi = 3.141592653589793238462643;
/*     Factor to convert degrees to radians. */
  d2r = pi/180.0;
/* */
/*     Factor to convert arcsec to radians. */
  as2r = d2r/3600;
/*----------------------------------------------------------------------- */
  *ierr = 0;
/*                                       Compute offsets. */
  x = (xoff0 - xpixsz*xpix)/1000.0;
  y = (ypixsz*ypix - yoff0)/1000.0;
/*                                       Compute temporaries. */
  xx = x*x;
  yy = y*y;
  xy = x*y;
  xxyy = xx + yy;
/*                                       Compute standard coordinates. */
  xi =     dssx[2] +
    x*(dssx[0] + x*(dssx[3] + x*dssx[7]))  +
    y*(dssx[1] + y*(dssx[5] + y*dssx[10])) +
    xy*(dssx[4] + x*dssx[8]  + y*dssx[9])  +
    xxyy*(dssx[6] + x*dssx[11] + dssx[12]*x*xxyy);
  eta =    dssy[2] +
    y*(dssy[0] + y*(dssy[3] + y*dssy[7]))  +
    x*(dssy[1] + x*(dssy[5] + x*dssy[10])) +
    xy*(dssy[4] + y*dssy[8]  + x*dssy[9])  +
    xxyy*(dssy[6] + y*dssy[11] + dssy[12]*y*xxyy);
/*                                       Convert to radians. */
  xi  = xi*as2r;
  eta = eta*as2r;
/*                                       Compute J2000.0 coordinates. */
  cdec = cos(dec0*d2r);
  tdec = tan(dec0*d2r);
  f = 1.0 - eta*tdec;
  *ra = atan((xi/cdec)/f)/d2r + ra0;
  *dec = atan(((eta+tdec)*cos((*ra-ra0)*d2r))/f)/d2r;
  if (fabs (dec0-*dec) > 90.0) {
/*                                       Wrong solution */
    *dec = -*dec;
    if (*ra>180.0) {
      *ra = *ra - 180.0; }
    else {
      *ra = *ra + 180.0; }
  } /* end of check for wrong solution */
} /* end dsseq */

void dsspix (double scale, double xpixsz, double ypixsz, double ra0, 
	     double dec0, double xoff0, double yoff0, double dssx[13], 
	     double dssy[13], double ra, double dec, 
	     float *xpix, float *ypix, Integer *ierr)
/*-----------------------------------------------------------------------
*     DSSPIX computes the pixel coordinates in a Digitized Sky Survey
*     plate corresponding to the specified J2000.0 equatorial
*     coordinate.  This requires inversion of the plate solution and
*     this it does by iteration.
*
*     Given:
*          scale       D     Approximate plate scale, arcsec/mm.
*          xpixsz      D     Plate pixel size in X and Y, micron.
*      and ypixsz      D
*          ra0,dec0    D     Plate centre J2000.0 right ascension and
*                            declination, in degrees.
*          xoff0,yoff0 D     Plate centre offsets, micron.
*          dssx(13)    D     The coefficients for the plate solution for
*          dssy(13)          computing standard plate coordinates
*                            (XI,ETA) in arcsec from plate offsets
*                            (X,Y), in mm.
*          ra,dec      D     Required J2000.0 right ascension and
*                            declination, in degrees.
*
*     Returned:
*          xpix,ypix   R     DSS pixel coordinates.
*          ierr        I     Error status, 0 means success.
*
*     Called:
*          {DSSCRD, EQSTD}
*
*     Algorithm:
*          The iteration formula is obtained by simultaneously solving
*          for DX and DY from the equations for the total differentials:
*
*              Dx = DX*dx/dX + Dy*dx/dY
*              Dy = DX*dy/dX + Dy*dy/dY
*
*          where
*
*                       x -> xi
*                       y -> eta
*                  DX, DY -> total differential of X and Y
*              d/dX, d/dY -> partial derivative with respect to X and Y
*
*          The equations for converting DSS pixel coordinates to
*          offsets from the plate centre are given on page 10 of the
*          booklet supplied with the Digitized Sky Survey CD set.
*
*     Notes:
*       1)
*
*     Author:
*          Mark Calabretta, Australia Telescope.
*          Origin; 1994/07/27  Code last modified; 1994/08/04
*          Translated to c by W. D. Cotton
*----------------------------------------------------------------------- */
{
  Integer   iter, niter;
  double    deta, detadx, detady, dx, dxi, dxidx, dxidy, dy, eta, eta0;
  double    tol, xoff, xi, xi0, yoff, z;
/*----------------------------------------------------------------------- */
/*     *ierr = 0 */
/*                                       Initialize. */
  niter = 50;
  tol = (min(xpixsz,ypixsz)/100.0)/1000.0;
/*                                       Convert to standard coordinates. */
  eqstd (ra0, dec0, ra, dec, &xi0, &eta0, ierr);
/*                                       Initial guess for plate offset. */
  xoff =  xi0/scale;
  yoff = eta0/scale;
/*                                       Iterate. */
  for (iter=0; iter<=niter; iter++)
    {
/*                                       Compute standard coordinates */
/*                                       and their derivatives. */
      dsscrd (dssx, dssy, xoff, yoff, &xi, &eta, &dxidx, &dxidy,
	      &detadx, &detady, ierr);
/*                                       Error terms. */
      dxi   =  xi0 - xi;
      deta  = eta0 - eta;
/*                                       Compute correction. */
      z  = dxidx*detady-dxidy*detadx;
      dx = (dxi*detady - deta*dxidy)/z;
      dy = (deta*dxidx - dxi*detadx)/z;
/*                                       Apply correction. */
      xoff = xoff + dx;
      yoff = yoff + dy;
/*                                       Test for convergence. */
      if ((fabs(dx)<tol) && (fabs(dy)<tol)) break;
    } /* end of iteration loop */
/*                                       Convert offsets to pixels. */
  *xpix = (xoff0 - 1000.0*xoff)/xpixsz;
  *ypix = (yoff0 + 1000.0*yoff)/ypixsz;
} /* end dsspix */

void dsscrd (double dssx[13], double dssy[13], double x, double y, 
	     double *xi, double *eta, double *dxidx, double *dxidy,
	     double *detadx, double *detady, Integer *ierr)
/*----------------------------------------------------------------------- 
*     DSSCRD computes the standard DSS plate coordinates and their
*     partial derivatives for use in inverting the plate solution
*     equations.
*
*     Given:
*          dssx(13)    D     The coefficients for the plate solution for
*          dssy(13)          computing standard plate coordinates
*                            (XI,ETA) in arcsec from plate offsets
*                            (X,Y), in mm.
*          x,y         D     (X,Y) plate offset, in mm.
*
*     Returned:
*          xi,eta      D     Standard plate coordinates, in arcsec.
*          dxidx       D     Derivative of XI  with respect to X.
*          dxidy       D     Derivative of XI  with respect to Y.
*          detadx      D     Derivative of ETA with respect to X.
*          detady      D     Derivative of ETA with respect to Y.
*          ierr        I     Error status, 0 means success.
*
*     Called:
*          none
*
*     Algorithm:
*          The equations for computing standard DSS plate coordinates
*          from plate offsets are given on page 11 of the booklet
*          supplied with the Digitized Sky Survey CD:
*
*             xi  = a1*x + a2*y + a3 + a4*x*x + a5*x*y + a6*y*y +
*                   a7*(x*x+y*y) + a8*x*x*x + a9*x*x*y + a10*x*y*y +
*                   a11*y*y*y + a12*x*(x*x+y*y) + a13*x*(x*x+y*y)**2
*
*             eta = b1*y + b2*x + b3 + b4*y*y + b5*x*y + b6*x*x +
*                   b7*(x*x+y*y) + b8*y*y*y + b9*x*y*y + b10*x*x*y +
*                   b11*x*x*x + b12*y*(x*x+y*y) + b13*y*(x*x+y*y)**2
*
*     Notes:
*       1) Adapted from the C function pltmodel() in the "getimage"
*          library supplied with the Digitized Sky Survey.  Note that
*          this routine has a bug in the computation of the derivative
*          of ETA with respect to Y.
*
*     Author:
*          Mark Calabretta, Australia Telescope.
*          Origin; 1994/07/26  Code last modified; 1994/08/04
*-----------------------------------------------------------------------*/
{
  double xx, xxyy, xy, yy;
/*----------------------------------------------------------------------- */
  *ierr = 0;
/*                                       Compute temporaries. */
  xx = x*x;
  yy = y*y;
  xy = x*y;
  xxyy = xx + yy;
  /*                                       Compute XI. */
  *xi =     dssx[2] +
    x*(dssx[0] + x*(dssx[3] + x*dssx[7]))  +
    y*(dssx[1] + y*(dssx[5] + y*dssx[10])) +
    xy*(dssx[4] + x*dssx[8]  + y*dssx[9])  +
    xxyy*(dssx[6] + x*dssx[11] + dssx[12]*x*xxyy);
  /*                                       Derivative of XI wrt X. */
  *dxidx =  dssx[0] +
    x*(2.0*(dssx[3] + dssx[6]) + 3.0*x*(dssx[7] + dssx[11])) +
    y*(dssx[4] + y*(dssx[9] + dssx[11])) +
    2.0*xy*dssx[8] + xxyy*(4.0*xx + xxyy)*dssx[12];
  /*                                       Derivative of XI wrt Y. */
  *dxidy =  dssx[1] +
    y*(2.0*(dssx[5] + dssx[6]) + 3.0*y*dssx[10]) +
    x*(dssx[4] + x*dssx[8]) +
    2.0*xy*(dssx[9] + dssx[11]) + 4.0*xy*xxyy*dssx[12];
  /*                                       Compute ETA. */
  *eta =    dssy[2] +
    y*(dssy[0] + y*(dssy[3] + y*dssy[7]))  +
    x*(dssy[1] + x*(dssy[5] + x*dssy[10])) +
    xy*(dssy[4] + y*dssy[8]  + x*dssy[9])  +
    xxyy*(dssy[6] + y*dssy[11] + dssy[12]*y*xxyy);
  /*                                       Derivative of ETA wrt X. */
  *detadx = dssy[1] +
    x*(2.0*(dssy[5] + dssy[6]) + 3.0*x*dssy[10]) +
    y*(dssy[4] + y*dssy[8]) +
    2.0*xy*(dssy[9] + dssy[11]) + 4.0*xy*xxyy*dssy[12];
  /*                                       Derivative of ETA wrt Y. */
  *detady = dssy[0] +
    y*(2.0*(dssy[3] + dssy[6]) + 3.0*y*(dssy[7] + dssy[11])) +
    x*(dssy[4] + x*(dssy[9] + dssy[11])) +
    2.0*xy*dssy[8] + xxyy*(4.0*yy + xxyy)*dssy[12];
} /* end dsscrd */

void eqstd (double ra0, double dec0, double ra, double dec, 
	    double *xi, double *eta, Integer *ierr)
/*-----------------------------------------------------------------------
*     eqstd converts J2000.0 equatorial coordinates to standard
*     coordinates on a Digitized Sky Survey plate.
*
*     Given:
*          ra0,dec0    D     Plate centre J2000.0 right ascension and
*                            declination, in degrees.
*          ra,dec      D     Required J2000.0 right ascension and
*                            declination, in degrees.
*
*     Returned:
*          xi,eta      D     Standard plate coordinates, in arcsec.
*          ierr        I     Error status, 0 means success.
*
*     Called:
*          none
*
*     Algorithm:
*          The equations for computing J2000.0 right ascension and
*          declination from the standard coordinates are given on page
*          11 of the booklet supplied with the Digitized Sky Survey CD
*          and these are readily invertible.  However, note that there
*          is a misprint in the equation for declination, the COS(RA0)
*          term should be COS(RA-RA0).
*
*     Notes:
*       1) Adapted from the C function transeqstd() in the "getimage"
*          library supplied with the Digitized Sky Survey.
*
*     Author:
*          Mark Calabretta, Australia Telescope.
*          Origin; 1994/07/26  Code last modified; 1994/08/05
*          Translated to c by W. D. Cotton, NRAO
----------------------------------------------------------------------- */
{
  double cdec, cdec0, cdra, d2r, f, pi, r2as, sdec, sdec0, sdra, z;

/*     Pi. */
    pi = 3.141592653589793238462643;
/*     Factor to convert degrees to radians. */
  d2r = pi/180.0;
/*     Factor to convert radians to arcsec. */
  r2as = 180.0*3600.0/pi;
/*----------------------------------------------------------------------- */
  *ierr = 0;
/*                                       Cache trigonometric evaluations. */
  z = dec*d2r;
  cdec  = cos(z);
  sdec  = sin(z);
  z = dec0*d2r;
  cdec0 = cos(z);
  sdec0 = sin(z);
  z = (ra-ra0)*d2r;
  cdra = cos(z);
  sdra = sin(z);
  /*                                       compute common factor. */
  f = r2as/(sdec*sdec0 + cdec*cdec0*cdra);
  /*                                       Compute standard coordinates. */
  *xi  = cdec*sdra*f;
  *eta = (sdec*cdec0 - cdec*sdec0*cdra)*f;
} /* end eqstd*/

void fixcoo (double xpixsz, double ypixsz, double ra0, double dec0, 
	     double ppo[6], double amdx[13], double amdy[13],
	     float cnpix[2], Integer dim0, Integer dim1, double scale,
	     char* ctype0, char* ctype1, float* cdelt0, float* cdelt1,
	     float* crpix0, float* crpix1, float* crota0, float* crota1,
	     double* crval0, double* crval1, Integer *ierr)
/*-----------------------------------------------------------------------
* call from ImageDescriptorFixDSS:
*  fixcoo ((double)desc->pixel_size[0], (double)desc->pixel_size[1], 
*          desc->plate_RA, desc->plate_DEC, desc->PPO,
*          desc->AMDX, desc->AMDY, desc->corn_pixel,
*          desc->axisdesc[0]->axis_length, &esc->axisdesc[1]->axis_length,
*          (double)desc->plate_scale, 
*          desc->axisdesc[0]->axis_name->sp, desc->axisdesc[1]->axis_name->sp,
*          &desc->axisdesc[0]->axis_increment, &desc->axisdesc[1]->axis_increment,
*          &desc->axisdesc[0]->axis_ref_pixel, &desc->axisdesc[1]->axis_ref_pixel,
*          &desc->axisdesc[0]->axis_rotation, &desc->axisdesc[1]->axis_rotation,
*          &desc->axisdesc[0]->axis_coord, &desc->axisdesc[1]->axis_coord,
*          &ierr);
*   Fix up coordinates in header (2 dim); give standard WCS positions.
*   Compute position at center using DSS formula and axis increments
*   from finite differences.  The rotation is determined by obtaining
*   the pixel location of a position 10 sec north of the field center.
*     Inputs:
*          xpixsz      D     Plate pixel size in X and Y, micron.
*      and ypixsz      D
*          ra0,dec0    D     Plate centre J2000.0 right ascension and
*                            declination, in degrees.
*          xoff0,yoff0 D     Plate centre offsets, micron.
*          dssx(13)    D     The coefficients for the plate solution for
*          dssy(13)          computing standard plate coordinates
*                            (XI,ETA) in arcsec from plate offsets
*                            (X,Y), in mm.
*          cnpix(2)    R     DSS corner pixel
*          inaxes(2)   I     dimensionality of image
*          scale       D     Plate scale asec/mm PLTSCALE
*   Output:
*      ierr            I    Error code: 0 => ok
*   Output:
*      ctype0, ctype1   C  CTYPEn (All should be allocated)
*      cdelt0, cdelt1   R  CDELTn
*      crpix0, crpix1   R  CRPIXn
*      crota0, crota1   R  CROTAn
*      crval0, crval1   D  CRVALn
*-----------------------------------------------------------------------*/
{
  float xcen, ycen, xdss, ydss, rot, xt, yt;
  double rat, dect, rae, dece, ran, decn;
/*                                       Coordinate types */
  strcpy (ctype0, "RA---ARC");
  strcpy (ctype1, "DEC--ARC");
/*                                       Get center RA, Dec */
  xcen = dim0 / 2;
  ycen = dim1 / 2;
  *crpix0 = xcen;
  *crpix1 = ycen;
  *crota0 = 0.0;
  *crota1 = 0.0;
  *cdelt0 = -1.7 / 3600.0;
  *cdelt0 = 1.7 / 3600.0;
  *crval0 = ra0;
  *crval1 = dec0;
/*                                       Pixel numbers in original */
  xdss = xcen + cnpix[0] - 0.5;
  ydss = ycen + cnpix[1] - 0.5;
/*                                       Get center position */
  dsseq (xpixsz, ypixsz, ra0, dec0, ppo[2], ppo[5], amdx, amdy,
	 xdss, ydss, crval0, crval1, ierr);
  if (*ierr!=0) return;
/*                                       Get rotation - go 10" north */
  rat = *crval0;
  dect = *crval1 + 10.0 / 3600.0;
  dsspix (scale, xpixsz, ypixsz, ra0, dec0, ppo[2], ppo[5],
          amdx, amdy, rat, dect, &xt, &yt, ierr);
  if (*ierr!=0) return;
  rot = atan2 (xt-xdss, yt-ydss);
  *crota1 = -rot * 57.2957795;
/*                                       1 pixel east */
  xdss = xdss - 1.0;
  dsseq (xpixsz, ypixsz, ra0, dec0, ppo[2], ppo[5], amdx, amdy,
         xdss, ydss, &rae, &dece, ierr);
  if (*ierr!=0) return;
  *cdelt0 = -(rae - *crval0) * cos (*crval1*1.74533e-2) / cos (rot);
/*                                       1 pixel north */
  xdss = xdss + 1.0;
  ydss = ydss + 1.0;
  dsseq (xpixsz, ypixsz, ra0, dec0, ppo[2], ppo[5], amdx, amdy,
         xdss, ydss, &ran, &decn, ierr);
  if (*ierr!=0) return;
/*                                       Declination increment */
  *cdelt1 = (decn - *crval1) / cos (rot);
} /* end of fixcoo */
