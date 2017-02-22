/*  ImageFITShead class implementation  */ 
/*  An ImageFITShead object contains the information necessary to parse a 
    FITS header card and store the information in an InfoList */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996, 1997,1998,2002
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
  
#include "myfitsio.h" 
#include "ifitshed.h" 
#include "mydefs.h" 
#include "zsubs.h" 
#include <stdio.h> 
#include <stdlib.h> 
  
  
/*  Constructor  */ 
  FITShead* MakeFITShead() 
{ 
  Integer i; 
  FITShead *me = (FITShead *) malloc (sizeof(FITShead)); 
  me->naxes = 0; 
  me->valid = 0; 
  me->next_key=0; 
  me->next_el=0; 
  for (i=0; i<20; i++)  /* init arrays */ 
    {me->sarray[i] = MakeString("        "); 
     me->farray[i] = 0; me->darray[i]=0.0; me->iarray[i]=0; 
     me->larray[i] = 0;} 
     me->number = 0;    /* The number of recognized keywords.  */ 
  
  for (i=0; i<50; i++) me->keywords[i] = NULL; 
  for (i=0; i<50; i++) me->keydim[i] = 0; 
/*  The following required keywords should be at the start in this order.  */ 
  me->keywords[me->number] = MakeString ("SIMPLE  "); 
  me->keytype[me->number++] = FLOGICAL; 
  
  me->keywords[me->number] = MakeString ("BITPIX  "); 
  me->keytype[me->number++] = FINTEGER; 
  
  me->keywords[me->number] = MakeString ("NAXIS   "); 
  me->keytype[me->number++] = FINTEGER; 
  
  me->keywords[me->number] = MakeString ("NAXIS?  "); 
  me->keytype[me->number++] = FINTEGER; 
  
  /*Following are optional keywords.  */ 
  
  me->keywords[me->number] = MakeString ("CRVAL?  "); 
  me->keytype[me->number++] = FDOUBLE; 
  
  me->keywords[me->number] = MakeString ("CTYPE?  "); 
  me->keytype[me->number++] = FSTRING; 
  
  me->keywords[me->number] = MakeString ("CRPIX?  "); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("CDELT?  "); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("CROTA?  "); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("BLANK   "); 
  me->keytype[me->number++] = FINTEGER; 
  
  me->keywords[me->number] = MakeString ("ORIGIN  "); 
  me->keytype[me->number++] = FSTRING; 
  
  me->keywords[me->number] = MakeString ("DATAMAX "); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("DATAMIN "); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("EPOCH   "); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("EQUINOX "); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("BSCALE  "); 
  me->keytype[me->number++] = FDOUBLE; 
  
  me->keywords[me->number] = MakeString ("BZERO   "); 
  me->keytype[me->number++] = FDOUBLE; 
  
  me->keywords[me->number] = MakeString ("OBJECT  "); 
  me->keytype[me->number++] = FSTRING; 
  
  me->keywords[me->number] = MakeString ("DATE-OBS"); 
  me->keytype[me->number++] = FSTRING; 
  
  me->keywords[me->number] = MakeString ("BUNIT   "); 
  me->keytype[me->number++] = FSTRING; 

/* following for perverted DSS headers */
  me->keywords[me->number] = MakeString ("PLTSCALE"); 
  me->keytype[me->number++] = FFLOAT; 

  me->keywords[me->number] = MakeString ("CNPIX?  "); 
  me->keydim[me->number] = 2;
  me->keytype[me->number++] = FINTEGER; 
  
  me->keywords[me->number] = MakeString ("XPIXELSZ"); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("YPIXELSZ"); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("PPO?    "); 
  me->keydim[me->number] = 6;
  me->keytype[me->number++] = FDOUBLE; 
  
  me->keywords[me->number] = MakeString ("AMDX?   "); 
  me->keydim[me->number] = 20;
  me->keytype[me->number++] = FDOUBLE; 
  
  me->keywords[me->number] = MakeString ("AMDY?   "); 
  me->keydim[me->number] = 20;
  me->keytype[me->number++] = FDOUBLE; 
  
  me->keywords[me->number] = MakeString ("OBJCTRA "); 
  me->keytype[me->number++] = FSTRING; 
  
  me->keywords[me->number] = MakeString ("OBJCTDEC"); 
  me->keytype[me->number++] = FSTRING; 
  
  me->keywords[me->number] = MakeString ("OBJCTX  "); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("OBJCTY  "); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("PLTLABEL"); 
  me->keytype[me->number++] = FSTRING; 
  
  me->keywords[me->number] = MakeString ("PLTRAH  "); 
  me->keytype[me->number++] = FINTEGER; 
  
  me->keywords[me->number] = MakeString ("PLTRAM  "); 
  me->keytype[me->number++] = FINTEGER; 
  
  me->keywords[me->number] = MakeString ("PLTRAS  "); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("PLTDECSN"); 
  me->keytype[me->number++] = FSTRING; 
  
  me->keywords[me->number] = MakeString ("PLTDECD "); 
  me->keytype[me->number++] = FINTEGER; 
  
  me->keywords[me->number] = MakeString ("PLTDECM "); 
  me->keytype[me->number++] = FINTEGER; 
  
  me->keywords[me->number] = MakeString ("PLTDECS "); 
  me->keytype[me->number++] = FFLOAT; 
  
/* following for heretical IRAF headers */
  me->keywords[me->number] = MakeString ("IRAF-MAX"); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("IRAF-MIN"); 
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("CD1_?   "); 
  me->keydim[me->number] = 2;
  me->keytype[me->number++] = FFLOAT; 
  
  me->keywords[me->number] = MakeString ("CD2_?   "); 
  me->keydim[me->number] = 2;
  me->keytype[me->number++] = FFLOAT; 
  
  /*  The following should always stay at the end  */ 
  me->keywords[me->number] = MakeString ("END     "); 
  me->keytype[me->number++] = NOSAVE; 
  return me; 
} /* end MakeFITShead */ 
  
