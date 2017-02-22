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
#include "wpos.h"
#include "position.h"
#include "precess.h"
#include "dsssubs.h"
#include "zsubs.h"
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

void rahms(double ra, int *h, int *m, float *s)
/* convert RA in degrees to hours min and seconds */
{double temp;
 int itemp;
 temp = ra; if (temp<0.0) temp = temp + 360.0;
 if (temp>360.0) temp = temp - 360.0;
 temp =temp / 15.0; itemp = (int)temp; *h = itemp;
 temp = temp - (double)itemp;
 temp = temp*60.0; itemp = (int)temp; *m = itemp;
 temp = temp - (double)itemp;
 temp = temp*60.0; *s = (float)temp;
} /* End of rahms */

void decdms(double dec, int *d, int *m, float *s)
/* convert dec in degrees to degrees, min, sec */
{double temp;
 int itemp;
 temp = dec; itemp = (int)temp; *d = itemp;
 temp = temp - (double)itemp;
 if (temp<0.0) temp = -temp;
 temp = temp*60.0; itemp = (int)temp; *m = itemp;
 temp = temp - (double)itemp;
 temp = temp*60.0; *s = (float)temp;
} /* End of decdms */

int hmsra(int h, int m, float s, double *ra)
/* convert RA in hours min and seconds to degrees*/
/* returns 0 if in 0-24 hours else 1 */
{
 *ra = h + m/60.0 + s/3600.0;
 *ra = *ra * 15.0;
 if (*ra<0.0) return 1;
 if (*ra>360.0) return 1;
 return 0;
 } /* End of hmsra */

int dmsdec(int d, int m, float s, double *dec)
/* convert dec in degrees, min, sec  to degrees*/
/* Note: this is also used for longitudes */
/* returns 0 if in range +/-360 else 1 */
{
 int absdec = d;
 if (absdec<0) absdec = -absdec;
 *dec = absdec + m/60.0 + s/3600.0;
 if (d<0) *dec = -(*dec);
 if (*dec<-360.0) return 1;
 if (*dec>360.0) return 1;
 return 0;
} /* End of dmsdec */

void AxisLabel(double pos, char* axis, char* label)
/* create appropriate character string describing a location */
/* pos is location on axis, angles in degrees                */
/* axis is FITS label for the axis                           */
/* label is output string, must be large enough for 17 char  */
/* label contains first 4 characters of axis and the value.  */
{double temp;
 int h, m, itemp, i, recog, toHours, isStokes, iStokes;
 float s;
 char rectypes[9][5]={"RA  ", "GLON", "ELON", "LL  ",
   "DEC ", "GLAT", "ELAT", "MM  ", "STOK"};
 char stoktypes[4][5]={"Ipol", "Qpol", "Upol", "Vpol"};
 char minus[2]="-";
/* convert to hours for "RA" or "LL" only */
 toHours = (!strncmp(axis, rectypes[0], 2))  || 
	   (!strncmp(axis, rectypes[3], 2));
/* is this a Stokes Axis */
 isStokes = !strncmp(axis, rectypes[8], 4);
/* make label (strip minus signs) */
  for (i=0;i<4;i++) label[i] = ' '; label[4] = 0;
  for (i=0;i<4;i++) {if (axis[i]==minus[0]) break; label[i]=axis[i];}
 recog = 0;  /* look for recognized position types */
 for (i=0;i<8;i++) recog = recog || (!strcmp (label, rectypes[i]));
 if(recog) { /* conversion for position */
   temp = pos; 
   if (temp>360.0) temp = temp - 360.0;
   if (toHours) 
     {if (temp<0.0) temp = temp + 360.0;
      temp = temp / 15.0; }
   itemp = (int)temp; h = itemp;
   temp = temp - (double)itemp;
   if (temp<0.0) temp = - temp;
   temp = temp*60.0; itemp = (int)temp; m = itemp;
   temp = temp - (double)itemp;
   temp = temp*60.0; s = (float)temp;}
  if (recog)
     {if (toHours)  /* display in hours */
	sprintf (&label[4], "%2.2d %2.2d %7.4f", h, m, s);
      else {        /* display in degrees */
	if (h <0) h = -h;
	sprintf (&label[4], "%3.2d %2.2d %6.3f", h, m, s);
	if (pos<0.0) label[4]='-'; /* neg declination */
      }
     }
  else if (isStokes)
    {iStokes = (int)(pos + 0.5) - 1;
     iStokes = max (min (iStokes, 3), 0);
     sprintf (&label[4], "     %s", stoktypes[iStokes]);}
  else  /* random type */
     sprintf (&label[4], "%13.6lg", pos);
} /* End of AxisLabel */

