/* routines to allow apparently random access reads of gzip compressed files */
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1998,2002
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
/*
*
*   Module Name:        gzipread.c
*
*   Purpose:
*       Read relatively arbitrary blocks from a gzip compressed file.  
*    Due to the structure of the compressed file, with backwards 
*    references to strings which may themselves have been backwards 
*    references etc., reading must progress from the beginning.  
*    This package of routines gives the appearance of arbitrary access 
*    but any motion backwards in the file results in reading from the 
*    beginning.
*
*   Usage notes:
*    - Once gz_init is called all I/O will be to the same physical file
*      until gz_close is called.
*    - Only one file can be accessed between the gz_init and gz_close 
*      calls.
*    - Changing the file position (or the file) associated with hFile
*      will cause unpredictable behavior.
*    - after gz_close another call to gz_init is required.
*    - gz_seek will read forward if the desired file position is later
*      in the file or start from the beginning if earlier.
*    - Reading a new file requires gz_close to shutdown the old one and
*      gz_init to start the new one.
*
*   The decompression routines were originally taken from the Canadian
*   Astronomy Data Centre data compression package as was much of the
*   documentation of the methods.
*
*       You can do whatever you like with this source file, though I would
*       prefer that if you modify it and redistribute it that you include
*       comments to that effect with your name and the date.  Thank you.
*
*   Modification History:
*   Nov. 1995 W. D. Cotton modify to work in a decompressing read package
*
************************************************************************
-*/
#include <stdio.h>
#include <stdlib.h>
#include "myconfig.h"
#include "zsubs.h"
#include "gzipread.h"
#include "gziputil.h"



/*+     Public routines
************************************************************************
*
*   Synopsis:
*       int     gz_init(int hFile)
*   Purpose:
*       Function to initialize internal structures and I/O for a new file.
*       File opening and closing are assumed to be taken care of outside
*       of these routines.  I/O is done with routines FileSeek and FileRead
*       gz_close must be called to delete any remaining structures.
*
*   Parameters:
*       int     hFile           : (in) FileRead handle for input I/O
*       int     hGzip           : (in) handle for file index structure
*
*   Values Returned:
*       int     PR_SUCCESS      : Normal completion.
*       int     PR_E_INC_LIT    : Code set is incomplete (the tables are
*                                 still built in this case)
*       int     PR_E_DATA       : The input is invalid (all zero length
*                                 codes or an oversubscribed set of lengths)
*       int     PR_E_MEMORY     : Memory allocation failure.
*
************************************************************************
-*/

int gz_init(int hFile)
{
/*    unsigned char       buff[EXTHDR];    *//* extended local header   */
    char                c;
    byte                dummy[6];
    byte                flags;          /* Compression flags.           */
    byte                magic[2];       /* The magic number.            */
    byte                method;         /* The compress                 */

/* save input file index */
    in_hFile = hFile;           /* input FileRead file handle */
    IO_done = 0;                /* haven't finished file */
    last_block = 0;             /* last compression block indicator */
    if (current_file) KillFITSfile(current_file); /* delete any extant one */
    current_file = MakeFITSfile();
    if (!GetFITSfile(in_hFile, current_file)) { /* get file descriptor */
      pr_format_message( PR_E_IO, NULL);
      return( PR_E_IO);}

/* initialize byte indices */
    in_byte_count = 1;
    out_byte_count = 1;
    input_IO_error = 0;
    gzip_type = -1;   /* set block type */

/* initialize window, bit, byte buffers */
    outcnt = 0;
    bk = 0;
    bb = 0;
    max_byte_buff = 0;
    next_byte_buff = MAX_BYTE;

/* deallocate any old huft structures */
   if (tlsave) PR_CHECK (huft_free(tlsave));
   tlsave = NULL;
   if (tdsave) PR_CHECK (huft_free(tdsave));
   tdsave = NULL;

/* initialize input */
   if (FileSeek(in_hFile, 0L)!=1) {
     pr_format_message( PR_E_EOI, NULL);
     return( PR_E_EOI);}

/*  Check the magic number and compression type.  */
    PR_CHECK (char_in (magic, 2));   /* file magic number */
    if (memcmp ((const void*)magic, GZIP_MAGIC, 2) != 0)
       {pr_format_message( PR_E_MAGIC, NULL);
	return( PR_E_MAGIC);}

    PR_CHECK (char_in (&method, 1)); /* compression method */
    if (method != DEFLATED)
      {pr_format_message (PR_E_METHOD, (int*)&method);
	return (PR_E_METHOD);}

    PR_CHECK (char_in( &flags, 1)); /* compression flags */
    if (flags & ENCRYPTED || flags & CONTINUATION || flags & RESERVED)
      { pr_format_message( PR_E_UNSUPPORT, NULL);
	return( PR_E_UNSUPPORT);}

/*  Skip over time stamp, extra flags, and os.    */
    PR_CHECK (char_in (dummy, 6));
    if ( ( flags & EXTRA_FIELD) != 0)
      { /* Skip the file length. */
	PR_CHECK (char_in (dummy, 2));}

/* Skip over the original file name.  */
    if ( ( flags & ORIG_NAME) != 0)
      { do
	  { PR_CHECK (char_in ((byte*)&c, 1));
	  } while ( c != '\0');
      }

/*  skip over the comment.  */
    if ((flags & COMMENT) != 0)
      { do
	{ PR_CHECK (char_in ((byte*)&c, 1));
	} while ( c != '\0');
      }

/* check for I/O error */
    if (input_IO_error)
      { pr_format_message( PR_E_IO, NULL);
	return( PR_E_IO);}

/* input should now be positioned at the beginning of the
   first compression block - remember this place*/
    first_block = in_byte_count;

/* allocate sliding window if necessary */
    if (!hWindow) hWindow = AllocMem(WSIZE);
    if (!hWindow) {
      pr_format_message( PR_E_MEMORY , NULL);
      return( PR_E_MEMORY);}
    window = NULL;

    return PR_SUCCESS;  /* finished OK */
} /* end gz_init */


