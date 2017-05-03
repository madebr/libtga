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

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* sections */
#define TGA_IMAGE_ID	0x01
#define TGA_IMAGE_INFO	0x02
#define TGA_IMAGE_DATA	0x04
#define TGA_COLOR_MAP	0x08
/* RLE */
#define TGA_RLE_ENCODE  0x10

/* color format */
#define TGA_RGB		0x20
#define TGA_BGR		0x40

/* orientation */
#define TGA_BOTTOM	0x0
#define TGA_TOP		0x1
#define	TGA_LEFT	0x0
#define	TGA_RIGHT	0x1

/* error codes */
enum {
	TGA_OK = 0, 		/* success */
	TGA_ERROR,
	TGA_OOM,		/* out of memory */
	TGA_OPEN_FAIL,
	TGA_SEEK_FAIL,
	TGA_READ_FAIL,
	TGA_WRITE_FAIL,
	TGA_UNKNOWN_SUB_FORMAT  /* invalid bit depth */
};

#define TGA_ERRORS 8  /* total number of error codes */

/* text strings corresponding to the error codes */
extern const char *
tga_error_strings[];

typedef uint32_t	tuint32;
typedef uint16_t	tuint16;
typedef uint8_t	tuint8;

typedef tuint8	tbyte;
typedef tuint16	tshort;
typedef tuint32	tlong;

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
	tbyte   alpha;          /* alpha bits */
	tbyte	horz;	        /* horizontal orientation */
	tbyte	vert;	        /* vertical orientation */
};

/* TGA image data */
struct _TGAData {  
	tbyte	*img_id;	/* image id */
	tbyte	*cmap;		/* color map */
	tbyte	*img_data;	/* image data */
	tuint32  flags;
};

/* TGA image handle */
struct _TGA {
	FILE*		fd;		/* file stream */
	tlong		off;		/* current offset in file*/
	int		last;		/* last error code */
	TGAHeader	hdr;		/* image header */
	TGAErrorProc 	error;		/* user-defined error proc */
};

TGA* TGAOpen(const char *name, const char *mode);

TGA* TGAOpenFd(FILE *fd);


int TGAReadHeader(TGA *tga);

int TGAReadImageId(TGA *tga, tbyte **id);

int TGAReadColorMap(TGA *tga, tbyte **cmap, tuint32 flags);

size_t TGAReadScanlines(TGA *tga, tbyte *buf, size_t sln, size_t n,
			     tuint32 flags);

int TGAReadImage(TGA *tga, TGAData *data);

void TGAFreeTGAData(TGAData *data);

int TGAWriteHeader(TGA *tga);

int TGAWriteImageId(TGA *tga, const tbyte *id);

int TGAWriteColorMap(TGA *tga, tbyte *cmap, tuint32 flags);

size_t TGAWriteScanlines(TGA *tga, tbyte *buf, size_t sln, size_t n,
			      tuint32 flags);

int TGAWriteImage(TGA *tga, TGAData *data);


const char* TGAStrError(tuint8 code);


void TGAClose(TGA *tga);

#define TGA_ERROR(tga, code) \
if((tga) && (tga)->error) (tga)->error(tga, code);\
fprintf(stderr, "Libtga:%s:%d: %s\n", __FILE__, __LINE__, TGAStrError(code));\
if(tga) (tga)->last = code\

#ifdef __cplusplus
}
#endif

#endif /* __TGA_H */