void ra2hms(double ra, char* rach, char* rast)
/* convert RA in degrees to hh mm ss.sss (17 chars) */
/* rach is FITS label for RA axis                   */
{double temp;
 int h, m, itemp, i, recog, toHours;
 float s;
 char rectypes[8][5]={"RA  ", "GLON", "ELON", "LL  ",
   "DEC ", "GLAT", "ELAT", "MM  "};
 char minus[2]="-";
/* convert to hours for "RA" or "LL" only */
 toHours = (!strncmp(rach, rectypes[0], 2))  || 
	   (!strncmp(rach, rectypes[3], 2));
 temp = ra; 
 if (temp>360.0) temp = temp - 360.0;
 if (toHours) 
    {if (temp<0.0) temp = temp + 360.0;
     temp = temp / 15.0; }
 itemp = (int)temp; h = itemp;
 temp = temp - (double)itemp;
 if (temp<0.0) temp = - temp;
 temp = temp*60.0; itemp = (int)temp; m = itemp;
 temp = temp - (double)itemp;
 temp = temp*60.0; s = (float)temp;
/* make label (strip minus signs) */
  for (i=0;i<4;i++) rast[i] = ' '; rast[4] = 0;
  for (i=0;i<4;i++) {if (rach[i]==minus[0]) break; rast[i]=rach[i];}
  recog = 0;  /* look for recognized types */
  for (i=0;i<8;i++) recog = recog || (!strcmp (rast, rectypes[i]));
  if (recog)
     {if (toHours)  /* display in hours */
	sprintf (&rast[4], "%2.2d %2.2d %6.4f", h, m, s);
      else         /* display in degrees */
	sprintf (&rast[4], "%3.2d %2.2d %6.3f", h, m, s);
     }
  else
     sprintf (&rast[4], "%13.6lg", ra);
} /* End of ra2hms */

void dec2dms(double dec, char* decch, char* decst)
/* convert dec in degrees to Dec dd mm ss.sss (13 char) */
/* decch is FITS label for Declination axis             */
/* gives simple display if axislabel not what expected  */
{double temp;
 int d, m, itemp, i, recog;
 float s;
 char sign[2]=" ", minus[2]="-";
 char rectypes[8][5]={"RA  ", "GLON", "ELON", "LL  ",
   "DEC ", "GLAT", "ELAT", "MM  "};
 temp = dec; if (temp<0.0) temp = -temp; itemp = (int)temp; d = itemp;
 temp = temp - (double)itemp;
 temp = temp*60.0; itemp = (int)temp; m = itemp;
 temp = temp - (double)itemp;
 temp = temp*60.0; s = (float)temp;
 if (dec<0.0) sign[0]=minus[0];   /* sign */
/* make label (strip minus signs) */
  for (i=0;i<4;i++) decst[i] = ' '; decst[4] = 0;
  for (i=0;i<4;i++) {if (decch[i]==minus[0]) break; decst[i]=decch[i];}
  recog = 0;  /* look for recognized types */
  for (i=0;i<8;i++) recog = recog || (!strcmp (decst, rectypes[i]));
  if (recog)
     sprintf (&decst[4], "%1s%2.2d %2.2d %6.3f", sign, d, m, s);
  else
     sprintf (&decst[4], "%13.6lg", dec);
} /* End of dec2dms */

#include <math.h>
#include <string.h>
#include <stdio.h>

int GetWPos(int nnaxes, float pix[3], double ref[3], float refpix[3], 
   float inc[3], float rot[3], char type[3][9], double pos[3])
