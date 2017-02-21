/* XFITSview header file */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1999
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
#include <stdlib.h>
#include <stdio.h>
#include "imag.h"
#ifndef XFITSVIEW
#define XFITSVIEW
#define MAXCOLOR MAXHIST /* defined in matx.h */


typedef struct {
/* FITS stuff */
    Logical valid;     /* If true then a FITS file is connected */
    Image  *Image;     /* Image */
    char   *pixarray;  /* image pixel array, 1 byte per pixel */
                       /* ordered by rows then column */
    FStrng *FileName;  /* FITS file name */
    FStrng *object;    /* object name  */
    FStrng *units;     /* Image units */
    Integer ndim;      /* number of dimensions */
    Integer dim[7];    /* axis dimensionality array  */
    FStrng *cname[7];  /* Axis labels  */
    double crval[7];   /* axis coordinate reference values */
    float crpix[7];    /* axis reference pixels  */
    float crinc[7];    /* axis increments  */
    float crot[7];     /* axis rotation  */
    float data_max;    /* maximum pixel value */
    float data_min;    /* minimum pixel value */
    float epoch;       /* Epoch of coordinate system */
    FStrng *obsdate;   /* observing date */
    int iImageNx;    /* Image width (in pixel array) */
    int iImageNy;    /* Image height (in pixel array) */
    int iNumPlanes;  /* Number of planes in the image */
    int iXPixel;     /* Cursor X pixel (in image) */
    int iYPixel;     /* Cursor Y pixel (in image) */
    int iFitted;     /* 1 if position fitted else 0 */
    float fXpixel;   /*  fitted X pixel number */
    float fYpixel;   /*  fitted Y pixel number */
    float fBpixel;   /*  fitted peak brightness */
    float PixRange[2];  /* range of pixel values to display */
  int   iNonlinear; /* mapping: 0=> linear, 1=> sqrt, 2=>histo. Eq. */
    int PlaneNo;      /* image plane number to display 0 rel */
} ImageData;

/* global data structures */
#ifdef XFITSVIEWMAIN
ImageData image[2];  /* images */
short  CurImag;      /* which image is current 0 or 1 */
FStrng *FITS_dir;    /* FITSfile directory */
FStrng *mark_dir;    /* Mark position directory */
FStrng *log_dir;     /* logging directory */
int    doLog;        /* it true position logging turned on */
float  usr_equinox;  /* Equinox desired by user */
Widget Display_shell;/* highest level widget */
#endif             /* end of declarations for main */

#ifndef XFITSVIEWMAIN
extern ImageData image[2];  /* images */
extern short  CurImag;      /* which image is current 0 or 1 */
extern FStrng *FITS_dir;    /* FITSfile directory */
extern FStrng *mark_dir;    /* Mark position directory */
extern FStrng *log_dir;     /* logging directory */
extern int    doLog;        /* it true position logging turned on */
extern float  usr_equinox;  /* Equinox desired by user */
extern Widget Display_shell;/* highest level widget */
#endif /* end of declarations for other files */

#endif /* XFITSVIEW */ 
