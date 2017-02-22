/*  ImageFITShead class header file.  */ 
/*  An ImageFITShead object contains the information necessary to parse a 
    FITS header card and store the information in an InfoList */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996, 1997,1998
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
#include "infolist.h" 
#include "mystring.h" 
#ifndef IMAGEFITSHEAD_H 
#define IMAGEFITSHEAD_H 
enum FITShtype {NOSAVE=0, FDOUBLE, FFLOAT, FSTRING, FINTEGER, FLOGICAL}; 
  
typedef struct IFITSHED { 
  Integer    valid;            /*  True if header already read and valid.  */ 
  Integer    next_key;         /*  Next keyword number to write (0-rel)  */ 
  Integer    next_el;          /*  Next element of array to write (0-rel)  */ 
  Integer    number;           /*  the number of recognized keywords  */ 
  Integer keytype[50];         /*  Keyword type  */ 
  FStrng  *keywords[50];       /*  keywords  */ 
  Integer keydim[50];          /* array dimension if > 0 */
  float   farray[20];          /*  Arrays for InfoList  */ 
  double  darray[20]; 
  Integer iarray[20]; 
  Logical larray[20]; 
  FStrng  *sarray[20]; 
  Integer naxes;            /*  number of axes  */ 
} FITShead; 
  
  
/*  Constructor  */ 
  FITShead* MakeFITShead(void); 
  
/* Destructor  */ 
  void  KillFITShead(FITShead *me); 
  
/*  parse card and update info list, returns 0 for success,  */ 
/*  1 when reached "END" card else falied.  */ 
/*  Unrecognized keywords are ignored.  */ 
  Integer eat_card(FITShead *me, FStrng *card, InfoList *ilist); 
  
/*  Validity and other checks at start of eat_card;  */ 
/*  Returns 1 if successful, 0 on error.  */ 
/*  Sets value of array_dim for array  */ 
  Integer valid_check(FITShead *me, FStrng *card, Integer *array_dim); 
  
/*  Any special processing needed after card is parsed can be done here:  */ 
   void check_card(FITShead *me, FStrng *keyword, Integer array_number, 
      FStrng *card, double dsave, float fsave, char *csave, Integer isave, 
       Logical lsave); 
  
  
/* Process unknown keywords  */ 
void unknown_keywords(FStrng *card); 
  
/*  Has the header been read successfully?  */ 
/*  Returns 1 if a valid header read else 0  */ 
Integer got_head(FITShead *me); 
  
#endif /* IMAGEFITSHEAD_H */ 
  