/* Determines World coordinates for a position of the first three axes */
/* takes pixel location (pix), FITS header info (ref, refpix, inc, rot */
/* type and returns coordinates as pos                                 */
/* returns 0 if successful otherwise:                                  */
/* 1 = angle too large for projection;                                 */
{  int i, j, naxes, iRet, Lonaxis=-1, Lataxis=-1, Othaxis[3]={1,1,1};
   char Lontypes[4][5]={"RA  ", "GLON", "ELON", "LL  "};
   int  Loncheck[4] = {2, 4, 4, 2};
   char Lattypes[4][5]={"DEC ", "GLAT", "ELAT", "MM  "};
   int  Latcheck[4] = {3, 4, 4, 2};
   naxes = nnaxes;
   if (naxes>3) naxes = 3;
/* find any Long axis */
   for (i=0;i<naxes;i++) {
      for (j=0;j<4;j++) 
	  if (!strncmp (type[i], Lontypes[j], Loncheck[j])) Lonaxis = i;}
/* find any Lat axis */
   for (i=0;i<naxes;i++) {
      for (j=0;j<4;j++) 
	 if (!strncmp (type[i], Lattypes[j], Latcheck[j])) Lataxis = i;}
/* get World coordinates for any position pair */
   if ((Lonaxis>=0) && (Lataxis>=0)) {
      iRet = worldpos(pix[Lonaxis], pix[Lataxis], 
	 ref[Lonaxis], ref[Lataxis], refpix[Lonaxis], refpix[Lataxis], 
	 inc[Lonaxis], inc[Lataxis], rot[Lataxis], 
	 &type[Lonaxis][4], &pos[Lonaxis], &pos[Lataxis]);}
   else { /* no position pair */
      Lonaxis = -1; Lataxis = -1; iRet = 0;}
   if (Lonaxis>=0) Othaxis[Lonaxis] = 0;
   if (Lataxis>=0) Othaxis[Lataxis] = 0;
/* do Other axes */
   for (i=0;i<naxes;i++){
      if (Othaxis[i]) pos[i] = ref[i] + inc[i] * (pix[i] - refpix[i]);
   }
   return iRet;
} /* end of GetWPos */

/* generic position translation */
/* get celestial position from pixel  */
int get_wpos(ImageDescriptor *desc, float pix[3], double pos[3])
{
  Integer   ierr;
  int       posOK, nax, i;
  char      axtype[3][9]; 
  float     crpix[3], crinc[3], crot[3], xpix, ypix;
  double    crval[3];
  double    ra, dec;
  char      ctype[5];

/* is this a DSS, IRAF or a rational file? */
  if (desc->isDSS) { /* DSS */
    xpix = desc->corn_pixel[0] + pix[0] - 0.5;
    ypix = desc->corn_pixel[1] + pix[1] - 0.5;
    dsseq ((double)desc->pixel_size[0], (double)desc->pixel_size[1], 
	    desc->plate_RA, desc->plate_DEC, desc->PPO[2], 
	    desc->PPO[5], desc->AMDX, desc->AMDY, 
	    xpix, ypix, &pos[0], &pos[1], &ierr);
    pos[2] = 0.0;
    posOK = ierr; 
  }
  else if (desc->isIRAF) { /* IRAF */
    xpix = desc->corn_pixel[0] + pix[0] - 0.5;
    ypix = desc->corn_pixel[1] + pix[1] - 0.5;
    strncpy (ctype, desc->axisdesc[0]->axis_name->sp+4, 4);  ctype[4] = 0;
    posOK = CDpos(xpix, ypix, 
		  desc->axisdesc[0]->axis_coord, 
		  desc->axisdesc[1]->axis_coord,
		  desc->axisdesc[0]->axis_ref_pixel, 
		  desc->axisdesc[1]->axis_ref_pixel,
		  desc->axisdesc[0]->axis_increment, 
		  desc->axisdesc[1]->axis_increment,
		  desc->axisdesc[1]->axis_rotation, 
		  desc->cd1, desc->cd2, ctype,
		  &pos[0], &pos[1]);
    pos[2] = 0.0;
  }
  else { /* Old style WCS */
    nax = min (3, (int)desc->num_axes);
     /* collect information in arrays */
     for (i=0; i<nax; i++)
       {
	 strncpy(axtype[i], desc->axisdesc[i]->axis_name->sp, 8);
	 axtype[i][8] = 0; 
	 crval[i] = desc->axisdesc[i]->axis_coord;
	 crpix[i] = desc->axisdesc[i]->axis_ref_pixel;
	 crinc[i] = desc->axisdesc[i]->axis_increment;
	 crot[i] = desc->axisdesc[i]->axis_rotation;
       }
     posOK = GetWPos(nax, pix, crval, crpix, crinc, crot, axtype, pos);
   }
 /* need to precess? */
   if ((desc->usr_equinox > 0.0) && 
   (fabs(desc->usr_equinox-desc->equinox)>1.0) && !posOK)
    {
      ra = pos[0]; dec = pos[1];
      if (fabs(desc->equinox-1950.0)<1.0) BtoJ (&ra, &dec);
      if (fabs(desc->equinox-2000.0)<1.0) JtoB (&ra, &dec);
      pos[0] = ra; pos[1] = dec;
    }

  return posOK;
} /*end get_wpos */

