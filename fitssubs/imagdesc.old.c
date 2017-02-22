/* ImageDescriptor class implementation  */ 
/*  An ImageDescriptor contains information about an image and an array 
    of AxisDescriptors, one for each axis of the array.  */ 
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
  
#include "imagdesc.h" 
#include "axisdesc.h" 
  
/* Constructor  */ 
ImageDescriptor* MakeImageDescriptor (Integer naxis, Integer *dims) 
{ 
  Integer i; 
  ImageDescriptor *this;
  this = (ImageDescriptor *) malloc (sizeof(ImageDescriptor)); 
  this->image_name = MakeString("anon    "); 
  this->date_obs = MakeString("01/01/01"); 
  this->units = MakeString("Unknown "); 
  this->num_axes = naxis; 
  this->epoch=0.0; 
  this->equinox=0.0; 
  this->usr_equinox = -1.0; 
  for (i = 0; i < naxis; i++) this->axisdesc[i] = MakeAxisDescriptor(); 
/* DSS stuff */
  this->plate_label = MakeString("Not DSS "); 
  this->isDSS = 0;
  this->plate_RA = 0.0;
  this->plate_DEC = 0.0;
  this->plate_scale = 0.0;
  this->corn_pixel[0] = 0.0;
  this->corn_pixel[1] = 0.0;
  this->pixel_size[0] = 0.0;
  this->pixel_size[1] = 0.0;
  for (i=0; i<6; i++)   this->PPO[i] = 0.0;
  for (i=0; i<20; i++)  this->AMDX[i] = 0.0;
  for (i=0; i<20; i++)  this->AMDY[i] = 0.0;
  return this; 
} /* end MakeImageDescriptor */ 
  
/*  Destructor  */ 
void KillImageDescriptor(ImageDescriptor* this) 
{ 
  Integer i; 
  if (!this) return;   /* anybody home? */ 
  KillString(this->image_name); 
  KillString(this->date_obs); 
  KillString(this->units); 
  KillString(this->plate_label); 
  for (i=0; i<this->num_axes; i++) KillAxisDescriptor(this->axisdesc[i]); 
  free (this); 
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
  if (desc1->isDSS != desc2->isDSS) return 0;  /* both DSS? */
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
  Integer hd, m, ret, i;
  float   sec, sign;
  double  ra, dec;
  char    strng[50];

/* initial values */
/*  desc->axisdesc[0]->axis_ref_pixel = x_pixel - desc->corn_pixel[0];*/
  desc->axisdesc[0]->axis_ref_pixel = 
    + desc->PPO[2] / desc->pixel_size[0] - desc->corn_pixel[0];
  desc->axisdesc[0]->axis_coord = desc->plate_RA;
  desc->axisdesc[0]->axis_increment = 
    -(desc->plate_scale * (0.001 * desc->pixel_size[0])) / 3600.0;
  StringFill(desc->axisdesc[0]->axis_name, "RA---ARC");
/*  desc->axisdesc[1]->axis_ref_pixel = y_pixel - desc->corn_pixel[1];*/
  desc->axisdesc[1]->axis_ref_pixel = 
    + desc->PPO[5] / desc->pixel_size[1] - desc->corn_pixel[1];
  desc->axisdesc[1]->axis_coord = desc->plate_DEC;
  desc->axisdesc[1]->axis_increment = 
    (desc->plate_scale * (0.001 * desc->pixel_size[1])) / 3600.0;
  StringFill(desc->axisdesc[1]->axis_name, "DEC--ARC");

/* use equinox rather than epoch */
  desc->epoch = desc->equinox;
/* use plate label as object */
  StringCopy(desc->image_name, desc->plate_label);

/* decode source position */
  /* ra */
  for (i=0; i<RA->length; i++) strng[i]=RA->sp[i]; strng[i] = 0;
  ret = sscanf (strng, "%d %d %f", &hd, &m, &sec);
  if (ret!=3) 
    {sprintf (szErrMess, "Error decoding DSS RA %s", RA->sp);
     ErrorMess(szErrMess); 
     return;}
  ra = 15.0*(hd + (m/60.0) + (sec/3600.0));
/* debug  desc->axisdesc[0]->axis_coord = ra;*/
/* declination */
  for (i=0; i<Dec->length; i++) strng[i]=Dec->sp[i]; strng[i] = 0;
  ret = sscanf (strng, "%d %d %f", &hd, &m, &sec);
  if (ret!=3) 
    {sprintf (szErrMess, "Error decoding DSS DEC %s", RA->sp);
     ErrorMess(szErrMess); 
     return;}
  if ((hd<0) || (Dec->sp[0]=='-')) /* check for sign */
    sign = -1.0;
  else
    sign = 1.0;
  if (hd<0) hd = - hd;
  dec = sign * (hd + (m/60.0) + (sec/3600.0));
/* debug  desc->axisdesc[1]->axis_coord = dec;*/
} /* end ImageDescriptorFixDSS */
  
  
  
