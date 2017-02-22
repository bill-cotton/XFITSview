/*  Utility routines related to celestial positions */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996
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
#include "precess.h"
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

void BtoJ (double *ra, double *dec)
/* ***********************************************************************/
/*   converts B1950 RA, Dec to J2000                                     */
/*   Using method on page B42 of The Astronomical Almanac (1990 ED.).    */
/*   Calling And Returned Arguments:                                     */
/*      ra  double  Right Ascension in degrees                           */
/*      dec double  Declination in degrees                               */
/*   Revised 90/05/07 J. J. Condon                                       */
/*   Translated to c 96/02/21 W. D. Cotton                               */
/* ***********************************************************************/
{
  int    i;
  double ra0, dec0, temp, r0ta, rat, dect;
  double sina, cosa, sindd, cosdd, rnorm;
  double pi = 3.1415926536;
  double a[3] = {-1.62557e-6,-0.31919e-6,-0.13843e-6};
  double m1[3] = {0.9999256782, 0.0111820610, 0.0048579479};
  double m2[3] = {-0.0111820611, 0.9999374784, -0.0000271474};
  double m3[3] = {-0.0048579477, -0.0000271765, +0.9999881997};
  double r0[3], r1[3], r[3];

/*    First convert input B1950 Ra, Dec to radians*/
  ra0 = *ra * pi / 180.0;
  dec0 = *dec * pi / 180.0;
/*    Then convert B1950 RA, Dec to cartesian coordinates*/
  r0[0] = cos (ra0) * cos (dec0);
  r0[1] = sin (ra0) * cos (dec0);
  r0[2] = sin (dec0);
/*    Remove aberration E-terms      */
/*    (r0ta = scalar product of r0 and a)*/
  r0ta = r0[0] * a[0] + r0[1] * a[1] + r0[2] * a[2];
  for (i=0; i<3; i++) r1[i] = r0[i] - a[i] + r0ta * r0[i];
/*    Precess from B1950 to J2000*/
  for (i=0; i<3; i++) r[i] = m1[i] * r1[0] + m2[i] * r1[1] + m3[i] * r1[2];
/*    Convert J2000 Cartesian coordinates to J2000 RA, Dec (radians)*/
  rnorm = sqrt (r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
  sindd = r[2] / rnorm;
  dect = asin (sindd);
  cosdd = sqrt (1.0 - sindd * sindd);
  cosa = r[0] / (rnorm * cosdd);
  sina = r[1] / (rnorm * cosdd);
  if (cosa != 0.0) rat = atan (sina / cosa);
  if (cosa == 0.0)
    {if (sina > 0.0) rat = pi / 2.0;
     if (sina < 0.0) rat = 1.50 * pi;}
    /*     Resolve 12h ambiguity of RA*/
    rat = rat * 12.0 / pi;
    if (cosa < 0.0) rat = rat + 12.0;
    if (rat < 0.0) rat = rat + 24.0;
    /*    Convert to degrees */
    *ra = rat * 15.0;
    *dec = dect * 180.0 / pi;
    } /* end BtoJ */

void JtoB (double *ra, double *dec)
/* ***********************************************************************/
/*   converts J2000 RA, Dec to B1950                                     */
/*   Using method on page B43 of The Astronomical Almanac (1990 ED.).    */
/*   Calling And Returned Arguments:                                     */
/*      ra  double  Right Ascension in degrees                           */
/*      dec double  Declination in degrees                               */
/*   Revised 90/10/15 J. J. Condon                                       */
/*   Translated to c 96/02/21 W. D. Cotton                               */
/* ***********************************************************************/
  {
  int    i, iter;
  double ra0, dec0, temp, sta, rat, dect;
  double sina, cosa, sindd, cosdd, rnorm, r1norm;
  double pi = 3.1415926536;
  double a[3] = {-1.62557e-6, -0.31919e-6, -0.13843e-6};
  double minv1[3] = {0.9999256795, -0.0111814828, -0.0048590040};
  double minv2[3] = {0.0111814828, 0.9999374849, -0.0000271557};
  double minv3[3] = {0.0048590039, -0.0000271771, +0.9999881946};
  double r0[3], r1[3], r[3], s[3], s1[3];
  /*    First convert input J2000 Ra, Dec to radians*/
  ra0 = *ra * pi / 180.0;
  dec0 = *dec * pi / 180.0;
  /*    Then convert J2000 RA, Dec to cartesian coordinates*/
  r0[0] = cos (ra0) * cos (dec0);
  r0[1] = sin (ra0) * cos (dec0);
  r0[2] = sin (dec0);
  /*    Precess from J2000 to B1950 */
  for (i=0; i<3; i++) r1[i] = minv1[i]*r0[0] + minv2[i]*r0[1] + minv3[i]*r0[2];
  /*    include aberration E-terms      */
  r1norm = sqrt (r1[0] * r1[0] + r1[1] * r1[1] + r1[2] * r1[2]);
  for (i=0; i<3; i++) s1[i] = r1[i] / r1norm;
  for (i=0; i<3; i++) s[i] = s1[i];
  /*    Three-Step iteration for r*/
  for (iter=0; iter<3; iter++)
    {
      /*  (sta = scalar product of s and a)*/
      sta = s[0] * a[0] + s[1] * a[1] + s[2] * a[2];
      /*  calculate or recalculate r*/
      for (i=0; i<3; i++) r[i] = s1[i] + a[i]  - sta * s[i];
      rnorm = sqrt (r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
      /*   calculate or recalculate s*/
      for (i=0; i<3; i++) s[i] = r[i] / rnorm;
    } /* end iteration loop */
  /*       Convert B1950 Cartesian coordinates (r-transpose) */
  /*       to B1950 RA, Dec (radians)*/
  sindd = r[2] / rnorm;
  dect = asin (sindd);
  cosdd = sqrt (1.0 - sindd * sindd);
  cosa = r[0] / (rnorm * cosdd);
  sina = r[1] / (rnorm * cosdd);
  if (cosa != 0.0) rat = atan (sina / cosa);
  if (cosa == 0.0) 
    {if (sina > 0.0) rat = pi / 2.0;
     if (sina < 0.0) rat = 1.50 * pi;}
  /*      Then convert to deg of dec, hr of ra, */
  /*      resolve 12h ambiguity of RA */
  *dec = dect * 180.0 / pi;
  rat = rat * 12.0 / pi;
  if (cosa < 0.0) rat = rat + 12.0;
  if (rat < 0.0) rat = rat + 24.0;
  /*      Finally convert B1950 RA to degrees*/
  *ra = rat * 15.0;
} /* end JtoB */
