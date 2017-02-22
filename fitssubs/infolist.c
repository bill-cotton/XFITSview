/* InfoList Class implementation  */ 
/*  An InfoList is a linked list of arbitrary pieces of information. 
    Elements of the list are objects of type InfoElement.
    Data array pointers must be cast to type char */ 
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
#include "infolist.h" 
  
/* Constructor  */ 
InfoList* MakeInfoList () 
{ 
  InfoList *temp = (InfoList *) malloc (sizeof(InfoList)); 
  temp->first = NULL; temp->last = NULL; 
  return temp; 
} /* end MakeInfoList */ 
  
/* Destructor  */ 
void KillInfoList(InfoList *me) 
{ 
  InfoElement *p, *pnext; 
  if (!me) return; /* anybody home? */ 
  p = me->first; 
  if ((me->first == NULL) || (me->last == NULL)) {free(me); return; }
 /*  Delete from start of list  */ 
 while (p) 
     {pnext = p->inext; KillInfoElement(p); p = pnext;} 
 if (me) free(me); me = NULL;
}/* end of KillInfoList  */ 
  
/* clear list  */ 
void ClearInfoList(InfoList *me) 
{ 
 InfoElement *p, *pnext; 
 if (!me) return; /* validity check */
 p = me->first; 
 if ((me->first == 0) || (me->last == 0)) return; 
 /*  Delete from start of list  */ 
 while (p) 
     {pnext = p->inext; KillInfoElement(p); p = pnext;} 
 me->first = NULL; me->last = NULL; 
} /* end of clear  */ 
  
/* Delete a member of an InfoList.  */ 
void InfoZap(InfoList *me, char *label) 
{InfoElement *el, *pe, *ie; 
 if (!me) return; /* validity check */
 if (!label) return; /* validity check */
 el = InfoFind(me, label); 
 if (el==0) return;  /*  Not found  */ 
 if (el==me->first)      /*  Delete first element of Info List?  */ 
     me->first = el->inext; 
 else 
     {pe = me->first; ie = me->first; 
      if (el==me->last)             /*  Last element?  */ 
	  {while (ie != el)           /*  Find previous element  */ 
	       {pe = ie; ie = ie->inext;} 
	   me->last = pe;           /*  Reset last element pointer  */ 
	   InfoAddLink(pe, NULL);}    /*  last element gets 0 link  */ 
      else                            /*  Element not at either end  */ 
	  {while (ie != el)           /*  Find previous element  */ 
	       {pe = ie; ie = ie->inext;} 
	   ie = ie->inext; 
	   InfoAddLink(pe, ie);}       /*  reset previous link  */ 
  } 
 KillInfoElement(el);                  /*  Delete element  */ 
} /* end InfoZap */ 
  
/* Checks of an element of a given label is in the list,  */ 
/* returns pointer to InfoElement; 0=> not found.  */ 
InfoElement* InfoFind(InfoList *me, char *label) 
{ 
  InfoElement *current; 
 if (!me) return NULL; /* validity check */
 if (!label) return NULL; /* validity check */
  if (me->first)       /* Don't search empty list  */ 
    {current = me->first; 
     while (current) 
       {if (InfoTestName(current, label)) return current; 
	current = current->inext;}} 
  return 0;   /* Didn;t find  */ 
} /* end InfoFind */ 
  
/* Finds type of element, returns 0 if element found  */ 
Integer  InfoFindType(InfoList *me, char *label, Integer *type, 
			       Integer *ndim, Integer *dim) 
{InfoElement *el; 
 Integer i, rc = 0; 
 if (!me) return -1; /* validity check */
 if (!label) return -1; /* validity check */
 if (!type) return -1; /* validity check */
 if (!ndim) return -1; /* validity check */
 if (!dim) return -1; /* validity check */
 el = InfoFind(me, label); 
 if (el)                                 /*  Entry found?  */ 
     {*type = el->itype; 
      *ndim = el->indim; 
      for (i=0; i<*ndim; i++) dim[i] = el->idim[i];} 
 else 
     rc = -1; 
 return rc; 
} /* end InfoFindType */ 
  
/* Store data for an element. Returns 0 if successful.  */ 
Integer InfoStore(InfoList *me, char *label, Integer type, Integer ndim, 
		  Integer *dim,  char *data) 
{InfoElement *el, *next; 
 Integer rc = 0; 
 if (!me) return -1; /* validity check */
 if (!label) return -1; /* validity check */
 if (!dim) return -1; /* validity check */
 if (!data) return -1; /* validity check */
 el = InfoFind(me, label); 
 if (el) 
     rc = InfoUpdate(el, type, ndim, dim, data);  /*  existing entry. */ 
 else   /*  Create new InfoElement and link in.  */ 
     {next = MakeInfoElement(label, type, ndim, dim, data); 
      /*   See if there are already any entries:  */ 
      if (me->first == 0) {me->first = next; me->last = next;} 
      else 
	  InfoAddLink(me->last, next); /*Add link to previous last element*/ 
      me->last = next;}                   /* Save new last element  */ 
 return rc; 
} /* end InfoStore */ 
  
/* Find element and return its contents, returns 0 if successful.  */ 
Integer InfoLookup  (InfoList *me, char *label, Integer *type, Integer *ndim, 
		     Integer *dim, char *data) 
{InfoElement *el; 
 Integer i, rc = 0;
 if (!me) return -1; /* validity check */
 if (!label) return -1; /* validity check */
 if (!type) return -1; /* validity check */
 if (!dim) return -1; /* validity check */
 if (!data) return -1; /* validity check */
 el  = InfoFind(me, label); 
 if (el)  /*  Does it exist?  */ 
     {*type = el->itype; 
      *ndim = el->indim; 
      for (i=0;i<*ndim;i++) dim[i] = el->idim[i]; 
      rc = InfoGet(el, data);} /*  Copy data  */ 
 else   /*  Doesn't exist  */ 
     {rc = 1; ndim = 0;} 
 return rc;}  /*  End of InfoLookup  */ 
  
