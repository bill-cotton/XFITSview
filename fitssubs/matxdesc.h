/*  header file for MatrixDescriptor class  */ 
/* a MatrixDescriptor contains structural (dimension) information about
   a Matrix.  */
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
#include "zsubs.h" 
#include <stdio.h> 
#ifndef MATRIXDESCRIPTOR_H
#define MATRIXDESCRIPTOR_H
  
typedef struct MATRIXDESC { 
    Integer  num_dim;     /* Number of dimensions  */ 
    Integer  dims[7];     /* dimension array.  */ 
    Integer  factor[7];   /* Factors to use in computing addresses  */ 
    Integer  window_lo[7];/* Pixel numbers of low side of window  */ 
    Integer  window_hi[7];/* Pixel numbers of high side of window  */ 
    Integer  num_row;     /* Number of rows in the FWindow memory  */ 
} MatrixDescriptor ; 
  
/*  Constructors  */ 
MatrixDescriptor* MakeMatrixDescriptor(Integer ndim, Integer *dim); 
  
/*  Destructor  */ 
void    KillMatrixDescriptor(MatrixDescriptor* desc); 
  
/* Copy  d2 to d1*/ 
void MatrixDescriptorCopy (MatrixDescriptor *d1, MatrixDescriptor *d2); 
  
/* Return true if the window is smaller than the matrix.  */ 
Logical IsAWindow(MatrixDescriptor* me); 
  
/*  Set window  */ 
void MatrixDescriptorSet_window_lo(MatrixDescriptor *me, Integer *lo); 
void MatrixDescriptorSet_window_hi(MatrixDescriptor *me, Integer *hi); 
  
/*  Pixel position,  Pixel numbers are 0 relative  */ 
Integer MatrixDescriptorGet_offset (MatrixDescriptor *me, Integer n, 
				    Integer *pix_pos); 
  
/*  Pixel position in patch,  Pixel numbers are 0 relative  */ 
   Integer get_patch_offset (MatrixDescriptor *me, Integer n, 
			     Integer *pix_pos, Integer patch_no) ; 
  
/*  Comparison,  returns 1 for true, 0 for false  */ 
    Logical MatrixDescriptorComp(MatrixDescriptor *desc1, 
				MatrixDescriptor *desc2); 
  
#endif /* MATRIXDESCRIPTOR_H */ 
