/* Information dialog box for XFITSview */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1997,1999,2002
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

#include <string.h>
#include <stdio.h>
#include <Xm/Xm.h> 
#include <Xm/DialogS.h> 
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include "imagedisp.h"
#include "infobox.h"
#include "wpos.h"

/* is the info box active? */
int InfoBoxActive = 0;

/* global structure for things to talk to each other */
typedef struct {
  Widget dialog;
  Widget parent;
  ImageDisplay *IDdata;
} InfoBoxStuff;
InfoBoxStuff Info;

void NextInfoLine (Widget form, Widget prev, Widget *line, 
		   char* string, int width)
/* add label widget with string at end of form; */
{
  XmString     xlinestr = NULL;

/* convert to wierd X-Windows form */
  xlinestr = XmStringCreateSimple (string);

  *line = XtVaCreateManagedWidget ("NextLine", xmLabelWidgetClass,
				   form,
				   XmNwidth,         width,
				   XmNlabelString,   xlinestr,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget,     prev,
				   XmNleftAttachment,  XmATTACH_FORM,
				   NULL);
  if (xlinestr) XmStringFree(xlinestr); xlinestr = NULL;
} /* end NextInfoLine */

/* button callbacks */
void DismissButCB (Widget w, XtPointer clientData, XtPointer callData)
/* Dismiss button hit */
{
  XtDestroyWidget (Info.dialog);
  InfoBoxActive = 0; /* mark as inactive */
} /* end DismissButCB */

void RefreshButCB (Widget w, XtPointer clientData, XtPointer callData)
/* Refresh button hit */
{
/* some validity checks */
  if (!Info.IDdata) return;
  if (!image[CurImag].valid) return;

/* destroy old */
  XtDestroyWidget (Info.dialog);
  InfoBoxActive = 0; /* mark as inactive */

/* make new */
  InfoBoxCB (Info.parent, (XtPointer)Info.IDdata, (XtPointer)NULL);

} /* end RefreshButCB */

