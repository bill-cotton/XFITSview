/* Display widget for images for XFITSview */
/* allows zoom and scroll            */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1997,1999,2002,2006,2017
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
#include <Xm/DrawingA.h> 
#include <Xm/MainW.h>
#include <Xm/ScrollBar.h>
#include <Xm/Form.h>
#include <X11/IntrinsicP.h>
#include <X11/cursorfont.h> 
#include "imagedisp.h"
#include "wpos.h"
#include "logger.h"
#include "menu.h"
#include "messagebox.h"
#include "position.h"
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define SBAR_WIDTH 16; /* width of scroll bars */

/* prototypes */
/* internal */
int  FitPos(ImageDisplay *IDdata);
void UpdateInfo (ImageDisplay *IDdata);

/* progress/cancel box in FITS2Pix.c*/
void WorkingCursor(int on, int verbose, char* filename);



void ZoomDisplay (ImageDisplay *IDdata, int iXsize, int iYsize, int iXpos, 
		  int iYpos, int Width, int Height)
/* Display zoomed image with pixel (iXpos, iYpos) in bottom */
/* left corner. */
/*  iXsize, iYsize = size of display area in pixels */
/*  Width, Height are the width and height of the region in the image */
{
  int xinc, yinc, xrep, yrep, ix, iy, iix, iiy, i, j, icol;
  long yaddr, addr, yaddr2, addr2, nxin, nxout, ival, maxaddr=0;
  XImage *work;
  unsigned int nx, ny;
  unsigned long cval;
  char *pixarray, *work_data;

/*  debug */
/*
  Dimension cwid,chei;
     XtVaGetValues (IDdata->canvas,
		    XmNwidth, &cwid,
		    XmNheight, &chei,
		    NULL);
fprintf (stderr," ZoomDisplay: canvas size: Width %d, Height %d\n",cwid,chei);
fprintf (stderr," ZoomDisplay: iXpos %d, iYpos %d,  Width %d, Height %d\n",
iXpos, iYpos, Width, Height);
*/

/* don't bother if there's no image yet */
  if (!IDdata) return;
  
/* is there a display yet? */
  if (!IDdata->canvas) return;
  if (!XtIsRealized (IDdata->canvas)) return;

/* a few more validity checks */
  if (!image[CurImag].valid) return;
  if (!image[CurImag].pixarray) return;

  pixarray = image[CurImag].pixarray;
  nxin = image[CurImag].iImageNx; 
  work = IDdata->work;
  nxout = work->bytes_per_line;
  work_data = work->data;
/* set increment, multiples in pixel array */
  if (IDdata->zoom>=1) /* no zoom or zoom in */
    { xinc = 1;
      xrep = IDdata->zoom;
      yinc = 1;
      yrep = IDdata->zoom;}
  else  /* zoom out */
    { xinc = -IDdata->zoom;
      xrep = 1;
      yinc = -IDdata->zoom;
      yrep = 1;}


/* ix, iy are image pixel numbers, iix, iiy are display pixels */

  if (IDdata->depth==8) /* 8 bit display goes faster */
    {
      /* copy selected portion of image to display */
      if (IDdata->zoom<=1) /* no zooming or zoom out */
	{iiy = -1;
	 for (iy=iYpos; iy<iYpos+Height; iy += yinc)  /* loop over columns */
	   {iiy++; iix = -1;
	    yaddr = iy * nxin; yaddr2 = iiy * nxout;
	    for (ix=iXpos; ix<iXpos+Width; ix += xinc)  /* loop down rows */
	      {iix++;
	       addr = yaddr + ix; addr2 = yaddr2 + iix;
	       *(work_data + addr2) = *(pixarray + addr);
	     }
	  }
       }
      else  /* zooming */
	{iiy = -1;
	 for (iy=iYpos; iy<iYpos+Height; iy++)  /* loop over columns */
	   {for (j=0; j<yrep; j++)
	      {iiy++; iix = -1;
	       yaddr = iy * nxin; yaddr2 = iiy * nxout;
	       for (ix=iXpos; ix<iXpos+Width; ix++)  /* loop down rows */
		 {addr = yaddr + ix; 
		  for (i=0; i<xrep; i++)
		    {iix++;
		     addr2 = yaddr2 + iix;
		     *(work_data + addr2) = *(pixarray + addr);
		   }
		}
	     }
	  }
       }
    }
  else  /* slowly for non 8-bit displays */
    {iiy = -1;
     for (iy=iYpos; iy<iYpos+Height; iy+=yinc)  /* loop over columns */
       {for (j=0; j<yrep; j++)
	  {iiy++; iix = -1;
	   for (ix=iXpos; ix<iXpos+Width; ix+=xinc)  /* loop down rows */
	     { for (i=0; i<xrep; i++)
		 {iix++;
		  ival = *(pixarray + addr);
		  cval = IDdata->coltab[ival];
		  XPutPixel (work, iix, iiy, cval);
		}
	     }
	 }
      }
   }

/* debug */
/*fprintf (stderr,"ZoomDisplay: max address = %lu\n",maxaddr);*/

/* write it out */
  XPutImage (XtDisplay (IDdata->canvas), XtWindow(IDdata->canvas), 
	     IDdata->gc, IDdata->work, 0, 0, 0, 0,
	     (unsigned int)iix, (unsigned int)iiy);
} /* end ZoomDisplay */

void ZoomDisplay24 (ImageDisplay *IDdata, int iXsize, int iYsize, int iXpos, 
		  int iYpos, int Width, int Height)
