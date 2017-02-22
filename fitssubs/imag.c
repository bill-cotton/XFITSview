/*    Image class implementation */ 
/*  An Image consists of a pixel Matrix, an Image descriptor and a  linked 
    list (InfoList) of information. */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1997,1998,2001,2002
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
  
#include "imag.h" 
#include "matx.h" 
#include "infolist.h" 
#include "myutil.h" 
#include "histo.h" 

/* internal prototypes */
void ImageDSSInfo(FITSin *file, Image *image);
void ImageIRAFInfo(FITSin *file, Image *image);
  
/* Constructor */ 
Image* MakeFImage ()
{ 
  Image *me = (Image *) malloc (sizeof(Image)); 
  me->matx = MakeMatrix(); 
  me->descript = NULL; 
  me->ilist = MakeInfoList(); 
  me->error = 0; 
  return me; 
} /* end MakeFImage */

/* destructor */
void KillFImage(Image *me)
{
  if (!me) return;  /* anybody home? */
  if (me->matx) KillMatrix(me->matx);
  if (me->descript) KillImageDescriptor(me->descript);
  if (me->ilist) KillInfoList(me->ilist);
  if (me) free(me); me = NULL;
}  /* end KillFImage */
  
  
/*  Construct the actual array and update descriptors. */ 
void build_image(Image *me, Integer ndim, Integer *dim, 
		 FITSfile* Ffile, Integer bitpix, Integer hbytes, 
		 Integer blank, double scale, double offset) 
{ 
  if (!me) return; /* validity check */
  if (me->descript) KillImageDescriptor(me->descript); 
  me->descript = MakeImageDescriptor(ndim, dim); 
  build_matrix(me->matx, ndim, dim, 0, 1, Ffile, bitpix, hbytes, 
		     blank, scale, offset); 
} /* end build_image */ 
  
  
/*  axis info */ 
void get_axis_info(Image *me, Integer axis_number, FStrng *axis_name, 
		   Integer *length, float *ref_pixel, double *coord, 
		   float *increment, float *rotation) 
{ 
 if (!me) return; /* validity check */
 if (!axis_name) return; /* validity check */
 if (!length) return; /* validity check */
 if (!ref_pixel) return; /* validity check */
 if (!coord) return; /* validity check */
 if (!increment) return; /* validity check */
 if (!rotation) return; /* validity check */
 StringCopy (axis_name, me->descript->axisdesc[axis_number]->axis_name); 
 *length = me->descript->axisdesc[axis_number]->axis_length; 
 *ref_pixel = me->descript->axisdesc[axis_number]->axis_ref_pixel; 
 *coord = me->descript->axisdesc[axis_number]->axis_coord; 
 *increment = me->descript->axisdesc[axis_number]->axis_increment; 
 *rotation = me->descript->axisdesc[axis_number]->axis_rotation; 
}  /* End get_axis_info */ 
  
void set_axis_info(Image *me, Integer axis_number, FStrng *axis_name, 
		   Integer length, float ref_pixel, double coord, 
		   float increment, float rotation)
{ 
 if (!me) return; /* validity check */
 if (!axis_name) return; /* validity check */
 StringCopy (me->descript->axisdesc[axis_number]->axis_name, axis_name); 
 me->descript->axisdesc[axis_number]->axis_length = length; 
 me->descript->axisdesc[axis_number]->axis_ref_pixel = ref_pixel; 
 me->descript->axisdesc[axis_number]->axis_coord = coord; 
 me->descript->axisdesc[axis_number]->axis_increment = increment; 
 me->descript->axisdesc[axis_number]->axis_rotation = rotation; 
}  /* End set_axis_info */ 
  