/*+
************************************************************************
*
*   Synopsis:
*       int     gz_seek(long index)
*
*   Purpose:
*        Position I/O in uncompressed file, reading forward from the 
*     previous position or the start of the file if necessary.
*     The input file may have been closed and opened before the call to
*     gz_seek as it repositions the file where gz_init or gz_read left it.
*
*   Parameters:
*       long index           : (in) the beginning byte number (0-rel).
*
*   Values Returned:
*       int     PR_SUCCESS      : Normal completion.
*       int     PR_E_EOI        : Attempt to read past end of data
*       int     PR_E_INC_LIT    : Code set is incomplete (the tables are
*                                 still built in this case)
*       int     PR_E_DATA       : The input is invalid (all zero length
*                                 codes or an oversubscribed set of lengths)
*       int     PR_E_MEMORY     : Memory allocation failure.
*
************************************************************************
-*/

int gz_seek(long inpos)
{
  int           ret, num_left, nread, orig;
  long          index, offset, start;
  GZboolean       cont, end;
  unsigned      t;      /* block type                           */
  unsigned long b;      /* bit buffer                           */
  unsigned      k;      /* number of bits in bit buffer         */
  char          dumbuf[10]; /* dummy buffer for I/O             */

/* convert position to the 1-rel system used internally  */
  index = inpos + 1;
  input_IO_error = 0; /* reset IO error flag */

/* position file depending on no move, forward or backward move */
  if (index >= out_byte_count) { /* here or forward */
    /* make sure I/O positioned correctly */
    offset = in_byte_count-1; /* 0-rel */}
  else 
    { /* go back to the beginning */
      offset = first_block-1;   /* offset to first compressed block*/
      out_byte_count = 1;       /* reset byte indices */
      in_byte_count = first_block;
      outcnt = 0;               /* window index */
      bb = 0;                   /* input bit buffer */
      bk = 0;                   /* input bit count */
      /* deallocate any old huft structures */
      if (tlsave) PR_CHECK (huft_free(tlsave));
      tlsave = NULL;
      if (tdsave) PR_CHECK (huft_free(tdsave));
      tdsave = NULL;
      gzip_type = -1;          /* reset block type */
    }
    ret = FileSeek(in_hFile, offset); /* reposition file */
    max_byte_buff = 0;            /* reset byte buffer */
    next_byte_buff = MAX_BYTE;
    if (ret!=1)  /* position failure */
      {pr_format_message( PR_E_EOI, NULL);
       return( PR_E_EOI);}

/* clear IO_done flags  */
  IO_done = 0;
  last_block = 0;

/* if necessary position input by reading up to the desired position */
/* check if current position is OK */
  if (index == out_byte_count)  return PR_SUCCESS;

/* lock sliding window memory */
  PR_CHECK_NULL (window = (cMemPtr)LockMem (hWindow));

/* is this the continuation of a previous block? */
  cont = gzip_type >= 0;

/* read to byte before the desired one  - may be over multiple blocks */
  num_left = 1;
  start = index - 1;
  while (num_left>0) {
    /* determine block compression type if at beginning of block (type=-1) */
    if (gzip_type < 0) {
      /* is input exhausted ? */
      if (last_block) { /* already done last compression block? */
	pr_format_message( PR_END, NULL);
	UnlockMem (hWindow);    /* unlock sliding window memory */
	return PR_END;}

      /* make local bit buffer */
      b = bb;
      k = bk;

      /* read in last block bit */
      NEEDBITS(1)
      last_block = last_block || ((int)b & 1);
      DUMPBITS(1)

      /* read in block type  */
      NEEDBITS(2)
      t = (unsigned)b & 3;
      DUMPBITS(2)

      /* restore the global bit buffer */
      bb = b;
      bk = k;

      /* set block type */
      gzip_type = t;
    }

    /* check for I/O error */
    if (input_IO_error)
      { pr_format_message( PR_E_IO, NULL);
	UnlockMem (hWindow);    /* unlock sliding window memory */
	return( PR_E_IO);}

    orig = num_left;
    /* decode by compression type */
    if (gzip_type == 0)
      ret = gun_stored (cont, start, &num_left, (cMemPtr)dumbuf, &end);
    else if (gzip_type == 1)
      ret = gun_fixed (cont, start, &num_left, (cMemPtr)dumbuf, &end);
    else if (gzip_type == 2)
      ret = gun_dynamic (cont, start, &num_left, (cMemPtr)dumbuf, &end);
    else /* bad type */
      { pr_format_message( PR_E_BLOCK, (int*)&t);
	UnlockMem (hWindow);    /* unlock sliding window memory */
	return( PR_E_BLOCK);}
    if (ret) {
      UnlockMem (hWindow);   /* unlock sliding window memory */
      return ret;}   /* error */

    /* check for other I/O error */
    if (input_IO_error)
      { pr_format_message( PR_E_IO, NULL);
	UnlockMem (hWindow);    /* unlock sliding window memory */
	return( PR_E_IO);}

    /* update starting pointer */
    nread = orig - num_left;
    start += nread;

    /* reset block type if previous block finished */
    if (end) gzip_type = -1;
    cont = !end;  /* decode a new block next time */
  } /* end loop reading data */

  UnlockMem (hWindow);   /* unlock sliding window memory */

  IO_done = last_block && (gzip_type<0); /* is I/O finished? */

/* are we where we want to be?*/
  if (out_byte_count!=index)
      { pr_format_message( PR_E_IO, NULL);
	input_IO_error = 1; /* treat as I/O error */
	return( PR_E_IO);}
	
  return PR_SUCCESS;  /* finished OK */
} /* end gz_seek */

/*+
************************************************************************
*
*   Synopsis:
*       int     gz_read(int *count, cMemPtr buffer)
*
*   Purpose:
*        Function to read next section of the decompressed file.
*
*   Parameters:
*       int     *count          : (in) the number of bytes desired
*                               : (out) the actual number
*       cMemPtr buffer          : (in) pointer to the output buffer.
*
*   Values Returned:
*       int     PR_SUCCESS      : Normal completion.
*       int     PR_E_EOI        : Attempt to read past end of data
*       int     PR_E_INC_LIT    : Code set is incomplete (the tables are
*                                 still built in this case)
*       int     PR_E_DATA       : The input is invalid (all zero length
*                                 codes or an oversubscribed set of lengths)
*       int     PR_E_MEMORY     : Memory allocation failure.
*
************************************************************************
-*/