/* 24 bit TrueColor visual version */
/* Display zoomed image with pixel (iXpos, iYpos) in bottom */
/* left corner. */
/*  iXsize, iYsize = size of display area in pixels */
/*  Width, Height are the width and height of the region in the image */
{
  int xinc, yinc, xrep, yrep, ix, iy, iix, iiy, i, j, icol;
  long yaddr, addr, nxin, nxout, ival;
  unsigned long cval;
  XImage *work;
  unsigned int nx, ny;
  char *pixarray, *work_data;

/*  debug */
/*  Dimension cwid,chei;
     XtVaGetValues (IDdata->canvas,
		    XmNwidth, &cwid,
		    XmNheight, &chei,
		    NULL);
fprintf (stderr," ZoomDisplay24: canvas size: Width %d, Height %d\n",cwid,chei);
*/

/* don't bother if there's no image yet */
  if (!IDdata) return;
  
/* is there a display yet? */
  if (!IDdata->canvas) return;
  if (!XtIsRealized (IDdata->canvas)) return;

/* a few more validity checks */
  if (!image[CurImag].valid) return;
  if (!image[CurImag].pixarray) return;

  pixarray = image[CurImag].pixarray;
  nxin = image[CurImag].iImageNx; 
  work = IDdata->work;
  nxout = work->bytes_per_line;
  work_data = work->data;
/* set increment, multiples in pixel array */
  if (IDdata->zoom>=1) /* no zoom or zoom in */
    { xinc = 1;
      xrep = IDdata->zoom;
      yinc = 1;
      yrep = IDdata->zoom;}
  else  /* zoom out */
    { xinc = -IDdata->zoom;
      xrep = 1;
      yinc = -IDdata->zoom;
      yrep = 1;}


/* ix, iy are image pixel numbers, iix, iiy are display pixels */
/* copy selected portion of image to display */

  if (IDdata->zoom<=1) /* no zooming or zoom out */
    {iiy = -1;
     for (iy=iYpos; iy<iYpos+Height; iy += yinc)  /* loop over columns */
       {iiy++; iix = -1;
	yaddr = iy * nxin;
	for (ix=iXpos; ix<iXpos+Width; ix += xinc)  /* loop down rows */
	  {iix++;
	   addr = yaddr + ix;
	   ival = (*(pixarray + addr));
	   XPutPixel (work, iix, iiy, IDdata->coltab[ival]);
	 }
      }
   }
  else  /* zooming */
    {iiy = -1;
     for (iy=iYpos; iy<iYpos+Height; iy++)  /* loop over columns */
       {for (j=0; j<yrep; j++)
	  {iiy++; iix = -1;
	   yaddr = iy * nxin;
	   for (ix=iXpos; ix<iXpos+Width; ix++)  /* loop down rows */
	     {addr = yaddr + ix; 
	      ival = *(pixarray + addr);
	      cval = IDdata->coltab[ival];
	      for (i=0; i<xrep; i++)
		{iix++;
		 XPutPixel (work, iix, iiy, cval);
	       }
	    }
	 }
      }
   }

/* write it out */
  XPutImage (XtDisplay (IDdata->canvas), XtWindow(IDdata->canvas), 
	     IDdata->gc, IDdata->work, 0, 0, 0, 0,
	     (unsigned int)iix, (unsigned int)iiy);
} /* end ZoomDisplay24 */

void ResetDisplay (ImageDisplay *IDdata)
/* set display parameters and ensures a single expose event to repaint the
   image on the display */
{
  int old_wid, old_hei;

/* save old display size */
  old_wid = IDdata->disp_wid;
  old_hei = IDdata->disp_hei;

/* reset display parameters*/
  SetDisplay(IDdata);

/* terrible hack - if SetDisplay made the display area larger then this
   will provoke an expose event.  If not then create one here. */
    if ((XtIsRealized (IDdata->canvas)) && (old_wid>=IDdata->disp_wid) && 
	(old_hei>=IDdata->disp_hei)) {
      PaintImage(IDdata); /* redraw */
    }
} /* end ResetDisplay */

