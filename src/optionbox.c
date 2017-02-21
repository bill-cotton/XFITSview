/* Option dialog box  for XFITSview */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,2002
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
#include <Xm/Xm.h> 
#include <Xm/DialogS.h> 
#include <Xm/MainW.h> 
#include <Xm/Scale.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/MessageB.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include "imagedisp.h"
#include "messagebox.h"
#include "FITS2Pix.h"

/* is the option box active? */
int OptionBoxActive = 0;

/* global structure for things to talk to each other */
typedef struct {
  ImageDisplay *BoxData;
  Widget dialog, pixran1, pixran2, planelab, data1, data2; /* box, min, max */
  Widget plane; /* Plane Number */
  short  xpos, ypos; /* location of the box */
} OptionBoxStuff;
OptionBoxStuff dia;

void ReadMinCB (Widget w, XtPointer clientData, XtPointer callData)
/* get minimum pixel value */
{
  ImageDisplay *IDdata = (ImageDisplay *)clientData;
  char     *value=NULL;
  float    temp;

/* read value */
  value = XmTextGetString (w);
  if (!value) /* error */
    {MessageShow ("Error reading minimum pixel value");
     return;}
  if (!sscanf (value, "%e", &temp))
   { /* error */
     MessageShow ("Error reading minimum pixel value");
     if (value) XtFree(value); value = NULL;
     return;}
  if (value) XtFree(value);value = NULL;

/* OK, save */
  image[CurImag].PixRange[0] = temp;

} /* end ReadMinCB */

void ReadMaxCB (Widget w, XtPointer clientData, XtPointer callData)
/* get maximum pixel value */
{
  ImageDisplay *IDdata = (ImageDisplay *)clientData;
  char     *value=NULL;
  float    temp;

/* read value */
  value = XmTextGetString (w);
  if (!value) /* error */
    {MessageShow ("Error reading maximum pixel value");
     return;}
  if (!sscanf (value, "%e", &temp))
   { /* error */
     MessageShow ("Error reading maximum pixel value");
     if (value) XtFree(value); value = NULL;
     return;}
  if (value) XtFree(value); value = NULL;

/* OK, save */
  image[CurImag].PixRange[1] = temp;

} /* end ReadMaxCB */

void ReadPlaneCB (Widget w, XtPointer clientData, XtPointer callData)
/* read Plane number */
{
  ImageDisplay *IDdata = (ImageDisplay *)clientData;
  char     *value=NULL;
  int      itemp;

/* read value */
  value = XmTextGetString (w);
  if (!value) /* error */
    {MessageShow ("Error reading plane number");
     return;}
  if (!sscanf (value, "%d", &itemp))
   { /* error */
     MessageShow ("Error reading plane number");
     if (value) XtFree(value); value = NULL;
     return;}
  if (value) XtFree(value); value = NULL;

/* internally 0 rel; externally 1 rel */
  itemp--;

/* check value */
  if ((image[CurImag].valid) && 
      ((itemp<0) || (itemp>=image[CurImag].iNumPlanes)))
    { /* error */
      MessageShow ("Error: plane number out of range");
      return;}

/* OK, save value */
  image[CurImag].PlaneNo = itemp;

} /* end ReadPlaneCB */

void MapFnCB (Widget w, int which, XmToggleButtonCallbackStruct *state)
/* Set mapping function type; which =0=>linear, 1=>nonlinear, 2=>hist. eq. */
{
  if (state->set) image[CurImag].iNonlinear = which;
} /* end MapFnCB */

/* button callbacks */
void OptOKButCB (Widget w, XtPointer clientData, XtPointer callData)
/* OK button hit */
{

  ReadMinCB (dia.data1, (XtPointer)dia.BoxData, NULL);
  ReadMaxCB (dia.data2, (XtPointer)dia.BoxData, NULL);
  ReadPlaneCB (dia.plane, (XtPointer)dia.BoxData, NULL);

  /* make it disappear but still exist */
  XtPopdown(dia.dialog);

} /* end OptOKButCB */

