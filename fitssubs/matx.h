/*  header file for Matrix class */ 
/* a Matrix is a multidimentional pixel array, in practice this is a memory
   resident sliding window on a disk file. */
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

#include "myio.h" 
#include "fwindow.h" 
#include "matxpos.h" 
#include "matxdesc.h" 
#include "mydefs.h" 
#include "fmem.h" 
#ifndef MATRIX_H
#define MATRIX_H
/*  Matrix class */ 
#if SYS_TYPE==WINDOWS 
#define MAXHIST 128 /* should be the same as MAXCOLOR in ?fitsvie?.h */
#elif SYS_TYPE==XWINDOWS 
#define MAXHIST 128 
#else
#define MAXHIST 64
#endif  
typedef struct MATRIX { 
  FWindow          *fw;     /* FWindow for data */ 
  Logical           temp;   /* If true Matrix is a temporary. */ 
  Logical           fixed;  /* If true fdata, file and values are fixed */ 
  float             blankv; /* Magic blanking value, 0.0 = no blanking */ 
  MatrixDescriptor *desc;   /* Matrix descriptor */ 
  MatrixPos        *last_pixel; /* Last pixel referenced */ 
  /* pixel to color index mapping function, these are amnipulated by
     routines in histo.c, values are unscalled image units */
  Logical           map_valid;  /* if true has valid pixel mapping */
  Integer           map_plane;  /* plane number of mapping */
  Integer           map_type;   /* mapping type */
  float             map_min;    /* min value in mapping */
  float             map_max;    /* max value in mapping */ 
  float             map_mean;   /* histogram mean */ 
  float             map_sigma;  /* histogram sigma */ 
  float             map_val[MAXHIST]; /* mapping function */
} Matrix; 
  
  
/* Constructor */ 
Matrix* MakeMatrix(void); 
  
/* Destructor */ 
void KillMatrix(Matrix *me); 
  
/* Construct the matrix */ 
void build_matrix(Matrix *me, Integer n, Integer *d, Logical t, Logical f, 
		  FITSfile *fname, Integer bitpix, Integer hbytes, 
		  Integer blank, double scale, double offset); 
  
/* Check that a MatrixPos is in me matrix; false => bad, true => ok. */ 
Logical IsValid(Matrix *me, MatrixPos *p); 
  
/* Read a new patch into the FWindow if necessary. */ 
void ReadPatch(Matrix *me, Integer i0, Integer i1, 
	       Integer i2, Integer i3, Integer i4, 
	       Integer i5, Integer i6); 
void ReadPatchPos(Matrix *me, MatrixPos *p); 
  
/* Determine the size in words of the image */ 
Integer MatrixTotal_size(Matrix *me); 
  
/* Determine the size in words of the patch (smaller at end of plane) */ 
Integer MatrixPatch_size(Matrix *me, MatrixPos *p); 
  
/* Determine the number of rows in the current patch */ 
Integer MatrixNrow_window(Matrix *me, MatrixDescriptor *d, MatrixPos *p); 
  
/*  Element access */ 
float MatrixGetPixel(Matrix *me, MatrixPos *p); 
  
/*  Functions to return offset in the current patch of a given pixel. */ 
/*  ReadPatch should heve been called to insure that the pixel is in memory. */ 
Integer MatrixPatch_offset(Matrix *me, Integer i0, Integer i1, 
			   Integer i2, Integer i3, Integer i4, 
			   Integer i5, Integer i6); 
  
Integer MatrixPatch_offsetPos(Matrix *me, MatrixPos *p); 
  
#endif /* MATRIX_H */ 
