/*
 *  tga.h - Libtga header
 *
 *  Copyright (C) 2001-2002, Matthias Brueckner
 *  This file is part of the TGA library (libtga).
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef __TGA_H
#define __TGA_H 1

#include <tgaconfig.h>  /* include our config header before anything else */
#include <stdarg.h>
#include <sys/types.h>
#include <stdio.h>

/* Tell C++ that we have C types and declarations. */
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif /* __cplusplus */

/* Some macros to cope with non-ANSI C or C++ compilers. */
#undef __P
#if defined STDC_HEADERS || defined __cplusplus
# define __P(args) args
#else
# define __P(args) ()
#endif /* defined STDC_HEADERS || defined __cplusplus */


/* sections */
#define TGA_IMAGE_ID	0x01
#define TGA_IMAGE_INFO	0x02
#define TGA_IMAGE_DATA	0x04
#define TGA_COLOR_MAP	0x08
/* RLE */
#define TGA_RLE_ENCODE  0x10

/* byte ordering */
#define TGA_RGB		0x20
#define TGA_BGR		0x40

/* orientation */
#define TGA_BOTTOM	0x0
#define TGA_TOP		0x1
#define	TGA_LEFT	0x0
#define	TGA_RIGHT	0x1

/* bit extraction flags */
#define TGA_ALPHA	0x0f
#define TGA_FLIP_HORZ	0x10
#define TGA_FLIP_VERT	0x20

/* version info */
#define LIBTGA_VER_MAJOR  	0
#define LIBTGA_VER_MINOR  	1
#define LIBTGA_VER_PATCH	7
#define LIBTGA_VER_STRING	"0.1.7"

/* error codes */
enum {  TGA_OK = 1, 		/* success */
	TGA_OOM = 2,		/* out of memory */
	TGA_OPEN_FAIL = 3,     	
	TGA_SEEK_FAIL = 4,    
	TGA_READ_FAIL = 5,     	
	TGA_WRITE_FAIL = 6,   	
	TGA_ERROR = 7,		
	TGA_WARNING = 8,      
	TGA_UNKNOWN_FORMAT = 9, /* not a valid TGA image */
	TGA_UNKNOWN_SUB_FORMAT = 10 /* invalid bit depth */
};

#define TGA_ERRORS 10	/* total number of error codes */

/* text strings corresponding to the error codes */
static char*
tga_error_strings[] = {
	"Success",
	"Out of memory",
	"Failed to open file",
	"Seek failed",
	"Read failed",
	"Write failed",
	"Error",
	"Warning",
	"Unknown format",
	"Unknown sub-format"
};


#if SIZEOF_UNSIGNED_INT == 4
        typedef unsigned int tuint32;
        typedef unsigned short tuint16;
#else
        typedef unsigned long tuint32;
        typedef unsigned int tuint16;
#endif

typedef unsigned char tuint8;

/* define types for non-UNIX && non-Cygwin environments */
#if defined WIN32 && !defined CYGWIN
# define size_t tuint32
# define off_t  tuint32
#endif


typedef tuint8        tbyte;
typedef tuint16	      tshort;
typedef tuint32	      tlong;

typedef struct _TGAHeader TGAHeader;
typedef struct _TGAData	  TGAData;
typedef struct _TGA	  TGA;

typedef void (*TGAErrorProc)(TGA*, int);


/* TGA image header */
struct _TGAHeader {
    	tbyte	id_len;		/* image id length */
	tbyte	map_t;		/* color map type */
	tbyte	img_t;		/* image type */
	tshort	map_first;	/* index of first map entry */
	tshort	map_len;	/* number of entries in color map */
	tbyte	map_entry;	/* bit-depth of a cmap entry */
	tshort	x;		/* x-coordinate */
	tshort	y;		/* y-coordinate */
	tshort	width;		/* width of image */
	tshort	height;		/* height of image */
	tbyte	depth;		/* pixel-depth of image */
	tuint8  alpha:4;        /* alpha bits */
	tuint8	horz:1;	        /* horizontal orientation */
	tuint8	vert:1;	        /* vertical orientation */
};

/* TGA image data */
struct _TGAData {  
	tbyte	*img_id;	/* image id */
	tbyte	*cmap;		/* color map */
	tbyte	*img_data;	/* image data */
	tuint32  flags;
};

/* TGA image representation */
struct _TGA {
	char*		name;		/* file name */
	FILE*		fd;		/* file stream */

	off_t		off;		/* current offset in file*/
	int 		last;		/* last error code */

	TGAHeader	hdr;		/* image header */
	TGAErrorProc 	error;		/* user-defined error proc */
};


__BEGIN_DECLS


TGA* TGAOpen __P((char *name, char *mode));


int TGAReadHeader __P((TGA *tga));

int TGAReadImageId __P((TGA *tga, tbyte **id));

int TGAReadColorMap __P((TGA *tga, tbyte **cmap, tuint32 flags));

size_t TGAReadScanlines __P((TGA *tga, tbyte *buf, size_t sln, size_t n,
			     tuint32 flags));

int TGAReadImage __P((TGA *tga, TGAData *data));


int TGAWriteHeader __P((TGA *tga));

int TGAWriteImageId __P((TGA *tga, const tbyte *id));

int TGAWriteColorMap __P((TGA *tga, tbyte *cmap, tuint32 flags));

size_t TGAWriteScanlines __P((TGA *tga, tbyte *buf, size_t sln, size_t n,
			      tuint32 flags));

int TGAWriteImage __P((TGA *tga, TGAData *data));


char* TGAStrError __P((tuint8 code));

off_t TGASeek __P((TGA *tga, off_t off, int whence));

void TGAbgr2rgb __P((tbyte *data, size_t size, size_t bytes));


void TGAClose __P((TGA *tga));


__END_DECLS


#define TGA_HEADER_SIZE         18
#define TGA_CMAP_SIZE(tga)      ((tga)->hdr.map_len * (tga)->hdr.map_entry / 8)
#define TGA_CMAP_OFF(tga) 	(TGA_HEADER_SIZE + (tga)->hdr.id_len)
#define TGA_IMG_DATA_OFF(tga) 	(TGA_HEADER_SIZE + (tga)->hdr.id_len + TGA_CMAP_SIZE(tga))
#define TGA_IMG_DATA_SIZE(tga)	((tga)->hdr.width * (tga)->hdr.height * (tga)->hdr.depth / 8)
#define TGA_SCANLINE_SIZE(tga)	((tga)->hdr.width * (tga)->hdr.depth / 8)
#define TGA_CAN_SWAP(depth)     (depth == 24 || depth == 32)

#define TGA_IS_MAPPED(tga)      ((tga)->hdr.map_t == 1)
#define TGA_IS_ENCODED(tga)     ((tga)->hdr.img_t > 8 && (tga)->hdr.img_t < 12)

#define TGA_ERROR(tga, code) \
	if((tga) && (tga)->error) (tga)->error(tga, code);\
        fprintf(stderr, "Error at %s line %d: %s", __FILE__, __LINE__,\
		TGAStrError(code));\
	if(tga) (tga)->last = code\


#endif /* __TGA_H */