/* Update ImageDescriptor for DSS perversions */
void ImageDSSInfo(FITSin *file, Image *image)
{
  FStrng *RA[1], *Dec[1], *Decsign[1], *ctemp[1];
  float  x_pixel, y_pixel, sec, fsign, ff;
  double darr[20];
  Integer ierr, ierr2, type, ndim, dim[5], i, iarr[2];
  int     hd, m;
  char    sign;

  if (!file) return; /* anybody home? */
  if (!image) return; /* anybody home? */

/* use presence of 'PLTLABEL' keyword in header as indicator of DSS */
  ctemp[0] = MakeString(" ");
  ierr = InfoLookup(file->ilist, "PLTLABEL", &type, &ndim, dim, (char*)ctemp);
  if (ierr) 
    {image->descript->isDSS = 0; KillString(ctemp[0]); return;}
  StringCopy (image->descript->plate_label, ctemp[0]);
  KillString(ctemp[0]);
  image->descript->isDSS = 1; /* must be one */
  ierr = InfoLookup(file->ilist, "CNPIX?  ", &type, &ndim, dim, (char*)iarr);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  for (i=0; i<2; i++) image->descript->corn_pixel[i] = iarr[i];  
  ierr = InfoLookup(file->ilist, "XPIXELSZ", &type, &ndim, dim, (char*)&ff);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  image->descript->pixel_size[0] = ff;
  ierr = InfoLookup(file->ilist, "YPIXELSZ", &type, &ndim, dim, (char*)&ff);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  image->descript->pixel_size[1] = ff;
  ierr = InfoLookup(file->ilist, "PLTSCALE", &type, &ndim, dim, (char*)&ff);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  image->descript->plate_scale = ff;
  ierr = InfoLookup(file->ilist, "EQUINOX ", &type, &ndim, dim, (char*)&ff);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  image->descript->equinox = ff;
  ierr = InfoLookup(file->ilist, "PPO?    ", &type, &ndim, dim, (char*)darr);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  for (i=0; i<dim[0]; i++) image->descript->PPO[i] = darr[i];  
  ierr = InfoLookup(file->ilist, "AMDX?   ", &type, &ndim, dim, (char*)darr);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  for (i=0; i<dim[0]; i++) image->descript->AMDX[i] = darr[i]; 
  ierr = InfoLookup(file->ilist, "AMDY?   ", &type, &ndim, dim,  (char*)darr);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  for (i=0; i<dim[0]; i++) image->descript->AMDY[i] = darr[i];  

/* plate pointing position */
  ierr = InfoLookup(file->ilist, "PLTRAM  ", &type, &ndim, dim, (char*)&m);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  ierr = InfoLookup(file->ilist, "PLTRAS  ", &type, &ndim, dim, (char*)&sec);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  ierr = InfoLookup(file->ilist, "PLTRAH  ", &type, &ndim, dim, (char*)&hd);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */

 image->descript->plate_RA = 15.0*((float)hd + (m/60.0) + (sec/3600.0));
  Decsign[0] = MakeString(" ");
  ierr = InfoLookup(file->ilist, "PLTDECSN", &type, &ndim, dim, 
		    (char*)Decsign);
  sign = Decsign[0]->sp[0];
  KillString(Decsign[0]);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  if (sign=='-') fsign = -1.0;
  else
    fsign = 1.0;
  ierr = InfoLookup(file->ilist, "PLTDECM ", &type, &ndim, dim, (char*)&m);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  ierr = InfoLookup(file->ilist, "PLTDECS ", &type, &ndim, dim, (char*)&sec);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  ierr = InfoLookup(file->ilist, "PLTDECD ", &type, &ndim, dim, (char*)&hd);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  image->descript->plate_DEC = fsign*((float)hd + (m/60.0) + (sec/3600.0));

  ierr = InfoLookup(file->ilist, "OBJCTX  ", &type, &ndim, dim, 
		    (char*)&x_pixel);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  ierr = InfoLookup(file->ilist, "OBJCTY  ", &type, &ndim, dim, 
		    (char*)&y_pixel);
  if (ierr) {image->descript->isDSS = 0; return;} /* bogus dudes */
  RA[0] = MakeString("0 0 0.0");
  Dec[0] = MakeString("0 0 0.0");
  ierr = InfoLookup(file->ilist, "OBJCTRA ", &type, &ndim, dim, (char*)RA);
  ierr2 = InfoLookup(file->ilist, "OBJCTDEC", &type, &ndim, dim, (char*)Dec);
/* update imageDescriptor */
  if (!ierr && !ierr2)
    ImageDescriptorFixDSS (image->descript, RA[0], Dec[0], x_pixel, y_pixel);
  if (ierr || ierr2) image->descript->isDSS = 0; /* not found everything */
  KillString(RA[0]);  /* clean up */
  KillString(Dec[0]);
} /* end ImageDSSInfo */

