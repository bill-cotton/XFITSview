/* utility header file for gzipread.c for gzip reading functions */
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

#ifndef GZIPUTIL_H
#define GZIPUTIL_H

typedef char            GZboolean;
typedef unsigned char   byte;

/* Tables for deflate from PKZIP's appnote.txt. */

static unsigned border[] = {    /* Order of the bit length code lengths */
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
static unsigned short cplens[] = {/* Copy lengths for literal codes 257..285 */
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
	/* note: see note #13 above about the 258 in this list. */
static unsigned short cplext[] = {/* Extra bits for literal codes 257..285 */
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99}; /* 99==invalid */
static unsigned short cpdist[] = {/* Copy offsets for distance codes 0..29 */
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
	257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
	8193, 12289, 16385, 24577};
static unsigned short cpdext[] = {/* Extra bits for distance codes */
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
	12, 12, 13, 13};

unsigned short mask_bits[] =
{
    0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};


/* 
 * Macros for inflate() bit peeking and grabbing.
 * The usage is:
 * 
 *      NEEDBITS(j)
 *      x = b & mask_bits[j];
 *      DUMPBITS(j)
 *
 * where NEEDBITS makes sure that b has at least j bits in it, and
 * DUMPBITS removes the bits from b.  The macros use the variable k
 * for the number of bits in b.  Normally, b and k are register
 * variables for speed, and are initialized at the beginning of a
 * routine that uses these macros from a global bit buffer and count.
 *
 * If we assume that EOB will be the longest code, then we will never
 * ask for bits with NEEDBITS that are beyond the end of the stream.
 * So, NEEDBITS should not read any more bytes than are needed to
 * meet the request.  Then no bytes need to be "returned" to the buffer
 * at the end of the last block.
 *
 * However, this assumption is not true for fixed blocks--the EOB code
 * is 7 bits, but the other literal/length codes can be 8 or 9 bits.
 * (The EOB code is shorter than other codes because fixed blocks are
 * generally short.  So, while a block always has an EOB, many other
 * literal/length codes have a significantly lower probability of
 * showing up at all.)  However, by making the first table have a
 * lookup of seven bits, the EOB code will be found in that first
 * lookup, and so will not require that too many bits be pulled from
 * the stream.
 */

#define NEEDBITS(n) {while(k<(n)) \
     {if (next_byte_buff>=max_byte_buff) load_byte_buffer();in_byte_count++;\
      b|=((unsigned long)byte_buffer[next_byte_buff++])<<k;k+=8;}}
#define DUMPBITS(n) {b>>=(n);k-=(n);}

/* 
 * The inflate algorithm uses a sliding 32K byte window on the uncompressed
 * stream to find repeated byte strings.  This is implemented here as a
 * circular buffer.  The index is updated simply by incrementing and then
 * and'ing with 0x7fff (32K-1). 
 */


#ifndef WSIZE
#  define WSIZE 0x00008000 /* window size--must be a power of two, and */
#endif                     /*  at least 32K for zip's deflate method */
cMemPtr     window;     /* window memory pointer */
int         hWindow;    /* handle for window memory */
unsigned long outcnt;   /* current position in window */

#define GZIP_MAGIC     "\037\213" /* Magic header for gzip files, 1F 8B */

/* gzip flag byte  */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

/* Compression methods (see algorithm.doc) */
#define DEFLATED    8

/* execute with error check macros */
#define PR_CHECK( s )           { int estatus; \
				  if ( ( estatus = ( s ) ) < 0 ) { \
				    return( estatus ); } }

#define PR_CHECK_NULL( s )      { if ( ( s ) == NULL ) {\
				  pr_format_message( PR_E_MEMORY , NULL);\
				  return( PR_E_MEMORY ); } }

/* 
 * Huffman code lookup table entry--this entry is four bytes for machines
 * that have 16-bit pointers (e.g. PC's in the small or medium model).
 * Valid extra bits are 0..13.  e == 15 is EOB (end of block), e == 16
 * means that v is a literal, 16 < e < 32 means that v is a pointer to
 * the next table, which codes e - 16 bits, and lastly e == 99 indicates
 * an unused code.  If a code with e == 99 is looked up, this implies an
 * error in the data. 
 */