int gz_read(int *count, cMemPtr buffer)
{
  int           ret, num_left, nread, orig;
  long          start, index;
  GZboolean       cont, end;
  unsigned      t;      /* block type                           */
  unsigned long b;      /* bit buffer                           */
  unsigned      k;      /* number of bits in bit buffer         */

/* set initial byte pointer */
  index = out_byte_count;

/* is input exhausted ? */
  if (IO_done) {
    pr_format_message( PR_END, NULL);
    return PR_END;}

/* lock sliding window memory */
  PR_CHECK_NULL (window = (cMemPtr)LockMem (hWindow));

/* is this the continuation of a previous block? */
  cont = gzip_type >= 0;

/* read to end of desired data - may be over several blocks */
  num_left = *count;
  *count = 0;
  start = index;
  while (num_left>0) {
    /* determine block compression type if at beginning of block (type=-1) */
    if (gzip_type < 0) {
      /* is input exhausted ? */
      if (last_block) { /* already done last compression block? */
	pr_format_message( PR_END, NULL);
	UnlockMem (hWindow);    /* unlock sliding window memory */
	return PR_END;}

      /* make local bit buffer */
      b = bb;
      k = bk;

      /* read in last block bit */
      NEEDBITS(1)
      last_block = last_block || ((int)b & 1);
      DUMPBITS(1)

      /* read in block type  */
      NEEDBITS(2)
      t = (unsigned)b & 3;
      DUMPBITS(2)

      /* restore the global bit buffer */
      bb = b;
      bk = k;

      /* set block type */
      gzip_type = t;
    } /* end of block setup */

    /* check for I/O error */
    if (input_IO_error)
      { pr_format_message( PR_E_IO, NULL);
	UnlockMem (hWindow);    /* unlock sliding window memory */
	return( PR_E_IO);}

    orig = num_left;
    /* decode by compression type */
    if (gzip_type == 0)
      ret = gun_stored (cont, start, &num_left, buffer, &end);
    else if (gzip_type == 1)
      ret = gun_fixed (cont, start, &num_left, buffer, &end);
    else if (gzip_type == 2)
      ret = gun_dynamic (cont, start, &num_left, buffer, &end);
    else /* bad type */
      { pr_format_message( PR_E_BLOCK, (int*)&t);
	UnlockMem (hWindow);    /* unlock sliding window memory */
	return( PR_E_BLOCK);}
    if (ret) {
      UnlockMem (hWindow);   /* unlock sliding window memory */
      return ret;}   /* error */

    /* update buffer pointer */
    nread = orig - num_left;
    buffer += nread;
    start += nread;
    *count += nread;

    /* check for I/O error */
    if (input_IO_error)
      { pr_format_message( PR_E_IO, NULL);
	UnlockMem (hWindow);    /* unlock sliding window memory */
	return( PR_E_IO);}

    /* reset block type if previous block finished */
    if (end) gzip_type = -1;
    cont = !end;  /* decode a new block next time */

  } /* end loop reading data */

  UnlockMem (hWindow);   /* unlock sliding window memory */

  IO_done = last_block && (gzip_type<0); /* is I/O finished? */

  return PR_SUCCESS;  /* finished OK */
} /* end gz_read */

/*+
************************************************************************
*
*   Synopsis:
*       int     gz_close(void)
*
*   Purpose:
*        Delete all structures associated with reading a compressed file
*
*   Parameters:
*
*   Values Returned:
*       int     PR_SUCCESS      : Normal completion.
*
************************************************************************
-*/

int gz_close(void)
{

/* deallocate any remaining huft structures */
  if (tlsave) PR_CHECK (huft_free(tlsave));
  tlsave = NULL;
  if (tdsave) PR_CHECK (huft_free(tdsave));
  tdsave = NULL;

/* deallocate sliding window memory */
  if (hWindow) DeallocMem (hWindow);
  hWindow = 0;
  if (current_file) KillFITSfile(current_file); 
  current_file = NULL;

  input_IO_error = 999;  /* just in case */

  return PR_SUCCESS;  /* finished OK */
} /* end gz_close */

/*+
************************************************************************
*
*   Synopsis:
*       FITSfile*   gz_current_file(void)
*
*   Purpose:
*        Return the pointer to the FITSfile structure for current file
*
*   Parameters:
*
*   Values Returned:
*       FITSfile*          :  FITSfile of currently active file
*
************************************************************************
-*/
FITSfile* gz_current_file(void)
{
  return current_file;
}  /* end gz_current_file */


/*+                    Internal routines
************************************************************************
*
*   Synopsis:
*       int load_byte_buffer(void)
*
*   Purpose:
*       Reads a buffer load of bytes from the input stream associated
*       with in_hFile.  Should be called iff next_byte_buff >= max_byte_buff.
*       If an I/O error occurs then global input_IO_error is set.
*
************************************************************************
-*/
void load_byte_buffer(void)
{
  int  ret, count;

  if (!in_hFile) input_IO_error = 1; /* file not open - ERROR */
  if (input_IO_error) return; /* error condition exists */
  /* debug */
  if (next_byte_buff<max_byte_buff) {
    sprintf (szErrMess," load_byte_buffer: next,max %d %d",
	     next_byte_buff,max_byte_buff);
    ErrorMess(szErrMess);
  }

  count = MAX_BYTE;  /* try to fill buffer */
  ret = FileRead(in_hFile, (MemPtr)byte_buffer, count);
  /* may be less that a buffer full left */
  max_byte_buff = ret;
  next_byte_buff = 0;
  if (ret<=0) {
    input_IO_error = 1; /* error = EOI? */
    pr_format_message( PR_E_EOI, NULL);}
} /* end of load_byte_buffer */


/*+
************************************************************************
*
*   Synopsis:
*       int char_in(byte *buff, int count)
*
*   Purpose:
*       read multiple bytes from input stream
*       Maintains the global variable in_byte_count giving the
*       current input byte index of the next byte to be read.
*       If an I/O error occurs then global input_IO_error is set.
*
*   Parameters:
*      byte* buffer (in)  pointer to memory to receive data
*      int   count  (in)  Number of bytes desired
*
*   Values Returned:
*       int    number of bytes read.
*
************************************************************************
-*/
int char_in(byte *buff, int count)
{
  int  ret;

  if (!in_hFile) input_IO_error = 1; /* file not open - ERROR */
  if (input_IO_error) return 0; /* error condition exists */

  ret = count;
  while (count--)
    {if (next_byte_buff >= max_byte_buff) load_byte_buffer();
     if (input_IO_error) break;
     in_byte_count++;
     *buff++ = byte_buffer[next_byte_buff++];
    }
  ret = ret - count; /* how many actually read */
  if (input_IO_error) return 0; /* error condition exists */
  return ret;
} /* end of char_in */


