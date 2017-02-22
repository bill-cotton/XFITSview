/*  This module contains machine specific routines such as data type 
  conversion.  Note: many of these can be inlined using the macroes in 
  zconvrt.h */ 
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
#include "myconfig.h" 
#include "zsubs.h" 
#include "mydefs.h" 
#include <stdio.h> 
#if SYS_TYPE==WINDOWS 
#include <windows.h>
#endif
  
/*  Union definitions for byte shuffle  */ 
 union Iequiv { 
   Integer full; 
   char parts[4]; 
 }; 
 union sequiv { 
   short full; 
   char parts[2];
 }; 
 union lequiv { 
   Integer full; 
   char parts[4]; 
 }; 
 union fequiv { 
   float full; 
   char parts[4]; 
 }; 
 union dequiv { 
   double full; 
   char parts[8]; 
 };
 union fleqv{ 
   Integer lpart; 
   float fpart; 
 }; 
  
/* error message */ 
void ErrorMess(char* message) 
{ 
#if SYS_TYPE==XWINDOW 
#include <stdio.h> 
/* ugly hack - this is from messagebox.c */
/* this should display the message in a scrolling box */
void MessageShow (char *message);
 if (!message) return;
  if (hwndErr)
    MessageShow(message);
/*    fprintf(stderr, "%s\n", message); */

#elif SYS_TYPE==WINDOWS
 if (!message) return;
#ifdef _WIN32
/* 32 bit windows */
  if (hwndErr)
    MessageBox (NULL, message, "FITSview ERROR", ERROR);
#else
/* 16 bit windows */
  if (hwndErr) 
    MessageBox (hwndErr, (LPCSTR)message, "FITSview Error", MB_OK); 
#endif

#elif SYS_TYPE==APPLESA
#define KEEP_GOING 1
#define EXITTOSHELL 2
	Str255 msg;
	short i, disposition;
 if (!message) return;
/* pervert message to Pascal */
	for (i=0;i<255;i++) {
	  if (!message[i]) break;
	  msg[i+1]=message[i];}
	msg[0]=i;
	ParamText(msg,nil,nil,nil);
	disposition =Alert(128, nil); /* alert defined in resource file */
#endif 
} /* end ErrorMess */ 
  
  
  
/*     Rational computers  */ 
#if DATA_TYPE==IEEEBIG 
  
/*   Translation of FITS data types  */ 
/* 16 bit to Integer  */ 
Integer zi16il (short in) 
{Integer out; 
 out = in; return out;} 
  
/* 32 bit to Integer  */ 
Integer zi32il (Integer in) 
{Integer out; 
 out = in; return out;} 
  
/* 32 bit IEEE to float  */ 
/*  NOTE: accepts input as Integer to avoid arithmetic problems  */ 
float zr32rl (Integer in) 
{union fleqv out; 
   out.lpart = in; return out.fpart;} 
  
/* 64 bit IEEE to double  */ 
double zd64dl (double in) 
{double dout; 
 dout = in; return dout;} 
  
/* Integer to 16 bit  */ 
short zili16l (Integer in) 
{short out; 
 out = in; return out;} 
  
/* Integer to 32 bit  */ 
Integer zili32 (Integer in) 
{long out; 
 out = in; return out;} 
  
/* float to 32 bit IEEE  */ 
/* NOTE: returns Integer to avoid arithmetic problems  */ 
Integer zrlr32 (float in) 
{union fleqv out; 
   out.fpart = in; return out.lpart;} 
  
/* double to 64 bit IEEE  */ 
double zdld64 (double out) 
{double dout; 
 dout = out; return dout;} 
  
#elif DATA_TYPE==IEEELIT 
/* Bizzarre , Vax-like machines  */ 
  
