/* Histogram equalization functions for Matrix Class */ 
/*  Histogram equalization is an attempt to have equal numbers of pixels
    in each of the allowed color index states */
/*-----------------------------------------------------------------------
*  Copyright (C) 1998,1999
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
#include "histo.h" 
#include "fmem.h" 
#define WORKSIZE 4096 /* size of work histogram */

/* internal prototypes */
long find_hist (Matrix *pixels, Integer plane, float lmax, float lmin,
		int *hHist);
void find_range (Matrix *pixels, float *hist, int nHist, int MapFun, 
		 float *newMax,  float *newMin);
void set_range (Matrix *pixels, float hmin, float hmax,
		float *newMin, float *newMax);
    
/* get plane max and min pixel values
   IMPORTANT NOTE: the FWindow should not be locked prior to this call.
   pixels = matrix for which to find extrema
   plane = plane number
   max (returned) = maximum pixel value (blanked if failure)
   min (returned) = minimum pixel value (blanked if failure)
   Returns 0 if worked else failed */
int get_extrema (Matrix *pixels, Integer plane, float *max, float *min) {
  float val, *valp, tmax, tmin, blanked = MagicBlank();
  float *lpImage;
  int   i, j, nx, ny, error;


  if (!pixels) return -1; /* sanity check */
  if (!max) return -1;
  if (!min) return -1;

  /* default (blanked) return values */
  *max = blanked;
  *min = blanked;

  /*  get/lock image buffer pointer */
  lpImage = (float *)LockFMem(pixels->fw->fdata);
  if (!lpImage) return -1;

  /* get dimensions */
  nx = pixels->desc->dims[0];
  ny = pixels->desc->dims[1];
  tmax = -1.0e35;
  tmin =  1.0e35;

  /* loop over image */
  for (j = 0; j<ny; j++) { 
    /* Read patch */
    ReadPatch (pixels, 0, j, plane, 0, 0, 0, 0);
    error= pixels->fw->error;
    if (error)  { /* I/O error */
      ErrorMess ( "Error finding max/min in FITS image");
      /* Clean up and bail out on error */
      UnlockFMem(pixels->fw->fdata); /* unlock Image Memory */
      return error+10;
    }
       
    /*  Add any offset from beginning of patch. */
    valp = lpImage +  MatrixPatch_offset(pixels, 0, j, plane,
					 0,0,0,0);
    for (i=0; i<nx; i++) { /* loop over row */
      val = *(valp++); /* get pixel value from buffer */
      if (val!=blanked) {  /* only valid values */
	if (val>tmax) tmax = val;
	if (val<tmin) tmin = val;
      } /* end of valid pixel */
    } /* end of loop over row */
  } /* end of loop over plane */

  UnlockFMem(pixels->fw->fdata); /* unlock Image Memory */

  /* set return values as scaled values*/
  *max = (pixels->fw->file->scale * tmax) + pixels->fw->file->offset;
  *min = (pixels->fw->file->scale * tmin) + pixels->fw->file->offset;
  return 0;
} /* end get_extrema */

/* get plausible range of pixel values
   pixels = matrix for which to find extrema
   plane = plane number
   max (input) maximum unscaled value in image, if blanked determine
       (output) = maximum unscaled pixel value.
   min (input) minimum unscaled value in image, if blanked determine
       (output) = minimum unscaled pixel value
   Returns 0 if worked else failed */
int get_range (Matrix *pixels, Integer plane, int NonLinear, float *max,
  float *min) {
  int   hHist=-1;
  float *hist, count, lmax, lmin;
  float scale, offset, blanked = MagicBlank();

  if (!pixels) return -1; /* sanity check */
  if (!max) return -1;
  if (!min) return -1;

/* get image scaling information */
  scale = pixels->fw->file->scale;
  if (fabs(scale)<1.0e-25) scale = 1.0;
  offset = pixels->fw->file->offset;

  /* get image range */
  lmax = *max;
  lmin = *min;
  if ((lmax==blanked) || (lmin==blanked) || (lmin>=lmax)) {
    get_extrema(pixels, plane, &lmax, &lmin);
     /* set clipping range to unscalled units */
     lmax = (lmax - offset) / scale;  
     lmin = (lmin - offset) / scale;
  }

  /* check if this is a repeat- may already have image info */
  if ((plane==pixels->map_plane) &&
      (pixels->map_mean!=blanked) && (pixels->map_sigma!=blanked)) {
    /* use values from previous time */
    pixels->map_type = NonLinear;
    set_range(pixels, lmin, lmax, min, max);
    /* save values */
    pixels->map_plane = plane;
    pixels->map_min = *min;
    pixels->map_max = *max;
    return 0;
  }

  count = (float) find_hist (pixels, plane, lmax, lmin, &hHist);
  /* if this failed it's probably still OK */
  if (count<=0.0) {
    DeallocMem(hHist);    /* deallocate work memory */
    return 0;
  }

  hist = (float*)LockMem(hHist); /* lock Histogram memory */
  if (hist==NULL) {
    return -2;     /* lock failure */
  }

  /* estimate range */
  find_range (pixels, hist, WORKSIZE, NonLinear, &lmax, &lmin);

  /* save values */
  pixels->map_plane = plane;
  pixels->map_min = lmin;
  pixels->map_max = lmax;
  pixels->map_type = NonLinear;
  *max = lmax;
  *min = lmin;

  UnlockMem(hHist);     /* unlock work memory */
  DeallocMem(hHist);    /* deallocate work memory */
  return 0;
} /* end get_range */

