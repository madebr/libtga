/*
 * tga.c - Some general purpose functions
 *
 * Copyright (C) 2001, Matthias Brückner
 * This file is part of the TGA image library (libtga).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

#include "tga.h"
#include "tgaconfig.h"


/* set flag */
void tga_set_flag(tga_ptr ptr, tga_uint_8 flag)
{
	ptr->flags |= flag;
}


/* check flag */
bool tga_is_flag(tga_ptr ptr, tga_uint_8 flag)
{
  return ptr->flags & flag;
}


bool tga_is_compressed(tga_info_ptr info)
{
  return info->img_t > 8 || info->img_t < 12;
}


tga_uint_8 tga_get_x_orientation(tga_info_ptr info)
{
	return (info->desc & (1 << 4)) ? TGA_RIGHT : TGA_LEFT;
}


tga_uint_8 tga_get_y_orientation(tga_info_ptr info)
{
	return (info->desc & (1 << 5)) ? TGA_TOP : TGA_BOTTOM;
}


/* init pointer to tga struct */
void tga_init_ptr(tga_ptr ptr,
		void *io_ptr,
		tga_io_func read_fn,
		tga_io_func write_fn,
		tga_seek_func seek_fn,
		tga_err_func err_fn,
		tga_err_func warn_fn)
{
	ptr->io_ptr = io_ptr;
	ptr->read_fn = read_fn;
	ptr->write_fn = write_fn;
        ptr->seek_fn = seek_fn;
	ptr->err_fn = err_fn;
	ptr->warn_fn = warn_fn;
}


/* swap byte 0 with byte 1, byte 2 with byte 3, ... */
void tga_swap_bytes(tga_byte *buf)
{
        tga_byte tmp;
        tga_uint_32 i = 0, len = strlen(buf);
        for(i = 0; i < len; ++i) {
                tmp = buf[i];
                buf[i] = buf[i + 1];
                buf[i + 1] = tmp;
        }
}


/* extract string from src[start] to src[end] */
tga_byte* tga_substr(tga_ptr ptr, __const tga_byte *src, tga_off_t start, tga_off_t end)
{
	tga_byte i, *ret = (tga_byte*)tga_malloc(ptr, end - start);
	for(start,i = 0; start < end; ++start, ++i) ret[i] = src[start];
	return ret;
}
