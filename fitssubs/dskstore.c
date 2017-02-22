/*  DskStore class implementation */ 
/*  An DskStore is an interface to a disk data file.  The file used is 
    the FITS data file and type conversion is done  when necessary */ 
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
  
#include "myutil.h" 
#include "myio.h" 
#include "mystring.h" 
#include "myutil.h" 
#include "fitsio.h" 
#include "fitsmem.h" 
#include "dskstore.h" 
#include "zsubs.h" 
#include "zconvrt.h" 
#include "mydefs.h" 
#include "gzipread.h" 
  
/* Constructors */ 
  
  DskStore* MakeDskStore(Integer s, FITSfile *fname, Integer bp, 
			 double scl, double off, 
			 Integer blnk, Integer h_size) 
{ 
  DskStore *me = (DskStore *) malloc (sizeof(DskStore)); 
  me->nref = 1; 
  me->size = s;
  me->Ffile = CopyFITSfile(fname);
  me->file_name = FITSfilename(fname); 
  me->save_file=1; 
  me->error = 0; 
  me->header_size = h_size; 
  me->read_only = 1; 
  me->bitpix=bp; 
  me->scale = scl; 
  me->offset = off; 
  me->blank=blnk; 
  me->hBuffer = AllocMem((long)2880); 
 if (!me->hBuffer) 
     {sprintf (szErrMess,"MakeDskStore: Memory allocation failed for buffer"); 
      ErrorMess(szErrMess); 
      me->error = 2; 
      return 0;}  /*end Allocate failed */ 
/* set number of FITS bytes per word */ 
  me->byteword = 4; 
  if (bp==8) me->byteword = 1; 
  if (bp==16) me->byteword = 2; 
  if (bp==-64) me->byteword = 8; 
/* gzip stuff */
  me->isGzip = 0;
  return me; 
} /* end MakeDskStore */ 
  
  /* Destructor */ 
  void KillDskStore(DskStore *me) 
{ 
  if (!me) return; /* anybody home? */ 
  if (me->hBuffer) DeallocMem(me->hBuffer);  /* delete buffer */ 
  if (me->isGzip) gz_close();  /* close gzip stuff */
  KillFITSfile(me->Ffile);
  KillString(me->file_name); 
  if (me) free(me); me = NULL;
} /* end KillDskStore */ 
  
/* Increment reference count */ 
void DskStoreInc_ref(DskStore *me) {me->nref++;} 
  
/* Decrement reference count, delete if it goes to 0 */ 
void DskStoreDec_ref(DskStore *me) 
{ 
  if (!--(me->nref)) KillDskStore(me); 
} /* end DskStoreDec_ref */ 
  
void DskStoreRead_file(DskStore *me, Integer nwords, Integer file_pos, 
		       fMemPtr memory) 
/* read section of file into memory with translation. */ 
/* this presumes that data is completely packed on disk with no holes */ 
/* so no alignment with FITS logical records is necessary. */ 
{ 
  Integer FITS_pos; 
  long  pos; 
  int   seekret, error; 
  FITSfile *gzip_current;
  
/* check object validity */ 
  if (!me) 
      {sprintf (szErrMess, "DskStoreRead: Invalid file object"); 
       ErrorMess(szErrMess); 
       return;} 
/* check memory - trap null pointer */ 
  if (!CanWriteMem((MemPtr)memory, (long)(nwords*4))) {me->error = 5;} 
/* Open file */ 
 me->hFile = FileOpen (me->Ffile); 
 if (!me->hFile) 
     {sprintf (szErrMess, "DskStoreRead_file: Error opening file %s", 
	       (const char *) me->file_name->sp); 
      ErrorMess(szErrMess); 
      me->error = 1;} 
 if (me->error) 
     {sprintf (szErrMess, "DskStoreRead_file: Error %d opening file %s", 
         me->error, (const char *) me->file_name->sp); 
      ErrorMess(szErrMess); 
      } 
  if ((me->byteword<=4) && (me->byteword>=1))
    FITS_pos=me->header_size+file_pos/(4/me->byteword); 
  else 
    FITS_pos=me->header_size+file_pos*(me->byteword/4); 
  if (me->error) 
      {sprintf (szErrMess, "DskStoreRead_file: Error %d in file %s", 
		me->error, 
          (const char *) me->file_name->sp); 
       ErrorMess(szErrMess); 
       DskStoreDec_ref(me); 
       return;} 
/*  no test for read past EOF - let I/O fail. */ 
  pos = FITS_pos; 
/* regular or gzip file? */
  if (!me->isGzip)
    { seekret = FileSeek (me->hFile, pos);  /* Position for read */ 
     error = seekret != 1;}
  else
    { /* check if file has changed */
      gzip_current = gz_current_file();
      if (!CompFITSfile(gzip_current, me->Ffile))
	gz_init (me->hFile);
      seekret = gz_seek (pos);  /* position gzip file */
      error = seekret != PR_SUCCESS;}
  if (error) 
      {sprintf (szErrMess, 
		"DskStoreRead_file: Error positioning file %s to %u", 
		(const char *) me->file_name->sp, pos); 
       ErrorMess(szErrMess); 
       me->error = 3; 
       DskStoreDec_ref(me); 
       return;} 
/* lock buffer memory */ 
  me->buff.buffer = (cMemPtr)LockMem(me->hBuffer); 
  if (!me->buff.buffer) {me->error=4; return;}   /* error check */ 

/* Loop reading FITS buffer full and converting to output memory. */ 
  me->byteno = 999999; 
  get_scl_array(me, nwords, memory); 

/*  if (me->isGzip) gz_close();   close gzip stuff */
/* close file */ 
  if (!FileClose(me->hFile)) 
    {sprintf (szErrMess, "KillDskStore: Error closing file %s", 
	      (const char *) me->file_name->sp); 
      ErrorMess(szErrMess);} 
  UnlockMem(me->hBuffer);  /* unlock buffer memory */ 
  me->buff.buffer = NULL; 
  return;}  /* End of DskStoreRead_file */ 
  