/* compute histogram equalization mapping function, this must be called
   before map_pixel;  after the image is opened (LoadFImage) and before
   the first read (Readpatch).
   IMPORTANT NOTE: the FWindow should not be locked prior to this call.
   pixels = matrix for which to make equalize histogram
   plane = plane number
   max = maximum unscaled pixel value (clip above), if blank find max, min
         may be set to unscaled new range on output.
   min = minimum unscaled pixel value (clip below), if blank find max, min
         may be set to new unscaled range on output.
   newRange = true (!=0) if a new display range to be determined
   Returns 0 if worked else failed, -1=input error, -2=allocation error */
int equalize (Matrix *pixels, Integer plane, float *max, float *min, 
   int newRange) {
  int   i, k, hHist, iHb, iHe;
  float *hist, count, lmax, lmin, range, irange, *map, xmax, xmin;
  float sum, perColor, fact, scale, offset, blanked = MagicBlank();

  if (!pixels) return -1; /* sanity check */
  if (!max) return -1;
  if (!min) return -1;

/* get image scaling information */
  scale = pixels->fw->file->scale;
  if (fabs(scale)<1.0e-25) scale = 1.0;
  offset = pixels->fw->file->offset;

  /* get clipping range */
  lmax = *max;
  lmin = *min;
  if ((lmax==blanked) || (lmin==blanked) || (lmin>=lmax)) {
    get_extrema(pixels, plane, &lmax, &lmin);
    /* set clipping range to unscalled units */
    lmax = (lmax - offset) / scale;
    lmin = (lmin - offset) / scale;
  }

  /* check if this is a repeat */
  range = max-min;
  if ((pixels->map_type==2) &&
      (pixels->map_valid) &&
      (pixels->map_plane == plane) &&
      (pixels->map_max!=blanked) &&
      (*max!=blanked) &&
      ((fabs(pixels->map_max-*max)< 1.0e-4*range) ||
        (fabs(pixels->fw->data_max)< 1.0e-4*range)) &&
      (pixels->map_min!=blanked) &&
      (*min!=blanked) &&
      ((fabs(pixels->map_min-*min)< 1.0e-4*range) ||
        (fabs(pixels->fw->data_min)< 1.0e-4*range))) {
     if (newRange) { /* range from last time */
       *min = pixels->map_min;
       *max = pixels->map_max;
     }
     return 0;
   }

  /* save values */
  pixels->map_valid = 0; /* invalidate */
  pixels->map_min = lmin;
  pixels->map_max = lmax;
  pixels->map_plane = plane;
  pixels->map_type = 2;

  count = (float) find_hist (pixels, plane, lmax, lmin, &hHist);
  /* if this failed it's probably still OK */
  if (count<=0.0) {
    DeallocMem(hHist);    /* deallocate work memory */
    return 0;
  }

  hist = (float*)LockMem(hHist); /* lock Histogram memory */
  if (hist==NULL) return -2;     /* lock failure */

  /* check for minimum number of pixels (10) */
  if (count<10.0) {
      ErrorMess ( "Too few pixels in Pixel Range - Please reset");
      /* Clean up and bail out on error */
      UnlockMem(hHist);     /* unlock work memory */
      DeallocMem(hHist);    /* deallocate work memory */
      return -3;
  } /* end of minimum count check */

  /* init histogram factors */
  range = lmax-lmin;
  if (range!=0.0) irange = (float)(WORKSIZE) / range;
  else irange =  (float)(WORKSIZE);
  fact = 1.0 / irange;

  /* estimate range if requested */
  if (newRange) {
    xmax = lmax;
    xmin = lmin;
    find_range (pixels, hist, WORKSIZE, 2, &lmax, &lmin);
    /* use part of histogram */
    iHb = (lmin - xmin) * irange;
    if (iHb<0) iHb = 0;
    if (iHb>=WORKSIZE) iHb = WORKSIZE-1;
    iHe = WORKSIZE - (xmax - lmax) * irange;
    if (iHe<0) iHe = 0;
    if (iHe>=WORKSIZE) iHe = WORKSIZE-1;
    count = 0.0; /* reset count for range to be used */
    for (i=iHb; i<=iHe; i++) count += hist[i];
  } else { /* use all of histogram */
    iHb = 0;
    iHe = WORKSIZE-1;
  }

  /* check for minimum number of cells in histogram (50) */
  if ((iHe - iHb)<50) { /* use total range */
    iHb = 0;
    iHe = WORKSIZE-1;
    lmin = pixels->map_min;
    lmax = pixels->map_max;
    ErrorMess ("Too few cells left in histogram - Showing total range");
  } /* end of minimum cell count check */

  /* convert to mapping function */

  /* loop over function */
  map = pixels->map_val+1;
  perColor = count / ((float)MAXHIST-1.0); /* pixels per color index */
  i = iHb;
  sum = 0.0;
  for (k=1; k<MAXHIST-1; k++) {
    while ((sum<perColor) && (i<=iHe)) {sum += hist[i++];}
    sum -= perColor; /* if more than allowed in one bin */
    *(map++) = xmin + (float) i * fact;
  } /* end of loop over mapping function */

  /* first and last cells are the extrema */
  pixels->map_val[0] = lmin;
  pixels->map_val[MAXHIST-1] = lmax;

  UnlockMem(hHist);     /* unlock work memory */
  DeallocMem(hHist);    /* deallocate work memory */

  pixels->map_valid = 1; /* now has valid mapping */
  return 0;
} /* end equalize */

