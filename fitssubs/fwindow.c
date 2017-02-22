/*  FWindow class implementation  */ 
/* an FWindow is a sliding memory redident window on a disk FITS file */  
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

#include "fwindow.h" 
  
 /* Constructor */ 
 /*total size in words, window size in words, Permanent file name.  */ 
 FWindow* MakeFWindow() 
{ 
  FWindow *me = (FWindow *) malloc (sizeof(FWindow)); 
  me->nref = 1; 
  me->error = 0; 
  me->first = -1; 
  me->last = -1; 
  me->written = 0; 
  me->valid=0; 
  me->next = 0; 
  me->frame=-1; 
  me->data_max=-1.0e20; 
  me->data_min=1.0e20; 
  me->fdata = NULL; /* Create buffer at build  */ 
  me->file = NULL;  /* Create DskStore later */ 
  return me; 
} /* end MakeFWindow */ 
  
/* Destructor  */ 
void KillFWindow(FWindow *me) 
{ 
  if (!me) return; /* anybody home? */ 
  if (me->file) DskStoreDec_ref(me->file); 
  if (me->fdata) FmemDec_ref(me->fdata); 
  if (me) free(me); me = NULL;
} /* end KillFWindow */ 
  
/* Construct window  */ 
 void build_window(FWindow *me, Integer size, Integer window, 
		  FITSfile *fname, Integer bitpix, Integer hbytes, 
		  Integer blank, double scale, double offset) 
{ 
  me->error = 0; 
  if (window<=0) window = size;  /* Default window is whole thing.  */ 
  me->first = -1; 
  me->last = -1; 
  me->frame = -1; 
  me->written = 0; 
  me->next = 0; 
  if (me->fdata) FmemDec_ref(me->fdata);/* Detach from old memory  */ 
  if (me->file) DskStoreDec_ref(me->file);  /* Detach from old file  */ 
  me->fdata = MakeFmem(window, 1);      /* Create buffer  */ 
  if (!me->fdata) {me->error=1; return;} /* allocation errors */
  me->file = MakeDskStore(size*sizeof(float), fname, bitpix, scale, 
			    offset, blank, hbytes); 
} /* end  build_window */ 
  
  
/* Increment reference count  */ 
void FWindowInc_ref(FWindow *me) 
{if (me) me->nref++;} 
  
/* Decrement reference count, delete if it goes to 0  */ 
/* returns true if it self destructs, false otherwise  */ 
Logical FWindowDec_ref(FWindow *me) 
{ 
  if (!me) return 0;  /* Anybody home? */ 
  if (--(me->nref)<=0)    /* Delete when reference count goes to 0  */ 
     {KillFWindow(me); 
      return 1;} 
else 
  return 0;}  /* end FWindowDec_ref  */ 
  
  
/*  Move window to new first floating word number reading previous values.  */ 
void FWindowFetch(FWindow *me, Integer new_first) 
  
{ 
  Integer nwd, nwords, nbytes, file_pos, fsize; 
  fMemPtr values; 
  if (!me) return;
  if (!me->file) return;       /* noop if no file  */ 
  if (!me->fdata) return;
  if (!me->fdata->values) /* lock memory if it's not locked */
    if (LockFMem(me->fdata)==NULL) me->error = 6;
  if (me->error) 
    {sprintf (szErrMess, "FWindowFetch: Error %ld exists, file position=%ld", 
	      me->error, new_first); 
     ErrorMess(szErrMess);} 
  values = (fMemPtr)me->fdata->values; 
  nwords = me->fdata->count * me->fdata->size; 
  fsize = me->file->size; 
  me->frame = new_first / nwords; 
  nbytes = nwords*sizeof(float); 
  file_pos = new_first*sizeof(float); 
  if ((file_pos+nbytes)>=fsize) nbytes = fsize-file_pos-1; 
  nwd = nbytes / sizeof(float); 
  me->first = new_first; 
  me->last = me->first + nbytes/sizeof(float) - 1; 
  me->next = 0; 
  DskStoreRead_file(me->file, nwd, file_pos, values); 
  me->error = me->file->error; 
  if (me->error) 
    {sprintf (szErrMess, "FWindowFetch:Error %ld moving window", me->error); 
     ErrorMess(szErrMess);} 
}  /*  End of FWindowFetch  */ 
  
/*  Move window to new first floating word number without  */ 
/*  reading previous values.  This will cause written to be set  */ 
/*  and the data returned in the window is undefined.  */ 
void FWindowMove(FWindow *me, Integer new_first) 
{ 
  Integer nwords; 
  if (!me) return; /* anybody home? */ 
  if (!me->file) return;       /* noop if no file  */ 
  if (me->error) return; 
  nwords = me->fdata->count * me->fdata->size; 
  me->first = new_first; 
  me->last = me->first + nwords - 1; 
  me->frame = new_first / nwords; 
  me->written = 1; 
  me->next = 0; 
}  /*  End of FWindowMove  */ 
  
  
/*  Fetch n words of data  */ 
void FWindowGet_data(FWindow *me, Integer n, fMemPtr f) 
{ 
  fMemPtr values; 
  if (!me) return;
  if (!f) return;
  if (!me->fdata->values) {me->error=6; return;} /* check memory */ 
  values = (fMemPtr)me->fdata->values; 
  if ((me->next<(me->last-me->first+1)) && 
      ((me->next+n-1)<(me->last-me->first+1))) 
     {while (n--)    /* all in memory  */ 
	  *f++ = *(values+(me->next)++); 
      return;} 
  while(n--)  /*  need to do i/o in middle  */ 
    {if (me->next>=(me->last-me->first+1)) 
       FWindowFetch(me, me->last+1);  /* do I/O  */ 
     *f++ = *(values+(me->next)++);} 
}  /*  end FWindowGet_data  */ 
  
  
