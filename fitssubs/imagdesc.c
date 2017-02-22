/* ImageDescriptor class implementation  */ 
/*  An ImageDescriptor contains information about an image and an array 
    of AxisDescriptors, one for each axis of the array.  */ 
/*-----------------------------------------------------------------------
*  Copyright (C) 1996, 1997,1998,2002
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
#include "imagdesc.h" 
#include "axisdesc.h" 
#include "dsssubs.h" 
  
/* Constructor  */ 
ImageDescriptor* MakeImageDescriptor (Integer naxis, Integer *dims) 
{ 
  Integer i; 
  ImageDescriptor *me;
  me = (ImageDescriptor *) malloc (sizeof(ImageDescriptor)); 
  me->image_name = MakeString("anon    "); 
  me->date_obs = MakeString("01/01/01"); 
  me->units = MakeString("Unknown "); 
  me->num_axes = naxis; 
  me->epoch=0.0; 
  me->equinox=0.0; 
  me->usr_equinox = -1.0; 
  for (i = 0; i < naxis; i++) me->axisdesc[i] = MakeAxisDescriptor(); 
/* DSS stuff */
  me->plate_label = MakeString("Not DSS "); 
  me->isDSS = 0;
  me->plate_RA = 0.0;
  me->plate_DEC = 0.0;
  me->plate_scale = 0.0;
  me->corn_pixel[0] = 0.0;
  me->corn_pixel[1] = 0.0;
  me->pixel_size[0] = 0.0;
  me->pixel_size[1] = 0.0;
  for (i=0; i<6; i++)   me->PPO[i] = 0.0;
  for (i=0; i<20; i++)  me->AMDX[i] = 0.0;
  for (i=0; i<20; i++)  me->AMDY[i] = 0.0;
/* IRAF stuff */
  me->isIRAF = 0;
  me->cd1[0] = 0.0;
  me->cd1[1] = 0.0;
  me->cd2[0] = 0.0;
  me->cd2[1] = 0.0;
  me->icd1[0] = 0.0;
  me->icd1[1] = 0.0;
  me->icd2[0] = 0.0;
  me->icd2[1] = 0.0;
  return me; 
} /* end MakeImageDescriptor */ 
  
/*  Destructor  */ 
void KillImageDescriptor(ImageDescriptor* me) 
{ 
  Integer i; 
  if (!me) return;   /* anybody home? */ 
  KillString(me->image_name); 
  KillString(me->date_obs); 
  KillString(me->units); 
  KillString(me->plate_label); 
  for (i=0; i<me->num_axes; i++) KillAxisDescriptor(me->axisdesc[i]); 
  if (me) free (me);  me = NULL;
} /* end KillImageDescriptor */ 
  
/*  copy  desc2 to desc1*/ 
void  ImageDescriptorCopy(ImageDescriptor *desc1, ImageDescriptor *desc2) 
{ 
  Integer i; 
  if (!desc1) return;
  if (!desc2) return;
  StringCopy(desc1->image_name, desc2->image_name); 
  StringCopy(desc1->date_obs, desc2->date_obs); 
  StringCopy(desc1->units, desc2->units); 
  desc1->num_axes = desc2->num_axes; 
  desc1->epoch=desc2->epoch; 
  desc1->equinox=desc2->equinox; 
  desc1->usr_equinox=desc2->usr_equinox; 
  for (i=0; i<desc1->num_axes; i++) 
    AxisDescriptorCopy(desc1->axisdesc[i], desc2->axisdesc[i]); 
/* DSS stuff */
  StringCopy(desc1->plate_label, desc2->plate_label); 
  desc1->isDSS = desc2->isDSS; 
  desc1->plate_RA = desc2->plate_RA; 
  desc1->plate_DEC = desc2->plate_DEC; 
  desc1->plate_scale = desc2->plate_scale; 
  desc1->corn_pixel[0] = desc2->corn_pixel[0]; 
  desc1->corn_pixel[1] = desc2->corn_pixel[1]; 
  desc1->pixel_size[0] = desc2->pixel_size[0]; 
  desc1->pixel_size[1] = desc2->pixel_size[1]; 
  for (i=0; i<6; i++)   desc1->PPO[i] = desc2->PPO[i]; 
  for (i=0; i<20; i++)  desc1->AMDX[i] = desc2->AMDY[i]; 
  for (i=0; i<20; i++)  desc1->AMDY[i] = desc2->AMDY[i]; 
/* IRAF stuff */
  desc1->isIRAF = desc2->isIRAF; 
  desc1->cd1[0] = desc2->cd1[0]; 
  desc1->cd1[1] = desc2->cd1[1]; 
  desc1->cd2[0] = desc2->cd2[0]; 
  desc1->cd2[1] = desc2->cd2[1]; 
  desc1->icd1[0] = desc2->icd1[0]; 
  desc1->icd1[1] = desc2->icd1[1]; 
  desc1->icd2[0] = desc2->icd2[0]; 
  desc1->icd2[1] = desc2->icd2[1]; 
}  /* end ImageDescriptorCopy */ 
  