void OptCancelButCB (Widget w, XtPointer clientData, XtPointer callData)
/* Cancel button hit */
{
  /* make it disappear but still exist */
  XtPopdown(dia.dialog);
  
} /* end OptCancelButCB */

void OptReloadButCB (Widget w, XtPointer clientData, XtPointer callData)
/* Reload button hit */
{
  int scrollx, scrolly;
  ReadMinCB (dia.data1, (XtPointer)dia.BoxData, NULL);
  ReadMaxCB (dia.data2, (XtPointer)dia.BoxData, NULL);
  ReadPlaneCB (dia.plane, (XtPointer)dia.BoxData, NULL);

/* reload current file */
  if (!image[CurImag].valid) return;  /* tests for valid image */
  if (!image[CurImag].FileName) return;  /* tests for valid name */
  if (!image[CurImag].FileName->sp) return;
  if (!image[CurImag].FileName->length) return;

/* save old scroll */
  scrollx = dia.BoxData->scrollx;
  scrolly = dia.BoxData->scrolly; 

  if (FITS2Pix (&image[CurImag], dia.BoxData, 1))
    {/* error */
      sprintf (szErrMess, "Error reading FITS file = %s", 
		image[CurImag].FileName->sp);
      MessageShow(szErrMess);
    }

/* reset scroll */
  dia.BoxData->scrollx = scrollx;
  dia.BoxData->scrolly = scrolly; 

/* reset display */
  PaintImage(dia.BoxData);

} /* end OptReloadButCB */

