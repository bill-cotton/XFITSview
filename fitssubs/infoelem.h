/* header file for InfoElement class  */ 
/* an InfoElement is an element of the linked list InfoList and contains
   an arbitary array of information */
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
#include "myio.h" 
#include "mystring.h" 
#ifndef INFOELEMENT_H
#define INFOELEMENT_H
/* Enumeration for data types  */ 
  enum InfoTyp {IDOUBLE=1, IFLOAT, ISTRING, IINTEGER, ILOGICAL}; 
  
typedef struct INFOELEMENT { 
  void        *inext;    /* Pointer to next InfoElement in list  */ 
                         /* 0 means last element in list  */ 
  FStrng      *iname;    /* element name  */ 
  Integer     itype ;    /* Data type: 1=double, 2=float, 3=string,  4=long */ 
  Integer     indim ;    /* Number of dimensions (max 10)  */ 
  Integer     idim[10];  /* Dimensionality array  */ 
  double      *ddata;    /* Pointer to double data array */ 
  float       *fdata;    /* Pointer to float data array  */ 
  char        *cdata;    /* array of pointers to strings  */ 
  Integer     *idata;    /* Pointer to long data array  */ 
  Logical     *ldata;    /* Pointer to logical data array.  */ 
}  InfoElement; 
  
/* Constructor  */ 
  InfoElement* MakeInfoElement (char *label, Integer type, Integer ndim, 
				Integer *dim, char *data); 
/* Destructor  */ 
  void KillInfoElement(InfoElement *me); 
  
/* Add link to next  */ 
  void InfoAddLink (InfoElement *me, InfoElement *next); 
  
/*  Compare element name with test string. true=match, else no match  */ 
    Logical InfoTestName (InfoElement *me, char *testname); 
  
/*  Update contents of an info element; returns 0 if successful.  */ 
  Integer InfoUpdate (InfoElement *me, Integer type, Integer ndim, 
		      Integer *dim, char *data); 
/* store data return of 0 OK*/ 
  Integer InfoSave (InfoElement *me, char* data); 
  
/*  return data return of 0 OK*/ 
  Integer InfoGet  (InfoElement *me, char* data); 
  
#endif /* INFOELEMENT */ 