/* Destructor  */ 
  void  KillFITShead(FITShead *me) 
{ 
  Integer i; 
  if (!me) return;  /* anybody home? */ 
/* kill strings */ 
  for (i=0; i<50; i++) KillString(me->keywords[i]); 
  for (i=0; i<20; i++) KillString(me->sarray[i]); 
  if (me) free (me); me = NULL;
}  /* end KillFITShead */ 
  
  
Integer eat_card(FITShead *me, FStrng *card, InfoList *ilist) 
/*  parse card and update info list, returns 0 for success,  */ 
/*  1 when reached "END" card else failed.  */ 
{char tstring[81]; 
 Integer key, num_check, key_num, i, j, array_number=0, array_dim, t_dim; 
 Logical  is_array, test; 
 Integer isave = 0; 
 Logical lsave = 0; 
 double dsave = 0.0; 
 float fsave = 0.0; 
 size_t nc; 
 char ssave[80]; 
 char value[81]; 
 Integer  olderr, ierr, iout, lenvalue, indim, idim[20], type; 
 Integer first_quote = -1, last_quote=-1, first_char,last_char; 
 Logical inquote=0;                     /* Find start of comment  */ 
  
 if (!me) return 1; /* validity check */
 if (!card) return 1; /* validity check */
 if (!ilist) return 1; /* validity check */
/*  Check validity and get array dimension  */ 
 if (!valid_check(me, card, &array_dim)) return -1; 
/*  Search list for keyword  */ 
 for (key=0; key<me->number; key++) 
     {num_check = StringFind_char(me->keywords[key], '?'); 
      is_array = (num_check >= 0);  /* Is me an array?  */ 
      if (num_check<0) num_check = 8;  /*  Check 8 char if not an array  */ 
      key_num = key; nc = num_check; 
      test = !strncmp(card->sp, me->keywords[key]->sp, nc); 
      if (test) break;}  /*  End of keyword search loop  */ 
 /*  Process unknown keywords  */ 
 if (!test) {unknown_keywords(card); return 0;} 
 /* check for "END" card  */ 
 nc=8; 
 if (!strncmp (card->sp, "END     ", nc)) return 1; 
 /* Don't save some special keywords  */ 
 test = test && (me->keytype[key_num] != NOSAVE); 
 if (!test) return 0; 
 array_number = 0; 
 t_dim = array_dim;  /* array dimension - header or other */
 if (me->keydim[key_num]>0) t_dim = me->keydim[key_num];

/*  If found add to InfoList  */ 
  if (test) 
    {j = 0; 
     if (is_array)   /*  If an array element parse the number.  */ 
       {for (i=num_check; i<8; i++) {tstring[j] = card->sp[i]; j++;} 
	tstring[j] = 0; 
	sscanf (tstring, "%i", &array_number); 
	if (array_number<1) array_number = 1; 
	if (array_number>t_dim) array_number = t_dim; 
	array_number = array_number - 1;}    /* 0-rel array index.  */ 
  
/*  Extract value from card image  */ 
     lenvalue = 0; 
     first_char = -1;
/*  Extract value field from card.  */ 
     inquote=0;                     /* Find start of comment  */ 
     for (i=10; i<80; i++) {
        if ((first_char<0) && /* find first non blank, non zero character */
	         (!((card->sp[i]==' ')||(card->sp[i]=='0')))) first_char=i;  
        if ((first_char<0) &&  /* last zero before decimal OK */
            ((card->sp[i]=='0')&&(card->sp[i+1]=='.'))) first_char=i;
	if (card->sp[i] == '\'') inquote = !inquote;
	if ((card->sp[i] == '/') && !inquote) 
	  break;                        /* Look for comment delimiter  */ 
	last_char = i;  /* save highest */
      }
     for (i=first_char; i<last_char; i++)   /* Copy value field to value  */
	value[lenvalue++] = card->sp[i]; 
     value[lenvalue] = 0; 
/* Fetch old InfoList entry if it exists and extract this element from value.*/ 
     if (!is_array) t_dim = 1; 
     switch (me->keytype[key_num])          /* save by data type  */ 
       {case NOSAVE:                          /*  This type is not saved  */ 
	 break; 
       case FDOUBLE: 
	 olderr = InfoLookup(ilist, me->keywords[key_num]->sp, &type, 
			     &indim, idim, (char*)me->darray); 
/* possible trouble with sscanf reading double */
    if (sscanf(value, "%le", &dsave)==0) dsave = 0.0;
	 if (olderr!=0)   /* Initialize new array  */ 
	     {indim = 1; idim[0] = t_dim; 
	      for (i=0; i<idim[0]; i++) me->darray[i] = 0.0;} 
	 me->darray[array_number] = dsave; 
	 ierr = InfoStore(ilist, me->keywords[key_num]->sp, FDOUBLE, 
				indim, idim, (char*)me->darray); 
	 break; 
       case FFLOAT: 
	 olderr = InfoLookup(ilist, me->keywords[key_num]->sp, &type, 
			     &indim, idim, (char*)me->farray); 
	 if (sscanf (value, "%e", &fsave)==0) fsave = 0.0;
	 if (olderr!=0)   /* Initialize new array  */ 
	     {indim = 1; idim[0] = t_dim; 
	     for (i=0; i<idim[0]; i++) me->farray[i] = 0.0;} 
	 me->farray[array_number] = fsave; 
	 ierr = InfoStore(ilist, me->keywords[key_num]->sp, FFLOAT, indim, 
			  idim, (char*)me->farray); 
	 break; 
       case FSTRING: 
	 olderr = InfoLookup(ilist, me->keywords[key_num]->sp, &type, 
			     &indim, idim, (char*)me->sarray); 
	 first_quote = -1; 
	 last_quote=-1; 
	 for (i=0;i<lenvalue;i++) 
	   { if ((value[i]=='\'') && (first_quote!=-1)) last_quote = i; 
	     if ((value[i]=='\'') && (first_quote==-1)) first_quote = i;} 
	 iout=0; 
	 for (i=first_quote+1;i<last_quote;i++) ssave[iout++] = value[i]; 
	 ssave[iout]=0; 
	 if (olderr!=0)   /* Initialize new array  */ 
	     {indim = 1; idim[0] = t_dim;} 
	 StringFill(me->sarray[array_number], ssave); 
	 ierr = InfoStore(ilist, me->keywords[key_num]->sp, FSTRING, indim, 
				idim, (char*)me->sarray); 
	 break; 
       case FINTEGER: 
	 olderr = InfoLookup(ilist, me->keywords[key_num]->sp, &type, 
			     &indim, idim, (char*)me->iarray); 
	 if (sscanf (value, "%li", &isave)==0) isave = 0; 
	 if (olderr!=0)   /* Initialize new array  */ 
	     {indim = 1; idim[0] = t_dim; 
	      for (i=0; i<idim[0]; i++) me->iarray[i] = 0;} 
	 me->iarray[array_number] = isave; 
	 ierr = InfoStore(ilist, me->keywords[key_num]->sp, FINTEGER, 
			  indim, idim, (char*)me->iarray); 
	 break; 
       case FLOGICAL:                  /* Look for T else false.  */ 
	 olderr = InfoLookup(ilist, me->keywords[key_num]->sp, &type, 
			     &indim, idim, (char*)me->larray); 
	 lsave = 0; 
	 for (i=0; i<80; i++) 
	   {if (value[i]== '/') break; /* Comment delimiter  */ 
	    if (value[i]=='T') {lsave = 1; break;}} 
	 if (olderr!=0)   /* Initialize new array  */ 
	     {indim = 1; idim[0] = t_dim; 
	     for (i=0; i<idim[0]; i++) me->larray[i] = 0;} 
	 me->larray[array_number] = lsave; 
	 ierr = InfoStore(ilist, me->keywords[key_num]->sp, FLOGICAL, 
			  indim, idim, (char*)me->larray); 
	 break; };    /* End of type specific update of InfoList  */ 
     if (ierr!=0)           /*  Error check  */ 
        {sprintf (szErrMess, "FITShead:eat_card: Error %d updating InfoList", 
		  ierr); 
	 ErrorMess(szErrMess); 
	  return -1;} 
 } /*   End of update of InfoList  */ 
 /*  Any special processing of card  */ 
 check_card(me, me->keywords[key_num], array_number, card, dsave, 
	    fsave, ssave, isave, lsave); 
 return 0;}   /* End of eat_card  */ 
  