/* determine pixel corresponding to a given celestial position */
int get_xypix(ImageDescriptor *desc, double xpos, double ypos, 
	      float *xpix, float *ypix)
{
  char      ctype[5];
  int       posOK;
  Integer   ierr;
  double    ra, dec;

/* need to precess? */
  if ((desc->usr_equinox > 0.0) && (fabs(desc->usr_equinox-desc->equinox)>1.0))
    {
      ra = xpos; dec = ypos;
      if (fabs(desc->equinox-1950.0)<1.0) JtoB (&ra, &dec);
      if (fabs(desc->equinox-2000.0)<1.0) BtoJ (&ra, &dec);
      xpos = ra; ypos = dec;
    }

/* is this a DSS, IRAF  or a rational file? */
  if (desc->isDSS) { /* DSS */
    dsspix ((double)desc->plate_scale, (double)desc->pixel_size[0], 
	    (double)desc->pixel_size[1], desc->plate_RA, 
	    desc->plate_DEC, desc->PPO[2], desc->PPO[5], desc->AMDX,
	    desc->AMDY, xpos, ypos, xpix, ypix, &ierr);
    *xpix = *xpix - desc->corn_pixel[0] + 1.0;
    *ypix = *ypix - desc->corn_pixel[1] + 1.0;
    posOK = !(ierr==0); /* perverse logic */
  }
  else if (desc->isIRAF) { /* IRAF */
    strncpy (ctype, desc->axisdesc[0]->axis_name->sp+4, 4);  ctype[4] = 0;
    posOK = CDpix(xpos, ypos, 
		  desc->axisdesc[0]->axis_coord, 
		  desc->axisdesc[1]->axis_coord,
		  desc->axisdesc[0]->axis_ref_pixel, 
		  desc->axisdesc[1]->axis_ref_pixel,
		  desc->axisdesc[0]->axis_increment, 
		  desc->axisdesc[1]->axis_increment,
		  desc->axisdesc[1]->axis_rotation, 
		  desc->icd1, desc->icd2, ctype,
		  xpix, ypix);
  }
  else {/* Old style WCS */
    strncpy (ctype, desc->axisdesc[0]->axis_name->sp+4, 4);  ctype[4] = 0;
    posOK = xypix(xpos, ypos, 
		  desc->axisdesc[0]->axis_coord, 
		  desc->axisdesc[1]->axis_coord,
		  desc->axisdesc[0]->axis_ref_pixel, 
		  desc->axisdesc[1]->axis_ref_pixel,
		  desc->axisdesc[0]->axis_increment, 
		  desc->axisdesc[1]->axis_increment,
		  desc->axisdesc[1]->axis_rotation, ctype,
		  xpix, ypix);
  }
  return posOK;
} /*end get_xypix */
