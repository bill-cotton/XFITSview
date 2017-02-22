/* lookup position from index, Equinox dialog boxes for XFITSview */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1998,2002,2017
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
#include <Xm/Xm.h> 
#include <Xm/DialogS.h> 
#include <Xm/MainW.h> 
#include <Xm/ScrollBar.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/MessageB.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/FileSB.h>
#include <math.h>
#include "xfitsview.h"
#include "imagedisp.h"
#include "FITS2Pix.h"
#include "position.h"
#include "wpos.h"
#include "precess.h"
#include "lookpos.h"
#include "messagebox.h"
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

/* internal prototypes */
int LookPosFile (char* indexfilename, char* FITSfilename);
int FixFname(char *dir, char *fname, char *OKname);
void lowcase(char* string); /* lowercase string */
void upcase(char* string); /* uppercase string */

/* is the position lookup box active? */
int LookupBoxActive = 0;

/* global structure for things to talk to each other */

typedef struct {
  Widget       dialog, equlab;
  Widget       data1, data2; /* RA, Dec text windows */
  ImageDisplay *IDdata;
  double       ra, dec;
} LookPosStuff;
LookPosStuff look;

typedef struct {
  ImageDisplay *BoxData;
  Widget dialog; /* box */
} EquinoxBoxStuff;
EquinoxBoxStuff equ;
/* is the Equinox box active? */
int EquinoxBoxActive = 0;


int ReadLookRA (Widget w)
/* read RA from text window, return 0 if OK, 1 if error */
{
  char     *value=NULL;
  int      h, m, iNumRead, bOK;
  float    s;
  Display *dpy = XtDisplay(w);
  double   ra;

/* read value */
  value = XmTextGetString (w);
  if (!value) /* error */
    {MessageShow ("Error reading RA for Lookup Position");
     return 1;}
  iNumRead = sscanf (value, "%d %d %f", &h, &m, &s);
  if (value) XtFree(value); value = NULL;
  bOK = 0;
  if (iNumRead==3) bOK = !hmsra(h, m, s, &ra);
  if (!bOK)
   { /* error */
     MessageShow ("Error reading RA for Lookup Position");
     return 1;}

/* OK, save */
  look.ra = ra;
  return 0;
} /* end ReadLookRA */

int ReadLookDec (Widget w)
/* read RA from text window, return 0 if OK, 1 if error */
{
  char     *value=NULL;
  int      d, m, iNumRead, bOK, bSouth, i;
  float    s;
  double   dec;
  Display *dpy = XtDisplay(w);

/* read value */
  value = XmTextGetString (w);
  if (!value) /* error */
    {MessageShow("Error reading Dec for Lookup Position");
     return 1;}
  bOK = 0;
  iNumRead = sscanf (value, "%d %d %f", &d, &m, &s);
  /* southern declination? look for minus sign*/
  bSouth = 0;
  for (i=0;i<20;i++) 
    {if (value[i]=='-') bSouth=1; 
     if (value[i]==0) break;}
  if (value) XtFree(value); value = NULL;
  if (iNumRead==3) bOK = !dmsdec(d, m, s, &dec);
  if (!bOK)
   { /* error */
     MessageShow("Error reading Dec for Lookup Position");
     return 1;}

  if (bSouth && (dec>0.0)) dec = -dec; /* make sure declination sign OK */

/* OK, save */
  look.dec = dec;
  return 0;
} /* end ReadLookDec */

void LookupCancelButCB (Widget filebox, XtPointer clientData, 
			XtPointer callData)
/* cancel button hit- bail out */
{
  /* make it disappear but still exist */
  XtPopdown(look.dialog);
} /* end LookupCancelButCB */

