/* Implementation of myutil utility library.  */ 
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
#include "myconfig.h" 
#include "myutil.h" 
  
/*  Convert a string to an integer.  Stops at first non number after first  */ 
/*  number found.  */ 
Integer String_to_Integer(FStrng *s) 
{ 
  Integer temp; 
  if (!s) return 0;
  sscanf (s->sp,"%ld", &temp); 
  return temp; 
}  /*  End of String_to_Integer  */ 
  
/*  Convert an integer to a string.  */ 
void Integer_to_String (FStrng* s, Integer i) 
{ 
  if (!s) return;
  sprintf(s->sp,"%ld", i); 
}  /*  End of Integer_to_String  */ 
  
/*  Convert date string ('dd/mm/yy') to Julian date.  */ 
double date_to_jd(FStrng *date) 
/*   Algorithm from ACM Algorithm number 199  */ 
/*  NOTE:  This routine is good from 1 Mar 1900 to 31 Dec 1999  */ 
{Integer day, month, year, ik1, ik2; 
 double k, d, m, y, jd; 
 char tdate[9]; 
 if (!date) return 0.0;
/*                                       Get date from string  */ 
 strcpy (tdate, date->sp); tdate[2]=0; tdate[5]=0; 
 sscanf (tdate,"%ld %ld %ld", &day, &month, &year); 
 y = year; m = month; d = day; 
 if (month<=2) 
     {m = month + 9; 
      y = year - 1;} 
 else 
     {m = month - 3;} 
 ik1 = (Integer)((1461.0 * y * 0.25)); 
 ik2 = (Integer)(((153.0 * m + 2.0) * 0.2)); 
 k = (Integer)(ik1 + ik2 + d); 
 ik1 = (Integer)(k); 
 jd = ik1 + 2415078.50;  /* This is good for 20th cent. only  */ 
 return jd;}  /*  End date_to_jd  */ 
  
/*  Convert Julian date to date string ('dd/mm/yy').  */ 
void jd_to_date(FStrng* s, double jd) 
/* Apdapted from ASM Algorithm no. 199  */ 
{Integer id, im, iy; 
 double  j, y, d, m; 
 if (!s) return;
 j = (Integer)(jd + 0.5 - 1721119.); 
 y = (Integer)((4.*j - 1.0) / 146097.); 
 j = 4.*j - 1.0 - 146097.*y; 
 d = (Integer)(j * 0.25); 
 j = (Integer)((4.0*d + 3.0) / 1461.0); 
 d = 4.0*d + 3.0 - 1461.0*j; 
 d = (Integer)((d+4.0) * 0.25); 
 m = (Integer)((5.0*d - 3.0) / 153.0); 
 id = (Integer)(5.0*d - 3.0 - 153.0*m); 
 id = (id + 5) / 5; 
 iy = (Integer) (j); 
 im = (Integer) (m); 
 if (im<10) 
     {im = im + 3;} 
 else 
     {im = im - 9; 
      iy = iy + 1;} 
 /*  Encode value  */ 
 sprintf (s->sp,"%2ld/%2ld/%2ld", id, im, iy); 
}  /*  End of jd_to_date  */ 
  
/* return the magic blanking value */ 
float MagicBlank() 
{ 
  float blank = -1.23456e+15; 
  return blank; 
} /* end MagicBlank */ 
  
/*  Return a NaN  */ 
float fnan() 
{float TNaN, NaN; 
 Integer *INaN = (Integer*)&TNaN; /*  Make a word with all bits on  */ 
 *INaN = ~0; 
 NaN = TNaN; 
 return NaN;}  /*  End fnan  */ 
  
/*  Determine if a value is a NaN (Not a Number).  */ 
Logical IsfNaN(float f) 
{Integer INaN, *test = (Integer*)&f; /*  copy to an integer  */ 
#if DATA_TYPE==IEEEBIG 
   INaN = 0x7f800000;  /* in the normal order */ 
#elif DATA_TYPE==IEEELIT 
/* Bizzarre , Vax-like machines  */ 
   INaN = 0x0000807f;  /* in screwball order */ 
#endif 
 return ((INaN & (*test))==INaN);}  /*  End IsNaN  */ 
  
/*  Test if a NaN.  */ 
Logical IsdNaN(double d) 
{Integer iNaNh, iNaNl; 
 union dequiv { 
   double full; 
   Integer parts[2]; 
 } test; 
#if DATA_TYPE==IEEEBIG 
   iNaNh = 0x7ff00000; iNaNl=0x00000000; /* in the normal order */ 
#elif DATA_TYPE==IEEELIT 
/* Bizzarre , Vax-like machines  */ 
   iNaNh = 0x0000f07f; iNaNl=0x00000000;  /* in screwball order */ 
#endif 
 test.full = d; 
 return (((iNaNh & test.parts[0])==iNaNh) && 
          ((iNaNl & test.parts[1])==iNaNl));}  /*  End IsNaN  */ 
  
  
