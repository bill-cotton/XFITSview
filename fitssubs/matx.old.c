/* Matrix class implementation */ 
/* a Matrix is a multidimentional pixel array, in practice this is a memory
   resident sliding window on a disk file. */
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
#include <math.h> 
#include "matx.h" 
#include "fwindow.h" 
  
float magicvalue; /* magic blanking value */
  
/* Constructor */ 
Matrix* MakeMatrix() 
{ 
  Matrix *this = (Matrix *) malloc (sizeof(Matrix)); 
  this->fw = MakeFWindow (); 
  this->temp = 1; 
  this->fixed = 0; 
  this->blankv = 0.0; 
  this->desc = NULL; 
  this->last_pixel = NULL; 
  magicvalue = MagicBlank(); /* set global value */
  return this; 
} /* end MakeMatrix */ 
  
/* Destructor */ 
void KillMatrix(Matrix *this) 
{ 
  if (!this) return; /* anybody home? */ 
  if (this->fw) FWindowDec_ref(this->fw); 
  if (this->desc) KillMatrixDescriptor(this->desc); 
  if (this->last_pixel) KillMatrixPos(this->last_pixel); 
  free(this); 
} /* end KillMatrix */ 
  
/*  Construct the matrix */ 
void build_matrix(Matrix *this, Integer n, Integer *d, Logical t, Logical f, 
		  FITSfile *fname, Integer bitpix, Integer hbytes, 
		  Integer blank, double scale, double offset) 
{ 
  Integer tsize, psize; 
  if (!this) return; /* validity check */
  this->temp = t; 
  this->fixed = f; 
  this->blankv = 0.0; 
  if (this->desc) KillMatrixDescriptor(this->desc); 
  this->desc = MakeMatrixDescriptor(n, d); 
  if (this->last_pixel) KillMatrixPos(this->last_pixel); 
  this->last_pixel = MakeMatrixPos(n, d); 
  ZeroMatrixPos(this->last_pixel); 
  tsize = MatrixTotal_size(this); 
  psize = MatrixPatch_size(this, this->last_pixel); 
  build_window(this->fw, tsize, psize, fname, bitpix, hbytes, blank, 
	       scale, offset); 
} /* end build_matrix */ 
  
/* Check that a MatrixPos is in this matrix 0 => bad, 1 => ok. */ 
Logical IsValid(Matrix *this, MatrixPos *p) 
{ 
  Integer i; 
  if (!this) return 0; /* validity check */
  for (i=0; i<p->ndims; i++) 
    if ((p->pos[i]<0) || (p->pos[i]>=this->desc->dims[i])) return 0; 
  return 1; 
} /* end IsValid */ 
  
/* Read a new patch into the FWindow if necessary. */ 
void ReadPatch(Matrix *this, Integer i0, Integer i1, 
	       Integer i2, Integer i3, Integer i4, 
	       Integer i5, Integer i6) 
{
 if (!this) return; /* validity check */
 this->last_pixel->pos[0] = i0; this->last_pixel->pos[1] = i1; 
 this->last_pixel->pos[2] = i2; this->last_pixel->pos[3] = i3; 
 this->last_pixel->pos[4] = i4; this->last_pixel->pos[5] = i5; 
 this->last_pixel->pos[6] = i6; 
 ReadPatchPos(this, this->last_pixel); 
}  /*  End of ReadPatch */ 
  
/* Read a new patch into the Fmem if necessary. */ 
void ReadPatchPos(Matrix *this, MatrixPos *p) 
{Integer npat, offset; 
 if (!this) return; /* validity check */
 if (!p) return;
 npat = MatrixDescriptorGet_offset(this->desc, p->ndims, p->pos) / 
   (this->desc->dims[0] * this->desc->num_row); 
 offset = npat * this->desc->dims[0] * this->desc->num_row; 
 if ((npat!=this->fw->frame) || (offset<this->fw->first) || 
     (offset>this->fw->last)) FWindowFetch(this->fw, offset); 
}  /* End of ReadPatchPos */ 
  
