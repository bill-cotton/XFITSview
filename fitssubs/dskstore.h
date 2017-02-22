/*  header file for DskStore class */ 
/*  An DskStore is an interface to a disk data file.  The file used is 
    the FITS data file and type conversion is done  when necessary */ 
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
#include "myutil.h" 
#include "myio.h" 
#include "mystring.h" 
#include "myutil.h" 
#include "fitsio.h" 
#include "fitsmem.h" 
#include "gzipread.h" 
#ifndef DSKSTORE_H
#define DSKSTORE_H 
  
typedef struct DSKSTORE { 
    Integer error;       /*  Current error code 0=OK */ 
    Integer nref;        /*  Number of references, object deleted when this */ 
                         /*  goes to 0; */ 
    Integer header_size; /*  FITS header size in bytes */ 
    Integer bitpix;      /*  number of bits per pixel in FITS convention. */ 
    double  scale;       /*  FITS scaling value */ 
    double  offset;      /*  FITS offset. */ 
    Integer blank;       /*  FITS integer blanking value */ 
    Integer byteword;    /*  no. FITS bytes per internal word. */ 
    Integer byteno;      /*  next byte number in FITS buffer */ 
    Integer size;        /*  Current file size in bytes. */ 
    Logical save_file;   /*  If true file is not destroyed when */ 
                         /*  object dies. */ 
    Logical read_only;   /*  Read only? */ 
    int     hFile;       /*  File Handle */ 
    int     hBuffer;     /*  Buffer handle */ 
    Logical isGzip;      /*  If true, file is gzipped */
    union { 
      cMemPtr  buffer;     /* I/O buffer with aliases */ 
      sMemPtr  sbuffer;    /* Shorts */ 
      lMemPtr  lbuffer;    /* Longs */ 
      fMemPtr  fbuffer;    /* Floats */ 
      dMemPtr  dbuffer;    /* Doubles */ 
    } buff; 
    FITSfile *Ffile;      /* FITS file descriptor */
    FStrng *file_name;    /*  FITS file name. */ 
  } DskStore; 
  
  /* Constructor */ 
  DskStore* MakeDskStore(Integer s, FITSfile *fname, Integer bp, 
			 double scl, double off, 
			 Integer blnk, Integer h_size); 
    /* fname FITS file; "none" => create temporary. */ 
    /* bp = bitpix */ 
    /* h_size = header size in bytes */ 
    /* scl = FITS scaling */ 
    /* off = FITS offset */ 
    /* blnk = FITS integer blanking value */ 
  
  
  /* Destructor */ 
  void KillDskStore(DskStore *me); 
  
  /* Increment reference count */ 
  void DskStoreInc_ref(DskStore *me); 
  
  /* Decrement reference count, delete if it goes to 0 */ 
  void DskStoreDec_ref(DskStore *me); 
  
  /* Read file. */ 
  /* file position is wrt beginning of data and units are for */ 
  /* internal data (Not external, FITS data). */ 
  void DskStoreRead_file(DskStore *me, Integer nwords, Integer file_pos, 
			 fMemPtr memory); 
  
  /*  FITS input routines */ 
  /* Read next buffer from file */ 
  void DskStoreRead_buffer(DskStore *me, Integer nread); 
  
  /* function to return scaled words in local format. */ 
  void get_scl_array(DskStore *me, Integer nwords, fMemPtr words); 
  
  
#endif /* DSKSTORE_H */ 
