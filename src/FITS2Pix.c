/* routines to load a FITS file to a ZPixmap for XFITSview*/
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1997,1999,2000,2002
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
#include <math.h>
#include "imag.h"
#include <Xm/Xm.h> 
#include <Xm/DrawingA.h> 
#include <Xm/MainW.h> 
#include <Xm/DialogS.h> 
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <X11/cursorfont.h> 
#include "imagedisp.h"
#include "messagebox.h"
#include "histo.h"

/* file global stuff for working boxes */
Widget   shell;
static Widget   dialog=NULL;
static Widget   line1, line2, line3, CancelButton;
static Boolean  stopped;

/* internal prototypes for progress/cancel box */
void WorkingCursor(int on, int verbose, char* filename);
Boolean CheckForCancel();

int FITS2Pix (ImageData *image, ImageDisplay *IDdata, int verbose)
/* Routine to convert a FITS image to a pixelarray with a color table. */
/* returns 0 if OK else failed. */
/*  if verbose if true then a progress/cancel box appears. */
{
  FITSin        *file;
  Integer        error, dataoff;
  Integer       *dims, loop;
  unsigned       long pasize, ilong, yaddr, addr, maxaddr=0;
  int            i, j, jj, nx, ny, icol, index;
  float val, valmin, valmax, unvmin, unvmax, truemin, truemax;
  float *valp, *lpImage, blanked, irange, c1, c2;
  double        scale, offset;
  char *ZP, message[100], nameonly[30];
  Display     *dpy = XtDisplay (IDdata->display);
  unsigned int unx, uny;
  int      lastPc, badrange, wasbad, percent, samefile, newRange, bugOut;
  XmString wierd = NULL;
  
  error = 0;
 /* Magic blanking value */
  blanked = MagicBlank();
/* set shell for working box */
  shell = IDdata->shell;

  /* return OK if no file name */
  if (!image->FileName) return 1;
  if (!image->FileName->sp) return 1;
  if (!image->FileName->length) return 1;
  /* is this a reload of the same file? */
  samefile = 0;
  if ((image->Image) && (image->valid) && (image->Image->matx->fw->valid)) 
    {
      samefile = StringComp (image->FileName, 
			     image->Image->matx->fw->file->file_name);
    } /* end of test if same file */

  /* don't rebuild if readinng same file as last time */
  if (!samefile) 
    {
      /*  Image/FITS stuff */
      file = MakeFITSin(image->FileName);
      
      if (image->Image) KillFImage(image->Image); /* zap any old image */
      image->valid = 0; /* no longer valid */
      image->Image = MakeFImage();
      LoadFImage(file, image->Image); /* read header, setup */
      KillFITSin(file);
      if (image->Image->error) {
	MessageShow ("Error reading FITS header");
	return 1;} /* failed? */
    } /* end of build image */

/* get image information */
  scale = image->Image->matx->fw->file->scale;
  if (fabs(scale)<1.0e-25) scale = 1.0;
  offset = image->Image->matx->fw->file->offset;
  image->epoch = image->Image->descript->epoch;
  StringCopy(image->obsdate, image->Image->descript->date_obs);
  StringCopy(image->object, image->Image->descript->image_name);
  StringCopy(image->units, image->Image->descript->units);
  image->ndim = image->Image->descript->num_axes;
  for (loop=0;loop<image->ndim;loop++)     /* axis info */
    {
      image->dim[loop] = image->Image->descript->axisdesc[loop]->axis_length;
      StringCopy(image->cname[loop],
		 image->Image->descript->axisdesc[loop]->axis_name);
      image->crval[loop] = image->Image->descript->axisdesc[loop]->axis_coord;
      image->crpix[loop] = 
	image->Image->descript->axisdesc[loop]->axis_ref_pixel;
      image->crinc[loop] = 
       image->Image->descript->axisdesc[loop]->axis_increment;
      image->crot[loop] = 
	image->Image->descript->axisdesc[loop]->axis_rotation;
   } /* End of loop over axes. */

  /* Set user requested equinox if header has one*/
  if (image->Image->descript->equinox>0.0)
    image->Image->descript->usr_equinox = usr_equinox;
  else
    image->Image->descript->usr_equinox = -1.0;

  image->valid = 1;
  if (image->dim[2]<1) image->dim[2] = 1; /*at least one plane */
  dims = image->dim;
  image->iImageNx = dims[0];
  image->iImageNy = dims[1];
  image->iNumPlanes = dims[2];
  image->data_max = image->Image->matx->fw->data_max; /* max. pixel value */
  image->data_min = image->Image->matx->fw->data_min; /* min. pixel value */

/* center scroll */
  IDdata->scrollx = image->iImageNx / 2;
  IDdata->scrolly = image->iImageNy / 2; 
  IDdata->showInfo = 0; /* turn off display of pixel information */

/*  Setup pixarray */
  if (!samefile)
    {
      /* delete old */
      if (image->pixarray) free ((char *) image->pixarray); image->pixarray=NULL;
      /* create new pix map */
      pasize = dims[0] * dims[1]; /* keep as 8 bit */
      ZP = (char*) malloc (pasize);
/* debug */
/*fprintf (stderr,"FITS2Pix: pasize=%lu, ZP=%lu\n", pasize, ZP);*/
      unx = image->iImageNx;
      uny = image->iImageNy;
      /* blank it out */
      for (ilong=0; ilong<pasize; ilong++) ZP[ilong] = 0;
      image->pixarray = ZP; /* save pointer */
    } /* end of build pixarray */

   valmin = image->data_min;
   valmax = image->data_max;
   valmin = blanked;
   valmax = blanked;
   newRange = 1;
   if (((image->PixRange[0]!=0.0) || 
	(image->PixRange[1]!=0.0))){ /*  User selected range */
     valmin = image->PixRange[0]; 
     valmax = image->PixRange[1];
     newRange = 0;
   }
/* unscaled values */
  unvmin = (valmin - offset) / scale;
  unvmax = (valmax - offset) / scale;


  if (image->PlaneNo+1>image->dim[2]) 
      image->PlaneNo = image->dim[2] - 1;  /* limits on plane */
  if (image->PlaneNo<0) image->PlaneNo = 0;

/* get filename from full path */
  index = 0;
  for (i=0;i<image->FileName->length;i++) 
    if (image->FileName->sp[i]=='/') index = i+1;
  strcpy(nameonly, &(image->FileName->sp[index]));

/* Progress / cancel dialog box */
  lastPc = 0;
  badrange = 1; wasbad = 0;  /* bad pixel range flag */
  WorkingCursor(True, verbose, nameonly);
/* working dialog will have three Label children: */
/* line1) name of the file being loaded */
/* line2) percent done */
/* line3) Warning about pixel values */

/*  get image buffer pointer */

   lpImage = (float *)LockFMem(image->Image->matx->fw->fdata);

/*  Get true max, min */
   truemin = 1.0e30; truemax = -1.0e30;

   /* setup for display */
   bugOut = 0;
   if (image->iNonlinear==2) { /* histogram equalization */
     bugOut = equalize (image->Image->matx, image->PlaneNo,
        &unvmax, &unvmin, newRange);
   } else if (newRange) { /* find plausible range? */
     bugOut = get_range (image->Image->matx, image->PlaneNo, image->iNonlinear,
        &unvmax, &unvmin);
   }
   if (bugOut) { /* failure in histogram equalization/pixel range */
     MessageShow ( "Error in histogram equalization");
     /* Clean up and bail out on error */
     UnlockFMem(image->Image->matx->fw->fdata); /* unlock Image Memory */
     return -1;
   }

   /* scaling for range of pixel values */
   irange = unvmax - unvmin;
   if (fabs(irange)<1.0e-25)
   	  irange = 1.0;
   else
      irange = 1.0 / irange;
   c1 = (MAXCOLOR - 1.0) * irange;
   c2 = unvmin * c1 - 0.5;

/*  Copy FITS image to Pixmap */
  nx = image->iImageNx; ny = image->iImageNy;
  for (j = 0; j<ny; j++) { /* loop over rows */
/* Read patch */
      ReadPatch (image->Image->matx, 0, j, image->PlaneNo, 0, 0, 0, 0);
       error= image->Image->matx->fw->error;
       if (error)  { /* I/O error */
          MessageShow ( "Error reading FITS image");
	  /* Clean up and bail out on error */
	  /* unlock Image Memory */
	  UnlockFMem(image->Image->matx->fw->fdata); 
	  return error+10;
       }

/*  Add any offset from beginning of patch. */
        dataoff = MatrixPatch_offset(image->Image->matx, 0, j, 
				     image->PlaneNo,0,0,0,0);
	valp = lpImage + dataoff;
/* pixarray is reversed top-to-bottom with image */
       jj = ny - j - 1;
       yaddr = jj * image->iImageNx;

/* messages */
  if (verbose) {
/* warning for bad pixel range */
    percent = ((float)(j) / (float)(ny)) * 100;
    if ((percent>10) && badrange)
      {sprintf (message, "WARNING: BAD PIXEL RANGE");
       wasbad = 1;
       wierd = XmStringCreateSimple (message);
       XtVaSetValues(line3, XmNlabelString, wierd, NULL);
       if (wierd) XmStringFree(wierd); wierd = NULL;}
    else if (wasbad)
      {sprintf (message, "Pixel range now OK");
       wierd = XmStringCreateSimple (message);
       XtVaSetValues(line3, XmNlabelString, wierd, NULL);
       if (wierd) XmStringFree(wierd); wierd = NULL;}
/* Progress */
    if (percent>lastPc+4)
      {sprintf (message, "Loading %d%% done", percent);
       wierd = XmStringCreateSimple (message);
       XtVaSetValues(line2, XmNlabelString, wierd, NULL);
       if (wierd) XmStringFree(wierd); wierd = NULL;
       lastPc = percent;
/*  check for disgruntled user */
       if (CheckForCancel())
	 {/* user pulled the plug; clean up and go home */
/* mark as invalid, clear pixarray, kill working box */
/*	   image->valid = 1;  image invalid */
/*	   if (image->Image) KillImage(image->Image);  zap image */
/*	   image->Image = NULL;*/
/*	   StringFill(image->FileName, "NONE");  reset file name */
/*	   if (image->pixarray)   delete pixel array */
/*     {if (image->pixarray) XtFree((char *)image->pixarray);*/
/*	      XtFree((XtPointer)image->pixarray);*/
/*	      image->pixarray = NULL;}*/
	   WorkingCursor(False, False, NULL); /* end progress box */
	   /* unlock Image Memory */
	   UnlockFMem(image->Image->matx->fw->fdata); 
	   return 0;}}
  } /* end of progress/cancel section */

/* read line */
    for (i=0; i<nx; i++)
      { if ((i<dims[0]) && (j<dims[1]))
          val = *(valp++);
        else 
          val = blanked;  /* blank fill edge? */
	if (val!=blanked)   /* Set color index. */
	  {if (val>truemax) truemax = val;
	   if (val<truemin) truemin = val;
           if (image->iNonlinear==1)  {/* square root */
	      if (val<unvmin) val=unvmin;
	      icol =  (((MAXCOLOR)-1.0) *
		sqrt(((val-unvmin) * irange))+0.5);
           } else if (image->iNonlinear==2) { /* histogram equalization */
               icol = map_pixel (image->Image->matx, val);
	   } else  /* Linear */
                icol = c1 * val - c2;
	   if (icol<1) icol = 1;  /* minimum color = 1 */
	   /*if (icol>=MAXCOLOR) icol = MAXCOLOR-1;*/
	   /* stay away from top edge of range */
	   if (icol>=MAXCOLOR-1) icol = MAXCOLOR-2;
	 }
	else
	  icol = 0;  /* blanked pixel */
/* pixel range check */
	badrange = badrange && ((icol<=1) || (icol==(MAXCOLOR-1)));
/* save pixel value */
	addr = yaddr + i;
	*(image->pixarray+addr) = IDdata->colut[icol];
      } /* end of loop over row */
   }  /* end of loop over image plane. */

/* debug */
/*fprintf (stderr,"FITS2Pix: pasize=%lu, max address = %lu\n",pasize, maxaddr);*/

  UnlockFMem(image->Image->matx->fw->fdata); /* unlock Image Memory */

/* use true max, min to update image info */
   image->data_min = truemin * scale + offset;
   image->data_max = truemax * scale + offset;


/* end progress message/cancel box */
  WorkingCursor(False, False, NULL);

  return 0;
}  /* end of FITS2Pix */

