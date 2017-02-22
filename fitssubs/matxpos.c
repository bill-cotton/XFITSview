/*  MatrixPos class implementation  */ 
/* a MatrixPos contains information relating to a pixel position in a Matrix
   and functions for manipulating them. Pixel numbers are 0 relative.*/
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
  
#include "matxpos.h" 

/* MatrixPos class implementation.  */ 
  
/* Constructor  */
MatrixPos* MakeMatrixPos(Integer n, Integer *p) 
{ 
  Integer i; 
  MatrixPos *temp = (MatrixPos *) malloc (sizeof(MatrixPos)); 
  temp->ndims = n; 
  for (i=0; i<7; i++) temp->pos[i] = 0; 
  for (i=0; i<n; i++) temp->pos[i] = p[i]; 
  return temp;} /* end MakeMatrixPos */ 
  
/* destructor  */ 
void KillMatrixPos(MatrixPos* me)
{ 
  if (me) free (me); me = NULL;
}  /* end KillMatrixPos */ 
  
  
/* copy pos2 to pos1 */ 
void MatrixPosCopy (MatrixPos *pos1, MatrixPos *pos2) 
{ 
  Integer i; 
  if (!pos1) return;
  if (!pos2) return;
  pos1->ndims = pos2->ndims; 
  for (i=0; i<7; i++) pos1->pos[i]=pos2->pos[i]; 
} /* end MatrixPosCopy */ 
  
/* increment a MatrixPos, returns false when done  */ 
/* Honors the window.  */ 
Logical inc_pixel(MatrixPos *me, MatrixDescriptor *d)
{Integer i=0; 
 if (!me) return 0;
 if (!d) return 0;
 me->pos[i]++;
 while (me->pos[i]>=d->window_hi[i])
     {me->pos[i] = d->window_lo[i]; i++;
      me->pos[i]++;
      if (i>=me->ndims-1) break;}
/* Done?  */ 
 if ((me->pos[i])>=d->dims[i])
     return 0;   /* Yes  */ 
 else 
     return 1;   /*  No  */ 
}  /*  End inc_pixel  */ 
  
Logical inc_patch(MatrixPos *me, MatrixDescriptor *d)
{Integer i=1; 
 if (!me) return 0;
 if (!d) return 0;
 me->pos[i] += d->num_row;
 while (me->pos[i]>=d->window_hi[i])
     {me->pos[i] = d->window_lo[i]; i++;
      me->pos[i]++;
      if (i>=me->ndims-1) break;}
/* Done?  */ 
 if ((me->pos[i])>=d->dims[i])
     {me->pos[me->ndims-1] =
	d->dims[me->ndims-1] + 100; /* Force invalid pixel */
      return 0;}   /* Yes  */ 
 else 
     return 1;   /*  No  */ 
}  /*  End increment patch  */ 
  
/* Zero a MatrixPos  */ 
void ZeroMatrixPos(MatrixPos *me)
{ 
  Integer i; 
 if (!me) return;
  for (i=0; i<7; i++) me->pos[i] = 0; 
}  /* end ZeroMatrixPos  */ 
  
/*  End of MatrixPos class  */
  
  
