/*  zsub header  */ 
/*  This module contains machine specific routines such as data type  
    conversion.  Note: many of these can be inlined using the macroes in 
    zconvrt.h  */ 
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
#ifndef ZSUBS_H 
#define ZSUBS_H 
  
/* Error message */ 
void ErrorMess(char* message); 
  
/*   Translation of FITS data types  */ 
/* 16 bit to Integer  */ 
Integer zi16il (short in); 
  
/* 32 bit to Integer  */ 
Integer zi32il (Integer in); 
  
/* 32 bit IEEE to float  */ 
/*  NOTE: accepts input as long to avoid arithmetic problems  */ 
float zr32rl (Integer in); 
  
/* 64 bit IEEE to double  */ 
double zd64dl (double out); 
  
/* Integer to 16 bit  */ 
short zili16l (Integer in); 
  
/* Integer to 32 bit  */ 
Integer zili32 (Integer in); 
  
/* float to 32 bit IEEE  */ 
/* NOTE: returns long to avoid arithmetic problems  */ 
Integer zrlr32 (float out); 
  
/* double to 64 bit IEEE  */ 
double zdld64 (double out); 

#endif /* ZSUBS_H */ 
  
