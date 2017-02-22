/*  Implementation for class FStrng string  */ 
/* an FStrng is an implementation of character strings with associated
   functions */
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
#include "mystring.h" 
#include "mydefs.h" 
#include "zsubs.h" 
#include <stdlib.h> 
#include <stdio.h> 
  
/* Constructors  */ 
FStrng*  MakeString(char * str) 
{ 
  FStrng *me; 
  if (!str) return NULL; 
  me = (FStrng *) malloc(sizeof(FStrng)); 
  me->length = strlen(str); 
  me->sp = (char *) malloc((size_t)me->length+1);
  strcpy (me->sp, str); 
  return me; 
} /* end MakeString */ 
  
FStrng*  MakeStringSize(Integer length) 
{ 
  Integer i; 
  FStrng *me; 
  me = (FStrng *) malloc(sizeof(FStrng)); 
  me->length = length; 
  me->sp = (char *) malloc((size_t)me->length+1);
  for (i=0; i<length; i++) me->sp[i] = ' '; me->sp[i] = 0; 
  return me; 
} /* end MakeStringSize */ 
  
/*  Destructor  */ 
void  KillString(FStrng* str) 
{ 
  if (!str) return;  /* nobody home */ 
  if (str->sp) free(str->sp); str->sp=NULL;/* delete actual string */ 
  if (str) free(str); str = NULL;
}  /* end KillString */ 
  
/* fill with char array */ 
void StringFill(FStrng *out, char * in) 
{Logical remake; 
 Integer i, length; 
 if (!out) return; 
 if (!in) return; 
 length = strlen(in); 
 remake = (out->sp == 0); 
 remake = remake || (out->length!=length); 
 if (remake)   /* If strings aren't same size remake output  */ 
   {if (out->sp!=0) {free(out->sp); out->sp=NULL;} 
    out->sp = (char*) malloc(length+1); 
    out->length = length;} 
 for (i=0; i<length; i++) out->sp[i] = in[i]; out->sp[i] = 0; 
} /* End StringFill */ 
  
  
  
/* Substrings  */ 
/* Return substring given by 0-rel indices first and last  */ 
FStrng* substring(FStrng* me, Integer first, Integer last) 
{ 
  Integer j, i; 
  char *tout; 
  FStrng *out; 
  if (!me) return NULL;
  if (((first<0) || (last<0))  || ((first>me->length) 
				  || (last>me->length))) 
   {sprintf (szErrMess, "substring: Error specifying substring %ld %ld %ld ", 
	     first, last, me->length); 
    ErrorMess(szErrMess); 
    return NULL;} 
  tout = (char*)malloc(last-first+1); 
  j=0;
  for (i=first; i<=last; i++) tout[j++] = me->sp[i]; 
  out = MakeString(tout); 
  if (tout) free(tout); tout = NULL;
  return out;}  /*  End of substring  */ 
  
/* Replace characters first to last with string s.  */ 
   void repstring(FStrng* me, Integer first, Integer last, const FStrng *s) 
{ 
  Integer i, j; 
  if (!me) return;
  if (!s) return;
  if ((first<0) || (last<0) || (first>me->length) || (last>me->length) 
      || ((last-first+1)<s->length)) 
    {sprintf (szErrMess, "repstring: Error specifying substring, %ld %ld %ld ", 
	      first, last, s->length); 
     ErrorMess(szErrMess); 
     return;} 
  j=s->length-1; 
  for (i=first; i<last; i++) me->sp[i] = ' ';  /* blank fill  */ 
  i=last; 
  while (j>=0) me->sp[i--] = s->sp[j--]; /* right justify copy  */ 
} /* End of repstring  */ 
  
/* Search for character in a string; return index if found; -1 if not.  */ 
Integer StringFind_char(FStrng* me, char ch) 
{ 
  Integer i; 
  if (!me) return -1;
  for (i=0; i<me->length; i++) if (me->sp[i]==ch) return i; 
  return -1;}  /*End of StringFind_char  */ 
  
/* Copy String in to String out  */ 
void StringCopy(FStrng* out, FStrng *in) 
{Logical remake; 
 Integer i; 
 if (!out) return;
 if (!in) return;
 remake = (out==0) || (out->sp == 0); 
 remake = remake || (out->length!=in->length); 
 if (remake)   /* If strings aren't same size remake output  */ 
   {if (out->sp!=0) free(out->sp); out->sp=NULL;
    out->sp = (char*) malloc(in->length+1); 
    out->length = in->length;} 
 for (i=0; i<in->length; i++) out->sp[i] = in->sp[i]; out->sp[i] = 0; 
}  /* End of StringCopy  */ 
  
/* Comparison  */ 
Logical StringComp (FStrng *s1, FStrng *s2) 
{Logical test; 
 Integer i; 
 if (!s1) return 0;
 if (!s2) return 0;
 test = (s1->length == s2->length); 
 if (!test) return 0; 
 for (i=0; i<s1->length; i++) test = (test && (s1->sp[i]==s2->sp[i])); 
 return test; 
}  /*  End StringComp  */ 
  
/*  Concatenate  */ 
FStrng* StringConcat (FStrng *s1, FStrng *s2) 
{ 
  Integer i, j; 
  char *tout; 
  FStrng *out; 
 if (!s1) return NULL;
 if (!s2) return NULL;
  tout = (char*)malloc(s1->length+s2->length+1); 
  j = 0; 
  for (i=0; i<s1->length; i++) tout[j++] = s1->sp[i]; 
  for (i=0; i<s2->length; i++) tout[j++] = s2->sp[i]; 
  tout[j] = 0; 
  out = MakeString(tout); 
  if (tout) free(tout); tout = NULL;
  return out; 
}  /*  End StringConcat  */ 
  
  
  