void OptionBoxCB (Widget parent, XtPointer clientData, XtPointer callData)
/* create dialog box for Options */
{
  Widget form=NULL, label1=NULL, label2=NULL, label3=NULL;
  Widget radio=NULL, sep=NULL;
  Widget OKbutton=NULL, CancelButton=NULL, ReloadButton=NULL;
  XmString     label = NULL, minpix = NULL, maxpix = NULL, pixran = NULL;
  XmString     linear = NULL, nonlinear = NULL, histEq=NULL;
  XmString     plalab = NULL, PixelStr = NULL, wierdstring = NULL;
  char         valuestr[60];
  ImageDisplay *IDdata = (ImageDisplay*)clientData;
  int          start;
  Arg          wargs[5]; 
#define OPTIONBOX_WIDTH 160


/* register IDdata */
  dia.BoxData = IDdata;

  /* don't make another one */
  if (OptionBoxActive) {
    if (XtIsRealized (dia.dialog))
      XMapRaised (XtDisplay(dia.dialog), XtWindow(dia.dialog));

    /* bring it back where we can see it */
    XtPopup(dia.dialog, XtGrabNonexclusive);
    
    /* put it some place reasonable */
    /*  where is parent? */
    XtVaGetValues (IDdata->shell, XmNx, &dia.xpos, XmNy, &dia.ypos,  NULL);
    dia.ypos += 160;
    if (dia.xpos<0) dia.xpos = 0;
    XMoveWindow (XtDisplay(IDdata->shell), XtWindow(dia.dialog), 
		 dia.xpos, dia.ypos);

    /* set values */
    /* range of values in plane - labels are difficult */
    sprintf (valuestr, "%g %g", image[CurImag].data_min, 
	     image[CurImag].data_max);
    wierdstring = XmStringCreateSimple (valuestr);
    XtSetArg (wargs[0], XmNlabelString, wierdstring);
    XtSetValues (dia.pixran2, wargs, 1);
    if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;

    /* range of planes in image */
    sprintf (valuestr, "Plane no. (1 - %d)", image[CurImag].iNumPlanes);
    wierdstring = XmStringCreateSimple (valuestr);
    XtSetArg (wargs[0], XmNlabelString, wierdstring);
    XtSetValues (dia.planelab, wargs, 1);
    if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;

    /* current min */
    sprintf (valuestr, "%f", image[CurImag].PixRange[0]);
    XmTextSetString (dia.data1, valuestr);

     /* current max */
    sprintf (valuestr, "%f", image[CurImag].PixRange[1]);
    XmTextSetString (dia.data2, valuestr);

    /* current plane number */
    sprintf (valuestr, "%d", image[CurImag].PlaneNo+1);
    XmTextSetString (dia.plane, valuestr);

    return;
  } /* end of update dialog */

  label = XmStringCreateSimple ("Options panel");
  minpix = XmStringCreateSimple ("Minimum pixel value");
  maxpix = XmStringCreateSimple ("Maximum pixel value");
  pixran = XmStringCreateSimple ("Pixel range in plane");
  linear = XmStringCreateSimple ("linear");
  nonlinear = XmStringCreateSimple ("nonlinear");
  histEq = XmStringCreateSimple ("Histogram Equalization");
/* mark as active */
  OptionBoxActive = 1;

  dia.dialog = XtVaCreatePopupShell ("OptionBox", xmDialogShellWidgetClass, 
				 IDdata->shell, 
				 XmNautoUnmanage, False,
				 XmNwidth,     OPTIONBOX_WIDTH,
				 XmNheight,    310,
				 XmNdeleteResponse, XmDESTROY,
				 NULL);

/* make Form widget to stick things on */
  form = XtVaCreateManagedWidget ("OptionForm", xmFormWidgetClass,
				  dia.dialog,
				  XmNautoUnmanage, False,
				  XmNwidth,     OPTIONBOX_WIDTH,
				  XmNheight,    310,
				  XmNx,           0,
				  XmNy,           0,
				  NULL);

/* info label widgets */
  label1 = XtVaCreateManagedWidget ("Label1", xmLabelWidgetClass, 
				    form, 
				    XmNwidth,           OPTIONBOX_WIDTH,
				    XmNlabelString,   label,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

/* actual pixel range in plane */
  dia.pixran1 = XtVaCreateManagedWidget ("PixelRange1", xmLabelWidgetClass,
				    form,
				    XmNwidth,           OPTIONBOX_WIDTH,
				    XmNlabelString,   pixran,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     label1,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

  sprintf (valuestr, "%g %g", image[CurImag].data_min, 
	   image[CurImag].data_max);
  PixelStr = XmStringCreateSimple (valuestr);
  dia.pixran2 = XtVaCreateManagedWidget ("PixelRange2", xmLabelWidgetClass, 
				    form, 
				    XmNwidth,           OPTIONBOX_WIDTH,
				    XmNlabelString,   PixelStr,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     dia.pixran1,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);
/* minimum pixrange */
  label2 = XtVaCreateManagedWidget ("OptionLabel2", xmLabelWidgetClass,
				    form,
				    XmNwidth,           OPTIONBOX_WIDTH,
				    XmNlabelString,   minpix,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     dia.pixran2,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

  sprintf (valuestr, "%f", image[CurImag].PixRange[0]);
  dia.data1 = XtVaCreateManagedWidget ("OptionData1", xmTextFieldWidgetClass, 
				    form, 
				    XmNwidth,           OPTIONBOX_WIDTH,
				    XmNvalue,   valuestr,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     label2,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

/* maximum pixrange */
  label3 = XtVaCreateManagedWidget ("OptionLabel3", xmLabelWidgetClass,
				    form,
				    XmNwidth,           OPTIONBOX_WIDTH,
				    XmNlabelString,   maxpix,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     dia.data1,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

  sprintf (valuestr, "%f", image[CurImag].PixRange[1]);
  dia.data2 = XtVaCreateManagedWidget ("OptionData2", xmTextFieldWidgetClass, 
				    form, 
				    XmNwidth,           OPTIONBOX_WIDTH,
				    XmNvalue,   valuestr,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     label3,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

/* Plane number */
  sprintf (valuestr, "Plane no. (1 - %d)", image[CurImag].iNumPlanes);
  plalab = XmStringCreateSimple (valuestr);
  dia.planelab = XtVaCreateManagedWidget ("OptionLabel3", xmLabelWidgetClass,
				    form,
				    XmNwidth,           OPTIONBOX_WIDTH,
				    XmNlabelString,   plalab,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     dia.data2,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

  sprintf (valuestr, "%d", image[CurImag].PlaneNo+1);
  dia.plane = XtVaCreateManagedWidget ("OptionData2", xmTextFieldWidgetClass, 
				    form, 
				    XmNwidth,           OPTIONBOX_WIDTH,
				    XmNvalue,   valuestr,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     dia.planelab,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);
/* separator */
    sep = XtVaCreateManagedWidget ("sep", xmSeparatorWidgetClass,
				   form, 
				   XmNwidth,           OPTIONBOX_WIDTH,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget,     dia.plane,
				   XmNleftAttachment,  XmATTACH_FORM,
				   NULL);
/* linear/nonlinear/histogram eq. radio buttons */
  start = image[CurImag].iNonlinear;
  radio = XmVaCreateSimpleRadioBox(form, "Mapping_type", start, 
				   (XtCallbackProc)MapFnCB,
				   XmNwidth,           OPTIONBOX_WIDTH,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget,     sep,
				   XmNleftAttachment,  XmATTACH_FORM,
				   XmVaRADIOBUTTON, linear, NULL, NULL, NULL,
				   XmVaRADIOBUTTON, nonlinear, NULL,NULL,NULL,
				   XmVaRADIOBUTTON, histEq, NULL,NULL,NULL,
				   NULL);
  XtManageChild(radio);

/* OK button */
  OKbutton = XtVaCreateManagedWidget (" OK ", xmPushButtonWidgetClass, 
				    form, 
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);
  XtAddCallback (OKbutton, XmNactivateCallback, OptOKButCB, (XtPointer)IDdata);

/* Cancel button */
  CancelButton = XtVaCreateManagedWidget ("Cancel", xmPushButtonWidgetClass, 
				    form, 
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNleftWidget,     OKbutton,
				    NULL);
  XtAddCallback (CancelButton, XmNactivateCallback, OptCancelButCB, 
		 (XtPointer)IDdata);

/* Reload button */
  ReloadButton = XtVaCreateManagedWidget ("Reload", xmPushButtonWidgetClass, 
				    form, 
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNleftWidget,     CancelButton,
				    NULL);
  XtAddCallback (ReloadButton, XmNactivateCallback, OptReloadButCB, 
		 (XtPointer)IDdata);

  if (label) XmStringFree(label);  label = NULL;
  if (minpix) XmStringFree(minpix); minpix = NULL;
  if (maxpix) XmStringFree(maxpix); maxpix = NULL;
  if (pixran) XmStringFree(pixran); pixran = NULL;
  if (linear) XmStringFree(linear); linear = NULL;
  if (nonlinear) XmStringFree(nonlinear); nonlinear = NULL;
  if (histEq) XmStringFree(histEq); histEq = NULL;
  if (plalab) XmStringFree(plalab); plalab = NULL;
  if (PixelStr) XmStringFree(PixelStr); PixelStr = NULL;

/* set it up */
  XtManageChild (dia.dialog);

/* put it some place reasonable */
/*  where is parent? */
     XtVaGetValues (IDdata->shell,
		    XmNx, &dia.xpos,
		    XmNy, &dia.ypos,
		    NULL);
  dia.ypos += 170;
  dia.xpos -= 10;
  if (dia.xpos<0) dia.xpos = 0;
  XMoveWindow (XtDisplay(IDdata->shell), XtWindow(dia.dialog), 
	       dia.xpos, dia.ypos);
} /* end OptionBox */

