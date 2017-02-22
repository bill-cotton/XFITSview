/*  header file for MatrixPos class  */ 
/* a MatrixPos contains information relating to a pixel position in a Matrix
   and functions for manipulating them. Pixel numbers are 0 relative.*/
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
#include "myio.h" 
#include "matxdesc.h" 
#ifndef MATRIXPOS_H 
#define MATRIXPOS_H 
  
typedef struct MATRIXPOS { 
    Integer ndims;   /* Number of dimensions  */ 
    Integer pos[7];  /* Pixel position on each axis  */ 
  } MatrixPos; 
  
  /* Constructor  */ 
  MatrixPos* MakeMatrixPos(Integer n, Integer *p); 
  
  /* destructor  */ 
  void KillMatrixPos(MatrixPos* me); 
  
  /* copy pos2 to pos1 */ 
  void MatrixPosCopy (MatrixPos *pos1, MatrixPos *pos2); 
  
  /* increment a MatrixPos, returns false when matrix exhausted.  */ 
  /* Honors the window.  */ 
  Logical inc_pixel(MatrixPos *me, MatrixDescriptor *d); 
  Logical inc_patch(MatrixPos *me, MatrixDescriptor *d); 
  
  /* Zero a MatrixPos  */ 
  void ZeroMatrixPos(MatrixPos *me); 
  
#endif /* MATRIXPOS_H */ 
