/* header file for Image class */ 
/*  An Image consists of a pixel Matrix, an Image descriptor and a  linked 
    list (InfoList) of information. */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1998
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
#include "imagdesc.h" 
#include "matx.h" 
#include "ifitsstr.h" 
#ifndef IMAGE_H
#define IMAGE_H
  
  
typedef struct IMAGE { 
  Matrix          *matx;         /* Matrix of pixel values */ 
  ImageDescriptor *descript;     /* Image descriptor */ 
  InfoList        *ilist;        /* Linked list of information */ 
  Integer          error;        /* error condition, 0=> OK */ 
} Image; 
  
/* Constructor */ 
Image* MakeFImage (void);

/* destructor */
void KillFImage(Image *me);
  
/*  Construct the actual array and update descriptors. */ 
void build_image(Image *me, Integer ndim, Integer *dim, 
		 FITSfile *Ffile, Integer bitpix, Integer hbytes, 
		 Integer blank, double scale, double offset); 
  
/*  axis info */ 
void get_axis_info(Image *me, Integer axis_number, FStrng *axis_name, 
		   Integer *length, float *ref_pixel, double *coord, 
		   float *increment, float *rotation); 
  
void set_axis_info(Image *me, Integer axis_number, FStrng *axis_name, 
		   Integer length, float ref_pixel, double coord, 
		   float increment, float rotation); 
  
/*  I/O  from FITS */ 
void LoadFImage(FITSin *s, Image *image); 
  
  
#endif /* IMAGE_H */ 
