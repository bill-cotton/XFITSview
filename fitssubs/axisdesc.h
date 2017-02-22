/*  header file for AxisDescriptor class.  */
/*  An axis descriptor  and labeling information for an  axis of of an 
    Image.  AxisDescriptors are elements of an  ImageDescriptor.  */ 
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
  
#include "myio.h" 
#include "mydefs.h" 
#include "mystring.h" 
#include <stdlib.h> 
#ifndef AXISDESCRIPTOR_H 
#define AXISDESCRIPTOR_H 
  
  typedef struct AXISDESCRIPTOR { 
    Integer axis_length;      /* Length in pixels of the axis  */ 
    float   axis_ref_pixel;   /* Reference pixel for coordinates  */ 
    double  axis_coord;       /* Coordinate value at reference pixel  */ 
    float   axis_increment;   /* Coordinate increment  */ 
    float   axis_rotation;    /* FITS-like Axis Rotation  */ 
    FStrng  *axis_name;       /* axis name  */ 
    } AxisDescriptor; 
  
/*  constructor  */ 
  AxisDescriptor*  MakeAxisDescriptor(void); 
  
/*  destructor  */ 
void KillAxisDescriptor(AxisDescriptor* ad); 
  
/* compare */ 
Logical AxisDescriptorComp (AxisDescriptor *desc1, AxisDescriptor *desc2); 
  
/* copy desc2 to desc1 */ 
void AxisDescriptorCopy (AxisDescriptor *desc1, AxisDescriptor *desc2); 
  
#endif /* AXISDESCRIPTOR_H */ 
