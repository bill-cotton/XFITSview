/*  header file for Fmem class */ 
/*  An Fmem is a block of memory of type float.  An Fmem can be used for 
    data types composed of a number of floats such as Complex by using a 
    size > 1 for the element size. */ 
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
  
#include <stdlib.h> 
#include "mydefs.h" 
#include "fitsmem.h" 
#ifndef FMEM_H 
#define FMEM_H 
  
typedef struct FMEM { 
    Integer count;       /*  element count of vector */ 
    Integer size;        /*  Size of an element in floats (for Complex etc.) */ 
    Integer nref;        /*  Number of references, object deleted when this */ 
                         /*  goes to 0; */ 
    int  hValues;        /*  memory handle */ 
    fMemPtr values;      /*  memory pointer (may be null) */ 
  } Fmem; 
  
  /* Constructor */ 
  Fmem* MakeFmem(Integer c, Integer s); 
  
  /* Destructor */ 
  void KillFmem(Fmem *me) ; 
  
  /* Increment reference count */ 
    void FmemInc_ref(Fmem *me); 
  
  /* Decrement reference count, delete if it goes to 0 */ 
    void FmemDec_ref(Fmem *me); 
  
  /* Lock memory returning pointer */ 
    MemPtr LockFMem(Fmem *me); 
  
  /* Unlock memory */ 
    void UnlockFMem(Fmem *me); 
  
#endif /* FMEM_H */ 