struct huft 
{
    byte         e;             /* number of extra bits or operation    */
    byte         b;             /* number of bits in this code or subcode */
    union 
    {
	unsigned short  n;      /* literal, length base, or distance base */
	struct huft     *t;     /* pointer to next level of table       */
    } v;
};
  struct huft *tlsave=NULL;          /* literal/length code table    */
  struct huft *tdsave=NULL;          /* distance code table          */

/* If BMAX needs to be larger than 16, then h and x[] should be ulg. */
#define BMAX 16         /* maximum bit length of any code (16 for explode) */
#define N_MAX 288       /* maximum number of codes in any set */
#define EXTHDR  16              /* Size of extend local header.         */


/*
 * Huffman code decoding is performed using a multi-level table lookup.
 * The fastest way to decode is to simply build a lookup table whose
 * size is determined by the longest code.  However, the time it takes
 * to build this table can also be a factor if the data being decoded
 * is not very long.  The most common codes are necessarily the
 * shortest codes, so those codes dominate the decoding time, and hence
 * the speed.  The idea is you can have a shorter table that decodes the
 * shorter, more probable codes, and then point to subsidiary tables for
 * the longer codes.  The time it costs to decode the longer codes is
 * then traded against the time it takes to make longer tables.
 *
 * This results of this trade are in the variables lbits and dbits
 * below.  lbits is the number of bits the first level table for literal/
 * length codes can decode in one step, and dbits is the same thing for
 * the distance codes.  Subsequent tables are also less than or equal to
 * those sizes.  These values may be adjusted either when all of the
 * codes are shorter than that, in which case the longest code length in
 * bits is used, or when the shortest code is *longer* than the requested
 * table size, in which case the length of the shortest code in bits is
 * used.
 *
 * There are two different values for the two tables, since they code a
 * different number of possibilities each.  The literal/length table
 * codes 286 possible values, or in a flat code, a little over eight
 * bits.  The distance table codes 30 possible values, or a little less
 * than five bits, flat.  The optimum values for speed end up being
 * about one bit more than those, so lbits is 8+1 and dbits is 5+1.
 * The optimum values may differ though from machine to machine, and
 * possibly even between compilers.  Your mileage may vary.
 */

int             lbits = 9;      /* bits in base literal/length lookup table */
int             dbits = 6;      /* bits in base distance lookup table */

unsigned        hufts=0;         /* track memory usage */

  
/* file global I/O variables */
int             in_hFile;       /* input file handle                    */
FITSfile       *current_file=0; /* FileRead file descriptor of in_hFile */
long            in_byte_count;  /* current input byte index             */
long            out_byte_count; /* current output byte index            */
int             input_IO_error; /* if non zero the error exists in input*/
GZboolean       last_block;     /* is this the last compressed block?   */
long            first_block;    /* offset to beginning of first block   */
GZboolean       IO_done;        /* if true input file exhausted         */
#define         MAX_BYTE 256    /* byte buffer size                     */
byte            byte_buffer[MAX_BYTE]; /* byte buffer                   */
int             next_byte_buff; /* next available byte in byte buffer   */
int             max_byte_buff;  /* number of bytes in byte buffer       */
int             gzip_type;      /* block type 0=stored, 1=fixed, 2=dynamic*/
unsigned long   bb;             /* bit buffer                           */
unsigned        bk;             /* bits in bit buffer                   */
GZboolean       MsgSup=0;       /* if true suppress error messages      */

/* function prototypes */
int  gun_stored(GZboolean cont, long start, int *count,
		cMemPtr buffer, GZboolean *end);
int  gun_fixed(GZboolean cont, long start, int *count,
	       cMemPtr buffer, GZboolean *end);
int  gun_dynamic(GZboolean cont, long start, int *count,
		 cMemPtr buffer, GZboolean *end);
byte get_byte(void);
int  char_in(byte *buff, int count);
void load_byte_buffer(void);
void pr_format_message(int code, int* arg);
int  huft_build(unsigned *b, unsigned n, unsigned s, unsigned short *d,
		unsigned short *e, struct huft **t, int *m);
int  huft_free(struct huft *t );
int  inflate_codes( GZboolean cont, long start, int *count,
		   cMemPtr buffer, GZboolean *end,
		   struct huft *tl, struct huft *td, int bl, int bd);

#endif /* GZIPUTIL_H */
