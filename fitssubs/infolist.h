/* header file for InfoList class  */ 
/*  An InfoList is a linked list of arbitrary pieces of information. 
    Elements of the list are objects of type InfoElement.
    Data array pointers must be cast to type char */ 
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
#include "infoelem.h" 
#ifndef INFOLIST_H
#define INFOLIST_H
  
typedef struct INFOLIST { 
  InfoElement  *first;   /* Pointer to first element in list.  */ 
  InfoElement  *last;    /* Pointer to last element in list.  */ 
} InfoList; 
  
/* Constructor  */ 
InfoList* MakeInfoList (void); 
  
/* Destructor  */ 
void KillInfoList(InfoList *me); 
  
/* Clear contents  */ 
void ClearInfoList(InfoList *me); 
  
  
/* Delete a member of an InfoList.  */ 
void InfoZap(InfoList *me, char *label); 
  
/* Checks if an element of a given label is in the list,  */ 
/* returns pointer to InfoElement; 0=> not found.  */ 
InfoElement* InfoFind(InfoList *me, char *label); 
  
/* Finds type of element, returns 0 if element found  */ 
Integer InfoFindType(InfoList *me, char *label, Integer *type, 
		     Integer *ndim, Integer *dim); 
  
/* Store data for an element. Returns 0 if successful.  */ 
Integer InfoStore(InfoList *me, char *label, Integer type, Integer ndim, 
		  Integer *dim, char *data) ; 
  
/* Find element and return its contents, returns 0 if successful.  */ 
Integer InfoLookup  (InfoList *me, char *label, Integer *type, Integer *ndim, 
		     Integer *dim, char *data); 
  
#endif /* INFOLIST */ 