/* Update ImageDescriptor for IRAF coordinates */
void ImageIRAFInfo(FITSin *file, Image *image)
{
  float  farr[5];
  Integer ierr, type, ndim, dim[5], i;

  if (!file) return; /* anybody home? */
  if (!image) return; /* anybody home? */

/* use presence of 'CD1_?' keyword in header as indicator of IRAF type */
  ierr = InfoLookup(file->ilist, "CD1_?   ", &type, &ndim, dim, (char*)farr);
  if (ierr) 
    {image->descript->isIRAF = 0; return;} /* nope - not IRAF CD matrix */
/* make sure  'CD2_?' keyword is there as well */
  ierr = InfoLookup(file->ilist, "CD2_?   ", &type, &ndim, dim, (char*)farr);
  if (ierr) 
    {image->descript->isIRAF = 0; return;} /* nope - not IRAF CD matrix */
  image->descript->isIRAF = 1; /* must be one */

  ierr = InfoLookup(file->ilist, "IRAF-MAX", &type, &ndim, dim, (char*)farr);
  if (ierr) {farr[0]=0.0;} /* use default */
  image->matx->fw->data_max = farr[0];   /* save data max */
  ierr = InfoLookup(file->ilist, "IRAF-MIN", &type, &ndim, dim, (char*)farr);
  if (ierr) {farr[0]=0.0;} /* use default */
  image->matx->fw->data_min = farr[0];   /* save data max */

  /* CD matrix */
  ierr = InfoLookup(file->ilist, "CD1_?   ", &type, &ndim, dim, (char*)farr);
  if (ierr) {image->descript->isIRAF = 0; return;} /* bogus dudes */
  for (i=0; i<2; i++) image->descript->cd1[i] = farr[i];  
  ierr = InfoLookup(file->ilist, "CD2_?   ", &type, &ndim, dim, (char*)farr);
  if (ierr) {image->descript->isIRAF = 0; return;} /* bogus dudes */
  for (i=0; i<2; i++) image->descript->cd2[i] = farr[i];  

  ImageDescriptorFixIRAF (image->descript); /* fix up image descriptor */

} /* end ImageIRAFInfo */

