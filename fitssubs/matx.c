/* Matrix class implementation */ 
/* a Matrix is a multidimentional pixel array, in practice this is a memory
   resident sliding window on a disk file. */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1998,2002
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
  Matrix *me = (Matrix *) malloc (sizeof(Matrix)); 
  me->fw = MakeFWindow (); 
  me->temp = 1; 
  me->fixed = 0; 
  me->blankv = 0.0; 
  me->desc = NULL; 
  me->last_pixel = NULL; 
  me->map_valid = 0;
  me->map_type = -1;
  me->map_min = MagicBlank();
  me->map_max = MagicBlank();
  me->map_mean = MagicBlank();
  me->map_sigma = MagicBlank();

  magicvalue = MagicBlank(); /* set global value */
  return me; 
} /* end MakeMatrix */ 
  
/* Destructor */ 
void KillMatrix(Matrix *me) 
{ 
  if (!me) return; /* anybody home? */ 
  if (me->fw) FWindowDec_ref(me->fw); 
  if (me->desc) KillMatrixDescriptor(me->desc); 
  if (me->last_pixel) KillMatrixPos(me->last_pixel); 
  if (me) free(me); me = NULL;
} /* end KillMatrix */ 
  
/*  Construct the matrix */ 
/* pixel mapping done in histo.c */
void build_matrix(Matrix *me, Integer n, Integer *d, Logical t, Logical f, 
		  FITSfile *fname, Integer bitpix, Integer hbytes, 
		  Integer blank, double scale, double offset) 
{ 
  Integer tsize, psize; 
  if (!me) return; /* validity check */
  me->temp = t; 
  me->fixed = f; 
  me->blankv = 0.0; 
  if (me->desc) KillMatrixDescriptor(me->desc); 
  me->desc = MakeMatrixDescriptor(n, d); 
  if (me->last_pixel) KillMatrixPos(me->last_pixel); 
  me->last_pixel = MakeMatrixPos(n, d); 
  ZeroMatrixPos(me->last_pixel); 
  tsize = MatrixTotal_size(me); 
  psize = MatrixPatch_size(me, me->last_pixel); 
  build_window(me->fw, tsize, psize, fname, bitpix, hbytes, blank, 
	       scale, offset); 
} /* end build_matrix */ 
  
/* Check that a MatrixPos is in me matrix 0 => bad, 1 => ok. */ 
Logical IsValid(Matrix *me, MatrixPos *p) 
{ 
  Integer i; 
  if (!me) return 0; /* validity check */
  if (!me->fw) return 0; /* validity check */
  /*  if (!me->fw->valid) return 0;  validity check */
  if (!me->desc) return 0; /* validity check */
  if (!me->desc->dims) return 0; /* validity check */
  if (!p) return 0; /* validity check */
  if (!p->pos) return 0; /* validity check */
  for (i=0; i<p->ndims; i++) 
    if ((p->pos[i]<0) || (p->pos[i]>=me->desc->dims[i])) return 0; 
  return 1; 
} /* end IsValid */ 
  
/* Read a new patch into the FWindow if necessary. */ 
void ReadPatch(Matrix *me, Integer i0, Integer i1, 
	       Integer i2, Integer i3, Integer i4, 
	       Integer i5, Integer i6) 
{
 if (!me) return; /* validity check */
 if (!me->last_pixel) return; /* validity check */
 me->last_pixel->pos[0] = i0; me->last_pixel->pos[1] = i1; 
 me->last_pixel->pos[2] = i2; me->last_pixel->pos[3] = i3; 
 me->last_pixel->pos[4] = i4; me->last_pixel->pos[5] = i5; 
 me->last_pixel->pos[6] = i6; 
 ReadPatchPos(me, me->last_pixel); 
}  /*  End of ReadPatch */ 
  
/* Read a new patch into the Fmem if necessary. */ 
void ReadPatchPos(Matrix *me, MatrixPos *p) 
{Integer npat, offset; 
 if (!me) return; /* validity check */
 if (!me->desc) return; /* validity check */
 if (!me->fw) return; /* validity check */
 /* if (!me->fw->valid) return;  validity check */
 if (!p) return;
 npat = MatrixDescriptorGet_offset(me->desc, p->ndims, p->pos) / 
   (me->desc->dims[0] * me->desc->num_row); 
 offset = npat * me->desc->dims[0] * me->desc->num_row; 
 if ((npat!=me->fw->frame) || (offset<me->fw->first) || 
     (offset>me->fw->last)) FWindowFetch(me->fw, offset); 
}  /* End of ReadPatchPos */ 
  
