/*  ImageFITSin header */ 
/*  A FITSin reads FITS file headers */ 
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
#include "ifitshed.h" 
#include "matx.h" 
#include "fitsio.h" 
#ifndef IMAGEFITSIN_H 
#define IMAGEFITSIN_H 
  
typedef struct IFITSSTR { 
  Logical   isopen;      /* If true file is open */ 
  Logical   got_head;    /* If true head has been read. */ 
  Integer   cardno;      /* Next card number (0 rel) for get or put. */ 
  Integer   byteno;      /* Next buffer byte number (0 rel) for get or put. */ 
  Integer   byte_per_word;/* number of bytes of FITS data per word. */ 
  int       hFile;        /*  File Handle */ 
  FITSfile  *Ffile;       /* FITS file descriptor */
  FStrng    *file_name;   /* Disk FITS file name */ 
  InfoList *ilist;        /* InfoList of header information */ 
  Integer   error;        /* Error condition, 0=OK. */ 
  int       hBuffer;      /* Buffer handle */ 
  MemPtr    buffer;       /* I/O buffer*/ 
  Integer   total_size;   /* The number of pixels in the image. */ 
  Integer   naxes;        /*  Number of axes */ 
  Integer   dim[10];      /*  Dimension of image array. */ 
  FITShead *head;         /*  Image FITS header object; */ 
  Logical  chkGzip;       /*  If true, check if file is gzipped */
  Logical  isGzip;        /*  if true, file is  gzip compressed*/
} FITSin; 
  
/*  Constructor */ 
FITSin* MakeFITSin (FITSfile *name); 
  
/*  Destructor */ 
void KillFITSin(FITSin *me); 
  
/*  Open and read header; returns number of bytes in FITS header. */ 
Integer FITSinOpen(FITSin *me); 
  
/*  close */ 
void FITSinClose(FITSin *me); 
  
/*  Header access forces agreement between the InfoList and the */ 
/*  external representation.  This routine also sets total_size */ 
/* return number of FITS blocks read (2880 byte) */ 
  Integer FITSin_get_head(FITSin *me); 
  
/* Read next buffer */ 
  void FITSinRead_buffer(FITSin *me); 
  
/* Read next header card image (80 char+1) */ 
  void get_next_card(FITSin *me, FStrng *card); 
  
/* Deep copy FITS file */
/* returns 1 if OK, 0 =  error */
int DeepCopyFITSfile(FITSfile *infile, FITSfile *outfile);

  
#endif /* IMAGEFITSIN_H */ 
  
