/* Macros for data conversion from FITS types to local.  These are somewhat 
   more efficient that the comparable routines in zsubs.  Values are expected
   in and left in local variables.  All assume basic IEEE types with 
   different byte orders. */
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

#include "mydefs.h" 
#ifndef ZCONVRT_H
#define ZCONVRT_H

/*  Union definitions for byte shuffle  */ 
 union Iequiv { 
   Integer full; 
   char parts[4]; 
 } IFits, ILocal; 

 union sequiv { 
   short full; 
   char parts[2]; 
 } SFits; 

 union lequiv { 
   long full; 
   char parts[4]; 
 } LFits; 

 union fequiv { 
   float full; 
   long lfull;
   char parts[4]; 
 } FFits, FLocal; 

 union dequiv { 
   double full; 
   char parts[8]; 
 } DFits, DLocal; 
  
/*     Rational computers, no conversion  */ 
#if DATA_TYPE==IEEEBIG 
  
/* 16 bit to Integer  */
/* input in SFits.full, output in ILocal.full */ 
#define ZI16IL {ILocal.full = SFits.full;}
  
/* 32 bit to Integer */ 
/* input in LFits.full, output in ILocal.full */ 
#define ZI32IL {ILocal.full = LFits.full;}
  
/* 32 bit IEEE to float  */ 
/* input in FFits.lfull, output in FLocal.full */ 
/*  NOTE: accepts input as long to avoid arithmetic problems  */ 
#define ZR32RL {FLocal.full=FFits.full;}
  
/* 64 bit IEEE to double  */ 
/* input in DFits.full, output in DLocal.full */ 
#define ZD64DL  {DLocal.full=DFits.full;}
  
#elif DATA_TYPE==IEEELIT 
/* Bizzarre , PC-like machines, reorder bytes  */ 

/* 16 bit to Integer  */
/* input in SFits.full, output in ILocal.full */ 
#define ZI16IL { \
   ILocal.parts[0] = SFits.parts[1]; \
   ILocal.parts[1] = SFits.parts[0]; \
 if (SFits.parts[0]<0) \
   {ILocal.parts[2] = -1; \
    ILocal.parts[3] = -1;} \
 else \
   {ILocal.parts[2] = 0; \
    ILocal.parts[3] = 0;} }
  
/* 32 bit to Integer */ 
/* input in LFits.full, output in ILocal.full */ 
#define ZI32IL  { \
 ILocal.parts[0] = LFits.parts[3]; \
 ILocal.parts[1] = LFits.parts[2]; \
 ILocal.parts[2] = LFits.parts[1]; \
 ILocal.parts[3] = LFits.parts[0]; }
  
/* 32 bit IEEE to float  */ 
/* input in FFits.lfull, output in FLocal.full */ 
/*  NOTE: accepts input as long to avoid arithmetic problems  */ 
#define ZR32RL  { \
 FLocal.parts[0] = FFits.parts[3]; \
 FLocal.parts[1] = FFits.parts[2]; \
 FLocal.parts[2] = FFits.parts[1]; \
 FLocal.parts[3] = FFits.parts[0]; }
  
  
/* 64 bit IEEE to double  */ 
/* input in DFits.full, output in DLocal.full */ 
#define ZD64DL { \
 DLocal.parts[0] = DFits.parts[7]; \
 DLocal.parts[1] = DFits.parts[6]; \
 DLocal.parts[2] = DFits.parts[5]; \
 DLocal.parts[3] = DFits.parts[4]; \
 DLocal.parts[4] = DFits.parts[3]; \
 DLocal.parts[5] = DFits.parts[2]; \
 DLocal.parts[6] = DFits.parts[1]; \
 DLocal.parts[7] = DFits.parts[0]; }
  
#endif 
#endif /* ZCONVRT */ 
  
  
  
  
