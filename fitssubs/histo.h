/*  header file for histogram equalization functions for Matrix Class */ 
/*  Histogram equalization is an attempt to have equal numbers of pixels
    in each of the allowed color index states */
/*-----------------------------------------------------------------------
*  Copyright (C) 1998
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
#include "matx.h" 
#ifndef HISTO_H
#define HISTO_H 
/* get plane max and min pixel values
   pixels = matrix for which to find extrema
   plane = plane number
   max (returned) = maximum pixel value.
   min (returned) = minimum pixel value
   Returns 0 if worked else failed */
int get_extrema (Matrix *pixels, Integer plane, float *max, float *min);

/* get plausible range of pixel values
   pixels = matrix for which to find extrema
   plane = plane number
   max (returned) = maximum pixel value.
   min (returned) = minimum pixel value
   Returns 0 if worked else failed */
int get_range (Matrix *pixels, Integer plane, int iNonLinear, float *max,
  float *min);

/* compute histogram equalization mapping function, this must be called
   before map_pixel;  after the image is opened (LoadFImage) and before
   the first read (Readpatch).
   pixels = matrix for which to make equalize histogram
   plane = plane number
   max = maximum pixel value (clip above)
   min = minimum pixel value (clip below)
   Returns 0 if worked else failed */
int equalize (Matrix *pixels, Integer plane, float *max, float *min, 
  int newRange);

/* Return color index for specified pixel value. Returns 0 (blanked) if 
   mapping function invalid.
   pixels = matrix for which to make equalize histogram,
   value = pixel value,
   returns color index (0 used only for blanked values */
int map_pixel(Matrix *pixels, float value);
  
#endif /* HISTO_H */ 
