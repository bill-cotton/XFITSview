/*  implementation of Fmem class */ 
/*  An Fmem is a block of memory of type float.  An Fmem can be used for 
    data types composed of a number of floats such as Complex by using a 
    size > 1 for the element size. */ 
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
  
#include <stdio.h>
#include "fmem.h" 
#include "fitsmem.h" 
  
  /* Constructor */ 
  Fmem* MakeFmem(Integer c, Integer s) 
{ 
  Fmem *me = (Fmem *) malloc (sizeof(Fmem)); 
  me->count = c; 
  me->size = s; 
  me->nref = 1;
  me->hValues = AllocMem(c*s*sizeof(float)); 
  if (!me->hValues) return NULL; /* allocation failed */
  me->values = NULL; 
  return me;} /* end MakeFmem */ 
  
  /* Destructor */ 
  void KillFmem(Fmem *me) 
{ 
  if (!me) return;  /* anybody home? */ 
  if (me->values) UnlockMem(me->hValues); 
  if (me->hValues) DeallocMem(me->hValues); 
  if (me) free(me); me = NULL;
}  /* end KillFmem */ 
  
/* Increment reference count */ 
void FmemInc_ref(Fmem *me) 
{if (me) me->nref++;} 
  
/* Decrement reference count, delete if it goes to 0 */ 
void FmemDec_ref(Fmem *me) 
{  if (!me) return;
   if (!--(me->nref))     /* Delete when reference count goes to 0 */ 
   KillFmem (me);} 
  
/* Lock memory returning pointer */ 
MemPtr LockFMem(Fmem *me) 
{ 
  if (!me) return NULL; 
  if (!me->hValues) return NULL; 
  me->values = (fMemPtr)LockMem(me->hValues);
  return (MemPtr)me->values;
} /* end LockFMem */ 
  
/* Unlock memory */ 
void UnlockFMem(Fmem *me) 
{ 
  if (!me) return; 
  if (!me->hValues) return; 
  if (!me->values) return; 
  UnlockMem(me->hValues); 
  me->values = NULL; 
  return; 
} /* end UnlockFMem */ 
  
