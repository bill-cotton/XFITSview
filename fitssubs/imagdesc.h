/*  header file for ImageDescriptor class  */ 
/*  An ImageDescriptor contains information about an image and an array 
    of AxisDescriptors, one for each axis of the array.  */ 
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
  
#include "mydefs.h" 
#include "myio.h" 
#include "axisdesc.h" 
#include "matxdesc.h" 
#include "mystring.h" 
#ifndef IMAGEDESCRIPTOR_H
#define IMAGEDESCRIPTOR_H
  
typedef struct IMAGEDESCRIPTOR { 
    Integer num_axes;            /*  Number of axes,  max 7  */ 
    float   epoch;               /*  Epoch of coordinates  */ 
    float   equinox;             /*  Equinox of coordinates  */ 
    float   usr_equinox;         /*  user requested equinox (-1=>use image)*/
    AxisDescriptor *axisdesc[7]; /*  axis descriptors  */ 
    FStrng  *image_name;         /*  Image name */ 
    FStrng  *date_obs;           /*  Observing date  */ 
    FStrng  *units;              /*  Image units  */ 
/* DSS position stuff */
    Logical isDSS;               /*  true iff a DSS image */
    double  PPO[6];              /*  Orientation coefs. */
    double  AMDX[20];            /*  Plate soln. x coef. */
    double  AMDY[20];            /*  Plate soln. y coef. */
    double  plate_RA;            /*  Pointing RA of PSS plate */
    double  plate_DEC;           /*  Pointing RA of PSS plate */
    float   plate_scale;         /*  plate scale asec/mm */
    float   corn_pixel[2];       /*  corner pixel of subimage */
    float   pixel_size[2];       /*  pixel size in microns (x,y) */
    FStrng  *plate_label;        /*  name of PSS plate */
/* IRAF position stuff */
    Logical isIRAF;              /* true iff an iraf image */
    float   cd1[2], cd2[2];      /* CD matrix */
    float   icd1[2], icd2[2];    /* inverse CD matrix */
  } ImageDescriptor; 
  
  /*  Constructor  */ 
  ImageDescriptor* MakeImageDescriptor(Integer naxis, Integer *dims); 
  
  /*  Destructor  */ 
  void KillImageDescriptor(ImageDescriptor* desc); 
  
  /*  copy  desc2 to desc1*/ 
  void  ImageDescriptorCopy(ImageDescriptor *desc1, ImageDescriptor *desc2); 
  
  /* comparison */ 
  Logical ImageDescriptorComp (ImageDescriptor *desc1, 
			       ImageDescriptor *desc2); 

  /* fixup DSS headers as best as possible */
/* DSS parameters should already be added to the ImageDescriptor;        */
/* this routine will fill in the Axis descriptors with plausible values. */
/* RA = 'hh mm ss.ss' of source                                          */
/* Dec = '+dd mm ss.s' of source                                         */
/* x_pixel = image x pixel corresponding to (RA,Dec)                     */
/* y_pixel = image y pixel corresponding to (RA,Dec)                     */
  void ImageDescriptorFixDSS (ImageDescriptor *desc, FStrng* RA,
			      FStrng* Dec, float x_pixel, float y_pixel);
  /* fixup IRAF headers as best as possible */
/* IRAF CD matrix should already be added to the ImageDescriptor;        */
/* this routine will fill in the Axis descriptors with plausible values  */
/* of increment and rotation                                             */
  void ImageDescriptorFixIRAF (ImageDescriptor *desc);
#endif /* IMAGEDESCRIPTOR_H */ 
  
  
