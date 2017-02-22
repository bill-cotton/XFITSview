/*    XFITSview : a FITS image viewer for X-windows */
/* This program requires the Motif library */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996-1999
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
*
*  You should have received a copy of the GNU General Public
*  License along with this program; if not, write to the Free
*  Software Foundation, Inc., 675 Massachusetts Ave, Cambridge,
*  MA 02139, USA.
*
*  Correspondence concerning XFITSview should be addressed as follows:
*         Internet email: bcotton@nrao.edu.
*         Postal address: William Cotton
*                         National Radio Astronomy Observatory
*                         520 Edgemont Road
*                         Charlottesville, VA 22903-2475 USA
*-----------------------------------------------------------------------*/
#define XFITSVIEWMAIN
#include <Xm/Xm.h> 
#include <Xm/DrawingA.h> 
#include <Xm/MainW.h> 
#include <stdlib.h>
#include <stdio.h>
#include <Xm/Separator.h>
#include <Xm/ScrolledW.h>
#include <Xm/Scale.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include "xfitsview.h"
#include "imagedisp.h"
#include "FITS2Pix.h"
#include "color.h"
#include "optionbox.h"
#include "moviebox.h"
#include "blinkbox.h"
#include "infobox.h"
#include "control.h"
#include "markpos.h"
#include "cursor.h"
#include "textfile.h"
#include "scrolltext.h"
#include "menu.h"
#include "messagebox.h"
#include "toolbox.h"


/* internal functions */
void InitImage (ImageDisplay *IDdata, int narg, char *filename);


/* stuff for error messages */
int hwndErr = 1;
char szErrMess[120];

int main ( int argc, char **argv ) 
{
  Widget       mainWindow, menu, control, toolbox, form;
  XtAppContext app;
  ImageData    image;
  ImageDisplay *IDdata;
  Arg          args[10];
  int          n;

/*  initialize toolkit */
  Display_shell = XtAppInitialize (&app, 
				   "XFitsview", 
				   NULL, 0, &argc, argv,  NULL, NULL, 0); 
/* Set initial size */
  n = 0;
/*  XtSetArg(args[n], XtNwidth, 640);  n++;
  XtSetArg(args[n], XtNheight, 480);  n++; */

/* create main window */
  mainWindow = XtCreateManagedWidget ("mainWindow", xmMainWindowWidgetClass,
                                         Display_shell, args, n);
/* make a form to hang everything on*/
  form = XtVaCreateManagedWidget ("form", xmFormWidgetClass,
				  mainWindow,
				  XmNwidth,     640,
				  XmNheight,    480,
				  XmNx,           0,
				  XmNy,           0,
				  NULL);


/* make image display window */
  IDdata = MakeDisplay (form, Display_shell);

/*  make display control panel */
  control = MakeControl (form, (XtPointer)IDdata);

/*  make toolbox panel */
  /* not ready for prime time */
  /*  toolbox = MakeToolbox (form, control, (XtPointer)IDdata); */

/* attach image display to control on form */
     XtVaSetValues(IDdata->display, 
		   XmNrightAttachment,  XmATTACH_FORM,
		   XmNleftAttachment,  XmATTACH_WIDGET,
		   XmNleftWidget,  control,
		   XmNtopAttachment,  XmATTACH_FORM,
		   XmNbottomAttachment,  XmATTACH_FORM,
		   NULL);

/*  make main menu */
  menu = MakeMainMenu (mainWindow, (XtPointer)&image, 
		       (XtPointer)IDdata);

/*  save some widget names */
  XtVaSetValues ( mainWindow, XmNmenuBar, menu, NULL );

/*  Presto - appear on the screen */
  XtRealizeWidget (Display_shell);

/*  create / init color map, cursor */
  SetupColorMap (Display_shell, IDdata);

/* set cursor if possible */
  IDdata->cursor = MakeImageCursor (XtDisplay (IDdata->display), 
				    XtWindow(IDdata->canvas));
  if (IDdata->cursor)
    XDefineCursor (XtDisplay (IDdata->display), XtWindow(IDdata->canvas),
		   IDdata->cursor);
    

/* initialize image, display file given as argument */
  InitImage (IDdata, argc, argv[1]);

/*   set display */
  SetDisplay(IDdata);

/* save application context */
  IDdata->app = app;

/* redraw message box if it contains any start up messages */
  MessageRefresh();

/* main event loop */
  XtAppMainLoop (app);
} /* end of main */

void InitImage (ImageDisplay *IDdata, int narg, char *filename)
/* initialize image data structures */
{
  int i,j,k;
  char direct[120];

  CurImag = 0; /* set current image pointer */
  FITS_dir = NULL;
  mark_dir = NULL;
  log_dir = NULL;
  doLog = 0;   /* position logging initially turned off */
  usr_equinox = -1.0;  /* use equinox of image */
/* Initialize structures */
  for (j=0;j<2;j++) {
    image[j].valid = 0;
    image[j].pixarray = NULL;
    image[j].FileName = MakeString("NONE");
    image[j].Image = NULL;
    image[j].object = MakeString("Unknown");
    image[j].units = MakeString("Unknown");
    image[j].obsdate = MakeString("Unknown");
    image[j].data_max = 0.0;
    image[j].data_min = 0.0;
    image[j].epoch = 0.0;
    image[j].ndim = 0;
    for (i=0; i<7; i++) {
      image[j].cname[i] = MakeString("none");
      image[j].dim[i] = 0;
      image[j].crval[i] = 0.0;
      image[j].crpix[i] = 0.0;
      image[j].crinc[i] = 0.0;
      image[j].crot[i] = 0.0;
    }
    image[j].iImageNx = 0;
    image[j].iImageNy = 0;
    image[j].iNumPlanes = 0;
    image[j].iXPixel = 0;
    image[j].iYPixel = 0;
    image[j].fXpixel = 0.0;
    image[j].fYpixel = 0.0;
    image[j].PixRange[0] = 0.0;
    image[j].PixRange[1] = 0.0;
    image[j].fBpixel = 0;
    image[j].iNonlinear = 0;
    image[j].PlaneNo = 0;
  } /* end of loop over ImageData structures */
/* was a FITS file name passed as an argument? */
  if (narg<2) return;

/* read FITS file to pixmap */
  StringFill (image[CurImag].FileName, filename);
/* read FITS file to PixMap */
  if (FITS2Pix (&image[CurImag], IDdata, 1))
    { /* error */
      sprintf (szErrMess, "Error reading FITS file = %80s", filename);
      MessageShow (szErrMess);
    }
/* get directory name */
  j = strlen(filename);
  k = 0;
  for (i=0;i<j;i++) if (filename[i]=='/') k = i+1;
  for (i=0;i<k;i++) direct[i]=filename[i]; direct[i]=0;
  FITS_dir = MakeString(direct);
  PaintImage(IDdata); /* draw image */
} /* end of InitImage */

