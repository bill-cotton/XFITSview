/* general definitions, version for FITSview.c  */
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
  
#include <stdio.h> 
#ifndef MYDEFS 
#define MYDEFS 
  
/* Typedef for logicals  1 = true, 0 = false.  */ 
  typedef int Logical; 
  
/* Typedef for integers  */ 
#ifdef _LONGLONG 
  typedef int Integer;  /* for 64 bit Dec machines make it 32 bits */
#else
  typedef long Integer; 
#endif
  
/* Typedef for byte variables  */ 
  typedef unsigned char Byte; 
  
  /* Macro for magic value blanking value  */ 
  
  
/* Max. size of FWindow memory.  */ 
#define MAX_WINDOW  32768
  
#if SYS_TYPE==WINDOWS
/* HUGE16 to support huge pointers in 16 bit Windows */
#ifdef _WIN32
/* 32 bit windows */
#define HUGE16
#else
/* 16 bit windows */
#define HUGE16 huge
#endif
#endif

#endif /* MYDEFS */ 
