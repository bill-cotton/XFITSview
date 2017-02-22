/* header file for gzipread.c which allows random read access to gzip 
compressed files */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996
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

#include "fitsmem.h"
#include "fitsio.h"

#ifndef GZIPREAD_H
#define GZIPREAD_H
/* 
*  The routines in this package use the I/O routines FileSeek, FileRead 
*  in fitsio.h as well as memory allocation and locking routines in 
*  fitsmem.h
*/

/*  Status codes */

#define	PR_SUCCESS	0       /* OK                                   */
#define	PR_END   	1       /* encountered end of data              */
#define	PR_E_BLOCK	(-1)	/* Bad block type in gzip data.		*/
#define	PR_E_CODE	(-2)	/* Bad code during decode.		*/
#define	PR_E_CRC	(-3)	/* CRC does not match data.		*/
#define	PR_E_DATA	(-4)	/* Data had bad format.			*/
#define	PR_E_EOI	(-5)	/* End of input stream reached.		*/
#define	PR_E_INC_LIT	(-6)	/* Incomplete literal set.		*/
#define	PR_E_IO		(-7)	/* Error durring io.			*/
#define	PR_E_MAGIC	(-8)	/* Magic number was not found.		*/
#define	PR_E_MEMORY	(-9)	/* Memory allocation failure.		*/
#define	PR_E_METHOD	(-10)	/* Compression method is unknown.	*/
#define	PR_E_UNSUPPORT  (-11)	/* Unsupported compression algorithm.	*/
#define	PR_E_BLEW_CORE  (-12)	/* Over filled index structure   	*/


/* initialize I/O for new file 
* Input file should have been opened using FileOpen and the file index 
* passed as hFile.  */

int gz_init (int hFile);

/* shutdown reading of gzipped file                                     */

int gz_close (void);

/* Position I/O  of uncompressed file
* index is byte number (0-rel) from the beginning of the uncompressed file */

int gz_seek (long index);

/* read next block of uncompressed file 
* count in the desired number of bytes, actual value returned and 
* buffer is the array to filled  */

int gz_read (int *count, cMemPtr buffer);

/* return the pointer to the FITSfile structure for current file        */
FITSfile* gz_current_file(void);

#endif /* GZIPREAD_H */