void SetDisplay (ImageDisplay* IDdata) 
/*  sets size and position of the Image display window */
{
     int iXHalf, iYHalf, inXZoom, inYZoom, iScrWid, iScrHei;
     int iZoom, ch1;
     Dimension cwid, chei;
     char *ZP = NULL;
     Display     *dpy = XtDisplay (IDdata->display);
     unsigned int unx, uny;
     unsigned long pasize, ilong;
     int value, slider_size, increment, page_increment;
     int sbar_width = SBAR_WIDTH; 

/* update info in control pixel info area*/
     UpdateInfo(IDdata);  

/* don't bother if there's no valid image yet */
     if (!IDdata) return; /* shouldn't happen */
     if (!image[CurImag].valid) { /* set display for no image */
      /* no scroll bars */
      if (XtIsRealized (IDdata->hscroll)) XtUnmapWidget (IDdata->hscroll);
      IDdata->hscr_vis = 0;
      if (XtIsRealized (IDdata->vscroll)) XtUnmapWidget (IDdata->vscroll);
      IDdata->vscr_vis = 0;
      /* no image - give default label */
      XtVaSetValues(IDdata->shell, XmNtitle, "XFITSview", NULL);
/* hide display */
      if (XtIsRealized (IDdata->canvas)) XtUnmapWidget (IDdata->canvas);
      return;}

/*                    maximum display size */
     XtVaGetValues (IDdata->display,
		    XmNwidth, &cwid,
		    XmNheight, &chei,
		    NULL);
/*     cwid = cwid - CONTROLWIDTH;  leave room for control panel */
     IDdata->disp_wid = cwid;
     IDdata->disp_hei = chei;

     iZoom = IDdata->zoom; /* for convinence */

/*         display should be an integral multiple of image pixels */
     if (iZoom>1)
        {IDdata->disp_wid =
            ((IDdata->disp_wid/iZoom)*iZoom);
         IDdata->disp_hei =
            ((IDdata->disp_hei/iZoom)*iZoom);}
/*                    zoomed image size */
     inXZoom = image[CurImag].iImageNx;
     if (iZoom>1) inXZoom =  inXZoom * iZoom;
     if (iZoom<0) inXZoom = -inXZoom / iZoom;
     inYZoom = image[CurImag].iImageNy;
     if (iZoom>1) inYZoom =  inYZoom * iZoom;
     if (iZoom<0) inYZoom = -inYZoom / iZoom;
/*                     scroll bar size */
     iScrWid = sbar_width;
     iScrHei = sbar_width;
/*                      scroll bars needed? (iterate to get it right) */
     if ((IDdata->disp_wid-iScrWid)>=inXZoom) iScrHei = 0;
     if ((IDdata->disp_hei-iScrHei)>=inYZoom) iScrWid = 0;
     if ((IDdata->disp_wid-iScrWid)>=inXZoom) iScrHei = 0;
     if ((IDdata->disp_hei-iScrHei)>=inYZoom) iScrWid = 0;
     if (image[CurImag].valid) /* something in display? */
              /* Display needn't be larger than display+scrollbars */
              /* This sets the image size for no & negative zooms */
       {IDdata->disp_wid=min(IDdata->disp_wid, inXZoom+iScrWid);
        IDdata->disp_hei=min(IDdata->disp_hei,inYZoom+iScrHei);}
/*                      correct for size of scroll bars */
     IDdata->disp_wid -= iScrWid; 
     if (IDdata->disp_wid<0) IDdata->disp_wid = iScrWid;
     IDdata->disp_hei -= iScrHei; 
     if (IDdata->disp_hei<0) IDdata->disp_hei = iScrHei;
/*   Display may still be too big */
     if (image[CurImag].valid) /* something in display? */
       {IDdata->disp_wid = min (IDdata->disp_wid, inXZoom);
        IDdata->disp_hei = min (IDdata->disp_hei, inYZoom);}
     else
       {IDdata->disp_wid += iScrWid;  /* no scroll bars if no image */
        IDdata->disp_hei += iScrHei;} 
/* leave at least the width of the scroll bars (SBAR_WIDTH) around the edge */
     IDdata->disp_wid = min (IDdata->disp_wid, (int)cwid-sbar_width);
     IDdata->disp_hei = min (IDdata->disp_hei, (int)chei-sbar_width);
/*                    display should have an even number  of rows */
/*     IDdata->disp_hei = 2 * ((IDdata->disp_hei+1)/2); */

/* resize window */
     if (XtIsRealized (IDdata->canvas)) XtMapWidget (IDdata->canvas);
     XtResizeWidget(IDdata->canvas, 
		    (Dimension)IDdata->disp_wid,
		    (Dimension)IDdata->disp_hei, 
		    (Dimension)0);

/*                      Half Size of display in image pixels */
     iXHalf = IDdata->disp_wid/2;
     if (iZoom>1) iXHalf =  iXHalf / iZoom;
     if (iZoom<0) iXHalf = -iXHalf * iZoom;
     iYHalf = IDdata->disp_hei/2;
     if (iZoom>1) iYHalf =  iYHalf / iZoom;
     if (iZoom<0) iYHalf = -iYHalf * iZoom;
     iXHalf = min (iXHalf, image[CurImag].iImageNx/2);
     iYHalf = min (iYHalf, image[CurImag].iImageNy/2);

/* setup and center scroll */
/*     IDdata->scrollx = image[CurImag].iImageNx / 2; */
/*     IDdata->scrolly = image[CurImag].iImageNy / 2; */
     IDdata->hscr_max = image[CurImag].iImageNx-2*iXHalf;
     IDdata->hscr_min = 1;
     if (IDdata->hscr_max<=IDdata->hscr_min) 
       IDdata->hscr_max = IDdata->hscr_min + 1;
     IDdata->hscr_half = iXHalf;
     value = IDdata->scrollx - iXHalf;
     slider_size = 2 * iXHalf;
     increment = iXHalf / 5; if (increment<1) increment = 1;
     page_increment = 3 * iXHalf / 2; if (page_increment<1) page_increment = 1;
     if (value>IDdata->hscr_max) value = IDdata->hscr_max;
     if (value<IDdata->hscr_min) value = IDdata->hscr_min;
     if (iScrHei)
       {
         /* keep X-Windows from blowing it's tiny mind */
         XmScrollBarSetValues (IDdata->hscroll, 1, 1, 1, 1, False);
	 XtVaSetValues(IDdata->hscroll, 
		     XmNheight,    iScrHei,
		     XmNvalue,     value,
		     XmNmaximum,   (Dimension)(IDdata->hscr_max+2*iXHalf),
		     XmNminimum,   (Dimension)(IDdata->hscr_min),
		     NULL);
	XmScrollBarSetValues (IDdata->hscroll, value, slider_size, increment, 
			      page_increment, False);}
     IDdata->vscr_max = image[CurImag].iImageNy-2*iYHalf;
     IDdata->vscr_min = 1;
     if (IDdata->vscr_max<=IDdata->vscr_min) 
       IDdata->vscr_max = IDdata->vscr_min + 1;
     IDdata->vscr_half = iYHalf;
     value = IDdata->scrolly - iYHalf;
     slider_size = 2 * iYHalf;
     increment = iYHalf / 5; if (increment<1) increment = 1;
     page_increment = 3 * iYHalf / 2; if (page_increment<1) page_increment = 1;
     if (value>IDdata->vscr_max) value = IDdata->vscr_max;
     if (value<IDdata->vscr_min) value = IDdata->vscr_min;
     if (iScrWid)
       {
         /* keep X-Windows from blowing it's tiny mind */
         XmScrollBarSetValues (IDdata->vscroll, 1, 1, 1, 1, False);
	 XtVaSetValues(IDdata->vscroll, 
		     XmNwidth,     iScrWid,
		     XmNvalue,     value,
		     XmNmaximum,   (Dimension)(IDdata->vscr_max+2*iYHalf),
		     XmNminimum,   (Dimension)(IDdata->vscr_min),
		     NULL);
	XmScrollBarSetValues (IDdata->vscroll, value, slider_size, increment, 
			      page_increment, False);}

/*     make horizonal scroll bar visible or invisible as necessary */
/*  iScrHei = 0 => no horizontal scroll */
     if (iScrHei) /* visible */
       {XtMapWidget (IDdata->hscroll);
	IDdata->hscr_vis = 1;}
     else /* invisible */
       {XtUnmapWidget (IDdata->hscroll);
	IDdata->hscr_vis = 0;}

/*     make vertical scroll bar visible or invisible as necessary */
/*  iScrWid = 0 => no vertical scroll */
     if (iScrWid) /* visible */
       {XtMapWidget (IDdata->vscroll);
	IDdata->vscr_vis = 1;}
     else /* invisible */
       {XtUnmapWidget (IDdata->vscroll);
	IDdata->vscr_vis = 0;}

/* make work ZPixmap for display*/
     /* delete old if necessary*/
     unx = IDdata->disp_wid;
     uny = IDdata->disp_hei;
     if ( !IDdata->work || 
	 (unx>IDdata->work->width) || (uny>IDdata->work->height)) 
       {
	 if (IDdata->work) 
	   {if (IDdata->work->data) free(IDdata->work->data);  IDdata->work->data=NULL; 
	   IDdata->work->data = NULL;
	   XtFree((XtPointer)IDdata->work);
	   IDdata->work = NULL;} 
	 
	 /* create new pix map */
	 pasize = (unx+10) * uny * ((IDdata->depth+1)/8);
	 /* 24 bit displays are different */
	 if (IDdata->depth>12 ) pasize = unx * uny * 4;
	 ZP = (char*) malloc (pasize);
	 IDdata->work = 
	   XCreateImage (dpy, 
			 DefaultVisual(dpy, DefaultScreen(dpy)), 
			 IDdata->depth, ZPixmap, 0, 
			 ZP, unx, uny, 32, 0); 
	 /* blank it out */
	 for (ilong=0; ilong<pasize; ilong++) 
	   IDdata->work->data[ilong] = 0;
       } /* end of create new ZPixmap */
} /* end SetDisplay */