/*+
************************************************************************
*
*   Synopsis:
*       void    pr_format_message(int code, int* arg)
*
*   Purpose:
*       print error message associated with code.
*       codes are defined in gzipread.h.
*
*   Parameters:
*       int     code            : (in) (negative) message index.
*       int     *arg            : (in) array of arguments
*
*   Values Returned:
*
*   Note: global parameter MsgSup is used to supress messages
************************************************************************
-*/
void    pr_format_message(int code, int* arg)
{
   char message[120];

   if (code==PR_E_BLOCK)
    sprintf (message, "Bad block type %d.", arg[0]);
   else if (code==PR_E_CODE)
    sprintf (message, "Bad code %d .", arg[0]);
   else if (code==PR_E_CRC)
    sprintf (message, "The CRC does not match");
   else if (code==PR_E_DATA)
    sprintf (message, "Data could not be decompressed.");
   else if (code==PR_E_EOI)
    sprintf (message, "Unexpected end of input detected.");
   else if (code==PR_E_INC_LIT)
    sprintf (message, "Incomplete literal tree.");
   else if (code==PR_E_IO)
    sprintf (message, "I/O Error decompressing gzip file.");
   else if (code==PR_E_MAGIC)
    sprintf (message, "Wrong magic number for gzip.");
   else if (code==PR_E_MEMORY)
    sprintf (message, "Memory allocation failure.");
   else if (code==PR_E_METHOD)
    sprintf (message, "Compression method %s is not supported.", &arg[0]);
   else if (code==PR_E_UNSUPPORT)
    sprintf (message, "Compression type %s is not supported.", &arg[0]);
   else if (code==PR_E_BLEW_CORE)
    sprintf (message, "Overran gzip file index structure");
   else return;     /* unknown message */

   if (!MsgSup) ErrorMess(message); /* sent message to user (in zsubs.h)*/

   return;
}


/*+
************************************************************************
*
*   Synopsis:
*       int     huft_build( b, n, s, d, e, t, m)
*
*   Purpose:
*       Given a list of code lengths and a maximum table size, make a set of
*       tables to decode that set of codes.  Return zero on success, one if
*       the given code set is incomplete (the tables are still built in this
*       case), two if the input is invalid (all zero length codes or an
*       oversubscribed set of lengths), and three if not enough memory.
*
*   Parameters:
*       unsigned        *b      : (mod) code lengths in bits (all assumed
*                                 <= BMAX)
*       unsigned        n       : (in)  Number of codes (assumed <= N_MAX)
*       unsigned        s       : (in)  number of simple-valued codes (0..s-1)
*       unsigned short  *d      : (mod) list of base values for non-simple codes
*       unsigned short  *e      : (mod) list of extra bits for non-simple codes
*       struct huft     **t     : (out) result: starting table.
*       int             *m      : (mod) maximum lookup bits, returns actual.
*
*   Values Returned:
*       int     PR_SUCCESS      : Normal completion.
*       int     PR_E_INC_LIT    : Code set is incomplete (the tables are
*                                 still built in this case)
*       int     PR_E_DATA       : The input is invalid (all zero length
*                                 codes or an oversubscribed set of lengths)
*       int     PR_E_MEMORY     : Memory allocation failure.
*
************************************************************************
-*/


