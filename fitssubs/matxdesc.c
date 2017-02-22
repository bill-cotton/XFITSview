/* MatrixDescriptor class implementation  */ 
/* a MatrixDescriptor contains structural (dimension) information about
   a Matrix. */
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
#include "myio.h" 
#include "matxdesc.h" 
#include "mydefs.h" 
#include "zsubs.h" 
#include <stdio.h> 
  
/*  Constructor  */ 
MatrixDescriptor* MakeMatrixDescriptor(Integer ndim, Integer *dim) 
{ 
  Integer i; 
  MatrixDescriptor* me; 
  me = (MatrixDescriptor*) malloc(sizeof(MatrixDescriptor)); 
  me->num_dim = ndim; 
  if (dim[0]>0) 
    {me->num_row=MAX_WINDOW/dim[0]; 
     if (me->num_row>dim[1]) me->num_row=dim[1];} 
 else 
   {me->num_row=1;} 
 for (i=0; i<7; i++) 
     {me->dims[i] = 0; 
      me->factor[i] = 0; 
      me->window_lo[i]=0; 
      me->window_hi[i]=0;} 
 for (i=0; i<ndim; i++) 
     {me->dims[i] = dim[i]; 
      me->window_hi[i] = dim[i]-1;} 
 /*  Address factors  */ 
 me->factor[0] = me->dims[0]; 
 for (i = 1; i<ndim; i++) 
   me->factor[i] = me->factor[i-1] * me->dims[i]; 
 return me; 
} /* end MakeMatrixDescriptor */ 
  
/*  Destructor  */ 
void    KillMatrixDescriptor(MatrixDescriptor* desc) 
{ 
  if (desc) free(desc); desc = NULL;
} /* end KillMatrixDescriptor */ 
  
/* Copy  d2 to d1*/ 
void MatrixDescriptorCopy (MatrixDescriptor *d1, MatrixDescriptor *d2) 
{ 
  Integer i; 
  if (!d1) return;
  if (!d2) return;
  d1->num_dim = d2->num_dim; d1->num_row = d2->num_row; 
  for (i=0; i<7; i++) 
    {d1->dims[i] = 0; d1->factor[i] = 0; 
     d1->window_lo[i]=0; d1->window_hi[i]=0;} 
  for (i=0; i<d1->num_dim; i++) 
    {d1->dims[i] = d2->dims[i];             /* Dimensions  */ 
     d1->window_lo[i] = d2->window_lo[i];   /* window  */ 
     d1->window_hi[i] = d2->window_hi[i]; 
     d1->factor[i] = d2->factor[i];} 
 } /* end MatrixDescriptorCopy */ 
  
/* Return true if the window is smaller than the matrix.  */ 
Logical IsAWindow(MatrixDescriptor* me) 
{ 
  Integer i; 
  if (!me) return 0;
  for (i=0; i<me->num_dim; i++) 
     if ((me->window_lo[i]!=0) || 
	 (me->window_hi[i]!=(me->dims[i]-1))) return 1; 
 return 0;}  /* end IsAWindow  */ 
  
  
/*  Set window  */ 
void MatrixDescriptorSet_window_lo(MatrixDescriptor *me, Integer *lo) 
{ 
  Integer i; 
  if (!me) return;
  if (!lo) return;
  for (i=0; i<me->num_dim; i++) 
     {if (lo[i]<=me->window_hi[i]) 
	 {me->window_lo[i] = lo[i];} 
     else 
         {sprintf (szErrMess, 
		"MatrixDescriptorSet_window_lo: Illegal window lo %d %d %d", 
             lo[0], lo[1], lo[2]); 
	  ErrorMess(szErrMess);} 
}}  /* end of set_window_lo  */ 
  
void MatrixDescriptorSet_window_hi(MatrixDescriptor *me, Integer *hi) 
{ 
  Integer i; 
  if (!me) return;
  if (!hi) return;
  for (i=0; i<me->num_dim; i++) 
     {if ((hi[i]>=me->window_lo[i]) && (hi[i]<me->dims[i])) 
	  {me->window_hi[i] = hi[i];} 
     else 
         {sprintf (szErrMess, 
		 "MatrixDescriptorSet_window_hi: Illegal window hi %d %d %d", 
             hi[0], hi[1], hi[2]); 
	  ErrorMess(szErrMess);} 
}}  /* end of set_window_hi  */ 
  
/*  Pixel position,  Pixel numbers are 0 relative  */ 
Integer MatrixDescriptorGet_offset (MatrixDescriptor *me, Integer n, 
				    Integer *pix_pos) 
{ 
  Integer point; 
  if (!me) return 0;
  if (!pix_pos) return 0;
  point = pix_pos[0]; 
  if (n > 1) 
    {point += pix_pos[1] * me->factor[0]; 
     if (n > 2) 
       {point += pix_pos[2] * me->factor[1]; 
	if (n > 3) 
	  {point += pix_pos[3] * me->factor[2]; 
	   if (n > 4) 
	     {point += pix_pos[4] * me->factor[3]; 
	      if (n > 5) 
		{point += pix_pos[5] * me->factor[4]; 
		 if (n > 6) point += pix_pos[6] * 
		   me->factor[5];}}}}} 
  return point; 
} /* end MatrixDescriptorGet_offset */ 
  
/*  Pixel position in patch,  Pixel numbers are 0 relative  */ 
Integer get_patch_offset (MatrixDescriptor *me, Integer n, 
			  Integer *pix_pos, Integer patch_no) 
{ 
  if (!me) return 0;
  if (!pix_pos) return 0;
  return MatrixDescriptorGet_offset(me, n, pix_pos) - 
    (patch_no * me->dims[0] * me->num_row); 
} /*end get_patch_offset */ 
  
/*  Comparison; window size must be the same on each axis.  */ 
Logical MatrixDescriptorComp(MatrixDescriptor *desc1, 
			     MatrixDescriptor *desc2) 
{ 
  Integer i; 
  if (!desc1) return 0;
  if (!desc2) return 0;
  if (desc1->num_dim != desc2->num_dim) return 0; 
  for (i = 0; i < desc1->num_dim; i++) 
    if ((desc1->window_hi[i]-desc1->window_lo[i]) != 
	(desc2->window_hi[i]-desc2->window_lo[i])) return 0; 
  return 1; 
}  /*  MatrixDescriptorComp  */ 
  
  
  
