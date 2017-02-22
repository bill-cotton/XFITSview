/* InfoElement class implementation  */ 
/* an InfoElement is an element of the linked list InfoList and contains
   an arbitary array of information */
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
#include "infoelem.h" 
  
/* Constructor  */ 
  InfoElement* MakeInfoElement (char *label, Integer type, Integer ndim, 
				Integer *dim, char *data) 
{ 
  InfoElement *me; 
  Integer i, j; 
  if (!label) return NULL; /* validity check */
  if (!dim) return NULL; /* validity check */
  if (!data) return NULL; /* validity check */
  me = (InfoElement *) malloc (sizeof(InfoElement));
  me->inext = NULL; 
  me->iname = MakeString(label); 
  me->itype = type; 
  me->indim = ndim; 
  for (j=0;j<10;j++) me->idim[j]=0;             /* Init dim  */ 
  for (i=0; i<ndim; i++) me->idim[i] = dim[i]; /* Copy dim.  */ 
  me->ddata=NULL; me->fdata=NULL; me->cdata=NULL; 
  me->idata=NULL; me->ldata=NULL; 
  InfoSave(me, data); 
  return me; 
} /* end MakeInfoElement */ 
  
  
/* Destructor  */ 
  void KillInfoElement(InfoElement *me) 
{ 
  if (!me) return; /* anybody home? */ 
  KillString(me->iname); 
  if (me->ddata!=0) free(me->ddata); me->ddata=NULL;
  if (me->fdata!=0) free(me->fdata); me->fdata=NULL;
  if (me->cdata!=0) free(me->cdata); me->cdata=NULL;
  if (me->idata!=0) free(me->idata); me->idata=NULL;
  if (me->ldata!=0) free(me->ldata); me->ldata=NULL;
  if (me) free(me); me = NULL;
}/* end of KillInfoElement  */ 
  
  
/* Add link to next  */ 
  void InfoAddLink (InfoElement *me, InfoElement *next) 
{
  if (!me) return; /* validity check */
  if (!next) return; /* validity check */
 me->inext = next;} /* end InfoAddLink */ 
  
/*  Compare element name with test string. true=match, else no match  */ 
    Logical InfoTestName (InfoElement *me, char *testname) 
{
  if (!me) return 0; /* validity check */
  if (!testname) return 0; /* validity check */
  return (!strcmp(me->iname->sp, testname));} 
  
  
/*  Update contents of an info element; returns 0 if successful.  */ 
Integer InfoUpdate (InfoElement *me, Integer type, Integer ndim, 
		    Integer *dim, char *data) 
{ 
  Integer i; 
  if (!me) return 1; /* validity check */
  if (!dim) return 1; /* validity check */
  if (!data) return 1; /* validity check */
  me->itype = type; 
  me->indim = ndim; 
  for (i=0; i<ndim; i++) me->idim[i] = dim[i]; /*   Copy dim.  */ 
  return InfoSave(me, data);                   /*   Copy data.  */ 
} /* end InfoUpdate */ 
  
/* Save data */ 
Integer InfoSave (InfoElement *me, char *data) 
{ 
  Integer total_size, j, i, *i_in, *i_out; 
  float  *f_in, *f_out; 
  double *d_in, *d_out; 
  Logical *l_in, *l_out; 
  FStrng **s_in, *t_in; 
  char *c_out;
  
  if (!me) return 1; /* validity check */
  if (!data) return 1; /* validity check */
  total_size = 1; /*   Count number of elements  */ 
  for (i = 0; i<me->indim; i++) 
    if (me->idim[i]>0) total_size *= me->idim[i]; 
  {switch (me->itype) { 
  
      case IDOUBLE :
	if (me->ddata) free(me->ddata);
	me->ddata = (double*) malloc(total_size*sizeof(double));
	if (!me->ddata) return 2; /* allocation error?  */
	d_in = (double *)data;  d_out = me->ddata;
	for (i = 0; i<total_size; i++) *d_out++ = *d_in++;
      break;

      case IFLOAT :
	if (me->fdata) free(me->fdata);
	me->fdata = (float*) malloc(total_size*sizeof(float));
	if (!me->fdata) return 2; /* allocation error?  */
	f_in = (float *)data;  f_out = me->fdata;
	for (i = 0; i<total_size; i++) *f_out++ = *f_in++;
	break;

      case ISTRING :
	s_in = (FStrng **)data;
	total_size = 0; /*   Count number of characters  */
	for (i = 0; i<me->idim[0]; i++)
	  if (s_in[i]->length>0) total_size += s_in[i]->length;
	if (me->cdata) free(me->cdata);
	me->cdata =
	  (char *) malloc((total_size+me->idim[0])*sizeof(char));
	if (!me->cdata) return 2; /* allocation error?  */
/* here have to swallow an array of Strings */
	c_out = me->cdata;
        for (j=0; j<me->idim[0]; j++) {
	  t_in = s_in[j];
	  for (i=0; i<t_in->length; i++) *c_out++ = t_in->sp[i];
	  *c_out++ = 0;
	}
	break;

      case IINTEGER :
	if (me->idata) free(me->idata);
	me->idata = (Integer*) malloc(total_size*sizeof(Integer));
	if (!me->idata) return 2; /* allocation error?  */
	i_in = (Integer *)data;  i_out = me->idata;
	for (i = 0; i<total_size; i++) *i_out++ = *i_in++;
	break;

      case ILOGICAL :
	if (me->ldata) free(me->ldata);
	me->ldata = (Logical*) malloc(total_size*sizeof(Logical));
	if (!me->ldata) return 2; /* allocation error?  */
	l_in = (Logical *)data;  l_out = me->ldata;
	for (i = 0; i<total_size; i++) *l_out++ = *l_in++;
	break;
      }};
 return 0;}  /*  End of InfoSave  */

/*  return data */
  Integer InfoGet  (InfoElement *me, char* data)
{
  Integer total_size, i, j, *i_in, *i_out; 
  float  *f_in, *f_out; 
  double *d_in, *d_out; 
  Logical *l_in, *l_out; 
  char *c_in, ctemp[80]; 
  FStrng **s_out; 
  
  total_size = 1; /*   Count number of elements  */ 
  for (i = 0; i<me->indim; i++) 
    if (me->idim[i]>0) total_size *= me->idim[i]; 
      {switch (me->itype) { 
  
      case IDOUBLE :
	d_out = (double *)data;  d_in = me->ddata;
	for (i = 0; i<total_size; i++) *d_out++ = *d_in++;
      break;

      case IFLOAT :
	f_out = (float *)data;  f_in = me->fdata;
	for (i = 0; i<total_size; i++) *f_out++ = *f_in++;
	break;

      case ISTRING :
/* here have to return an array of Strings */
	s_out = (FStrng **)data;
	c_in = me->cdata;
        for (j=0; j<me->idim[0]; j++) {
	  i = 0;
	  while (*c_in) ctemp[i++] = *c_in++; ctemp[i] = 0;
	  c_in++; /* past terminating null */
	  StringFill(s_out[j], ctemp);}
	break;

      case IINTEGER :
	i_out = (Integer *)data;  i_in = me->idata;
	for (i = 0; i<total_size; i++) *i_out++ = *i_in++;
	break;

      case ILOGICAL :
	l_out = (Logical *)data;  l_in = me->ldata;
	for (i = 0; i<total_size; i++) *l_out++ = *l_in++;
	break;
      }};
 return 0;}  /*  End of InfoGet  */