void PaintImage (ImageDisplay* IDdata)
/* redraw the image on the canvas computing zoom and scroll */
{
  int nXDIB, nYDIB, iZoom, iSrcWid, iSrcHei, iXHalf, iYHalf, K;
  int iXSize, iYSize, iXPos, iYPos, iXPage, iYPage;
  Dimension cWidth, cHeight;
  int i, ch1;
  char TitleString[500], *cptr;

/* blank if there's no image yet */
     if (!IDdata) return; /* this shouldn't happen */
     if (!image[CurImag].valid) {
       if (XtIsRealized (IDdata->canvas)) 
	 XClearArea (XtDisplay (IDdata->canvas), XtWindow (IDdata->canvas), 
		     0, 0, 0, 0, TRUE);
     }
     if (!image[CurImag].pixarray) return;

/* reset window title to file name */
  if (image[CurImag].valid) { /* title for image only */
    cptr = image[CurImag].FileName->sp;
    ch1 = 0;
    for (i=0; i<image[CurImag].FileName->length;i++)
      if (cptr[i]=='/') ch1 = i+1;
    if (image[CurImag].iNumPlanes>1)
      {sprintf (TitleString,"%s, plane %d",
		&cptr[ch1],
		image[CurImag].PlaneNo+1);}
    else /* one plane */
      {sprintf (TitleString,"%s", &cptr[ch1]);}
    XtVaSetValues(IDdata->shell, 
		  XmNtitle,   TitleString,
		  NULL);
  } /* end of label */

  cWidth = IDdata->disp_wid;   /* current display size */
  cHeight = IDdata->disp_hei;

  nXDIB = image[CurImag].iImageNx;
  nYDIB = image[CurImag].iImageNy;
  iZoom = IDdata->zoom;
  if (iZoom == 0) iZoom = 1;
  /* size of display in image pixels */
  iXHalf = IDdata->disp_wid;  /* divide by two later */
  if (iZoom>1) iXHalf =  iXHalf / iZoom;
  if (iZoom<0) iXHalf = -iXHalf * iZoom;
  iYHalf = IDdata->disp_hei; /* divide by two later */
  if (iZoom>1) iYHalf =  iYHalf / iZoom;
  if (iZoom<0) iYHalf = -iYHalf * iZoom;
  iXHalf = min (iXHalf, nXDIB);
  iYHalf = min (iYHalf, nYDIB);
  iSrcWid = iXHalf;
  iSrcHei = iYHalf;
  iXHalf = iXHalf / 2;
  iYHalf = iYHalf / 2;
  /* Size of display area */
  iXSize = IDdata->disp_wid;
  iYSize = IDdata->disp_hei;
  iXSize = min (iXSize, nXDIB);
  iYSize = min (iYSize, nYDIB);
  if (iZoom>1) {iYSize=iYSize*iZoom; iXSize=iXSize*iZoom;}

/*  for negative zooms, iDisp* is set in SetDisplay */
  iXSize = min (iXSize, (int)cWidth);
  iYSize = min (iYSize, (int)cHeight);
  iXSize = max (iXSize, 1);
  iYSize = max (iYSize, 1);
  /* "page" size for scrolling */
  iXPage = iSrcWid;
  iYPage = iSrcHei;
  iXPos = IDdata->scrollx - iXHalf;
  iYPos = IDdata->scrolly - iYHalf;
  iXPos = max (iXPos, 0);
  iYPos = max (iYPos, 0);
  iXPos = min (iXPos, image[CurImag].iImageNx-iXHalf*2);
  iYPos = min (iYPos, image[CurImag].iImageNy-iYHalf*2);
  IDdata->iXCorn = iXPos;
  IDdata->iYCorn = iYPos;

/* copy Pixarray to display */
  if (IDdata->trueColor) /* slow, 24 bit color */
    ZoomDisplay24 (IDdata, iXSize, iYSize, iXPos, iYPos, iSrcWid, iSrcHei);
  else /* fast, 8 bit color */
    ZoomDisplay (IDdata, iXSize, iYSize, iXPos, iYPos, iSrcWid, iSrcHei);

} /* end PaintImage */

void hscrollCB (Widget w, XtPointer clientData, XtPointer callData)
/* call back for motion in horizonal scroll */
{
    ImageDisplay *IDdata = (ImageDisplay *)clientData;
    XmScrollBarCallbackStruct *call_data = 
      (XmScrollBarCallbackStruct *) callData;

/* read value of scrollbar */
    IDdata->scrollx = call_data->value+IDdata->hscr_half;
    PaintImage((ImageDisplay*)clientData); /* redraw */
} /* end hscrollCB */

void vscrollCB (Widget w, XtPointer clientData, XtPointer callData)
/* call back for motion in vertical scroll */
{
    ImageDisplay *IDdata = (ImageDisplay *)clientData;
    XmScrollBarCallbackStruct *call_data = 
      (XmScrollBarCallbackStruct *) callData;
/* read value of scrollbar */
    IDdata->scrolly = call_data->value+IDdata->vscr_half;
/* reset scroll position and redraw */
  PaintImage((ImageDisplay*)clientData);
} /* end vscrollCB */