/*  Total size */ 
Integer MatrixTotal_size(Matrix *this) 
{Integer i, size; 
 if (!this) return 0; /* validity check */
 size = this->desc->dims[0]; 
 for (i=1; i<this->desc->num_dim; i++) 
   if (this->desc->dims[i]>0) size *= this->desc->dims[i]; 
 return size; 
} /* End MatrixTotal_size */ 
  
/* Determine the size in words of a patch */ 
Integer MatrixPatch_size(Matrix *this, MatrixPos *p) 
{Integer size;
   if (!this) return 0; /* validity check */
   if (!p) return 0; /* validity check */
   size = this->desc->dims[0]; 
   if ((p->pos[1]+this->desc->num_row) >= this->desc->window_hi[1]) 
     {size = size*this->desc->num_row;} 
   else 
     {size = size * this->desc->num_row;} 
 return size; 
} /* end MatrixPatch_size */ 
  
/* Determine the number of rows in the current window patch */ 
Integer MatrixNrow_window(Matrix *this, MatrixDescriptor *d, MatrixPos *p) 
{Integer nrow, nleft, *win_hi, norow, *pos; 
 if (!this) return 0; /* validity check */
 if (!d) return 0; /* validity check */
 if (!p) return 0; /* validity check */
 win_hi = d->window_hi; 
 norow = d->num_row; 
 pos = p->pos; 
 nleft = win_hi[1] - pos[1]+ 1; 
 if (norow >= nleft) 
   {nrow = nleft;} 
 else 
   {nrow = norow;} 
 return nrow; 
}   /*  End MatrixNrow_window */ 
  
/*  Element access */ 
float MatrixGetPixel(Matrix *this, MatrixPos *p) 
{ 
  fMemPtr pf, values; 
  float   retval; 
  if (!this) return magicvalue; /* validity check */
  if (!p) return magicvalue; /* validity check */
  values =  (fMemPtr) LockFMem(this->fw->fdata); /* lock memory */ 
  if (!IsValid(this, p))  /* Valid position? */ 
    {sprintf (szErrMess, "MatrixGetPixel:Illegal address %ld %ld %ld", 
	      p->pos[0], p->pos[1], p->pos[2]); 
     ErrorMess(szErrMess); 
     pf = values;} 
  else 
    {ReadPatchPos(this, p);  /*  Move FWindow if necessary */ 
     pf = (values + get_patch_offset(this->desc, p->ndims, p->pos, 
				     this->fw->frame));} 
  retval = (*pf);
  if (retval!=magicvalue) /* apply scaling */
       retval = retval * this->fw->file->scale + this->fw->file->offset; 
  UnlockFMem(this->fw->fdata);  /* unlock memory */ 
  return retval; 
}  /*  End MatrixGetPixel */ 
  
/*  Functions to return offset in the current patch of a given pixel. */ 
/*  SetPatch should heve been called to insure that the pixel is in memory. */ 
    Integer MatrixPatch_offset(Matrix *this, Integer i0, Integer i1, 
			       Integer i2, Integer i3, Integer i4, 
			       Integer i5, Integer i6) 
	{
     if (!this) return 0; /* validity check */
	 this->last_pixel->pos[0] = i0; this->last_pixel->pos[1] = i1; 
	 this->last_pixel->pos[2] = i2; this->last_pixel->pos[3] = i3; 
	 this->last_pixel->pos[4] = i4; this->last_pixel->pos[5] = i5; 
	 this->last_pixel->pos[6] = i6; 
	 return MatrixPatch_offsetPos(this, this->last_pixel); 
}  /*  End of MatrixPatch_offset */ 
  
Integer MatrixPatch_offsetPos(Matrix *this, MatrixPos *p) 
{ 
   if (!this) return 0; /* validity check */
   if (!p) return 0; /* validity check */
   return get_patch_offset(this->desc, p->ndims, p->pos, this->fw->frame); 
}  /*  End of MatrixPatch_offsetPos */ 
  
/*  End of Matrix class */ 