int huft_build(unsigned *b, unsigned n, unsigned s, unsigned short *d,
	       unsigned short *e, struct huft **t, int *m)
{
    unsigned    a;              /* counter for codes of length k        */
    unsigned    c[BMAX+1];      /* bit length count table               */
    unsigned    f;              /* i repeats in table every f entries   */
    int         g;              /* maximum code length                  */
    int         h;              /* table level                          */
    unsigned    i;              /* counter, current code                */
    unsigned    j;              /* counter                              */
    int         k;              /* number of bits in current code       */
    int         l;              /* bits per table (returned in m)       */
    unsigned    *p;             /* pointer into c[], b[], or v[]        */
    struct huft *q;             /* points to current table              */
    struct huft r;              /* table entry for structure assignment */
    struct huft *u[BMAX];       /* table stack                          */
    unsigned    v[N_MAX];       /* values in order of bit length        */
    int         w;              /* bits before this table == (l * h)    */
    unsigned    x[BMAX+1];      /* bit offsets, then code stack         */
    unsigned    *xp;            /* pointer into x                       */
    int         y;              /* number of dummy codes added          */
    unsigned    z;              /* number of entries in current table   */


    /* Generate counts for each bit length */

   /* (void) memzero(c, sizeof(c)); */
    j = BMAX+1;
     for (i=0; i<j; i++) c[i] = 0;

    p = b;
    i = n;
    do {
	c[*p++]++;                      /* assume all entries <= BMAX */
    } while (--i);

    if (c[0] == n)                      /* null input--all zero length codes */
    {
	*t = (struct huft *)NULL;
	*m = 0;
	return( PR_SUCCESS);
    }

    /* Find minimum and maximum length, bound *m by those */
    l = *m;
    for (j = 1; j <= BMAX; j++)
      {if (c[j]){break;}
     }
    k = j;                              /* minimum code length */
    if ((unsigned)l < j) {l = j;}
    for (i = BMAX; i; i--)
      {if (c[i]){break; }
     }
    g = i;                              /* maximum code length */
    if ((unsigned)l > i) {l = i;}
    *m = l;

    /* Adjust last length count to fill out codes, if needed */
    for (y = 1 << j; j < i; j++, y <<= 1)
    {if ((y -= c[j]) < 0)
       {
	 pr_format_message( PR_E_DATA, NULL);
	 return( PR_E_DATA);    /* bad input: more codes than bits */
       }
   }

    if ((y -= c[i]) < 0)
      {
	pr_format_message( PR_E_DATA, NULL);
	return( PR_E_DATA);     /* bad input: more codes than bits */
      }
    c[i] += y;

    /* Generate starting offsets into the value table for each length */
    x[1] = j = 0;
    p = c + 1;
    xp = x + 2;
    while (--i)                         /* note that i == g from above */
      { *xp++ = (j += *p++);}


    /* Make a table of values in order of bit lengths */
    p = b;
    i = 0;
    do {
      if ((j = *p++) != 0) {v[x[j]++] = i;}
    } while (++i < n);


    /* Generate the Huffman codes and for each, make the table entries */
    x[0] = i = 0;                       /* first Huffman code is zero */
    p = v;                              /* grab values in bit order */
    h = -1;                             /* no tables yet--level -1 */
    w = -l;                             /* bits decoded == (l * h) */
    u[0] = (struct huft *)NULL;         /* just to keep compilers happy */
    q = (struct huft *)NULL;            /* ditto */
    z = 0;                              /* ditto */


    /* go through the bit lengths (k already is bits in shortest code) */
    for (; k <= g; k++)
      {a = c[k];
       while (a--)
	 { /* here i is the Huffman code of length k bits for value *p
	    * make tables up to required level*/
	    while (k > w + l)
	    {   h++;
		w += l;       /* previous table always l bits */

		/* compute minimum size table less than or equal to l bits */
		/* upper limit on table size */
		z = (z = g - w) > (unsigned)l ? l : z;
		/* try a k-w bit table */
		if ((f = 1 << (j = k - w)) > a + 1)
		  {/* too few codes for k-w bit table */
		    f -= a + 1;    /* deduct codes from patterns left */
		    xp = c + k;
		    while (++j < z) /* try smaller tables up to z bits */
		    {if ((f <<= 1) <= *++xp)
		       {
			 break; /* enough codes to use up j bits */
		       }
		     f -= *xp;  /* else deduct codes from patterns */
		   }
		  }
		z = 1 << j;             /* table entries for j-bit table */


		/* allocate and link in new table */
		if ((q = (struct huft *) malloc((z + 1)*sizeof(struct huft)))
			== (struct huft *)NULL)
		  {
		    if (h) {PR_CHECK( huft_free(u[0]));}
		    pr_format_message( PR_E_MEMORY, NULL);
		    return( PR_E_MEMORY);       /* not enough memory */
		  }
		hufts += z + 1;         /* track memory usage */
		*t = q + 1;             /* link to list for huft_free() */
		*(t = &(q->v.t)) = (struct huft *)NULL;
		u[h] = ++q;             /* table starts after link */


		/* connect to last table, if there is one */

		if (h)
		  {
		    x[h] = i;           /* save pattern for backing up */
		    r.b = (byte)l;      /* bits to dump before this table */
		    r.e = (byte)(16 + j);/* bits in this table */
		    r.v.t = q;          /* pointer to this table */
		    j = i >> (w - l);   /* (get around Turbo C bug) */
		    u[h-1][j] = r;      /* connect to last table */
		  }
	      }

	    /* set up table entry in r */
	    r.b = (byte)(k - w);
	    if (p >= v + n)
	      {
		r.e = 99;               /* out of values--invalid code */
	      }
	    else if (*p < s)
	      {
		r.e = (byte)(*p < 256 ? 16 : 15);/* 256 is end-of-block code */
		r.v.n = (byte)(*p);     /* simple code is just the value */
		p++;                    /* one compiler does not like *p++ */
	      }
	    else
	      {
		r.e = (byte)e[*p - s];  /* non-simple--look up in lists */
		r.v.n = d[*p++ - s];
	      }

	    /* fill code-like entries with r */
	    f = 1 << (k - w);
	    for (j = i >> w; j < z; j += f)
	    {q[j] = r;}

	    /* backwards increment the k-bit code i */
	    for (j = 1 << (k - 1); i & j; j >>= 1)
	      { i ^= j;}
	    i ^= j;

	    /* backup over finished tables */
	    while ((i & ((1 << w) - 1)) != x[h])
	      {
		h--;                    /* don't need to update q */
		w -= l;
	      }
	  }
     }

    /* Return true PR_E_INC_LIT if we were given an incomplete table */
    if ( y != 0 && g != 1)
      {
	pr_format_message( PR_E_INC_LIT, NULL);
	return( PR_E_INC_LIT);
      }
    else
    {return( PR_SUCCESS);}
}

/*+
************************************************************************
*
*   Synopsis:
*       int     huft_free( t)
*
*   Purpose:
*       Free the malloc'ed tables built by huft_build(), which makes a linked
*       list of the tables it made, with the links in a dummy first entry of
*       each table.
*
*   Parameters:
*       struct huft     *t      : (in)  Table to free.
*
*   Values Returned:
*       int     PR_SUCCESS      : Normal completion.
*
************************************************************************
-*/

int             huft_free(struct huft *t)
{
    struct huft *p;
    struct huft *q;

    /* Go through linked list, freeing from the malloced (t[-1]) address. */
    p = t;
    while (p != (struct huft *)NULL)
      {
	q = (--p)->v.t;
	if (p) free((byte*)p); p = NULL;
	p = q;
      }
    return( PR_SUCCESS);
}

/*+
************************************************************************
*
*   Synopsis:
*       int     inflate_codes( GZboolean cont, long start, int *count,
*                      cMemPtr buffer, GZboolean *end,
*                      struct huft *tl, struct huft *td, int bl, int bd)
*
*   Purpose:
*       inflate (decompress) the codes in a deflated (compressed) block.
*       Transfers may start in the middle of a block and terminate
*       before the end.
*
*   Parameters:
*       GZboolean cont          : (in) if true continue previous I/O
*                                      processing (not at start of block).
*       long   start            : (in) first requested byte index in
*                                      decompressed stream.
*                               : (out) next byte if end of block was
*                                       encountered
*       int    *count           : (in) requested number of bytes.
*                               : (out) bytes remaining in original request
*       cMemPtr  buffer         : (in) pointer to output buffer array
*       GXboolean *end          : (out) if true the end of the compression
*                                  block was encountered.
*       struct huft     *tl     : (in)  literal/length decoder table.
*       struct huft     *td     : (in)  distance decoder table.
*       int             bl      : (in)  Number of bits decoded by tl.
*       int             bd      : (in)  Number of bits decoded by td.
*
*   Values Returned:
*       int     PR_SUCCESS      : Normal completion.
*       int     PR_E_DATA       : Data is undecodable.
*
************************************************************************
-*/