void InfoBoxCB (Widget parent, XtPointer clientData, XtPointer callData)
/* create dialog box for source information */
{
  Widget       form, line[15];
  Widget       DismissButton, RefreshButton;
  char         linestr[500], *axislab, coordst[50], cs[50];
  float        incr;
  int          loop, iLine, iMaxLine=14;
  ImageDisplay *IDdata = (ImageDisplay*)clientData;
/*  ImageData    *image = (ImageData*) IDdata->image;*/
  int          width = 400; /* width of box */
  XmString     label = NULL;

/* some validity checks */
  if (!IDdata) return;
  if (!image[CurImag].valid) return;

/* register info for use by refresh */
  Info.IDdata = IDdata;
  Info.parent = parent;

  /* don't make another one */
  if (InfoBoxActive) {
    if (XtIsRealized (Info.dialog))
	XMapRaised (XtDisplay(Info.dialog), XtWindow(Info.dialog));
    return;
  }

/* mark as active */
  InfoBoxActive = 1; 

  Info.dialog = XtVaCreatePopupShell("Source information", 
				     xmDialogShellWidgetClass, 
				     IDdata->shell, 
				     XmNwidth,     width,
				     XmNheight,    300,
				     XmNdeleteResponse, XmDESTROY,
				     NULL);

/* make Form widget to stick things on */
  form = XtVaCreateManagedWidget ("InfoForm", xmFormWidgetClass,
				  Info.dialog,
				  XmNwidth,     width,
				  XmNheight,    300,
				  XmNx,           0,
				  XmNy,           0,
				  NULL);
/* source info */ 
/* file name */
  sprintf(linestr, "Information about %s ",image[CurImag].FileName->sp);
  label = XmStringCreateSimple (linestr);
  line[0] = XtVaCreateManagedWidget ("FirstLine", xmLabelWidgetClass,
				       form,
				       XmNwidth,         width,
				       XmNlabelString,   label,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNleftAttachment,  XmATTACH_FORM,
				       NULL);
  if (label) XmStringFree(label); label = NULL;
  iLine = 0;
/* object name, units */
  sprintf(linestr, "object = %s  units = %s",
	  image[CurImag].object->sp, image[CurImag].units->sp);
  NextInfoLine (form, line[iLine], &line[iLine+1], linestr, width);
  iLine++;

/* max., min. values in plane */
  sprintf(linestr, "max. value = %f    min value = %f ",
	  image[CurImag].data_max, image[CurImag].data_min);
  NextInfoLine (form, line[iLine], &line[iLine+1], linestr, width);
  iLine++;

/* observing date, coordinate equinox */
  sprintf(linestr, "Observing date = %s  coordinate equinox= %6.1f",
	  image[CurImag].obsdate->sp, image[CurImag].Image->descript->equinox);
  NextInfoLine (form, line[iLine], &line[iLine+1], linestr, width);
  iLine++;

/* axis info label line */
  sprintf(linestr," Axis  type     dim     value      ref. pix   inc.      rot.   ");
  NextInfoLine (form, line[iLine], &line[iLine+1], linestr, width);
  iLine++;
/*    1 RA---SIN 1024  hh mm ss.sss     512.0  100.000     0.0 */

/* loop over axes */
  for (loop=0;loop<image[CurImag].ndim;loop++)
    {axislab =  image[CurImag].cname[loop]->sp;
     if ((!strncmp (axislab, "RA  ", 4)) ||
	 (!strncmp (axislab, "RA--", 4)) ||
	 (!strncmp (axislab, "LL  ", 4)))
/* check useage of cs in ra2hms */
       {ra2hms(image[CurImag].crval[loop], axislab, cs);
	incr = image[CurImag].crinc[loop] * 3600.0; /* increment in arcsec. */
	sprintf(linestr, "%4d %8.8s%5ld %13.13s%10.1f%9.3f%8.1f",
		loop+1, axislab, image[CurImag].dim[loop],
		cs+4, image[CurImag].crpix[loop],
		incr, image[CurImag].crot[loop]);}
     else  if ((!strncmp (axislab, "DEC", 3)) || 
	       (!strncmp (axislab, "GLON", 4)) ||
	       (!strncmp (axislab, "GLAT", 4)) ||
	       (!strncmp (axislab, "ELON", 4)) ||
	       (!strncmp (axislab, "ELAT", 4)) ||
	       (!strncmp (axislab, "MM  ", 4)))  
       {dec2dms(image[CurImag].crval[loop], axislab, cs);
	incr = image[CurImag].crinc[loop] * 3600.0; /* increment in arcsec. */
	sprintf(linestr, "%4d %8.8s%5ld %13.13s%10.1f%9.3f%8.1f",
		loop+1, axislab, image[CurImag].dim[loop],
		cs+4, image[CurImag].crpix[loop],
		incr, image[CurImag].crot[loop]);}
     else   /* other axes */
       {sprintf(linestr, "%4d %8.8s%5ld %13g%10.1f%9g%8.1f",
		loop+1, axislab, image[CurImag].dim[loop],
		image[CurImag].crval[loop], image[CurImag].crpix[loop],
		image[CurImag].crinc[loop], image[CurImag].crot[loop]);} /* end type */
     NextInfoLine (form, line[iLine], &line[iLine+1], linestr, width);
     iLine++;
     if (iLine>=iMaxLine) break;
     
   }  /* end of axis loop */
/* tell coordinate type */
  if (iLine<iMaxLine) {
    if (image[CurImag].Image->descript->isDSS) {
      sprintf (linestr, "Using DSS coordinates");
      NextInfoLine (form, line[iLine], &line[iLine+1], linestr, width);
      iLine++;
    }
    else if (image[CurImag].Image->descript->isIRAF) {
      sprintf (linestr, "Using IRAF coordinates");
      NextInfoLine (form, line[iLine], &line[iLine+1], linestr, width);
      iLine++;
    }
  } /* end coordinate type label */

/* Dismiss button */
  DismissButton = 
    XtVaCreateManagedWidget ("Dismiss", xmPushButtonWidgetClass, 
			     form, 
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNleftAttachment,  XmATTACH_FORM,
			     NULL);
  XtAddCallback (DismissButton, XmNactivateCallback, DismissButCB, 
		 (XtPointer)IDdata);
  
/* Refresh button */
  RefreshButton = 
    XtVaCreateManagedWidget ("Refresh", xmPushButtonWidgetClass, 
			     form, 
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     NULL);
  XtAddCallback (RefreshButton, XmNactivateCallback, RefreshButCB, 
		 (XtPointer)IDdata);

/* set it up */
  XtManageChild (Info.dialog);
} /* end OptionBox */

