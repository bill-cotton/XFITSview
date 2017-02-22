/*  myutil library of miscelleneous utility routines.  */ 
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
  
#include <stdio.h> 
#include "mydefs.h" 
#include "mystring.h" 
#ifndef MYUTIL_H 
#define MYUTIL_H 
  
/*  Convert a string to an integer.  Stops at first non number after first  */ 
/*  number found.  */ 
extern Integer String_to_Integer(FStrng *s); 
  
/*  Convert an integer to a string.  */ 
extern void Integer_to_String(FStrng *s, Integer i); 
  
/*  Convert date string ('dd/mm/yy') to Julian date.  */ 
extern double date_to_jd(FStrng *date); 
  
/*  Convert Julian date to date string ('dd/mm/yy').  */ 
extern void jd_to_date(FStrng *s, double jd); 
  
/* return the magic blanking value */ 
float MagicBlank(void); 
  
/*  Return a NaN  */ 
extern float fnan(void); 
  
/*  Determine if a value is a NaN (Not a Number).  */ 
/*  argument in raw IEEE form (and byte order).  */ 
Logical IsfNaN(float f); 
  
/*  Double precision (in raw IEEE form)  */ 
Logical IsdNaN(double d); 
  
#endif /* MYUTIL_H */ 
  
  
  
