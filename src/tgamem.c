/*
 * tgamem.c - Memory management functions
 *
 * Copyright (C) 2001, Matthias Brückner
 * This file is part of the TGA library (libtga).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

#include "tga.h"

#include <malloc.h>


/* malloc(3) with error checking */
void* tga_malloc(tga_ptr ptr, tga_uint_32 len)
{
	void *tmp = (void*)malloc(len);
	if(!tmp) tga_error(ptr, "tga_malloc(tgamem.c)", TGA_ERROR_MEM);
	bzero(tmp, len);
	return tmp;
}

/* useless until now */
void tga_free(void *buf)
{
	free(buf);
}

/* free all memory assigned with ptr and ptr itself */
void tga_free_tga(tga_ptr ptr)
{
	if(ptr) {
		free(ptr->img_id);
		free(ptr->img_data);
		free(ptr->color_map);
		free(ptr);
	}
	ptr = NULL;
}

/* copy n bytes from src to dest + off */
void tga_memcpy(const void *src, void *dest, tga_off_t off, tga_uint_32 n)
{
	dest = memcpy(dest + off, src, n);
}

/* realloc(3) */
void* tga_realloc(tga_ptr ptr, void *buf, tga_uint_32 size)
{
	buf = realloc(buf, size);
	if(buf == NULL && size != 0) {
		tga_error(ptr, "tga_realloc(tgamem.c)", TGA_ERROR_MEM);
	}
	return buf;
}
		