int     inflate_codes( GZboolean cont, long start, int *count,
		      cMemPtr buffer, GZboolean *end,
		      struct huft *tl, struct huft *td, int bl, int bd)
{
  unsigned long         e;      /* table entry flag/number of extra bits*/
  unsigned long         n, d;   /* length and index for copy    */
  unsigned long         w;      /* current window position      */
  struct huft           *t;     /* pointer to table entry       */
  unsigned              ml, md; /* masks for bl and bd bits     */
  unsigned long         b;      /* bit buffer*/
  unsigned              k;      /* number of bits in bit buffer */
  byte                  bite;
  long               last_byte; /* last byte requested in output */
  long                  max_do; /* maximum number of bytes to copy */

  static int     num_left=0;    /* number of bytes left in copy  */
  static unsigned long dsave;   /* previous input buffer pointer */
  unsigned long  wmask=WSIZE-1; /* mask for window index         */
  /*unsigned long  wsize=WSIZE;    window size                   */

  /*  make local copies of globals  */
  b = bb;                       /* initialize bit buffer */
  k = bk;
  w = outcnt;                   /* initialize window position */

  last_byte = start + *count; /* last desired byte */

  *end = 0;   /* end of block not yet encountered */

  /* finish any copy left incomplete from previous call */
  if (cont && (num_left>0)) {
    /* don't copy more than satisfies the current request
       may have to copy the rest on the next call*/
    n = num_left;
    d = dsave;
    max_do = last_byte-out_byte_count; /* max. number to do */
    if (n>max_do) n = max_do;
    num_left -= n;  /* how many will be left to copy */

    /* do the copy */
    if (n>0) { /* check if loop should be done at all */
      do {w &= wmask; d &= wmask;  /* wrap window indices */
	  bite = window[d++];
	  window[w++] = bite;
	  /* update output byte count */
	  if (out_byte_count++ >= start) { /* requested data? */
	  *(buffer++) = (unsigned char)bite;
	  *count -= 1;}
      } while (--n); /* end copy */
    } /* end if copy */
    dsave = d; /* save index for next call */
  } /* end finish copy left over */
  if (*count<=0) return PR_SUCCESS; /* done ? */

  /* inflate the coded data */
  ml = mask_bits[bl];           /* precompute masks for speed */
  md = mask_bits[bd];
  while (*count>0)              /* do until end of block or request done */
    {
      NEEDBITS((unsigned)bl)
	if ((e = (t = tl + ((unsigned)b & ml))->e) > 16)
	  {do
	      {if (e == 99)
		  {pr_format_message(PR_E_DATA, NULL);
		   return( PR_E_DATA);}
	       DUMPBITS(t->b)
	       e -= 16;
	       NEEDBITS(e)
	     } while ((e = (t = t->v.t +
			     ((unsigned)b & mask_bits[e]))->e) > 16);
	 }
      DUMPBITS(t->b)
      if (e == 16)              /* then it's a literal */
	{
	 w &= wmask;  /* wrap buffer */
	 window[w++] = (byte)t->v.n;
	 /* update output byte count */
	 if (out_byte_count++ >= start) { /* requested data ? */
	   *buffer++ = (unsigned char)t->v.n;
	   *count -= 1;}
       }
      else                      /* it's an EOB or a length */
	{/* exit if end of block */
	    if (e == 15) {
	      *end = 1;
	      break;}

	    /* get length of block to copy */
	    NEEDBITS(e)
	    n = t->v.n + ((unsigned)b & mask_bits[e]);
	    DUMPBITS(e);

	    /* decode distance of block to copy */
	    NEEDBITS((unsigned)bd)
	    if ((e = (t = td + ((unsigned)b & md))->e) > 16)
	      { do {
		    if (e == 99)
		      {pr_format_message( PR_E_DATA, NULL);
		       return( PR_E_DATA);}
		    DUMPBITS(t->b)
		    e -= 16;
		    NEEDBITS(e)
		  } while ((e = (t = t->v.t +
				 ((unsigned)b & mask_bits[e]))->e) > 16);
	      }
	    DUMPBITS(t->b)
	    NEEDBITS(e)
	    d = w - t->v.n - ((unsigned)b & mask_bits[e]);
	    DUMPBITS(e)

	    /* don't copy more than satisfies the current request
	     may have to copy the rest on the next call*/
	     num_left = n;
	     max_do = last_byte-out_byte_count; /*max. number to do */
	     if (n>max_do) n = max_do;
	     num_left -= n;  /* how many will be left to copy */

	    /* do the copy */
	    if (n>0) { /* check if loop should be done at all */
	      do {w &= wmask; d &= wmask; /* wrap window indices */
		  bite = window[d++];
		  window[w++] = bite;
  /* debug */
  /*if ((d>=wsize)||(w>=wsize)){sprintf(szErrMess,
  "blew window(lower) n,w,d,wsize,wmask= %lu %lu %lu %lu %lu", 
  n,w,d,wsize,wmask); 
  ErrorMess(szErrMess);}*/
		  /* update output byte count */
		  if (out_byte_count++ >= start) { /* requested data ? */
		    *buffer++ = (unsigned char)bite;
		    *count -= 1;}
	      } while (--n);} /* end copy */
	    dsave = d;/* save indices for next call */
	  } /* end of EOB/length segment */

      /* check for I/O error */
      if (input_IO_error)
	{pr_format_message( PR_E_IO, NULL);
	 return( PR_E_IO);}

    } /* end loop over block */

  /*  restore the globals from the locals  */
  outcnt = w;                   /* restore global window pointer */
  bb = b;                       /* restore global bit buffer */
  bk = k;

  /* done */
  return PR_SUCCESS;
}

/*+
************************************************************************
*
*   Synopsis:
*       int gun_stored(GZboolean cont, long start, int *count,
*                      cMemPtr buffer, GZboolean *end);
*       (originally) int        inflate_stored()
*
*   Purpose:
*      Read and decompress the input stream for appropriate storage type.
*      "decompress" an inflated type 0 (stored) block.
*      Transfers may start in the middle of a block and terminate
*      before the end.  Routine will return when a compression block is
*      finished independent of whether or not the I/O is finished.
*
*   Parameters:
*       GZboolean cont          : (in) if true continue previous I/O
*                                      processing (not at start of block).
*       long   start            : (in) first requested byte index in
*                                      decompressed stream.
*                               : (out) next byte if end of block was
*                                       encountered
*       int    *count           : (in) requested number of bytes.
*                               : (out) bytes remaining in original request
*       cMemPtr buffer          : (in) pointer to output buffer array
*       GZboolean *end          : (out) if true the end of the compression
*                                  block was encountered.
*
*   Values Returned:
*       int     PR_SUCCESS      : Normal completion.
*       int     PR_E_DATA       : An error occured in the data.
*
************************************************************************
-*/