void   ImageRedispCB (Widget w, XtPointer clientData, XtPointer callData)
/* redisplay image canvas */
{
  ImageDisplay *IDdata = (ImageDisplay *)clientData;
  XmDrawingAreaCallbackStruct *cb = (XmDrawingAreaCallbackStruct *)callData;
  XExposeEvent  *event = (XExposeEvent *) cb->event;

/* NOP unless widget is canvas */
    if (!IDdata) return;
    if (!IDdata->canvas) return;
    if (w!=IDdata->canvas) return;

  /*fprintf (stderr,"RedispCB: x %d, y %d, nx %d, ny %d, count %d\n",
	 event->x, event->y, event->width, event->height, event->count);*/

/* return if there are more expose events in the queue */
    if (event->count) return;

/* reset display */
    SetDisplay(IDdata);
/* redraw exposed area from ImageData */
    PaintImage (IDdata);
  } /* end of ImageRedispCB */

void   ImageResizeCB (Widget w, XtPointer clientData, XtPointer callData)
/* resize image canvas */
{
    ImageDisplay  *IDdata = (ImageDisplay *)clientData;

    if (!IDdata) return;
    if (!IDdata->canvas) return;

/*fprintf(stderr,"ImageResizeCB: window resized\n "); debug */

/* reset display */
     ResetDisplay(IDdata);
/* redraw exposed area from ImageData */
/*    PaintImage (IDdata); */
  } /* end of ImageResizeCB */

/* event handlers */
void ButtonEH (Widget w, XtPointer data, XEvent *event)
/* event handler for button press in image display window */
{
  int          ix, iy, isOK;
  float        fx, fy, z;
  ImageDisplay *IDdata = (ImageDisplay*)data;

if (!IDdata) return;
if (!image[CurImag].valid) return;

/* determine image pixel */
  fx = (float)event->xbutton.x;
  fy = (float)event->xbutton.y;
  z = IDdata->zoom;
  if (z>1)  /* zoomed? Take care of sub pixel precision*/
    {fx = (fx - (z*0.5) + 0.5) / z;
     fy = (fy - (z*0.5) + 0.5) / z;}
  if (z<0) 
    {fx = -fx * z; fy = -fy * z; }
  fx = fx + IDdata->iXCorn;
  fy = image[CurImag].iImageNy - (fy + IDdata->iYCorn) - 1.0;
  ix = (int)(fx+0.5); iy = (int)(fy+0.5);
  IDdata->showInfo = 1; /* have something to show */
  if (event->xbutton.button==1) /* left button - display pixel value*/
    { image[CurImag].fXpixel = fx;
      image[CurImag].fYpixel = fy;
      image[CurImag].iXPixel = ix;
      image[CurImag].iYPixel = iy;
      image[CurImag].iFitted = 0; 
      UpdateInfo(IDdata);  /* write info in control info area*/
      isOK = 1;
    } /* end Left button */

  if (event->xbutton.button==3) /* right button - fit position */
    { image[CurImag].iXPixel = ix;
      image[CurImag].iYPixel = iy;
      image[CurImag].iFitted = FitPos(IDdata);
      UpdateInfo(IDdata);  /* write info in control info area*/
      isOK = image[CurImag].iFitted==1;
    } /* end Right button */
  if (doLog &&  isOK) DoLogger(); /* log if necessary */
} /* end ButtonEH */

