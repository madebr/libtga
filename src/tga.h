/*
 * tga.h - Libtga header
 *
 * Copyright (C) 2001, Matthias Brückner
 * This file is part of the TGA library (libtga).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

#ifndef __TGA_H
#define __TGA_H

#include <tgaconfig.h>  /* include our config header before anything else */
#include <stdio.h>
#include <malloc.h>


/* Tell C++ that we have C types and declarations. */
#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS
# define __END_DECLS
# define bool int
# define true 1
# define false 0
#endif /* __cplusplus */

/* Some macros to cope with non-ANSI C or C++ compilers. */
#undef __P
#if defined STDC_HEADERS || defined __cplusplus
# define __P(args) args
#else
# define __P(args) ()
#endif /* defined STDC_HEADERS || defined __cplusplus */


/* error codes */
#define TGA_OK		0x0
#define TGA_ERROR	0x1	/* unknown error */
#define TGA_ERROR_MEM	0x2	/* memory allocation failed */
#define TGA_ERROR_READ	0x3	/* read operation failed */
#define TGA_ERROR_WRITE	0x4	/* write operation failed */
#define TGA_BAD_ARG	0x5	/* invalid argument given to function */
#define TGA_BAD_FD	0x6	/* invalid file descriptor */
#define TGA_BAD_FORMAT	0x7	/* invalid format */

/* sections */
#define	TGA_IMAGE_ID		0x01	/* image id */
#define TGA_IMAGE_INFO		0x02	/* image header */
#define TGA_IMAGE_DATA		0x08	/* image data */
#define TGA_COLOR_MAP		0x10	/* color map */

/* flags */
#define TGA_FORMAT_NEW	        0x100	/* new tga format */
#define TGA_FORMAT_OLD	        0x200	/* original tga format */

/* orientation */
#define TGA_BOTTOM	0x01
#define TGA_TOP		0x02
#define	TGA_LEFT	0x04
#define	TGA_RIGHT	0x08


/* version info */
#define TGA_LIBTGA_VER_MAJOR    0
#define TGA_LIBTGA_VER_MINOR    1
#define TGA_LIBTGA_VER_RELEASE  0
#define TGA_LIBTGA_VER_STRING   "0.1.0"



/* Type definitions */
#if SIZEOF_UNSIGNED_INT == 4
        typedef unsigned int tga_uint_32;
        typedef unsigned short tga_uint_16;
#else
        typedef unsigned long tga_uint_32;
        typedef unsigned int tga_uint_16;
#endif

typedef unsigned char tga_uint_8;

typedef tga_uint_8 tga_byte;
typedef tga_uint_8 tga_err_t;
typedef tga_uint_8 tga_size_t;
typedef tga_uint_32 tga_off_t;


struct tga_info {	/* tga info structure */
	tga_uint_8	id_len;
	tga_uint_8	map_t;
	tga_uint_8	img_t;
	tga_uint_16	map_first_entry;
	tga_uint_16	map_len;
	tga_uint_8	map_entry_size;
	tga_uint_16	x;
	tga_uint_16	y;
	tga_uint_16	width;
	tga_uint_16	height;
	tga_uint_8	depth;
	tga_uint_8	desc;
};

typedef struct tga_info* tga_info_ptr;

typedef struct tga* tga_ptr;

typedef void (*tga_err_func)(tga_ptr, char*, tga_err_t);
typedef void (*tga_io_func)(tga_ptr, void*, tga_uint_32);
typedef void (*tga_seek_func)(tga_ptr, tga_off_t, tga_off_t);

struct tga {
	tga_uint_8 *img_id;		/* image id field */
	tga_uint_8 *img_data;		/* image data */
	tga_uint_8 *color_map;		/* color map data */

	tga_err_func err_fn;	/* error function */
	tga_err_func warn_fn;	/* warn function */
 	tga_io_func read_fn; 	/* read function */
	tga_io_func write_fn;	/* write function */
	tga_seek_func seek_fn;	/* seek function */

        tga_uint_32 flags;	/* flags */
        tga_byte *sig;  	/* file signature in file footer */
	tga_byte *io_ptr;	/* IO pointer (e.g FILE*) */
};



static const tga_uint_8 num_errors = 7;

static char *str_errors[7] = {
	"unknown error",
	"memory allocation error",
	"read error",
	"write error",
	"bad argument",
	"bad file descriptor",
	"bad format"
};

static const char signature[16] = "TRUEVISION-XFILE";

/* functions */

__BEGIN_DECLS


/* read functions */
void tga_read_tga __P((tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags));

void tga_read_info __P((tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags));

void tga_set_read_fn __P((tga_ptr ptr, tga_io_func read_fn));

void tga_read_chunk __P((tga_ptr ptr, tga_byte *buf, tga_uint_8 len));

void tga_read_data __P((tga_ptr ptr, tga_byte *buf, tga_uint_32 len));

void tga_default_read_data __P((tga_ptr ptr, tga_byte *buf, tga_uint_32 len));

void tga_read_image_data __P((tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags));


/* write functions */
void tga_write_tga __P((tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags));

void tga_write_info __P((tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags));

void tga_set_write_fn __P((tga_ptr ptr, tga_io_func write_fn));