int gun_stored(GZboolean cont, long start, int *count,
	       cMemPtr buffer, GZboolean *end)
{
  unsigned             n;
  static unsigned      num_left;/* no. of bytes left in block   */
  static unsigned long b;       /* bit buffer                   */
  static unsigned      k;       /* number of bits in bit buffer */
  unsigned long        w;       /* current window position      */
  unsigned long wsize=WSIZE;    /* size of window               */

  /*  make local copies of globals */
  b = bb;                       /* initialize bit buffer */
  k = bk;
  w = outcnt;                   /* initialize window position */

  if (!cont) { /* beginning of block start up */
    /* go to byte boundary */
    n = k & 7;
    DUMPBITS(n);

    /* get the length and its complement  */
    NEEDBITS(16)
    n = ((unsigned)b & 0xffff);
    DUMPBITS(16)
    NEEDBITS(16)
      if ( n != (unsigned)( ( ~b) & 0xffff))
	{   /* An error in the compressed data. */
	  pr_format_message( PR_E_DATA, NULL);
	  return( PR_E_DATA);
	}
    DUMPBITS(16)
    num_left = n;
  } /* end of block startup */

  /* check for I/O error */
  if (input_IO_error)
    {   pr_format_message( PR_E_IO, NULL);
	return( PR_E_IO);}

  /* read and output the compressed data   */
  /* read until the request or current block is exhausted */
  n = *count; if (n<num_left) n = num_left;

  while (n--)
    {
      NEEDBITS(8)
      out_byte_count++;  /* update output byte count */
      window[w++] = (unsigned char)b;
      if (w >= wsize) w = 0; /* wrap buffer */
      if (out_byte_count >= start) { /* requested data ? */
	*buffer++ = (unsigned char)b;
	*count -= 1;}
      DUMPBITS(8)
    }

  /* check for I/O error */
  if (input_IO_error)
    {   pr_format_message( PR_E_IO, NULL);
	return( PR_E_IO);}

  /* update number remaining */
  num_left -= n;

  /* finished with block ? */
  *end = (num_left == 0);

  /* restore the globals from the locals  */
  outcnt = w;                   /* restore global window pointer */
  bb = b;                       /* restore global bit buffer */
  bk = k;
  return( PR_SUCCESS);
}

/*+
************************************************************************
*
*   Synopsis:
*       int gun_fixed(GZboolean cont, long start, int *count,
*                      cMemPtr buffer, GZboolean *end);
*       (originally) int        inflate_fixed()
*
*   Purpose:
*      Read and decompress the input stream for appropriate storage type.
*      decompress an inflated type 1 (fixed Huffman codes) block.
*      Transfers may start in the middle of a block and terminate
*      before the end.  Routine will return when a compression block is
*      finished independent of whether or not the I/O is finished.
*
*   Parameters:
*       GZboolean cont          : (in) if true continue previous I/O
*                                      processing
*       long    start           : (in) first requested byte index in
*                                      decompressed stream.
*                               : (out) next byte if end of block was
*                                       encountered
*       int     count           : (in) requested number of bytes.
*                               : (out) bytes remaining in original request
*       cMemPtr  buffer         : (in) pointer to output buffer array
*       GZboolean end           : (out) if true the end of the compression
*                                  block was encountered.
*
*   Values Returned:
*       int     PR_SUCCESS      : Normal completion.
*       int     PR_E_DATA       : An error occured in the data.
*
************************************************************************
-*/

int gun_fixed(GZboolean cont, long start, int *count,
	       cMemPtr buffer, GZboolean *end)
{
  int                  i;               /* temporary variable           */
  static struct huft *tl;               /* literal/length code table    */
  static struct huft *td;               /* distance code table          */
  static int            bl;             /* lookup bits for tl           */
  static int            bd;             /* lookup bits for td           */
  static unsigned       l[288];         /* length list for huft_build   */

  /* restore huft pointers */
  tl = tlsave;
  td = tdsave;

  /*  Setup for beginning of block */
  if (!cont) {
    /*  set up literal table  */
    for (i = 0; i < 144; i++) {l[i] = 8;}
    for (; i < 256; i++) {l[i] = 9;}
    for (; i < 280; i++) {l[i] = 7;}
    for (; i < 288; i++)          /* make a complete, but wrong code set */
      {l[i] = 8;}
    bl = 7;
    PR_CHECK( huft_build(l, 288, 257, cplens, cplext, &tl, &bl));

    /* set up distance table */
    for (i = 0; i < 30; i++)      /* make an incomplete code set */
    {l[i] = 5;}
    bd = 5;
    MsgSup = 1; /* mask incomplete literal message */
    i = huft_build( l, 30, 0, cpdist, cpdext, &td, &bd);
    MsgSup = 0; /* reenable error messages */
    if ( i != PR_SUCCESS && i != PR_E_INC_LIT)
      {PR_CHECK( huft_free(tl));
       return i;}
  } /* end of beginning of block setup */

  /* decompress until an end-of-block code or request satisfied   */
  PR_CHECK (inflate_codes(cont, start, count, buffer, end,
				 tl, td, bl, bd));

  /* if end of block free the decoding tables  */
  if (*end) {
    PR_CHECK (huft_free(tl));
    PR_CHECK (huft_free(td));
    tl = NULL; td = NULL;}

  /* save huft pointers */
  tlsave = tl;
  tdsave = td;

  return (PR_SUCCESS);
}

/*+
************************************************************************
*
*   Synopsis:
*     (originally) int  inflate_dynamic()
*       int gun_dynamic(GZboolean cont, long start, int *count,
*                      cMemPtr buffer, GZboolean *end);
*
*   Purpose:
*      Read and decompress the input stream for appropriate storage type.
*      decompress an inflated type 2 (dynamic Huffman codes) block.
*      Transfers may start in the middle of a block and terminate
*      before the end.  Routine will return when a compression block is
*      finished independent of whether or not the I/O is finished.
*
*   Parameters:
*       GZboolean cont          : (in) if true continue previous I/O
*                                      processing
*       long    start           : (in) first requested byte index in
*                                      decompressed stream.
*                               : (out) next byte if end of block was
*                                       encountered
*       int     count           : (in) requested number of bytes.
*                               : (out) bytes remaining in original request
*       cMemPtr  buffer         : (in) pointer to output buffer array
*       GZboolean end           : (out) if true the end of the compression
*
*   Values Returned:
*       int     PR_SUCCESS      : Normal completion.
*       int     PR_E_DATA       : Data could not be decompressed.
*
************************************************************************
-*/

