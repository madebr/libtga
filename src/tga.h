/*
 * tga.h - Libtga header
 *
 * Copyright (C) 2001-2002, Matthias Brueckner
 * This file is part of the TGA library (libtga).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

#ifndef __TGA_H
#define __TGA_H 1

#include <tgaconfig.h>  /* include our config header before anything else */
#include <stdarg.h>
#include <sys/types.h>

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
#define	TGA_IMAGE_ID	0x01	/* image id */
#define TGA_IMAGE_INFO	0x02	/* image header */
#define TGA_IMAGE_DATA	0x04	/* image data */
#define TGA_COLOR_MAP	0x08	/* color map */

/* orientation */
#define TGA_BOTTOM	0x01
#define TGA_TOP		0x02
#define	TGA_LEFT	0x04
#define	TGA_RIGHT	0x08

/* version info */
#define TGA_LIBTGA_VER_MAJOR  	0
#define TGA_LIBTGA_VER_MINOR  	1
#define TGA_LIBTGA_VER_PATCH	6
#define TGA_LIBTGA_VER_STRING	"0.1.6"
				/* 1.0.0-pre6 */

/* error codes */
#define TGA_OK			0x01	/* success */
#define TGA_BAD_PTR		0x02	/* invalid pointer */
#define TGA_OOM			0x03	/* out of memory */
#define TGA_OPEN_FAIL		0x04	/* file open failed */
#define TGA_SEEK_FAIL		0x05	/* file seek failed */
#define TGA_READ_FAIL		0x06	/* file read failed */
#define TGA_WRITE_FAIL		0x07	/* file write failed */
#define TGA_ERROR		0x08	/* error */
#define TGA_WARNING		0x09	/* library warning */

#define TGA_ERRORS		9	/* total number of error codes */

/* text strings corresponding to the above error codes */
static const char*
tga_error_strings[] = {
	"Success."
	"Invalid pointer.",
	"Out of memory.",
	"Failed to open file.",
	"Seek failed.",
	"Read failed.",
	"Write failed.",
	"Error.",
	"Warning."
};


/* type definitions */
#if SIZEOF_UNSIGNED_INT == 4
        typedef unsigned int tuint32;
        typedef unsigned short tuint16;
#else
        typedef unsigned long tuint32;
        typedef unsigned int tuint16;
#endif /* SIZEOF_UNSIGNED_INT == 4 */


typedef unsigned char tuint8;
typedef unsigned char tbyte;


typedef struct {
	tuint8	id_len;		/* image id length */
	tuint8	map_t;		/* color map type */
	tuint8	img_t;		/* image type */
	tuint16	map_first;	/* index of first map entry */
	tuint16	map_len;	/* number of entries in color map */
	tuint8	map_entry;	/* bit-depth of a cmap entry */
	tuint16	x;		/* x-coordinate */
	tuint16	y;		/* y-coordinate */
	tuint16	width;		/* width of image */
	tuint16	height;		/* height of image */
	tuint16	depth;		/* bit-depth of image */
	tuint16	desc;		/* image descriptor byte */
} TGAHeader;


typedef struct {
	char*		name;		/* file name */
	FILE		*fd;		/* file stream */
	tuint32 	flags;
	
	off_t		row;		/* current scanline */
	off_t		off;		/* current offset in file*/

	int 		last;		/* last error code */

	TGAHeader	hdr;		/* image header */

	void (*error)(int, char*, va_list);	/* error proc */
	void (*message)(int, char*, va_list); 	/* message proc */

} TGA;



#define TGA_HEADER_SIZE 	18
#define TGA_CMAP_SIZE(tga) 	( (tga)->hdr.map_len * (tga)->hdr.map_entry / 8 )
#define TGA_CMAP_OFF(tga) 	( TGA_HEADER_SIZE + (tga)->hdr.id_len )
#define TGA_IMG_DATA_OFF(tga) 	( TGA_HEADER_SIZE + (tga)->hdr.id_len + TGA_CMAP_SIZE(tga) )
#define TGA_IMG_DATA_SIZE(tga)	( (tga)->hdr.width * (tga)->hdr.height * (tga)->hdr.depth / 8 )
#define TGA_SCANLINE_SIZE(tga)	( (tga)->hdr.width * (tga)->hdr.depth / 8 )


__BEGIN_DECLS


TGA* TGAOpen __P((char *name, char *mode));

void TGAClose __P((TGA *tga));


size_t TGARead __P((TGA *tga, tbyte *buf, size_t size, size_t n));

int TGAReadHeader __P((TGA *tga));

int TGAReadImageId __P((TGA *tga, tbyte **id));

int TGAReadColorMap __P((TGA *tga, tbyte **cmap));

size_t TGAReadScanlines __P((TGA *tga, tbyte *buf, size_t sln, size_t n));


size_t TGAWrite __P((TGA *tga, const tbyte *buf, size_t size, size_t n));

int TGAWriteHeader __P((TGA *tga));

int TGAWriteImageId __P((TGA *tga, const tbyte *id));

int TGAWriteColorMap __P((TGA *tga, const tbyte *cmap));

size_t TGAWriteScanlines __P((TGA *tga, const tbyte *buf, size_t sln, size_t n));


void TGAError __P((TGA *tga, int code, char *fmt, ...));

void TGAMessage __P((TGA *tga, int code, char *fmt, ...));


off_t TGASeek __P((TGA *tga, off_t off, int whence));


int TGAGetXOrientation __P((TGA *tga));

int TGAGetYOrientation __P((TGA *tga));

int TGAIsEncoded __P((TGA *tga));


__END_DECLS


#endif /* __TGA_H */