void LookupOKButCB (Widget w, XtPointer clientData, XtPointer callData)
/* OK button hit - find image and load, centering on requested position */
{
  int iRet, bOK, iX, iY, iScroll, iHalf, samefile;
  float xp, yp;
  double ra, dec;
  char ctype[5], indexfilename[120], FITSfilename[120], fullFITSname[120];
  Display *dpy = XtDisplay(w);
  ImageDisplay *IDdata = (ImageDisplay*)clientData;

/* read selected values */
  if (ReadLookRA(look.data1)) return;
  if (ReadLookDec(look.data2)) return;
  ra = look.ra; dec = look.dec;  /* save unprecessed coords */

  if (FITS_dir)
    iRet = FixFname (FITS_dir->sp, "findex.txt", indexfilename);
  else
    iRet = FixFname ("./", "findex.txt", indexfilename);
  if (iRet)
    {sprintf (szErrMess, "No file = %s", indexfilename);
     MessageShow (szErrMess); return;}

/* lookup filename */
  if (LookPosFile (indexfilename, FITSfilename))
    {MessageShow("Could not find desired image - Sorry");
     return;}
  if (FITS_dir)
    iRet = FixFname (FITS_dir->sp, FITSfilename, fullFITSname);
  else
    iRet = FixFname ("./", FITSfilename, fullFITSname);
  if (iRet)
    {sprintf (szErrMess, "No file = %s", fullFITSname);
     MessageShow (szErrMess); return;}

/* load new image */
  samefile = 0;
  StringFill(image[CurImag].FileName, fullFITSname);
  if ((image[CurImag].Image) && (image[CurImag].valid)) 
    samefile = StringComp (image[CurImag].FileName, 
			   image[CurImag].Image->matx->fw->file->file_name);
  if (!samefile) /* don't reload same file */
    {if (FITS2Pix (&image[CurImag], look.IDdata, 1))
       {  /* error */
	 sprintf (szErrMess, "Error reading FITS file = %s", fullFITSname);
	 MessageShow (szErrMess);return;}}

/* convert position to pixel */
  strncpy (ctype, image[CurImag].cname[0]->sp+4, 4); ctype[4] = 0;
  iRet = get_xypix(image[CurImag].Image->descript, ra, dec, &xp, &yp);
  bOK = (iRet==0);

/*  check that pixel is in image. */
  bOK = bOK && (xp>0.0) && (yp>0.0) && 
    (xp<=(float)image[CurImag].dim[0]) &&
      (yp<=(float)image[CurImag].dim[1]);

/* everything OK? */
  if (!bOK) 
    {sprintf (szErrMess, "invalid position in image %7.2f, %7.2f", xp, yp);
     MessageShow (szErrMess);
     return;}


/* set scrollbar */ 
  iX = (int)(xp - 0.5);
  iY = (int)(yp - 0.5);
  iScroll = image[CurImag].iImageNy - iY;
  look.IDdata->scrolly = iScroll;
  look.IDdata->scrollx = iX;
  if (look.IDdata->vscr_vis) 
  {
    iScroll -= look.IDdata->vscr_half;
    iScroll = min (iScroll, look.IDdata->vscr_max);
    iScroll = max (iScroll, look.IDdata->vscr_min);
    XtVaSetValues(look.IDdata->vscroll,  XmNvalue, iScroll, NULL);
  }
  if (look.IDdata->hscr_vis) 
    {
    iScroll = iX - IDdata->hscr_half;
    iScroll = min (iScroll, look.IDdata->hscr_max);
    iScroll = max (iScroll, look.IDdata->hscr_min);
    /* set scroll bar */
    XtVaSetValues(look.IDdata->hscroll,  XmNvalue, iScroll, NULL);
  }

/* repaint image */
  ResetDisplay (look.IDdata);

} /* end PosOKButCB */