void StopFITSLoadButCB (Widget dialog, XtPointer clientData, 
			XtPointer callData)
/* User hit "cancel" button */
{
  stopped = True;
} /* end StopFITSLoad */

void WorkingCursor(int on, int verbose, char* filename)
/* if verbose create working dialog else change cursor to hourglass */
{
  static int locked, boxup=0;
  static Cursor cursor=(Cursor)NULL;
  extern Widget shell;
  XSetWindowAttributes attrs;
  Display *dpy = XtDisplay(shell);
  XEvent event;
  Arg args[1];
  XmString str = NULL;
  int  newdialog;
  Widget form;
  char cstring[120];

/* keep track of how many times this is "locked" */
  on? locked++:locked--;
if (locked>1 || locked ==1 && on ==0)
  return; /* already locked */
  if (on) stopped = False; /* initialize */
  if (!cursor) /* make sure hourglass cursor initialized */
    cursor = XCreateFontCursor(dpy, XC_watch);
/* if on and verbose bring up working dialog */
  if (on && verbose)
    {
/* make working dialog box */
      newdialog = !dialog;
      if (newdialog)
	{
	  dialog = XtVaCreatePopupShell ("Load FITS", 
					 xmDialogShellWidgetClass, 
					 shell, 
					 XmNwidth,     250,
					 XmNheight,    100,
					 XmNdeleteResponse, XmDESTROY,
					 NULL);
	  /* make Form widget to stick things on */
	  form = XtVaCreateManagedWidget ("WorkingForm", xmFormWidgetClass,
					  dialog,
					  XmNwidth,     250,
					  XmNheight,    100,
					  XmNx,           0,
					  XmNy,           0,
					  NULL);

	  /* info label widgets */
	  sprintf (cstring, "FITS file %s",filename);
	  str = XmStringCreateSimple (cstring);
	  line1 = XtVaCreateManagedWidget ("Line1", xmLabelWidgetClass, 
					   form, 
					   XmNlabelString,   str,
					   XmNtopAttachment, XmATTACH_FORM,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNleftAttachment,  XmATTACH_FORM,
					   NULL);
	  if (str) XmStringFree(str); str = NULL;

	  sprintf (cstring, "Loading 0%% done");
	  str = XmStringCreateSimple (cstring);
	  line2 = XtVaCreateManagedWidget ("Line2", xmLabelWidgetClass, 
					   form, 
					   XmNlabelString,   str,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget,     line1,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNleftAttachment,  XmATTACH_FORM,
					   NULL);
	  if (str) XmStringFree(str); str = NULL;
	  
	  sprintf (cstring, "  ");
	  str = XmStringCreateSimple (cstring);
	  line3 = XtVaCreateManagedWidget ("Line3", xmLabelWidgetClass, 
					   form, 
					   XmNlabelString,   str,
					   XmNtopAttachment, XmATTACH_WIDGET,
					   XmNtopWidget,     line2,
					   XmNrightAttachment, XmATTACH_FORM,
					   XmNleftAttachment,  XmATTACH_FORM,
					   NULL);
	  if (str) XmStringFree(str); str = NULL;

	  /* Cancel button */
	  CancelButton = XtVaCreateManagedWidget ("Cancel", 
				        xmPushButtonWidgetClass, 
					form, 
					XmNbottomAttachment,XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNleftAttachment,  XmATTACH_FORM,
					NULL);
	  XtAddCallback (CancelButton, XmNactivateCallback, 
			 StopFITSLoadButCB,  NULL);
/* Shazam appear: */
	  XtManageChild (dialog);
	  boxup = True;
	} /* end create box */
      else /* dialog exists - update */
	{
	  sprintf (cstring, "FITS file %s",filename);
	  str = XmStringCreateSimple (cstring);
	  XtVaSetValues(line1, XmNlabelString, str, NULL);
	  if (str) XmStringFree(str); str = NULL;

	  sprintf (cstring, "Loading 0%% done");
	  str = XmStringCreateSimple (cstring);
	  XtVaSetValues(line2, XmNlabelString, str, NULL);
	  if (str) XmStringFree(str); str = NULL;
	  
	  sprintf (cstring, "  ");
	  str = XmStringCreateSimple (cstring);
	  XtVaSetValues(line3, XmNlabelString, str, NULL);
	  if (str) XmStringFree(str); str = NULL;

/* Shazam appear: */
	  XtMapWidget (dialog);
	  boxup = True;
	  XFlush(dpy);
	} /* end refil box labels */
    } /* end of start up working dialog */
/* only change cursor */
  else if (on && (!verbose))
    {
      boxup = False;
      attrs.cursor = on ? cursor : None;
      XChangeWindowAttributes(dpy, XtWindow(shell), CWCursor, &attrs);
      XFlush(dpy);
    } /* end of change cursor */
/* done with working dialog */
  else if (!on)
    {
/* reset cursor */
      if (!boxup) {
	attrs.cursor = on ? cursor : None;
	XChangeWindowAttributes(dpy, XtWindow(shell), CWCursor, &attrs);
      }
      XFlush(dpy);
/* kill box if user aborted load - I haven't a clue as to why this is needed */
      if (dialog) { 
      XtDestroyWidget(dialog);
      dialog = NULL;}

/* hide dialog */
/*      if (dialog&&boxup) XtUnmapWidget (dialog); */
      boxup = False;
    } /* end of shutdown/reset */
} /* end WorkingCursor */

Boolean CheckForCancel()
/* See if user hit cancel button */
{
  extern Widget shell;
  Display *dpy = XtDisplay(shell);
  Window win = XtWindow(CancelButton);
  XEvent event;

/* Make sure all our requests get to the server */
  XFlush(dpy);

/* take care of pending expose events */
  XmUpdateDisplay(shell);

/* check the event queue for the cancel button */
  while(XCheckMaskEvent(dpy,
			ButtonPressMask | ButtonReleaseMask | 
			ButtonMotionMask | PointerMotionMask |
			KeyPressMask | KeyReleaseMask,
			&event)) { /* caught possibly intresting one */
    if (event.xany.window==win)
      {XtDispatchEvent(&event); /* it's in the cancel button - do it */
      stopped = True;}
/*    else */ /* throw it away and ring the bell */
/*      XBell(dpy, 50); */
  }
  return stopped;
} /* End CheckForCancel */