/* Read image from a FITSstream */ 
void LoadFImage(FITSin *file, Image *image)
{
/* Fetch information via file InfoList */ 
    FStrng *object[1], *ctype[9], *date_obs[1], *units[1]; 
    Integer ndim, dim[9], naxes, naxis[9], bitpix, blank, ix, ierr[15]; 
    Integer head_bytes, type; 
    int     hasCoord;
    Logical getMaxMin = 0;
    float crpix[9], cdelt[9], crota[9], epoch, data_max, data_min; 
    double crval[9], scale, offset; 
    object[0] = MakeString("Unknown "); 
    date_obs[0] = MakeString("Unknown "); 
    units[0] = MakeString("Unknown "); 

 if (!file) return; /* validity check */
 if (!image) return; /* validity check */
    for (ix=0; ix<9; ix++) ctype[ix] = MakeString("Unknown ");
  
    if ((!file) || (!image)) image->error = 2;  /* check that inputs exist */ 
/*  Open file which reads header. */ 
    head_bytes = 0; 
    head_bytes = FITSinOpen(file); 
    image->error = file->error;  /* save error code */ 
/*  close no matter - all we need now is the header. */ 
    if (file) FITSinClose(file); 
/* error? */ 
    if (image->error)  
      {sprintf (szErrMess, 
		"LoadImage: Error reading header %d",image->error); 
       ErrorMess(szErrMess); 
       return;}
/*  Read information from InfoList */
    ierr[0] = InfoLookup(file->ilist, "NAXIS   ", &type, &ndim, dim, 
			 (char*)&naxes); 
    ierr[1] = InfoLookup(file->ilist, "NAXIS?  ", &type, &ndim, dim, 
			 (char*)naxis); 
    ierr[2] = InfoLookup(file->ilist, "CRVAL?  ", &type, &ndim, dim, 
			 (char*)crval);
    hasCoord = !ierr[2]; /* check if coordinates found */
    /*  Default values are 0 */ 
    if (ierr[2]) {for (ix=0;ix<naxes;ix++) crval[ix] = 0.0; ierr[2]=0;} 
  
    ierr[3] = InfoLookup(file->ilist, "CTYPE?  ", &type, &ndim, dim, 
			 (char*)ctype); 
    /*  Default values are blank */ 
    if (ierr[3]) {for (ix=0;ix<naxes;ix++) StringFill(ctype[ix], "        "); 
		  ierr[3]=0;}
  
    ierr[4] = InfoLookup(file->ilist, "CRPIX?  ", &type, &ndim, dim, 
			 (char*)crpix); 
    /*  Default values are 0 */ 
    if (ierr[4]) {for (ix=0;ix<naxes;ix++) crpix[ix] = 0.0; ierr[4]=0;} 
  
    ierr[5] = InfoLookup(file->ilist, "CDELT?  ", &type, &ndim, dim, 
			 (char*)cdelt); 
    /*  Default values are 0 */ 
    hasCoord = hasCoord && !ierr[5]; /* check if coordinates found */
    if (ierr[5]) {for (ix=0;ix<naxes;ix++) cdelt[ix] = 0.0; ierr[5]=0;} 
  
    ierr[6] = InfoLookup(file->ilist, "CROTA?  ", &type, &ndim, dim, 
			 (char*)crota); 
    /*  Default values are 0 */
    if (ierr[6]) {for (ix=0;ix<naxes;ix++) crota[ix] = 0.0; ierr[6]=0;} 
  
    ierr[7] = InfoLookup(file->ilist, "OBJECT  ", &type, &ndim, dim, 
			 (char*)object); 
    if (ierr[7]) {StringFill(object[0], "        "); ierr[7] = 0;} 
  
    ierr[8] = InfoLookup(file->ilist, "DATE-OBS", &type, &ndim, dim, 
			 (char*)date_obs); 
    if (ierr[8]) {StringFill(date_obs[0], "unknown "); ierr[8] = 0;} 
  
    ierr[9] =  InfoLookup(file->ilist, "BITPIX  ", &type, &ndim, dim, 
			  (char*)&bitpix); 
/* no default BITPIX */ 
  
    ierr[10] = InfoLookup(file->ilist, "BSCALE  ", &type, &ndim, dim,
			  (char*)&scale); 
    if (ierr[10]) {scale = 1.0; ierr[10]=0;} 
  
    ierr[11] = InfoLookup(file->ilist, "BZERO   ", &type, &ndim, dim, 
			  (char*)&offset); 
    if (ierr[11]) {offset = 0.0; ierr[11]=0;} 
  
    ierr[12] = InfoLookup(file->ilist, "BLANK   ", &type, &ndim, dim, 
			  (char*)&blank); 
    if (ierr[12]) {blank = 0; ierr[12]=0;} 
/*  get Max, min from header */ 
  
    ierr[13] = InfoLookup(file->ilist, "DATAMAX ", &type, &ndim, dim, 
			  (char*)&data_max); 
    if (ierr[13]) {
      data_max=1.0; 
      getMaxMin = 1; /* lookup in file */
      if (bitpix==8) data_max = 256.0*scale + offset; 
      if (bitpix==16) data_max = 32768.0*scale + offset; 
      ierr[13] = 0;} 
    image->matx->fw->data_max = data_max; 
  
    ierr[13] = InfoLookup(file->ilist, "DATAMIN ", &type, &ndim, dim, 
			  (char*)&data_min); 
    if (ierr[13]) {
      data_min=0.0; 
      getMaxMin = 1; /* lookup in file */
      if (bitpix==8) data_min = offset; 
      if (bitpix==16) data_min = -32767.0*scale + offset; 
      ierr[13] = 0;} 
    image->matx->fw->data_min = data_min; 

    ierr[13] = InfoLookup(file->ilist, "EQUINOX ", &type, &ndim, dim, 
			  (char*)&epoch); 
    /* try EPOCH */
    if (ierr[13]) ierr[13] = InfoLookup(file->ilist, "EPOCH   ", &type, &ndim, dim, 
			  (char*)&epoch); 
    if (ierr[13]) {epoch = 0; ierr[13]=0;} 
  
    ierr[14] = InfoLookup(file->ilist, "BUNIT   ", &type, &ndim, dim, 
			  (char*)units); 
    if (ierr[14]) {StringFill(units[0], "Unknown "); ierr[14] = 0;} 
  
/* error check */ 
    for (ix=0; ix<14; ix++) if (ierr[ix]!=0) 
      {sprintf (szErrMess, 
		"LoadImage: Error obtaining FITS header Info %d",ix);
       ErrorMess(szErrMess); 
       file->error = 1; 
       image->error = file->error; 
       return;} 
  
/*  Build image */ 
    build_image(image, naxes, naxis, file->Ffile, bitpix, head_bytes, 
		blank, scale, offset); 
    if (image->matx->fw->error) 
      {sprintf (szErrMess, 
		"LoadImage: Error building image %d",image->matx->fw->error); 
       ErrorMess(szErrMess); 
       image->error = image->matx->fw->error; return;} /* error? */

    StringCopy(image->descript->image_name, object[0]);
    StringCopy(image->descript->date_obs, date_obs[0]); 
    StringCopy(image->descript->units, units[0]); 
    image->descript->epoch = epoch; 
    image->descript->equinox = epoch; /* correct past mislabeling*/
/*  Set axis information */ 
    for (ix=0; ix<naxes; ix++) { 
      StringCopy(image->descript->axisdesc[ix]->axis_name, ctype[ix]); 
      image->descript->axisdesc[ix]->axis_length = naxis[ix]; 
      image->descript->axisdesc[ix]->axis_ref_pixel = crpix[ix]; 
      image->descript->axisdesc[ix]->axis_coord = crval[ix]; 
      image->descript->axisdesc[ix]->axis_increment = cdelt[ix]; 
      image->descript->axisdesc[ix]->axis_rotation = crota[ix]; 
    } 
/*  blanking */ 
    image->matx->blankv = 0.0; 
    if (blank!=0) image->matx->blankv = MagicBlank(); 
  
/* copy gzip info to image */
    image->matx->fw->file->isGzip = file->isGzip;

/* zap temporary strings */ 
    KillString(object[0]); 
    KillString(date_obs[0]); 
    KillString(units[0]); 
    for (ix=0; ix<9; ix++) KillString(ctype[ix]); 

/* Update ImageDescriptor for any DSS perversions if necessary */
/*    if (!hasCoord) ImageDSSInfo(file, image); */
    ImageDSSInfo(file, image);
    hasCoord = hasCoord || image->descript->isIRAF;

/* Update ImageDescriptor for IRAF coordinates if necessary */
/*    if (!hasCoord) ImageIRAFInfo(file, image);*/
    ImageIRAFInfo(file, image);

    /* Lookup max/min in image if necessary */
    if (getMaxMin) {
      if (get_extrema(image->matx, 0, &image->matx->fw->data_max, 
		      &image->matx->fw->data_min)) {
	image->matx->fw->data_max = data_max; /* failed - use guess */
	image->matx->fw->data_min = data_min;
      }
    } /* end of lookup max/min */

    return; 
}  /*  End of LoadIFmage */ 
  
  
