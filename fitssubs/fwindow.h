/*  header file for FWindow class  */ 
/*  An FWindow consists of a scrolling floating point window (frame)  
    on a disk file.  */ 
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
#include "fmem.h" 
#include "dskstore.h" 
#ifndef FWINDOW_H 
#define FWINDOW_H 
  
typedef struct FWINDOW { 
    Integer error;        /*  Current error code 0=OK  */ 
    Integer  nref;        /*  Number of references, object deleted when this*/ 
                          /*  goes to 0;  */ 
    Integer  first;       /*  First float word of file in fdata  */ 
    Integer  last;        /*  Last float word of file in fdata  */ 
    Integer  next;        /*  Index of next read or write in fdata.  */ 
    Integer  frame;       /*  Frame number in file.  */ 
    Logical  written;     /*  If true write current window.  */ 
    Logical  valid;       /*  It true contains valid data.  */ 
    float    data_max;    /*  Maximum data value  */ 
    float    data_min;    /*  Minimum data value  */ 
    Fmem    *fdata;       /*  Memory block  */ 
    DskStore *file;       /*  Disk data file object.  */ 
  } FWindow; 
  
  /* Constructor */ 
  FWindow* MakeFWindow(void); 
  
  /* Destructor ; applications should use FWindowDec_ref */ 
  void KillFWindow(FWindow *me); 
  
  /* Construct window  */ 
  void build_window(FWindow *me, Integer size, Integer window, 
		    FITSfile *fname, Integer bitpix, Integer hbytes, 
		    Integer blank, double scale, double offset); 
  
  /* Increment reference count  */ 
  void FWindowInc_ref(FWindow *me); 
  
  /* Decrement reference count, delete if it goes to 0  */ 
  /* returns true if self destruct else false.  */ 
  Logical FWindowDec_ref(FWindow *me); 
  
  /*  Move window to new first floating word number reading previous values.*/ 
  void FWindowFetch(FWindow *me, Integer new_first); 
  
  /*  Move window to new first floating word number without  */ 
  /*  reading previous values.  Me will cause written to be set  */ 
  /*  and the data returned in the window is undefined.  */ 
  void FWindowMove(FWindow *me, Integer new_first); 
  
  /*  Fetch n words of data  */ 
  void FWindowGet_data(FWindow *me, Integer n, fMemPtr f); 
  
#endif /* FWINDOW_H */ 
