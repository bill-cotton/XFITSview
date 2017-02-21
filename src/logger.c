/* position logging  for XFITSview */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1997
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
#include "logger.h"
#include "menu.h"
#include "wpos.h"
#include "imagedisp.h"

TextFilePtr LogText = NULL; /* logging TextFile */

/* internal prototypes */
/* initialize log file, argument not used */
void LoggerInit (XPointer arg);
/* file selection canceled, arg not used */
void LoggerCancel (XPointer arg);

/* public functions */

/* toggle position logging on/off */
/* toggle start/stop logging callback clientData = ImageDisplay */
void LoggerCB (Widget w, XtPointer clientData, XtPointer callData)
{
  ImageDisplay  *IDdata= (ImageDisplay *)clientData;
/* turn on or off ?*/
   if (doLog)
     { /* turn off */
       if (!LogText) return; /* bail out if no log TextFile */
       doLog = 0;
       MenuMarkLogger (1); /* reset menu item label */
       TextFileKill(LogText); /* delete structures */
       LogText = NULL;
     }
   else
     {/* turn on */
       if (LogText) TextFileKill (LogText); /* shouldn't need this */
       if (!log_dir) log_dir = MakeString(" ");
       LogText = TextFileMake (IDdata->shell, NULL, log_dir->sp);
       /* ask for file name and initialize file */
       TextFileFind (2, LogText, LoggerInit, LoggerCancel);
       doLog = 1;
       MenuMarkLogger (2); /* reset menu item label */
     }
} /* end LoggerCB */

/* log position */
void DoLogger(void)
{
   int          ret;
   Logical      valid, posOK;
   Integer      ipos[7], ndim, error, i;
   char         flux[20], equistr[7], pixel[23], line[100];
   char         axtype[3][9], label[3][30];
   double       pos[3];
   float        pix[3], fblank, val;
   MatrixPos    *impos;
   ImageData    *Image = &image[CurImag];

   if (!Image) return; /* Validity check */
   if (!Image->Image) return; /* Validity checks */
   if (!Image->valid) return; 
   if (!LogText) return; 
   
/* encode pixel location (convert to 1 relative) */
   sprintf (pixel, "(%7.2f,%7.2f,%4d)", 
	    Image->fXpixel+1.0, Image->fYpixel+1.0, Image->PlaneNo+1);

/*  get flux  */
     ndim = Image->ndim;  /* Number of dimensions  */
     fblank = MagicBlank();
/*  fitted or current?  */
    if (Image->iFitted>0) /* fitted values  */
      {valid = 1;
       val = Image->fBpixel;}
    else  /* current position  */
      {val = 0;
       for (i=3; i<7; i++) ipos[i] = 0;
/* Note: Image class pixel numbers are 0 rel; geometry is 1 rel.  */
       ipos[0] = Image->iXPixel; ipos[1] = Image->iYPixel;
       ipos[2] = Image->PlaneNo;
       impos = MakeMatrixPos(ndim, ipos);
       valid = (ndim>1) && IsValid(Image->Image->matx, impos);
       if (valid)
         {
          val = MatrixGetPixel(Image->Image->matx, impos);
          error = Image->Image->matx->fw->error;
          if (error)   /* I/O error  */
            {ErrorMess("DoLogger: Error reading pixel value");
	     Image->valid = 0; /* mark as invalid */
             KillMatrixPos(impos);
	     return;}
         } /* end of read valid pixel */
        KillMatrixPos(impos); /* destroy what you create */
      } /* end of read current pixel */
    if ((val==fblank) || (!valid)) /* blanked or invalid? - quit*/
      {return;}
    sprintf (flux, "%f",val);
    flux[7] = 0;   /* only 6 char */

/* equinox */
    sprintf (equistr, "  ????");
    if ((usr_equinox>0.0) && (Image->Image->descript->equinox>0.0))
      {if (usr_equinox==2000.0) sprintf (equistr, " J2000");
      if(usr_equinox==1950.0) sprintf (equistr, " B1950");}
    if ((usr_equinox<0.0) && (Image->Image->descript->equinox>0.0))
      {if (Image->Image->descript->equinox==2000.0) sprintf (equistr, " J2000");
      if (Image->Image->descript->equinox==1950.0) sprintf (equistr, " B1950");}

/* celestial position  */
   pix[0] = Image->fXpixel+1.0; pix[1] = Image->fYpixel+1.0;
   pix[2] = (float)(Image->PlaneNo)+1.0;
   strncpy(axtype[0], Image->cname[0]->sp, 8);
   strncpy(axtype[1], Image->cname[1]->sp, 8);
   axtype[0][8] = 0; axtype[1][8] = 0;
   posOK = !get_wpos(Image->Image->descript, pix, pos);
   AxisLabel(pos[0], axtype[0], label[0]);  /* human readable  */
   label[1][0] = 0;
   if (ndim>=2) AxisLabel(pos[1], axtype[1], label[1]);
   label[0][17]=0; /* only 17 characters */
   label[1][17]=0; /* only 17 characters */
   if (!posOK)   /* valid position? if not quit */
     {return;} /* bail out - logging OK */
      
/* Open Log File */
   if (TextFileOpen (LogText, 2) != 1) {
     ErrorMess ("Error opening logging file");
     TextFileKill(LogText);
     LogText = NULL;
     doLog = 0;
     MenuMarkLogger (1); /* reset menu item label */
     return;
   } /* end of error trap */
   
/* write output line */
   sprintf (line, "%s %17s,%17s,%6s,%6s,%s", 
      pixel, label[0], label[1], equistr,flux, Image->FileName->sp);
   if (!TextFileWrite(LogText, line)) {
     ErrorMess ("Error writing logging file");
     TextFileClose(LogText);
     TextFileKill(LogText);
     LogText = NULL;
     doLog = 0;
     MenuMarkLogger (1); /* reset menu item label */
     return;
   } /* end of error trap */
   TextFileClose(LogText); /* close file */
   return;
} /* end DoLogger */

/* initialize log file, argument not used */
/* called after file selected */
void LoggerInit (XPointer arg)
{
   char line[100];
   
/* Open/create Text File */
   if (TextFileOpen (LogText, 2) != 1) {
     ErrorMess ("Error opening logging file");
     TextFileKill(LogText);
     LogText = NULL;
     doLog = 0;
     MenuMarkLogger (1); /* reset menu item label */
     return;
   } /* end of error trap */
   
/* Add first entry */
   sprintf (line, "XFITSview position logging: (pixel) celestial pos., equinox, value, filename");
   if (!TextFileWrite(LogText, line)) {
     ErrorMess ("Error writing logging file");
     TextFileClose(LogText);
     TextFileKill(LogText);
     LogText = NULL;
     doLog = 0;
     MenuMarkLogger (1); /* reset menu item label */
     return;
   } /* end of error trap */
   TextFileClose(LogText); /* close file */

} /* end LoggerInit */

/* file selection canceled, arg not used */
void LoggerCancel (XPointer arg)
{
/* shut down, bail out */
/*if (LogText&&LogText->State) TextFileClose(LogText);  close file if open*/
  if (LogText) TextFileKill(LogText); /* delete structure */
  LogText = NULL;
  doLog = 0;
  MenuMarkLogger (1); /* reset menu item label */
} /* end LoggerCancel */