/* Return color index for specified pixel value. 
	Returns 0 (blanked) if  mapping function invalid.
   Note: this should not be passed blanked pixels.
   pixels = matrix for which to make equalize histogram,
   value = unscaled pixel value,
   returns color index (0 used only for blanked values */
int map_pixel(Matrix *pixels, float value) {
  int next, skip;
  float *map = pixels->map_val;

  if (!pixels) return 0; /* sanity check */
  if (!pixels->map_valid) return 0; /* mapping defined */

  /* clip to range */
  if (value<=map[0]) return 1;
  if (value>=map[MAXHIST-1]) return MAXHIST-1;

  /* lookup in table  - use binary search*/
  next = (MAXHIST / 2) - 1;
  skip = MAXHIST/4;
  while (skip>0) {
     if (value>map[next]) next += skip;
     else next -= skip;
     skip = skip / 2;
  }
     if (value<map[next]) next--;
     if (next<1) next = 1; /* 0 reserved for blank */
    return next;
 /* for (i=2; i<MAXHIST; i++) if (value < *map++) return i-1;     */

  /*return MAXHIST-1; *//* just in case it gets here */
} /* end  map_pixel */
  
/* compute histogram 
   pixels = matrix for which to make equalize histogram
   plane = plane number
   max = maximum unscaled pixel value (clip above)
   min = minimum unscaled pixel value (clip below)
   hHist = histogram handle
   Returns number of cells counter else failed, -1=input error, 
           -2=allocation error */
long find_hist (Matrix *pixels, Integer plane, float lmax, float lmin,
    int *hHist) {
  float val, *hist, *valp, *lpImage, blanked = MagicBlank();
  int   i, j, k, nx, ny, error;
  int   index;
  float range, irange;
  long  count;

  if (!pixels) return -1; /* sanity check */
  if (!hHist) return -1;

  /*  get/lock image buffer pointer */
  lpImage = (float *)LockFMem(pixels->fw->fdata);
  if (!lpImage) return -2;

  /* allocate a work array for 4096 histogram levels */
  *hHist = AllocMem((long)(WORKSIZE*sizeof(float)));
  if (*hHist==0) return -2;    /* allocation failure */
  hist = (float*)LockMem(*hHist);
  if (hist==NULL) return -2;   /* lock failure */

  /* get dimensions */
  nx = pixels->desc->dims[0];
  ny = pixels->desc->dims[1];

  /* init histogram stuff */
  range = lmax-lmin;
  if (range!=0.0) irange = (float)(WORKSIZE) / range;
  else irange =  (float)(WORKSIZE);
  count = 0;
  for (k=0; k<WORKSIZE; k++) hist[k] = 0.0;

  /* loop over image */
  for (j = 0; j<ny; j++) {
    /* Read patch */
    ReadPatch (pixels, 0, j, plane, 0, 0, 0, 0);
    error= pixels->fw->error;
    if (error)  { /* I/O error */
      ErrorMess ( "Error finding histogram of FITS image");
      /* Clean up and bail out on error */
      UnlockFMem(pixels->fw->fdata); /* unlock Image Memory */
      UnlockMem(*hHist);              /* unlock work memory */
      DeallocMem(*hHist);             /* deallocate work memory */
      return -(error+10);
    }

    /*  Add any offset from beginning of patch. */
    valp = lpImage +  MatrixPatch_offset(pixels, 0, j, plane,
				    0,0,0,0);
    for (i=0; i<nx; i++) { /* loop over row */
      val = *(valp++); /* get pixel value from buffer */
      if (val!=blanked) {  /* only valid values */
	     index = (int) ((irange * (val - lmin)) + 0.5);
	     if ((index>=0) && (index<WORKSIZE)) {
  	       count += 1;           /* total number of pixels */
	       hist[index] += 1.0;     /* accumulate histogram */
        }
      } /* end of valid pixel */
    } /* end of loop over row */
  } /* end of loop over plane */

  UnlockFMem(pixels->fw->fdata); /* unlock Image Memory */
  UnlockMem(*hHist);              /* unlock work memory */

  return count;
} /* end find_hist */