/* comparison */ 
/* window size must be the same on each axis.  */ 
Logical ImageDescriptorComp (ImageDescriptor *desc1, 
			     ImageDescriptor *desc2) 
{ 
  Integer i; 
  if (!desc1) return 0;
  if (!desc2) return 0;
  if (desc1->num_axes != desc2->num_axes) return 0; 
  if (desc1->epoch!=desc2->epoch) return 0; 
  if (desc1->equinox!=desc2->equinox) return 0; 
  for (i=0; i<desc1->num_axes; i++) 
    if (!(AxisDescriptorComp(desc1->axisdesc[i], desc2->axisdesc[i]))) 
      return 0; 
  if (desc1->isDSS != desc2->isDSS) return 0;   /* both DSS? */
  if (desc1->isIRAF != desc2->isIRAF) return 0;  /* both IRAF? */
 return 1; 
}  /* ImageDescriptorComp  */ 

/* fixup DSS (Digitized Sky Survey (PSS)) headers as best as possible    */
/* DSS parameters should already be added to the ImageDescriptor;        */
/* this routine will fill in the Axis descriptors with plausible values. */
/* RA = 'hh mm ss.ss' of source                                          */
/* Dec = '+dd mm ss.s' of source                                         */
/* x_pixel = image x pixel corresponding to (RA,Dec)                     */
/* y_pixel = image y pixel corresponding to (RA,Dec)                     */
  void ImageDescriptorFixDSS (ImageDescriptor *desc, FStrng* RA, FStrng* Dec,
			      float x_pixel, float y_pixel)
{
  Integer ret;

  if (!desc) return; /* anybody home? */

/* use equinox rather than epoch */
  desc->epoch = desc->equinox;
/* use plate label as object */
  StringCopy(desc->image_name, desc->plate_label);

/* fix up header */
  fixcoo ((double)desc->pixel_size[0], (double)desc->pixel_size[1], 
          desc->plate_RA, desc->plate_DEC, desc->PPO, desc->AMDX, desc->AMDY, 
          desc->corn_pixel,
          desc->axisdesc[0]->axis_length, desc->axisdesc[1]->axis_length,
          (double)desc->plate_scale, 
          desc->axisdesc[0]->axis_name->sp, desc->axisdesc[1]->axis_name->sp,
          &desc->axisdesc[0]->axis_increment, &desc->axisdesc[1]->axis_increment,
          &desc->axisdesc[0]->axis_ref_pixel, &desc->axisdesc[1]->axis_ref_pixel,
          &desc->axisdesc[0]->axis_rotation, &desc->axisdesc[1]->axis_rotation,
          &desc->axisdesc[0]->axis_coord, &desc->axisdesc[1]->axis_coord,
          &ret);
} /* end ImageDescriptorFixDSS */
  
  /* fixup IRAF headers as best as possible */
/* IRAF CD matrix should already be added to the ImageDescriptor;        */
/* this routine will fill in the Axis descriptors with plausible values  */
/* of increment and rotation.                                            */
/* Method is from Hanish and Wells 1988 WCS draft memo (never adopted).  */
  void ImageDescriptorFixIRAF (ImageDescriptor *desc)
{
  float rot1, rot2, det, sdet;

  if (!desc) return; /* anybody home? */
/* coordinate increments */
  desc->axisdesc[0]->axis_increment = sqrt (desc->cd1[0]*desc->cd1[0] +
                                            desc->cd2[0]*desc->cd2[0]);
  desc->axisdesc[1]->axis_increment = sqrt (desc->cd1[1]*desc->cd1[1] +
                                            desc->cd2[1]*desc->cd2[1]);
  /* Work out signs*/
  det = desc->cd1[0]*desc->cd2[1] - desc->cd1[1]*desc->cd2[0];
  if (det>0.0) sdet = 1.0; /* sign function */
  else {
    sdet = -1.0;
    /*   if negative, it must be RA*/
    desc->axisdesc[0]->axis_increment = -desc->axisdesc[0]->axis_increment;
  }
  /*  rotation, average over skew */
  rot1 = 57.296 * atan2 (sdet*desc->cd1[1], desc->cd2[1]);
  rot2 = 57.296 * atan2 (-sdet*desc->cd2[0], desc->cd1[0]);
/* coordinate rotation */
  desc->axisdesc[0]->axis_rotation = 0.0;
  desc->axisdesc[1]->axis_rotation = -0.5 * (rot1+rot2);

/* inverse matrix */
  det = desc->cd1[0]*desc->cd2[1] - desc->cd1[1]*desc->cd2[0];
  if (fabs(det)>1.0e-20) { /* determinate */
    desc->icd1[0] =  desc->cd2[1] / det;
    desc->icd1[1] = -desc->cd1[1] / det;
    desc->icd2[0] = -desc->cd2[0] / det;
    desc->icd2[1] =  desc->cd1[0] / det;
  }
  else { /* indeterminate - use identity matrix */
    desc->icd1[0] = 1.0;
    desc->icd1[1] = 0.0;
    desc->icd2[0] = 0.0;
    desc->icd2[1] = 1.0;
  }
} /* end ImageDescriptorFixIRAF */
  