ImageDisplay* MakeDisplay (Widget parent, Widget shell)
/* Make image display widget */
{
  Widget        display;   /* return display widget */
  ImageDisplay *IDdata;     /* display data structure */
  int           i;
  Dimension     cwid, chei;
  int           sbar_width = SBAR_WIDTH;
  
  IDdata = (ImageDisplay*) XtMalloc (sizeof (ImageDisplay));

/* set size */
/*                    maximum display size */
  XtVaGetValues (parent,
		 XmNwidth, &cwid,
		 XmNheight, &chei,
		 NULL);
  cwid = cwid - CONTROLWIDTH; /* leave room for control panel */
/*  chei = chei - 40; */

  IDdata->disp_wid = cwid;
  IDdata->disp_hei = chei;

/* save main window */
  IDdata->parent = parent;

/* set top level form */
  IDdata->shell = shell;

/* make Form widget for display */
  IDdata->display = XtVaCreateManagedWidget ("display", xmFormWidgetClass,
				     parent,
/* add extra pixels for scroll bars */
				     XmNwidth,     IDdata->disp_wid+sbar_width,
				     XmNheight,    IDdata->disp_hei+sbar_width,
 				     XmNtopAttachment,  XmATTACH_FORM,
				     XmNrightAttachment,  XmATTACH_FORM,
				     NULL);

/* drawing canvas */
  IDdata->canvas = 
    XtVaCreateManagedWidget ("canvas", xmDrawingAreaWidgetClass, 
			     IDdata->display, 
/*			     XmNwidth,     IDdata->disp_wid-sbar_width,
			     XmNheight,    IDdata->disp_hei-sbar_width,*/
			     XmNwidth,     1,
			     XmNheight,    1,
			     XmNtopAttachment,  XmATTACH_FORM,
			     XmNleftAttachment,  XmATTACH_FORM,
			     NULL);
/*   Add callbacks to handle resize and exposures.  */
  XtAddCallback (IDdata->canvas, XmNexposeCallback, ImageRedispCB, 
		 (XtPointer)IDdata);

/* trap button press */
  XtAddEventHandler (IDdata->canvas, ButtonPressMask, FALSE, 
		     (XtEventHandler)ButtonEH, 
		     (XtPointer)IDdata);

/* add horizonal scroll bar ("scale" in X-Windows) */

  IDdata->hscroll = 
    XtVaCreateManagedWidget ("hscroll", xmScrollBarWidgetClass, 
				       IDdata->display,
				       XmNwidth,     IDdata->disp_wid,
				       XmNheight,     sbar_width,
     				       XmNmaximum,   IDdata->disp_wid,
				       XmNminimum,            1,
     				       XmNvalue,              1,
				       XmNshowValue,       False,
				       XmNorientation, XmHORIZONTAL,
				       XmNprocessingDirection, XmMAX_ON_RIGHT,
/*				       XmNtopAttachment, XmATTACH_WIDGET,
				       XmNtopWidget,     IDdata->canvas,*/
				       XmNbottomAttachment, XmATTACH_FORM,
				       XmNleftAttachment,  XmATTACH_FORM,
				       XmNrightAttachment,  XmATTACH_FORM,
   				       XmNrightOffset,     sbar_width,
				       NULL);
  IDdata->hscr_vis = 1;  /* it's visible */
/* add call backs */
  XtAddCallback(IDdata->hscroll, XmNvalueChangedCallback, hscrollCB, 
		(XtPointer) IDdata);
/*  XtAddCallback(IDdata->hscroll, XmNdragCallback, hscrollCB, 
		(XtPointer) IDdata); */

/* add vertical scroll bar */

  IDdata->vscroll = 
    XtVaCreateManagedWidget ("vscroll", xmScrollBarWidgetClass, 
				       IDdata->display,
				       XmNheight,     IDdata->disp_hei,
				       XmNwidth,          sbar_width,
     				       XmNmaximum,   IDdata->disp_hei,
				       XmNminimum,            1,
     				       XmNvalue,              1,
				       XmNshowValue,       False,
				       XmNorientation, XmVERTICAL,
				       XmNprocessingDirection, XmMAX_ON_BOTTOM,
   				       XmNtopAttachment,  XmATTACH_FORM,
   				       XmNbottomAttachment,  XmATTACH_FORM,
   				       XmNbottomOffset,     sbar_width,
   				       XmNrightAttachment,  XmATTACH_FORM,
/*				       XmNleftAttachment, XmATTACH_WIDGET,
				       XmNleftWidget,   IDdata->canvas,*/
				       NULL);
  IDdata->vscr_vis = 1;  /* it's visible */
/* add call back */
  XtAddCallback(IDdata->vscroll, XmNvalueChangedCallback, vscrollCB, 
		(XtPointer) IDdata);
/*  XtAddCallback(IDdata->vscroll, XmNdragCallback, vscrollCB, 
		(XtPointer) */

/* make dummy widget the size of the display which can detect when 
   it's size has changed */
  IDdata->goober = 
    XtVaCreateManagedWidget ("goober", xmDrawingAreaWidgetClass, 
			     IDdata->display, 
			     XmNtopAttachment,  XmATTACH_FORM,
			     XmNleftAttachment,  XmATTACH_FORM,
			     XmNbottomAttachment,  XmATTACH_FORM,
			     XmNrightAttachment,  XmATTACH_FORM,
			     NULL);
/* go boom  XtUnmapWidget (IDdata->goober);  make invisible */
  XtAddCallback (IDdata->goober, XmNresizeCallback, ImageResizeCB,
		 (XtPointer)IDdata);
/* number of colors */
  IDdata->ncolors = XDisplayCells (XtDisplay (IDdata->display), 
				 XDefaultScreen (XtDisplay (IDdata->display)));
  if (IDdata->ncolors>MAXCOLOR) IDdata->ncolors = MAXCOLOR;
  for (i=0; i<MAXCOLOR; i++) 
    {IDdata->colut[i]=i;  /* need ramp for True color */
     IDdata->coltab[i]=i; /* need ramp for True color */
     IDdata->red[i]=0;
     IDdata->green[i]=0;
     IDdata->blue[i]=0;}

/* bit planes in display */
  IDdata->depth = XDisplayPlanes (XtDisplay (IDdata->display), 
				 XDefaultScreen (XtDisplay (IDdata->display)));
/* context */
  IDdata->gc = XCreateGC (XtDisplay (IDdata->display), 
			  DefaultRootWindow (XtDisplay(IDdata->display)),
			  0, NULL );
/* init image */
  IDdata->work = NULL;
  IDdata->zoom = 1;
  IDdata->scrollx = 0;
  IDdata->scrolly = 0;
  IDdata->iXCorn = 0;
  IDdata->iYCorn = 0;
/* no color map yet */
/*??  IDdata->cmap = 0;*/

/* no slider controls yet */
  IDdata->BriScroll = NULL;
  IDdata->ConScroll = NULL;
  IDdata->value[0] = 0;
  IDdata->value[1] = 0;
  IDdata->Info1 = NULL;
  IDdata->Info2 = NULL;
  IDdata->Info3 = NULL;
  IDdata->Info4 = NULL;
  IDdata->Info5 = NULL;
  IDdata->Info6 = NULL;
  IDdata->Info7 = NULL;
  IDdata->showInfo = 0;

/* return Image display structure */
  return IDdata;

} /* end MakeDisplay */

void FitPosCB (Widget w, XtPointer clientData, XtPointer callData)
/* menu callback routine to fit position */
{
  int          isOK;
  ImageDisplay *IDdata = (ImageDisplay *)clientData;

/* use last position selected */
  image[CurImag].iFitted = FitPos(IDdata);
  UpdateInfo(IDdata);  /* write info in control info area*/
  isOK = image[CurImag].iFitted==1;
  if (doLog &&  isOK) DoLogger(); /* log if necessary */
  FitPos(IDdata);
} /* End FitPosCB */

int FitPos(ImageDisplay *IDdata)
/* routine to fit a point near the position of the cursor: */
/* IDdata->iXPixel, iYPixel */
/* Sets Dis[iDs] values fXpixel, fYpixel, fBpixel */
/* returns 1 if worked otherwise 0, -1 => fit failed. */
{ int iXcen, iYcen, iX, iY, iXp, iYp;
  float data[9][9], fblank;
  Logical valid;
  Integer error, pos[7] = {0, 0, 0, 0, 0, 0, 0}, ndim;
  MatrixPos *impos;
  float s, dx[2];
  Display *dpy = XtDisplay(IDdata->canvas);

  ndim=image[CurImag].ndim;
  iXcen = image[CurImag].iXPixel;
  iYcen = image[CurImag].iYPixel;
  fblank = MagicBlank();
/* default return values */
  image[CurImag].fBpixel = 0.;
  image[CurImag].fXpixel = 0.;
  image[CurImag].fYpixel = 0.;

/* make matrix pos */
  impos = MakeMatrixPos(ndim, pos);

  if (!image[CurImag].valid) return 0;

/* use 9x9 values around center */
  impos->pos[2] = image[CurImag].PlaneNo;
  WorkingCursor(True, False, NULL); /* indicate working */
  for (iY=0; iY<9; iY++) {
    iYp = iYcen + iY - 4;
    impos->pos[1] = iYp;
    for (iX=0; iX<9; iX++) {
      iXp = iXcen + iX - 4;
      impos->pos[0] = iXp;
/* check validity */
      valid = IsValid(image[CurImag].Image->matx, impos);
      if (valid)
/* read pixel values */
	{data[iY][iX] = MatrixGetPixel(image[CurImag].Image->matx, impos);
	 error = image[CurImag].Image->matx->fw->error;
	 if (error)   /* I/O error */
	   {MessageShow("FitPos: error reading pixel value");
	    KillMatrixPos(impos);
	    IDdata->showInfo= 0; /* file no longer available? */
	    image[CurImag].valid = 0; /* mark as invalid */
	    return 0;}/* failed */
       }
      else  /* blank */
	data [iY][iX] = fblank;
    }
  }  /*  end of loop loading image data in array */
  WorkingCursor(False, False, NULL); /* reset cursor */
  KillMatrixPos(impos);
  if (pfit (data, &s, dx, fblank))
    {/*fprintf (stderr, "fit failed\n");*/
     XBell(dpy, 50); 
     return -1;}  /* fit failed*/
  /* Check for crazy values */
  if ((dx[0]<-4.0) || (dx[0]>4.0) || (dx[1]<-4.0) || (dx[1]>4.0)) return -1;
  image[CurImag].fBpixel = s;
  image[CurImag].fXpixel = (float)iXcen + dx[0];
  image[CurImag].fYpixel = (float)iYcen + dx[1];
  return 1;
} /* end fit pos */