void LookPosCB (Widget parent, XtPointer clientData, XtPointer callData)
/* create dialog box for specifying position of desired image */
{
  Widget form, toplabel, label1, label2;
  Widget data1, data2, sep1, sep2;
  Widget FileButton, OKButton, CancelButton;
  XmString     RA=NULL, Dec=NULL, label=NULL, equstr=NULL;
  XmString     wierdstring = NULL;
  Arg          wargs[5]; 
  char         valuestr[30], ctype[5];
  int          h, d, m, toHours;
  short        xpos, ypos;
  float        s;
  ImageDisplay *IDdata = (ImageDisplay*)clientData;
  Display *dpy = XtDisplay(parent);


/* validity checks */
  if (!IDdata) return;

/* register IDdata */
  look.IDdata = IDdata;

  /* don't make another one */
  if (LookupBoxActive) {
    if (XtIsRealized (look.dialog))
	XMapRaised (XtDisplay(IDdata->shell), XtWindow(look.dialog));

    /* bring it back where we can see it */
    XtPopup(look.dialog, XtGrabNonexclusive);
    
    /* put it some place reasonable */
    /*  where is parent? */
    XtVaGetValues (IDdata->shell, XmNx, &xpos, XmNy, &ypos,  NULL);
    ypos += 160;
    if (xpos<0) xpos = 0;
    XMoveWindow (XtDisplay(IDdata->shell), XtWindow(look.dialog), 
		 xpos, ypos);

    /* set values */
    /* equinox */
    if (usr_equinox>0.0)
      sprintf(valuestr,"Equinox %7.1f", usr_equinox);
    else 
      sprintf(valuestr,"Equinox of image");
    wierdstring = XmStringCreateSimple (valuestr);
    XtSetArg (wargs[0], XmNlabelString, wierdstring);
    XtSetValues (look.equlab, wargs, 1);
    if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;

    /* RA */
    look.ra = 0.0;
    sprintf (valuestr, "00 00 00.0");
    XmTextSetString (look.data1, valuestr);

    /* dec */
    sprintf (valuestr, "+00 00 00.0");
    look.dec = 0.0;
    XmTextSetString (look.data2, valuestr);
 
    return;
  }/* end of update dialog */

  label = XmStringCreateSimple ("Celestial Position to find");
/* mark as active */
  LookupBoxActive = 1;

  look.dialog = XtVaCreatePopupShell ("LookupPos", xmDialogShellWidgetClass, 
				 IDdata->shell, 
				 XmNautoUnmanage, False,
				 XmNwidth,     180,
				 XmNheight,    150,
				 XmNdeleteResponse, XmDESTROY,
				 NULL);

/* make Form widget to stick things on */
  form = XtVaCreateManagedWidget ("LookupForm", xmFormWidgetClass,
				  look.dialog,
				  XmNautoUnmanage, False,
				  XmNwidth,     180,
				  XmNheight,    150,
				  XmNx,           0,
				  XmNy,           0,
				  NULL);

/* info label widgets */
  toplabel = XtVaCreateManagedWidget ("Label", xmLabelWidgetClass, 
				    form, 
				    XmNwidth,           180,
				    XmNlabelString,   label,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

/* Equinox */
  if (usr_equinox>0.0)
    sprintf(valuestr,"Equinox %7.1f", usr_equinox);
  else 
    sprintf(valuestr,"Equinox of image");
  equstr = XmStringCreateSimple (valuestr);
  look.equlab = XtVaCreateManagedWidget ("EquLabel", xmLabelWidgetClass, 
				    form, 
				    XmNwidth,           180,
				    XmNlabelString,   equstr,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     toplabel,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

  RA = XmStringCreateSimple ("RA");

/* RA */
  label1 = XtVaCreateManagedWidget ("RA", xmLabelWidgetClass,
				    form,
				    XmNheight,    30,
				    XmNlabelString,   RA,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     look.equlab,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);
  look.ra = 0.0;
  sprintf (valuestr, "00 00 00.0");
  look.data1 = XtVaCreateManagedWidget ("RA data", xmTextFieldWidgetClass, 
				    form, 
				    XmNvalue,        valuestr,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     look.equlab,
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNleftWidget,     label1,
				    XmNrightAttachment,  XmATTACH_FORM,
				    NULL);
/* separator */
    sep1 = XtVaCreateManagedWidget ("sep1", xmSeparatorWidgetClass,
				   form, 
				   XmNwidth,           180,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget,     look.data1,
				   XmNleftAttachment,  XmATTACH_FORM,
				   NULL);
/* Dec */
  Dec = XmStringCreateSimple ("Dec");
  label2 = XtVaCreateManagedWidget ("Dec", xmLabelWidgetClass,
				    form,
				    XmNheight,    30,
				    XmNlabelString,   Dec,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     sep1,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

  sprintf (valuestr, "+00 00 00.0");
  look.dec = 0.0;
  look.data2 = XtVaCreateManagedWidget ("Dec data", xmTextFieldWidgetClass, 
				    form, 
				    XmNvalue,        valuestr,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget,     sep1,
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNleftWidget,     label2,
				    XmNrightAttachment,  XmATTACH_FORM,
				    NULL);

/* separator */
    sep2 = XtVaCreateManagedWidget ("sep2", xmSeparatorWidgetClass,
				   form, 
				   XmNwidth,           180,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget,     look.data2,
				   XmNleftAttachment,  XmATTACH_FORM,
				   NULL);
/* Cancel button */
  CancelButton = XtVaCreateManagedWidget ("Cancel", xmPushButtonWidgetClass, 
				    form, 
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    NULL);
  XtAddCallback (CancelButton, XmNactivateCallback, LookupCancelButCB, 
		 (XtPointer)IDdata);

/* OK button */
  OKButton = XtVaCreateManagedWidget ("Lookup", xmPushButtonWidgetClass, 
				    form, 
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNleftWidget,     CancelButton,
				    XmNrightAttachment, XmATTACH_FORM,
				    NULL);
  XtAddCallback (OKButton, XmNactivateCallback, LookupOKButCB, 
		 (XtPointer)IDdata);

  if (equstr) XmStringFree(equstr); equstr = NULL;
  if (label) XmStringFree(label); label = NULL;
  if (RA) XmStringFree(RA); RA = NULL;
  if (Dec) XmStringFree(Dec); Dec = NULL;

/* set it up */
  XtManageChild (look.dialog);

/* put it some place reasonable */
/*  where is parent? */
     XtVaGetValues (IDdata->shell,
		    XmNx, &xpos,
		    XmNy, &ypos,
		    NULL);
  ypos += 200;
  if (xpos<0) xpos = 0;
  XMoveWindow (XtDisplay(IDdata->shell), XtWindow(look.dialog), 
	       xpos, ypos);
} /* end LookPosCB */

int LookPosRead(FILE *hFile, char *fname, double *ra, double *dec, 
		float *delt_ra, float *delt_dec, int *prio, int *iErr)
/* routine to read next position from file */
/* Returns 1 if OK -1 = comment, else 0. */
/* File expected open on call and is closed if EOF or error. */
/* iErr = error return; 0=OK else an error occured;  */
/* iErr = -1 means the position is out of the image. */
{
  int h, d, rm, dm, iNumByte, iNumRead, iRet, toHours;
  float rs, ds, xp, yp;
  int bRAOK, bDecOK, bOK, bSouth, i;
  char szLine[120], ctype[5];

  if (!fgets (szLine, 120, hFile)) {
/*   error or EOF */
    if (ferror(hFile)) *iErr = 2;  /* Check if error */
    fclose(hFile);  /* close */
    return 0;}  /*  end of error check */
/* check for comment */
  if (szLine[0]=='!') return -1;
  iNumByte = strlen(szLine);
  iNumRead = 0; bOK = False; bRAOK = False; bDecOK = False;
  if (iNumByte>10){ /* decode line */
    iNumRead = sscanf (szLine, "%s %d %d %f %d %d %f %f %f %d", 
		       fname, &h, &rm, &rs, &d, &dm, &ds, delt_ra, 
		       delt_dec, prio);
    if (iNumRead>=3) bRAOK = !hmsra(h, rm, rs, ra);
    if (iNumRead>=6) bDecOK = !dmsdec(d, dm, ds, dec);

    /* southern declination? look for minus sign*/
    bSouth = 0;
    for (i=5;i<120;i++) 
      {if (szLine[i]=='-') bSouth=1; 
       if (szLine[i]==0) break; }
    if (bSouth && (*dec>0.0)) *dec = -*dec;

    bOK = bRAOK && bDecOK;
    bOK = bOK && (iNumRead>=8) && (*delt_ra>0.0) && (*delt_dec>0.0);
  }  /* End of decode valid line */
  if (bOK)   /* everything OK? */
    {*iErr = 0;
     return 1;}
  else  /* bogus dudes */
    {*iErr = 3;
     fclose(hFile);  /* close */
     sprintf (szErrMess, "Error with file entry %s", szLine);
     MessageShow (szErrMess);
     return 0;}
}  /* end LookPosRead */

int LookPosFile (char* indexfilename, char* FITSfilename)
/* lookup which FITS file is selected from index file name */
/* indexfilename is the full path to the index file */
/* FITSfilename contains only the file name */
/* returns 0 if OK */
{
  int i, iRet, iLoop, iXcen, iYcen, iScroll, iHalf, iMore, iErr, iNumOut=0;
  int iNumByte, iNumRead, bOK, bFound, prio, last_prio;
  double ra, dec, last_dist, dist;
  float equinox, delt_ra, delt_dec, ra_test, dec_test;
  char  dummy[20], szLine[120], fname[120];
  FILE   *hFile;

  iErr = 0;
  bFound = 0;
  last_prio=-1;
  last_dist = 1000.0;
/* Open file */
  hFile = fopen (indexfilename, "rt");
  if (hFile==NULL) 
     {sprintf (szErrMess, "Error opening index file %s", indexfilename);
      MessageShow (szErrMess);
      return 1;}

/* read index equinox */
  if (!fgets (szLine, 120, hFile)) {
/*   error or EOF */
    fclose(hFile);  /* close */
    MessageShow ("Error reading equinox from index table");
    return 1;}  /*  end of error check */
  bOK = False;
  iNumByte = strlen(szLine);
  if (iNumByte>10){ /* decode line */
    iNumRead = sscanf (szLine, "%s %f", dummy, &equinox);
    bOK = iNumRead==2;}
  if (!bOK) {
    fclose(hFile);  /* close */
    sprintf (szErrMess, "Error reading equinox from %s", szLine);
    MessageShow (szErrMess);
    return 1;}  /*  end of error check */

/* may have to precess the requested position */
  if ((usr_equinox > 0.0) && (usr_equinox != equinox))
    {if (usr_equinox==1950.0) BtoJ (&look.ra, &look.dec);
     if (usr_equinox==2000.0) JtoB (&look.ra, &look.dec);}

/* loop over entries */
  iMore = (int)(!iErr);
  while (iMore) {
    iMore = LookPosRead(hFile, fname, &ra, &dec, &delt_ra, &delt_dec, 
			&prio, &iErr);
    if ((!iMore) || (iErr>0)) break;
/* does this match? */
    if (iErr==0 && (iMore>0)) { 
      ra_test  = fabs (look.ra - ra);
      if (ra_test>180.0) ra_test -= 360.0; /* wrap around */
      ra_test = fabs (ra_test);
      dec_test = fabs (look.dec - dec);
      dist = ra_test*ra_test + dec_test*dec_test;
      if ((ra_test<=delt_ra) && (dec_test<=delt_dec) && 
	  (dist<=last_dist) &&
	  (prio>=last_prio))
	{ /* take this one */
	  strcpy (FITSfilename, fname);
	  bFound = 1;
	  last_prio = prio;
	  last_dist = dist;
	}
    }
  }  /* end of loop over index file */

  if (!bFound) { /* error if no match */
    MessageShow ("No FITS images contain requested position");
    return 1;}  /*  end of error check */

/* must be OK if it gets here */
  return 0;
} /* end LookPosFile */

void EquiSetCB (Widget w, int which, XmToggleButtonCallbackStruct *state)
/* Set equinox type; which =0=>use header, 1=>J2000 2=> B1950 */
{
  float equin;

  if (!state->set) return; /* ignore buttons being cleared */

  equin = -1.0; /* default = use header */
  if ((state->set) && (which==1)) equin = 2000.0;
  if ((state->set) && (which==2)) equin = 1950.0;

/* set equinox */
  if (image[CurImag].valid)
    image[CurImag].Image->descript->usr_equinox = equin;
  usr_equinox = equin;
/* done with box */
  XtDestroyWidget (equ.dialog);
  EquinoxBoxActive = 0;
} /* end EquiSetCB */

/* button callbacks */
void EquiSetCancelButCB (Widget w, XtPointer clientData, XtPointer callData)
/* Cancel button hit */
{
  if (!EquinoxBoxActive) return;
  XtDestroyWidget (equ.dialog);
  EquinoxBoxActive = 0;
} /* end EquiSetCancelButCB */

void SetEquCB (Widget parent, XtPointer clientData, XtPointer callData)
/* create dialog box for Setting equinox */
{
  int    iEquin;
  Widget form;
  Widget label1, radio;
  Widget CancelButton, ReloadButton;
  XmString     label = NULL;
  XmString     defau = NULL;
  XmString     J2000 = NULL;
  XmString     B1950 = NULL;
  ImageDisplay *IDdata = (ImageDisplay*)clientData;


/* register IDdata */
  equ.BoxData = IDdata;

  /* don't make another one */
  if (EquinoxBoxActive) {
    if (XtIsRealized (equ.dialog))
	XMapRaised (XtDisplay(equ.dialog), XtWindow(equ.dialog));
    return;
  }
  EquinoxBoxActive = 1;

  label = XmStringCreateSimple ("Set Equinox for coordinates");
  defau = XmStringCreateSimple ("Use equinox of image");
  J2000 = XmStringCreateSimple ("J2000");
  B1950 = XmStringCreateSimple ("B1950");

  equ.dialog = XtVaCreatePopupShell ("EquinoxBox", xmDialogShellWidgetClass, 
				 IDdata->shell, 
				 XmNautoUnmanage, False,
				 XmNwidth,     180,
				 XmNheight,    150,
				 XmNdeleteResponse, XmDESTROY,
				 NULL);

/* make Form widget to stick things on */
  form = XtVaCreateManagedWidget ("OptionForm", xmFormWidgetClass,
				  equ.dialog,
				  XmNautoUnmanage, False,
				  XmNwidth,     180,
				  XmNheight,    150,
				  XmNx,           0,
				  XmNy,           0,
				  NULL);

/* info label widgets */
  label1 = XtVaCreateManagedWidget ("Label1", xmLabelWidgetClass, 
				    form, 
				    XmNwidth,           180,
				    XmNlabelString,   label,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment,  XmATTACH_FORM,
				    NULL);

/* linear/nonlinear radio buttons */
  iEquin = 0;
  if (usr_equinox==2000.0) iEquin = 1;
  if (usr_equinox==1950.0) iEquin = 2;
  radio = XmVaCreateSimpleRadioBox(form, "Equinox_type", iEquin, 
				   (XtCallbackProc)EquiSetCB,
				   XmNwidth,           180,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget,     label1,
				   XmNleftAttachment,  XmATTACH_FORM,
				   XmVaRADIOBUTTON, defau, NULL, NULL, NULL,
				   XmVaRADIOBUTTON, J2000,NULL,NULL,NULL,
				   XmVaRADIOBUTTON, B1950,NULL,NULL,NULL,
				   NULL);
  XtManageChild(radio);

/* Cancel button */
  CancelButton = XtVaCreateManagedWidget ("Cancel", xmPushButtonWidgetClass, 
				    form, 
				    XmNbottomAttachment, XmATTACH_FORM,
				    NULL);
  XtAddCallback (CancelButton, XmNactivateCallback, EquiSetCancelButCB, 
		 (XtPointer)IDdata);

  if (label) XmStringFree(label); label = NULL;
  if (defau) XmStringFree(defau); defau = NULL;
  if (J2000) XmStringFree(J2000); J2000 = NULL;
  if (B1950) XmStringFree(B1950); B1950 = NULL;

/* set it up */
  XtManageChild (equ.dialog);

} /* end SetEquCB */

int FixFname(char *dir, char *fname, char *OKname)
/* find the case of the filename that the system recognizes   */
/* input  dir   = directory name, should include final '/'    */
/* input  fname = file name,                                  */
/* output OKname = full path name that the system will buy.   */
/*        memory should be wallocated in calling routine (120)*/
/* return 0 if OK else failed                                 */
/* If the initial name works (can open the file readonly) then*/
/* OKname is built from this, otherwise all lower case, then  */
/* all uppercase is tried.                                    */
{
  char tname[120];
  FILE *file;

  strcpy (tname, fname); /* copy file name */
  
/* try initial name */
  strcpy (OKname, dir);
  strcat (OKname, tname);
  file = fopen ((const char *)OKname, "rb");
  if (file) /* find it? */
    {fclose (file); return 0;}

/* try lower case */
  lowcase(tname);
  strcpy (OKname, dir);
  strcat (OKname, tname);
  file = fopen ((const char *)OKname, "rb");
  if (file) /* find it? */
    {fclose (file); return 0;}

/* try upper case */
  upcase(tname);
  strcpy (OKname, dir);
  strcat (OKname, tname);
  file = fopen ((const char *)OKname, "rb");
  if (file) /* find it? */
    {fclose (file); return 0;}

/* nothing worked */
  return 1;
} /* end FixFname */

void upcase(char* string)
 /* uppercase string */
{int i, j;
 char t;
 char uc[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
 char lc[] = {"abcdefghijklmnopqrstuvwxyz"};
 i = 0;
 while (string[i])
   {t = string[i];
    for (j=0;j<26;j++) 
      if (t==lc[j]) {string[i]=uc[j]; break;}
    i++;
   } /* loop over string */
} /* end upcase */

void lowcase(char* string)
 /* lowercase string */
{int i, j;
 char t;
 char uc[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
 char lc[] = {"abcdefghijklmnopqrstuvwxyz"};
 i = 0;
 while (string[i])
   {t = string[i];
    for (j=0;j<26;j++) 
      if (t==uc[j]) {string[i]=lc[j]; break;}
    i++;
   } /* loop over string */
} /* end lowcase */