/*  Total size */ 
Integer MatrixTotal_size(Matrix *me) 
{Integer i, size; 
 if (!me) return 0; /* validity check */
 if (!me->desc) return 0; /* validity check */
 size = me->desc->dims[0]; 
 for (i=1; i<me->desc->num_dim; i++) 
   if (me->desc->dims[i]>0) size *= me->desc->dims[i]; 
 return size; 
} /* End MatrixTotal_size */ 
  
/* Determine the size in words of a patch */ 
Integer MatrixPatch_size(Matrix *me, MatrixPos *p) 
{Integer size;
   if (!me) return 0; /* validity check */
   if (!me->desc) return 0; /* validity check */
   if (!p) return 0; /* validity check */
   size = me->desc->dims[0]; 
   if ((p->pos[1]+me->desc->num_row) >= me->desc->window_hi[1]) 
     {size = size*me->desc->num_row;} 
   else 
     {size = size * me->desc->num_row;} 
 return size; 
} /* end MatrixPatch_size */ 
  
/* Determine the number of rows in the current window patch */ 
Integer MatrixNrow_window(Matrix *me, MatrixDescriptor *d, MatrixPos *p) 
{Integer nrow, nleft, *win_hi, norow, *pos; 
 if (!me) return 0; /* validity check */
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
float MatrixGetPixel(Matrix *me, MatrixPos *p) 
{ 
  fMemPtr pf, values; 
  float   retval; 
  if (!me) return magicvalue; /* validity check */
  if (!me->fw) return magicvalue; /* validity check */
  /*  if (!me->fw->valid) return magicvalue;  validity check */
  if (!p) return magicvalue; /* validity check */
  values =  (fMemPtr) LockFMem(me->fw->fdata); /* lock memory */ 
  if (!IsValid(me, p))  /* Valid position? */ 
    {sprintf (szErrMess, "MatrixGetPixel:Illegal address %ld %ld %ld", 
	      p->pos[0], p->pos[1], p->pos[2]); 
     ErrorMess(szErrMess); 
     pf = values;} 
  else 
    {ReadPatchPos(me, p);  /*  Move FWindow if necessary */ 
     pf = (values + get_patch_offset(me->desc, p->ndims, p->pos, 
				     me->fw->frame));} 
  retval = (*pf);
  if (retval!=magicvalue) /* apply scaling */
       retval = retval * me->fw->file->scale + me->fw->file->offset; 
  UnlockFMem(me->fw->fdata);  /* unlock memory */ 
  return retval; 
}  /*  End MatrixGetPixel */ 
  
/*  Functions to return offset in the current patch of a given pixel. */ 
/*  SetPatch should heve been called to insure that the pixel is in memory. */ 
    Integer MatrixPatch_offset(Matrix *me, Integer i0, Integer i1, 
			       Integer i2, Integer i3, Integer i4, 
			       Integer i5, Integer i6) 
	{
     if (!me) return 0; /* validity check */
     if (!me->last_pixel) return 0; /* validity check */
     me->last_pixel->pos[0] = i0; me->last_pixel->pos[1] = i1; 
     me->last_pixel->pos[2] = i2; me->last_pixel->pos[3] = i3; 
     me->last_pixel->pos[4] = i4; me->last_pixel->pos[5] = i5; 
     me->last_pixel->pos[6] = i6; 
     return MatrixPatch_offsetPos(me, me->last_pixel); 
}  /*  End of MatrixPatch_offset */ 
  
Integer MatrixPatch_offsetPos(Matrix *me, MatrixPos *p) 
{ 
   if (!me) return 0; /* validity check */
   if (!me->fw) return 0; /* validity check */
   /*   if (!me->fw->valid) return 0;  validity check */
   if (!p) return 0; /* validity check */
   return get_patch_offset(me->desc, p->ndims, p->pos, me->fw->frame); 
}  /*  End of MatrixPatch_offsetPos */ 
  
/*  End of Matrix class */ 