/* determine a plausible range of pixel values,
    use from the mean of the peak of the distibution (assumed sky level)
    - sigma to mean + 0.1NUMCOLOR*sigma for linear displays and
    0.5*NUMCOLOR*sigma for others.
    hist = histogram
    nHist = number of values in hist
    MapFun = 0-> Linear, 1->nonlinear, 2-> histogram equalization
    min (input) min value tabulated in hist
        (output) ~ sky level (mode) + 1 sigma 
    max (input) max value tabulated in hist
        (output) depends on mapping function */
void find_range (Matrix *pixels, float *hist, int nHist, int MapFun,   
		 float *newMax, float *newMin) {
    float hmax = *newMax, hmin = *newMin;
    float sum, sum2, count, mean, sigma;
    double arg;
    float fact, xi;
	 int i, k, iB, iE;
    
    /* determine the sigma and mean of the distribution */
    count = 0.0;
    sum = 0.0;
    sum2 = 0.0;
    for (i=0; i<nHist; i++) {
      if (hist[i]>0.0) { /* ignore empty cells */
        xi = (float)i;
        sum += xi * hist[i];
        sum2 += xi * xi * hist[i];
        count += hist[i];
      }
    }
    if (count>2.0) {
      mean = sum / count;
      arg = (sum2/count) - mean*mean;
      if (arg>0.0) sigma = sqrt(arg);
      else sigma = 0.5 * fabs (mean);
    } else { /* trouble - histogram bad, return */
      return;
    }
    
    /* narrow the range to clip the tails of the distribution */
    /* iterate */
    for (k=0; k<2; k++) {
      /* use mean +/- 3 sigma */
      iB = (int)(0.5 + mean - 3.0 * sigma);
      if (iB<0) iB = 0;
      iE = (int)(0.5 + mean + 3.0 * sigma);
		if (iE>=nHist) iE = nHist - 1;
      /* redetermine the sigma and mean of the distribution */
		count = 0.0;
      sum = 0.0;
      sum2 = 0.0;
      for (i=iB; i<=iE; i++) {
        if (hist[i]>0.0) { /* ignore empty cells */
          xi = (float)i;
          sum += xi * hist[i];
          sum2 += xi * xi * hist[i];
          count += hist[i];
        }
      }
      if (count>2.0) {
        mean = sum / count;
        arg = (sum2/count) - mean*mean;
	if (arg>0.0) sigma = sqrt(arg);
	else sigma = 0.5 * fabs (mean);
      } else { /* trouble - histogram bad, return */
        return;
      }
    } /* end iteration */

    /* convert mode and sigma to unscaled pixel units */
    fact = (hmax - hmin) / ((float)nHist);
    pixels->map_mean = hmin + mean * fact;
    pixels->map_sigma = sigma * fact;
    pixels->map_type = MapFun;

    set_range (pixels, hmin, hmax, newMin, newMax);
} /* end find_range */

/* set display min and max based on image histogram mean and sigma
   all are in unscaled units. Histogram properties from pixels.
   pixels = matrix structucture
   min = image minimum
   max = image maximum
   newMin = minimum for display
   newMax = maximum for display  */
void set_range (Matrix *pixels, float hmin, float hmax,
		float *newMin, float *newMax) {
    /* upper limit depends on mapping type */
    if (pixels->map_type==0) { /* linear*/
      *newMin = pixels->map_mean - pixels->map_sigma;
      *newMax = pixels->map_mean + 0.1 * MAXHIST*pixels->map_sigma;
   } else if (pixels->map_type==1) { /* non linear */
      *newMin = pixels->map_mean;
      *newMax = pixels->map_mean + 0.3 * MAXHIST*pixels->map_sigma;
   } else if (pixels->map_type==2) { /* histogram equalization*/
      *newMin = pixels->map_mean + pixels->map_sigma;
      *newMax = pixels->map_mean + 0.5 * MAXHIST*pixels->map_sigma;
   }
   if (*newMin<hmin) *newMin = hmin;
   if (*newMax>hmax) *newMax = hmax;

} /* end find_range */