void UpdateInfo (ImageDisplay *IDdata)
/* update information about selected pixel in control window */
{
  Integer      ndim;
  int          i, posOK;
  XmString     wierdstring = NULL;
  char         jerkstring[100];
  Arg          wargs[5]; 
  float        fblank, val, pix[7];
  char         axtype[3][9], label[3][20];
  double       pos[3];
  Logical      valid;
  Integer error, ipos[7] = {0, 0, 0, 0, 0, 0, 0};
  MatrixPos    *impos=NULL;
  Display *dpy = XtDisplay(IDdata->display);
  static Cursor cursor=(Cursor)NULL;
  extern Widget shell;
  XSetWindowAttributes attrs;

  /* check that info defined for current image */
  if ((!image[CurImag].Image) || (!image[CurImag].Image->descript)) 
    IDdata->showInfo = 0;

/* need to blank display? */
  if (!IDdata->showInfo)
    {
      wierdstring = XmStringCreateSimple ("    ");
      XtSetArg (wargs[0], XmNlabelString, wierdstring);
      XtSetValues (IDdata->Info1, wargs, 1);
      XtSetValues (IDdata->Info2, wargs, 1);
      XtSetValues (IDdata->Info3, wargs, 1);
      XtSetValues (IDdata->Info4, wargs, 1);
      XtSetValues (IDdata->Info5, wargs, 1);
      XtSetValues (IDdata->Info6, wargs, 1);
      XtSetValues (IDdata->Info7, wargs, 1);
      if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;
      return;
    } /* end blank info area */
/* return OK if no valid image */
  if (!image[CurImag].valid) return;
  ndim = image[CurImag].ndim;  /* Number of dimensions */
/* make sure hourglass cursor initialized */
  if (!cursor) cursor = XCreateFontCursor(dpy, XC_watch);
/*  fitted or current */
  if (ndim>2)
    sprintf (jerkstring, "(%7.2f,%7.2f, %d)",
	     image[CurImag].fXpixel+1.0, image[CurImag].fYpixel+1.0,
	     image[CurImag].PlaneNo+1);
  else
    sprintf (jerkstring, "(%7.2f,%7.2f)",
	     image[CurImag].fXpixel+1.0, image[CurImag].fYpixel+1.0);
  wierdstring = XmStringCreateSimple (jerkstring);
  XtSetArg (wargs[0], XmNlabelString, wierdstring);
  XtSetValues (IDdata->Info1, wargs, 1);
  if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;

/*  get flux */
  fblank = MagicBlank();
/*  fitted or current? */
  if (image[CurImag].iFitted>0) /* fitted values */
    {valid = 1;
     if (image[CurImag].fBpixel==fblank)
       /* blanked */
       sprintf (jerkstring, "pixel blanked");
     else
       sprintf (jerkstring, "value=%f", image[CurImag].fBpixel);}
  else  /* current position */
    {val = 0;
     for (i=3; i<7; i++) ipos[i] = 0;
/* Note: Image class pixel numbers are 0 rel; geometry is 1 rel. */
     ipos[0] = image[CurImag].iXPixel; ipos[1] = image[CurImag].iYPixel;
     ipos[2] = image[CurImag].PlaneNo;
/* make Matrix */
     impos = MakeMatrixPos(ndim, ipos);
     valid = (ndim>1) && IsValid(image[CurImag].Image->matx, impos);
     if (valid)
/* get pixel value */
       {
	WorkingCursor(True, False, NULL); /* indicate working */
	val = MatrixGetPixel(image[CurImag].Image->matx, impos);
	WorkingCursor(False, False, NULL); /* reset cursor */
	error = image[CurImag].Image->matx->fw->error;
	if (error)   /* I/O error */
	  {MessageShow("UpdateInfo: Error reading pixel value");
	   IDdata->showInfo= 0; /* file no longer available? */
	   image[CurImag].valid = 0; /* mark as invalid */
	   val = fblank;}
	if (val==fblank)
	  /* blanked */
	  sprintf (jerkstring, "pixel blanked");
	else
	  sprintf (jerkstring, "value=%f", val);
      }
   }
  KillMatrixPos(impos);
  if (!valid) sprintf (jerkstring, "invalid pixel");
/* write second line (flux density) */
  jerkstring[16] = 0; /* limit size of string */
  wierdstring = XmStringCreateSimple (jerkstring);
  XtSetArg (wargs[0], XmNlabelString, wierdstring);
  XtSetValues (IDdata->Info2, wargs, 1);
  if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;

/* celestial position */
/* equinox is line 3 */
  if ((usr_equinox>0.0) && (image[CurImag].Image->descript->equinox>0.0))
    sprintf(jerkstring,"Equinox %6.1f", usr_equinox);
  else if (image[CurImag].Image->descript->equinox>0.0)
    sprintf(jerkstring,"Equinox %6.1f", image[CurImag].Image->descript->equinox);
  else
    sprintf(jerkstring,"Equinox unknown");
  wierdstring = XmStringCreateSimple (jerkstring);
  XtSetArg (wargs[0], XmNlabelString, wierdstring);
  XtSetValues (IDdata->Info3, wargs, 1);
  if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;


/*  Get position */
  pix[0] = image[CurImag].fXpixel+1.0; 
  pix[1] = image[CurImag].fYpixel+1.0;
  pix[2] = (float)image[CurImag].PlaneNo+1.0;
  if (valid)
    {strncpy(axtype[0], image[CurImag].cname[0]->sp, 8);
     strncpy(axtype[1], image[CurImag].cname[1]->sp, 8);
     strncpy(axtype[2], image[CurImag].cname[2]->sp, 8);
     axtype[0][8] = 0; axtype[1][8] = 0; axtype[2][8] = 0;
     posOK = !get_wpos(image[CurImag].Image->descript, pix, pos);
     AxisLabel(pos[0], axtype[0], label[0]);  /* human readable */
     if (ndim>=2) AxisLabel(pos[1], axtype[1], label[1]);
     if (ndim>=3) AxisLabel(pos[2], axtype[2], label[2]);
     if (posOK) {  /* valid position */
/* write fourth line (first axis) */
       wierdstring = 
	 XmStringCreateSimple (label[0]);
       XtSetArg (wargs[0], XmNlabelString, wierdstring);
       XtSetValues (IDdata->Info4, wargs, 1);
       if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;
       
/* write fifth line (second axis) */
       if (ndim>=2) 
	 {wierdstring = 
	    XmStringCreateSimple (label[1]);
	  XtSetArg (wargs[0], XmNlabelString, wierdstring);
	  XtSetValues (IDdata->Info5, wargs, 1);
	  if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;}

/* write sixth line (third axis) */
       if (ndim>=3) 
	 {wierdstring = 
	    XmStringCreateSimple (label[2]);
	  XtSetArg (wargs[0], XmNlabelString, wierdstring);
	  XtSetValues (IDdata->Info6, wargs, 1);
	  if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;}
     }}
  else {  /* invalid position */
    sprintf (jerkstring, "invalid pixel");
/* write third line (invalid pixel message) */
    wierdstring = 
      XmStringCreateSimple (jerkstring);
    XtSetArg (wargs[0], XmNlabelString, wierdstring);
    XtSetValues (IDdata->Info3, wargs, 1);
    if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;
  }  /* end of give position on invalid */
/*  fitted or current? */
  if (image[CurImag].iFitted>0) /* fitted values */
    sprintf (jerkstring, "fitted");
  else if (image[CurImag].iFitted<0) /* fit failed */
    sprintf (jerkstring, "fit failed!");
  else /* no fit */
    sprintf (jerkstring, "");

/* write seventh line  */
  wierdstring = 
    XmStringCreateSimple (jerkstring);
  XtSetArg (wargs[0], XmNlabelString, wierdstring);
  XtSetValues (IDdata->Info7, wargs, 1);
  if (wierdstring) XmStringFree(wierdstring); wierdstring = NULL;
} /* end UpdateInfo */