void tga_write_data __P((tga_ptr ptr, tga_byte *buf, tga_uint_32 len));

void tga_default_write_data __P((tga_ptr ptr, tga_byte *buf, tga_uint_32 len));

void tga_write_chunk __P((tga_ptr ptr, tga_byte *buf, tga_uint_8 len));

void tga_write_image_data __P((tga_ptr ptr, tga_info_ptr info, tga_uint_32 len));


/* seek functions */
void tga_set_seek_fn __P((tga_ptr ptr, tga_seek_func seek_fn)); 

void tga_seek __P((tga_ptr ptr, tga_off_t off, tga_uint_8 whence));

void tga_default_seek __P((tga_ptr ptr, tga_off_t off, tga_uint_8 whence));


/* error functions */
char* tga_get_str_error __P((tga_err_t errno));

void tga_error __P((tga_ptr ptr, char *msg, tga_err_t errno));

void tga_default_error __P((tga_ptr ptr, char *msg, tga_err_t errno));

void tga_warning __P((tga_ptr ptr, char *msg, tga_err_t errno));

void tga_default_warning __P((tga_ptr ptr, char *msg, tga_err_t errno));

void tga_set_error_fn __P((tga_ptr ptr, tga_err_func err_fn, tga_err_func warn_fn));


/* memory functions */
void* tga_malloc __P((tga_ptr ptr, tga_uint_32 len));

void tga_free __P((void *buf));

void tga_free_tga __P((tga_ptr ptr));

void tga_memcpy __P((const void *src, void *dest, tga_off_t off, tga_uint_32 n));

void* tga_realloc __P((tga_ptr ptr, void *buf, tga_uint_32 size));


/* wrapper functions */
void tga_set_flag __P((tga_ptr ptr, tga_uint_8 flag));

bool tga_is_flag __P((tga_ptr ptr, tga_uint_8 flag));

bool tga_is_compressed __P((tga_info_ptr info));


tga_uint_8 tga_get_x_orientation __P((tga_info_ptr info));

tga_uint_8 tga_get_y_orientation __P((tga_info_ptr info));


void tga_init_ptr __P((tga_ptr ptr, 
			void *io_ptr, 
			tga_io_func read_fn, 
			tga_io_func write_fn, 
			tga_seek_func seek_fn,
			tga_err_func err_fn,
			tga_err_func warn_fn));


/* utility functions */
tga_byte* tga_substr __P((tga_ptr ptr, const tga_byte *src, tga_off_t start, tga_off_t end));

void tga_swap_bytes __P((tga_byte *buf));


/* little-, big-endian aware byte extract/copy functions */

/* extract 4 bytes from buf */
static inline tga_uint_32 tga_get_uint_32 __P((tga_byte *buf))
{
        tga_uint_32 t;
#ifdef WORDS_BIGENDIAN
	t = buf[0];
	t <<= 24;
	t |= buf[1];
	t <<= 16;
	t |= buf[2];
	t <<= 8;
	t |= buf[3];
#else /* little-endian */
	t = buf[1];
	t <<= 24;
	t |= buf[0];
	t <<= 16;
	t |= buf[3];
	t <<= 8;
	t |= buf[2];
#endif /* WORDS_BIGENDIAN */
	return t;
}

/* extract 2 bytes from buf */
static inline tga_uint_16 tga_get_uint_16 __P((tga_byte *buf))
{
        tga_uint_16 t;
#ifdef WORDS_BIGENDIAN
	t = buf[0];
	t <<= 8;
	t |= buf[1];
#else /* little-endian */
	t = buf[1];
	t <<= 8;
	t |= buf[0];
#endif /* WORDS_BIGENDIAN */
	return t;
}

/* copy 2 bytes from src to dest + off */
static inline void tga_cpy_uint_16 __P((const tga_byte *src, tga_byte *dest, tga_off_t off))
{
#ifndef WORDS_BIGENDIAN
	dest[off]     = src[0];
	dest[off + 1] = src[1];
#else
	dest[off]     = src[1];
	dest[off + 1] = src[0];
#endif /* WORDS_BIGENDIAN */
}

/* copy 3 bytes from src to dest + off */
static inline void tga_cpy_uint_24 __P((const tga_byte *src, tga_byte *dest, tga_off_t off))
{
#ifdef WORDS_BIGENDIAN
	dest[off]     = src[0];
	dest[off + 1] = src[1];
	dest[off + 2] = src[2];
#else
	dest[off]     = src[1];
	dest[off + 1] = src[0];
	dest[off + 2] = src[2];
#endif /* WORDS_BIGENDIAN */
}

/* copy 4 bytes from src to dest + off */
static inline void tga_cpy_uint_32 __P((const tga_byte *src, tga_byte *dest, tga_off_t off))
{
#ifdef WORDS_BIGENDIAN
	dest[off]     = src[0];
	dest[off + 1] = src[1];
	dest[off + 2] = src[2];
	dest[off + 3] = src[3];
#else
	dest[off]     = src[1];
	dest[off + 1] = src[0];
	dest[off + 2] = src[3];
	dest[off + 3] = src[4];
#endif /* WORDS_BIGENDIAN */
}


__END_DECLS


#endif /* __TGA_H */