/* FITS input */ 
/* Read  buffer (2880 bytes) */ 
void DskStoreRead_buffer(DskStore *me, Integer nread) 
{ 
  int nred, ret, error; 
  if (nread<1) return; 
/* regular of gzip file */
  if (!me->isGzip)
    {nred = FileRead (me->hFile, me->buff.buffer, (int)nread); 
     error = nred != nread; }
  else
    {nred = nread;
     ret = gz_read ((int*)&nred, (cMemPtr)me->buff.buffer);
     error = (ret != PR_SUCCESS) || (nred != nread); }
  me->byteno = 0; 
  if(error)         /*  Error check - must read at least nread bytes. */ 
    {sprintf (szErrMess, "DskStoreRead_buffer: Error reading %s %d", 
	      (const char *)me->file_name->sp, nred); 
     ErrorMess(szErrMess); 
     me->error = 1;} 
} /* end DskStoreRead_buffer */ 
  
void get_scl_array(DskStore *me, Integer nwords, fMemPtr data) 
/* function to return scaled words in local format. */ 
/* Me routine doesn't need to worry about words split between buffers */ 
/* since it is only to be used for files where all words are the same type. */ 
{Integer i; 
 Integer sword, lword, stemp, ltemp, fword; 
 Integer nread, nleft, bytewrd; 
 float ftemp, blankv; 
 double dtemp; 
/* local version of some variables to cut down on memory traffic */
 Integer byteno, blank;
 cMemPtr  buffer;     /* I/O buffer */ 
 sMemPtr  sbuffer;    /* Shorts */ 
 lMemPtr  lbuffer;    /* Longs */ 
 fMemPtr  fbuffer;    /* Floats */ 
 dMemPtr  dbuffer;    /* Doubles */ 

 if (!me) return; /* validity check */
 if (!data) return;
/* set local variables */
 blank = me->blank;
 byteno = me->byteno;
 blankv = MagicBlank(); 
 nleft = nwords; 
 if (blank==0)   /*  No blanking */
    {switch (me->bitpix) 
	 {case 8:             /*  Scaled bytes */ 
	   bytewrd = 1; 
	   buffer = me->buff.buffer+byteno/bytewrd;
	     for (i=0; i<nwords; i++) 
		 {if (byteno>=2880) 
		      {nread = 2880; 
		       if (nleft<2880) nread = nleft*bytewrd; 
		       if (nread<0) nread=0; 
		       DskStoreRead_buffer(me, nread); 
		       byteno=0; 
		       buffer = me->buff.buffer;
		       nleft = nleft - 2880; if (me->error) return;} 
		  ltemp = *buffer++; byteno++;
		  *(data++) = (float)ltemp;}
	     me->byteno = byteno; /* save buffer pointer */
	     return; 
	 case 16:            /*  scaled short */ 
	   bytewrd = 2; 
	   sbuffer = me->buff.sbuffer+byteno/bytewrd;
	     for (i=0; i<nwords; i++) 
		 {if (byteno>=2880) 
		      {nread = 2880; if (nleft<1440) nread = nleft*bytewrd; 
		       if (nread<0) nread=0; 
		       DskStoreRead_buffer(me, nread); byteno=0;
		       sbuffer = me->buff.sbuffer;
		       nleft = nleft - 1440; if (me->error) return;} 
		  SFits.full = (*sbuffer++); 
		  ZI16IL /* convert */
		  *(data++) =  ILocal.full;
		  byteno += 2;} 
	     me->byteno = byteno; /* save buffer pointer */
	     return; 
	 case 32:            /*  scaled long */ 
	   bytewrd = 4; 
	   lbuffer = me->buff.lbuffer+byteno/bytewrd;
	     for (i=0; i<nwords; i++) 
		 {if (byteno>=2880) 
		      {nread = 2880; if (nleft<720) nread = nleft*bytewrd; 
		       if (nread<0) nread=0; 
		       DskStoreRead_buffer(me, nread); byteno=0;
		       lbuffer = me->buff.lbuffer;
		       nleft = nleft - 720; if (me->error) return;} 
		  LFits.full = (*lbuffer++); 
		  ZI32IL /* convert */
		  *(data++) =  ILocal.full;
		  byteno += 4;} 
	     me->byteno = byteno; /* save buffer pointer */
	     return; 
	 case -32:           /*  IEEE 754 32 bit float. */ 
	   bytewrd = 4; 
	   fbuffer = me->buff.fbuffer+byteno/bytewrd;
	     for (i=0; i<nwords; i++) 
		 {if (byteno>=2880) 
		      {nread = 2880; if (nleft<720) nread = nleft*bytewrd; 
		       if (nread<0) nread=0; 
		       DskStoreRead_buffer(me, nread); byteno=0;
		       fbuffer = me->buff.fbuffer;
		       nleft = nleft - 720; if (me->error) return;} 
	     /*  May always be blanked */ 
		  if (IsfNaN (*fbuffer)) 
		    {*(data++) = blankv; fbuffer++;} 
		  else 
		    {FFits.lfull = (*(lMemPtr)fbuffer++); 
		     ZR32RL /* convert */
		     *(data++) =  FLocal.full;}
		  byteno += 4;} 
	     me->byteno = byteno; /* save buffer index */
	     return; 
	 case -64:           /*  IEEE 754 64 bit float. */ 
	   bytewrd = 8; 
	   dbuffer = me->buff.dbuffer+byteno/bytewrd;
	     for (i=0; i<nwords; i++) 
		 {if (byteno>=2880) 
		      {nread = 2880; if (nleft<360) nread = nleft*bytewrd; 
		       if (nread<0) nread=0; 
		       DskStoreRead_buffer(me, nread); byteno=0; 
		       dbuffer = me->buff.dbuffer;
		       nleft = nleft - 360; if (me->error) return;} 
	     /*  May always be blanked */ 
		  if (IsdNaN (*dbuffer)) 
		    {*(data++) = blankv; dbuffer++;} 
		  else 
		    {DFits.full = (*dbuffer++); 
		     ZD64DL /* convert */
		     dtemp =  DLocal.full;
                     if (dtemp>1.0e38) dtemp = 1.0e38; 
                     if (dtemp<-1.0e38) dtemp = -1.0e38; 
		    *(data++) = dtemp;} 
		  byteno += 8;} 
	     me->byteno = byteno; /* save buffer pointer */
	     return; 
          default:  /* Oops - shouldn't get here */ 
	   sprintf (szErrMess, "get_scl_array: Illegal BITPIX %d", 
		      me->bitpix); 
	   ErrorMess(szErrMess); 
	 }  /* End of switch */ 
 }  /*  End of unblanked section */ 
 else                            /*  Blanked */ 
    {switch (me->bitpix) 
	 {case 8:             /*  Scaled bytes */ 
	   bytewrd = 1; 
	   buffer = me->buff.buffer+byteno/bytewrd;
	     for (i=0; i<nwords; i++) 
		 {if (byteno>=2880) 
		      {nread = 2880; if (nleft<2880) nread = nleft*bytewrd; 
		       if (nread<0) nread=0; 
		       DskStoreRead_buffer(me, nread); byteno=0; 
		       buffer = me->buff.buffer;
		       nleft = nleft - 2880; if (me->error) return;} 
		  if (*buffer==blank) 
		      {*(data++) = blankv; buffer++;} 
		  else 
		    {ltemp = *buffer++; 
		     *(data++) = (float)ltemp;} 
		  byteno += 1; } 
	     me->byteno = byteno; /* save buffer pointer */
	     return; 
	 case 16:            /*  scaled short */ 
	   bytewrd = 2; 
	   sbuffer = me->buff.sbuffer+byteno/bytewrd;
	     for (i=0; i<nwords; i++) 
		 {if (byteno>=2880) 
		      {nread = 2880; if (nleft<1440) nread = nleft*bytewrd; 
		       if (nread<0) nread=0; 
		       DskStoreRead_buffer(me, nread); byteno=0;
		       sbuffer = me->buff.sbuffer;
		       nleft = nleft - 1440; if (me->error) return;} 
		  SFits.full = (*sbuffer++); 
		  ZI16IL /* convert */
		  stemp =  ILocal.full;
		  if (stemp==blank) 
		      {*(data++) = blankv;} 
		  else 
		      *(data++) = stemp; 
		  byteno += 2;} 
	     me->byteno = byteno; /* save buffer pointer */
	     return; 
	 case 32:            /*  scaled long */ 
	   bytewrd = 4; 
	   lbuffer = me->buff.lbuffer+byteno/bytewrd;
	     for (i=0; i<nwords; i++) 
		 {if (byteno>=2880) 
		      {nread = 2880; if (nleft<720) nread = nleft*bytewrd; 
		       if (nread<0) nread=0; 
		       DskStoreRead_buffer(me, nread); byteno=0;
		       lbuffer = me->buff.lbuffer;
		       nleft = nleft - 720; if (me->error) return;} 
		  LFits.full = (*lbuffer++); 
		  ZI32IL /* convert */
		  ltemp =  ILocal.full;
		  if (ltemp==blank) 
		      {*(data++) = blankv;} 
		  else 
		      *(data++) = ltemp; 
		  byteno += 4;} 
	     me->byteno = byteno; /* save buffer pointer */
	     return; 
	 case -32:           /*  IEEE 754 32 bit float. */ 
	   bytewrd = 4; 
	   fbuffer = me->buff.fbuffer+byteno/bytewrd;
	     for (i=0; i<nwords; i++) 
		 {if (byteno>=2880) 
		      {nread = 2880; if (nleft<720) nread = nleft*bytewrd; 
		       if (nread<0) nread=0; 
		       DskStoreRead_buffer(me, nread); byteno=0;
		       fbuffer = me->buff.fbuffer;
		       nleft = nleft - 720; if (me->error) return;} 
		  if (IsfNaN (*fbuffer)) 
		    {*(data++) = blankv; fbuffer++;} 
		  else 
		    {FFits.lfull = (*(lMemPtr)fbuffer++); 
		     ZR32RL /* convert */
		     *(data++) =  FLocal.full;}
		  byteno += 4;} 
	     me->byteno = byteno; /* save buffer index */
	     return; 
	 case -64:           /*  IEEE 754 64 bit float. */ 
	   bytewrd = 8; 
	   dbuffer = me->buff.dbuffer+byteno/bytewrd;
	   bytewrd = 8; 
	     for (i=0; i<nwords; i++) 
		 {if (byteno>=2880) 
		      {nread = 2880; if (nleft<360) nread = nleft*bytewrd; 
		       if (nread<0) nread=0; 
		       DskStoreRead_buffer(me, nread); byteno=0;
		       dbuffer = me->buff.dbuffer;
		       nleft = nleft - 360; if (me->error) return;} 
		  if (IsdNaN (*dbuffer)) 
		    {*(data++) = blankv; dbuffer++;} 
		  else 
		    {DFits.full = (*dbuffer++); 
		     ZD64DL /* convert */
		     dtemp =  DLocal.full;
                     if (dtemp>1.0e38) dtemp = 1.0e38; 
                     if (dtemp<-1.0e38) dtemp = -1.0e38; 
		    *(data++) = dtemp;} 
		  byteno += 8;} 
	     me->byteno = byteno; /* save buffer pointer */
	     return; 
          default:  /* Oops - shouldn't get here */ 
	   sprintf (szErrMess, "get_scl_array: Illegal BITPIX %d", 
		      me->bitpix); 
	   ErrorMess(szErrMess); 
	 }  /* End of switch */ 
 }  /*  end of blanked section */ 
}  /*  End of get_scl_array */ 
