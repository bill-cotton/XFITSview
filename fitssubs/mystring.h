/*  header file for FDtrng string class  */ 
/* an FStrng is an implementation of character strings with associated
   functions */
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
#include <string.h> 
#ifndef MYSTRING_H 
#define MYSTRING_H 
  
typedef struct FSTRNG { 
    Integer length;    /*  Length of string  */ 
    char *sp;          /*  String pointer    */ 
  } FStrng; 
  
  
/* Constructors  */ 
  FStrng*  MakeString(char * str) ; 
  FStrng*  MakeStringSize(Integer length) ; 
  
/*  Destructor  */ 
void  KillString(FStrng* str); 
  
/* fill with char array */ 
void StringFill(FStrng *out, char * in); 
  
/* Substring  */ 
/* Return substring given by 0-rel indices first and last  */ 
FStrng* substring(FStrng* this, Integer first, Integer last); 
  
/* Replace characters first to last with string s.  */ 
void repstring(FStrng* this, Integer first, Integer last, const FStrng *s); 
  
/* Search for character in a string; return index if found; -1 if not.  */ 
Integer StringFind_char(FStrng* this, char ch); 
  
/*  String copy  */ 
void strcopy(FStrng *out, FStrng *in); 
  
/* Copy String in to String out  */ 
void StringCopy(FStrng* out, FStrng *in); 
  
/* Comparison  */ 
Logical StringComp (FStrng *s1, FStrng *s2); 
  
/* Concatenate  */ 
FStrng* StringConcat(FStrng *s1, FStrng *s2); 
  
  
#endif /* STRING */ 