/* call backs for zoom */
void Zoom25CB (Widget w, XtPointer clientData, XtPointer callData)
{
    ImageDisplay  *IDdata = (ImageDisplay *)clientData;

/* NOP if same zoom */
    if (IDdata->zoom==-4) return;

    IDdata->zoom = -4; /* set new zoom factor */

/* reset display */
    ResetDisplay(IDdata);
/* mark in menu */
    MenuMarkZoom (0);
  } /* end of Zoom25CB */

void Zoom50CB (Widget w, XtPointer clientData, XtPointer callData)
{
    ImageDisplay  *IDdata = (ImageDisplay *)clientData;

/* NOP if same zoom */
    if (IDdata->zoom==-2) return;

    IDdata->zoom = -2; /* set new zoom factor */

/* reset display */
    ResetDisplay(IDdata);
/* mark in menu */
    MenuMarkZoom (1);
  } /* end of Zoom50CB */

void Zoom100CB (Widget w, XtPointer clientData, XtPointer callData)
{
    ImageDisplay  *IDdata = (ImageDisplay *)clientData;

/* NOP if same zoom */
    if (IDdata->zoom==1) return;

    IDdata->zoom = 1; /* set new zoom factor */

/* reset display */
    ResetDisplay(IDdata);
/* mark in menu */
    MenuMarkZoom (2);
  } /* end of Zoom100CB */

void Zoom200CB (Widget w, XtPointer clientData, XtPointer callData)
{
    ImageDisplay  *IDdata = (ImageDisplay *)clientData;

/* NOP if same zoom */
    if (IDdata->zoom==2) return;

    IDdata->zoom = 2; /* set new zoom factor */

/* reset display */
    ResetDisplay(IDdata);
/* mark in menu */
    MenuMarkZoom (3);
  } /* end of Zoom200CB */

void Zoom400CB (Widget w, XtPointer clientData, XtPointer callData)
{
    ImageDisplay  *IDdata = (ImageDisplay *)clientData;

/* NOP if same zoom */
    if (IDdata->zoom==4) return;

    IDdata->zoom = 4; /* set new zoom factor */

/* reset display */
    ResetDisplay(IDdata);
/* mark in menu */
    MenuMarkZoom (4);
  } /* end of Zoom400CB */

void Zoom800CB (Widget w, XtPointer clientData, XtPointer callData)
{
    ImageDisplay  *IDdata = (ImageDisplay *)clientData;

/* NOP if same zoom */
    if (IDdata->zoom==8) return;

    IDdata->zoom = 8; /* set new zoom factor */

/* reset display */
    ResetDisplay(IDdata);
/* mark in menu */
    MenuMarkZoom (5);
  } /* end of Zoom800CB */

void Zoom1600CB (Widget w, XtPointer clientData, XtPointer callData)
{
    ImageDisplay  *IDdata = (ImageDisplay *)clientData;

/* NOP if same zoom */
    if (IDdata->zoom==16) return;

    IDdata->zoom = 16; /* set new zoom factor */

/* reset display */
    ResetDisplay(IDdata);
/* mark in menu */
    MenuMarkZoom (6);
  } /* end of Zoom1600CB */