/*   Translation of FITS data types  */ 
/* 16 bit to Integer  */ 
Integer zi16il (short in) 
{union sequiv inu; 
 union Iequiv outu; 
 inu.full = in; 
/*  shuffle bytes; zero pad  */ 
 outu.parts[0] = inu.parts[1]; 
 outu.parts[1] = inu.parts[0]; 
 if (inu.parts[0]<0)    /* sign extend if necessary  */ 
   {outu.parts[2] = -1; 
    outu.parts[3] = -1;} 
 else 
  {outu.parts[2] = 0; 
   outu.parts[3] = 0;} 
 return outu.full;} 
  
/* 32 bit to Integer  */ 
Integer zi32il (Integer in) 
{union lequiv inu; 
 union Iequiv outu; 
 inu.full = in; 
/*  shuffle bytes  */ 
 outu.parts[0] = inu.parts[3]; 
 outu.parts[1] = inu.parts[2]; 
 outu.parts[2] = inu.parts[1]; 
 outu.parts[3] = inu.parts[0]; 
 return outu.full;} 
  
/* 32 bit IEEE to float  */ 
/*  NOTE: accepts input as Integer to avoid arithmetic problems  */ 
float zr32rl (Integer in) 
{union lequiv inu; 
 union fequiv outu; 
 inu.full = in; 
/*  shuffle bytes  */ 
 outu.parts[0] = inu.parts[3]; 
 outu.parts[1] = inu.parts[2]; 
 outu.parts[2] = inu.parts[1]; 
 outu.parts[3] = inu.parts[0]; 
 return outu.full;} 
  
  
/* 64 bit IEEE to double  */ 
double zd64dl (double in) 
{union dequiv inu, outu; 
 inu.full = in; 
/*  shuffle bytes  */ 
 outu.parts[0] = inu.parts[7]; 
 outu.parts[1] = inu.parts[6]; 
 outu.parts[2] = inu.parts[5]; 
 outu.parts[3] = inu.parts[4]; 
 outu.parts[4] = inu.parts[3]; 
 outu.parts[5] = inu.parts[2]; 
 outu.parts[6] = inu.parts[1]; 
 outu.parts[7] = inu.parts[0]; 
 return outu.full;} 
  
/* Integer to 16 bit  */ 
short zili16 (Integer in) 
{union Iequiv inu; 
 union sequiv outu; 
 inu.full = in; 
/*  shuffle bytes; wrap overflow  */ 
 outu.parts[0] = inu.parts[1]; 
 outu.parts[1] = inu.parts[0]; 
 return outu.full;} 
  
/* Integer to 32 bit  */ 
Integer zili32 (Integer in) 
{union Iequiv inu; 
 union lequiv outu; 
 inu.full = in; 
/*  shuffle bytes  */ 
 outu.parts[0] = inu.parts[3]; 
 outu.parts[1] = inu.parts[2]; 
 outu.parts[2] = inu.parts[1]; 
 outu.parts[3] = inu.parts[0]; 
 return outu.full;} 
  
/* float to 32 bit IEEE  */ 
/* NOTE: returns Integer to avoid arithmetic problems  */ 
Integer zrlr32 (float in) 
{union fequiv inu; 
 union lequiv outu; 
 inu.full = in; 
/*  shuffle bytes  */ 
 outu.parts[0] = inu.parts[3]; 
 outu.parts[1] = inu.parts[2]; 
 outu.parts[2] = inu.parts[1]; 
 outu.parts[3] = inu.parts[0]; 
 return outu.full;} 
  
  
/* double to 64 bit IEEE  */ 
double zdld64 (double in) 
{union dequiv inu, outu; 
 inu.full = in; 
/*  shuffle bytes  */ 
 outu.parts[0] = inu.parts[7]; 
 outu.parts[1] = inu.parts[6]; 
 outu.parts[2] = inu.parts[5]; 
 outu.parts[3] = inu.parts[4]; 
 outu.parts[4] = inu.parts[3]; 
 outu.parts[5] = inu.parts[2]; 
 outu.parts[6] = inu.parts[1]; 
 outu.parts[7] = inu.parts[0]; 
 return outu.full;} 
  
  
#endif 