int gun_dynamic(GZboolean cont, long start, int *count,
	       cMemPtr buffer, GZboolean *end)
{
  int           i;                      /* temporary variables          */
  unsigned      j;
  static unsigned       l;              /* last length                  */
  static unsigned       m;              /* mask for bit lengths table   */
  static unsigned       n;              /* number of lengths to get     */
  static struct huft *tl;               /* literal/length code table    */
  static struct huft *td;               /* distance code table          */
  static int            bl;             /* lookup bits for tl           */
  static int            bd;             /* lookup bits for td           */
  static unsigned       nb;             /* number of bit length codes   */
  static unsigned       nl;             /* number of literal/length codes*/
  static unsigned       nd;             /* number of distance codes      */
#ifdef PKZIP_BUG_WORKAROUND
  static unsigned    ll[288+32];/* literal/length and distance code lengths*/
#else
  static unsigned    ll[286+30];/* literal/length and distance code lengths*/
#endif
  static unsigned       long b;         /* bit buffer                   */
  static unsigned       k;              /* number of bits in bit buffer */


  /* make local bit buffer  */
  b = bb;
  k = bk;

  /* restore huft pointers */
  tl = tlsave;
  td = tdsave;

  /*  Setup for beginning of block */
  if (!cont) {
    /*  read in table lengths  */

    NEEDBITS(5)
    nl = 257 + ((unsigned)b & 0x1f);      /* number of literal/length codes */
    DUMPBITS(5)
    NEEDBITS(5)
    nd = 1 + ((unsigned)b & 0x1f);        /* number of distance codes */
    DUMPBITS(5)
    NEEDBITS(4)
    nb = 4 + ((unsigned)b & 0xf);         /* number of bit length codes */
    DUMPBITS(4)
#ifdef PKZIP_BUG_WORKAROUND
    if (nl > 288 || nd > 32)
#else
    if (nl > 286 || nd > 30)
#endif
      { /* Bad lengths. */
	pr_format_message( PR_E_DATA, NULL);
	return( PR_E_DATA);
      }

    /*  read in bit-length-code lengths */
    for (j = 0; j < nb; j++)
      {NEEDBITS(3)
       ll[border[j]] = (unsigned)b & 7;
       DUMPBITS(3)}
    for (; j < 19; j++) {ll[border[j]] = 0;}

    /* build decoding table for trees--single level, 7 bit lookup  */

    bl = 7;
    if ( ( i = huft_build( ll, 19, 19, NULL, NULL, &tl, &bl)) !=
	    PR_SUCCESS)
      {if ( i == PR_E_INC_LIT)   {PR_CHECK( huft_free(tl));}
       return i;                   /* incomplete code set */
     }
    /* read in literal and distance code lengths  */
    n = nl + nd;
    m = mask_bits[bl];
    i = l = 0;
    while ( (unsigned) i < n)
      {NEEDBITS((unsigned)bl)
       j = (td = tl + ((unsigned)b & m))->b;
       DUMPBITS(j)
       j = td->v.n;
       if (j < 16)                      /* length of code in bits (0..15) */
	 { ll[i++] = l = j;}            /* save last length in l */
       else if (j == 16)           /* repeat last length 3 to 6 times */
	 {
	   NEEDBITS(2)
	     j = 3 + ((unsigned)b & 3);
	   DUMPBITS(2)
	     if ((unsigned)i + j > n)
	       {pr_format_message( PR_E_DATA, NULL);
		 return( PR_E_DATA);}
	    while (j--) {ll[i++] = l;}
	 }
       else if (j == 17)           /* 3 to 10 zero length codes */
	 {NEEDBITS(3)
	    j = 3 + ((unsigned)b & 7);
	  DUMPBITS(3)
	    if ((unsigned)i + j > n)
	      {pr_format_message( PR_E_DATA, NULL);
	       return( PR_E_DATA);}
	    while (j--){ll[i++] = 0;}
	  l = 0;
	}
       else                        /* j == 18: 11 to 138 zero length codes */
	 {NEEDBITS(7)
	  j = 11 + ((unsigned)b & 0x7f);
	  DUMPBITS(7)
	  if ((unsigned)i + j > n)
	    {pr_format_message( PR_E_DATA, NULL);
	     return( PR_E_DATA);}
	    while (j--) {ll[i++] = 0;}
	  l = 0;
	}
     }


    /* free decoding table for trees  */
    PR_CHECK( huft_free(tl));

    /*  restore the global bit buffer  */
    bb = b;
    bk = k;

    /*  build the decoding tables for literal/length and distance codes  */
    bl = lbits;
    if ((i = huft_build(ll, nl, 257, cplens, cplext, &tl, &bl)) != PR_SUCCESS)
    {
	if ( i == PR_E_INC_LIT)
	{
	    PR_CHECK( huft_free(tl));
	}
	return i;                   /* incomplete code set */
    }
    bd = dbits;
    if ((i = huft_build(ll + nl, nd, 0, cpdist, cpdext, &td, &bd)) !=
	    PR_SUCCESS)
    {
	if (i == PR_E_INC_LIT)
	{
#ifdef PKZIP_BUG_WORKAROUND
	    i = 0;
	}
#else
	    PR_CHECK( huft_free(td));
      }
	PR_CHECK( huft_free(tl));
	return( i);                   /* incomplete code set */
#endif
    }
    } /* end of start of new block */

    /* decompress until an end-of-block code or request satisfied   */
    PR_CHECK(inflate_codes( cont, start, count, buffer, end,
				    tl, td, bl, bd));


    /* free the decoding tables, return  */

    if (*end) { /* delete tables if finished with block */
      PR_CHECK( huft_free(tl));
      PR_CHECK( huft_free(td));
      tl = NULL; td = NULL;}

     /* save huft pointers */
     tlsave = tl;
     tdsave = td;

     return( PR_SUCCESS);
}