/* Validity check at start of eat_card. 1=OK, 0=failed  */ 
Integer valid_check(FITShead *me, FStrng *card, Integer *array_dim) 
{/* validity check if "SIMPLE = T" hasn't been found this better be it  */ 
  size_t nc=15; 
 if (!me) return 0; /* validity check */
 if (!card) return 0; /* validity check */
 if (!array_dim) return 0; /* validity check */
  *array_dim = me->naxes;  /* Only one kind of array for an Image.  */ 
    if (!me->valid) 
	{if (!strncmp (card->sp, "SIMPLE  =      ", nc)) 
	     {/* look for "     T"  */ 
              if (strstr(card->sp, "     T")) 
                 {me->valid = 1; return 1;}} 
	else 
	  {sprintf (szErrMess, 
		    "FITShead::valid_check: Invalid FITS (no SIMPLE = ... T)"); 
	   ErrorMess(szErrMess); 
	   return 0 ;}} 
  return 1; 
}  /*  End of valid_check  */ 
  
/* Process unknown keywords  */ 
void unknown_keywords(FStrng *card) 
{/* ignore  */ 
} 
  
/* Final processing of card values  */ 
void check_card(FITShead *me, FStrng *keyword, Integer array_number, 
   FStrng *card, double dsave, float fsave, char *csave, Integer isave, 
   Logical lsave) 
/*   Trap some special keywords and save values.  */ 
{ 
   if (!me) return; /* validity check */
   if (!strncmp (keyword->sp,"NAXIS   ",8)) me->naxes = isave; 
  }  /* end check_card */ 
  
/*  Has the header been read successfully?  */ 
/*  Returns 1 if a valid header read else 0  */ 
Integer got_head(FITShead *me) 
{
  if (!me) return 0; /* validity check */
  return me->valid;} 
